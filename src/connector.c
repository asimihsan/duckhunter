/*!
 * duckhunter: src/connector.c
 * https://github.com/asimihsan/duckhunter
 *
 * Copyright 2013 Asim Ihsan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * https://github.com/bbockelm/lcmaps-plugins-process-tracking/blob/7c837ff/src/proc_police.c
 */

#include "duckhunter.h"

int create_connector_socket() {
    int conn_sock = socket(PF_NETLINK, SOCK_DGRAM | SOCK_NONBLOCK,
                           NETLINK_CONNECTOR);
    if (conn_sock < 0) {
        perror("Unable to open proc connector socket!");
        exit(1);
    }
    return conn_sock;
}

void select_proc_connector(int conn_sock) {
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = CN_IDX_PROC;

    bind(conn_sock, (struct sockaddr *)&addr, sizeof(addr));
}

void set_socket_filter(int conn_sock) {
    /**
     *  -   Load (BPF_LD) a halfword, 16 bits (BPF_H) from the absolute
     *      offset (BPF_ABS) to the nlmsg_type member of the nlmsghdr
     *      struct. Since that structure should be at the start of the
     *      message the accumulator now has that value.
     *  -   Jump (BPF_JMP) if a constant (BPF_K) is equal to (BPF_JEQ) a
     *      value, remembering to deal with networking ordering. If true
     *      jump one statement (1), else jump no statements (0).
     *
     *      If we do not jump one statement for the first two clauses we
     *      hit the "return everything" line. This means that if we hit an
     *      error just throw everything into user space and let it deal with
     *      the return. However, if we do jump a statement, we continue
     *      with the processing.
     *
     *  -   BPF_RET tells the kernel to return some amount of bytes.
     *  -   BPF_K means we give this number of bytes as an argument.
     *  -   0xffffffff means allow the largest possible message size, i.e.
     *      all of it. If this were 0 the kernel would never wake us up.
     */
    struct sock_filter filter[] = {
        /**
         *  Accept if msg type != NLMSG_DONE.
         */
        BPF_STMT(BPF_LD | BPF_H | BPF_ABS,
                 offsetof(struct nlmsghdr, nlmsg_type)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htons(NLMSG_DONE),
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

        /**
         *  Accept if not from process connector system.
         */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, id)
                                 + offsetof(struct cb_id, idx)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htonl(CN_IDX_PROC),
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, id)
                                 + offsetof(struct cb_id, val)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htonl(CN_VAL_PROC),
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

        /**
         *  Accept exec messages if it isn't a thread.
         */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, what)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htonl(PROC_EVENT_EXEC),
                 0, 7),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct exec_proc_event,
                                            process_pid)),
        BPF_STMT(BPF_ST, 0),
        BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct exec_proc_event,
                                            process_tgid)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X,
                 0,
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

        /**
         *  Accept exit messages if it isn't a thread.
         */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, what)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htonl(PROC_EVENT_EXIT),
                 0, 7),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct exit_proc_event,
                                            process_pid)),
        BPF_STMT(BPF_ST, 0),
        BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct exit_proc_event,
                                            process_tgid)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X,
                 0,
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

        /**
         *  Accept fork messages if parent and child are not threads.
         */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, what)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 htonl(PROC_EVENT_FORK),
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0),

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct fork_proc_event,
                                            parent_pid)),
        BPF_STMT(BPF_ST, 0),
        BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct fork_proc_event,
                                            parent_tgid)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X,
                 0,
                 1, 0),
        BPF_STMT(BPF_RET | BPF_K, 0),

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct fork_proc_event,
                                            child_pid)),
        BPF_STMT(BPF_ST, 0),
        BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 NLMSG_LENGTH(0) + offsetof(struct cn_msg, data)
                                 + offsetof(struct proc_event, event_data)
                                 + offsetof(struct fork_proc_event,
                                            child_tgid)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X,
                 0,
                 1, 0),

        BPF_STMT(BPF_RET | BPF_K, 0),
        BPF_STMT(BPF_RET | BPF_K, 0xffffffff),
    };

    struct sock_fprog fprog;
    memset(&fprog, 0, sizeof(fprog));
    fprog.filter = filter;
    fprog.len = sizeof filter / sizeof filter[0];
    if (setsockopt(conn_sock, SOL_SOCKET, SO_ATTACH_FILTER,
                   &fprog, sizeof(fprog)) < 0) {
        printf("setting socket filter failed: %s\n", strerror(errno));
    }
}

