/*!
 * duckhunter: include/nlcn_msg.h
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

#ifndef NLCN_MSG_H
#define NLCN_MSG_H

typedef struct __attribute__((aligned(NLMSG_ALIGNTO))) {
    struct nlmsghdr nl_hdr;
    struct __attribute__((packed)) {
        struct cn_msg cn_msg;
        enum proc_cn_mcast_op cn_mcast;
    } nl_body;  
} nlcn_op_msg;

typedef struct __attribute__((aligned(NLMSG_ALIGNTO))) {
    struct nlmsghdr nl_hdr;
    struct __attribute__((packed)) {
        struct cn_msg cn_msg;
        struct proc_event proc_ev;
    } nl_body;  
} nlcn_ev_msg;

#endif