int send_connector_message(int conn_sock, bool enable) {
    int rc;
    nlcn_op_msg nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    nlcn_msg.nl_body.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.nl_body.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.nl_body.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    nlcn_msg.nl_body.cn_mcast = enable ? PROC_CN_MCAST_LISTEN :
                                         PROC_CN_MCAST_IGNORE;

    rc = send(conn_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if (rc == -1) {
        perror("netlink send error");
        return -1;
    }
    return 0;
}

int send_subscribe_message(int conn_sock) {
    return send_connector_message(conn_sock, true);
}

int send_unsubscribe_message(int conn_sock) {
    return send_connector_message(conn_sock, false);
}

void handle_connector_event(uv_poll_t *req, int status, int events) {
    int rc;
    connector_context_t *context = (connector_context_t *)req;
    nlcn_ev_msg *nlcn_msg;

    struct msghdr msghdr;
    struct sockaddr_nl addr;
    struct iovec iov[1];
    char buf[sizeof(nlcn_ev_msg)];

    /**
     *  Netlink allows for multi-part messages, and although the process
     *  connector doesn't use this let's handle it to future-proof the code.
     */
    msghdr.msg_name = &addr;
    msghdr.msg_namelen = sizeof addr;
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 1;
    msghdr.msg_control = NULL;
    msghdr.msg_controllen = 0;
    msghdr.msg_flags = 0;

    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof buf;

    rc = recvmsg(context->sockfd, &msghdr, 0);
    if (rc == 0) {
        return;
    } else if (rc == -1) {
        switch(errno) {
            case EINTR:
            case EWOULDBLOCK:
                return;
        }
        perror("error in netlink socket receive!");
        return;
    }

    /**
     *  Netlink allows any process to send messages to any other process. We
     *  need to make sure that the message actually comes from the kernel.
     */
    if (addr.nl_pid != 0)
        return;

    /**
     *  Iterate over multiple Netlink messages; currently there will always
     *  only be one message, but in the future there may not be.
     */
    for (struct nlmsghdr *nlmsghdr = (struct nlmsghdr *)buf;
         NLMSG_OK (nlmsghdr, rc);
         nlmsghdr = NLMSG_NEXT (nlmsghdr, rc))
    {
        nlcn_msg = (nlcn_ev_msg *)nlmsghdr;
        if ((nlcn_msg->nl_hdr.nlmsg_type == NLMSG_ERROR) ||
            (nlcn_msg->nl_hdr.nlmsg_type == NLMSG_NOOP)) {
            continue;
        }

        /**
         *  Make sure the Netlink message comes from the process connector
         *  subsystem.
         */
        if ((nlcn_msg->nl_body.cn_msg.id.idx != CN_IDX_PROC) ||
            (nlcn_msg->nl_body.cn_msg.id.val != CN_VAL_PROC)) {
            continue;
        }

        start_processing_event(nlcn_msg);
    }
}

connector_context_t *create_connector_context(int conn_sock) {
    connector_context_t *context;

    context = (connector_context_t *)malloc(sizeof *context);
    context->sockfd = conn_sock;
    uv_poll_init_socket(loop, &context->poll_handle, conn_sock);
    context->poll_handle.data = context;
    return context;
}

void connector_close_cb(uv_handle_t *handle) {
    connector_context_t *context = (connector_context_t *)handle->data;
    free(context);
}

void destroy_connector_context(connector_context_t *context) {
    uv_close((uv_handle_t *)&context->poll_handle, connector_close_cb);    
}
