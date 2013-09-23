#include "duckhunter.h"

void start_processing_event(nlcn_ev_msg * nlcn_msg) {
    proc_event_baton_t *baton = g_slice_new0(proc_event_baton_t);
    baton->req.data = (void *)baton;
    switch(nlcn_msg->nl_body.proc_ev.what) {
        case PROC_EVENT_FORK:
            baton->event = FORK;
            baton->operation = UPDATE_PARENT_EXE;
            break;

        case PROC_EVENT_EXEC:
            baton->event = EXEC;
            baton->operation = UPDATE_PROCESS_EXE;
            break;

        case PROC_EVENT_EXIT:
            baton->event = EXIT;
            baton->operation = UPDATE_PROCESS_EXE;
            break;

        case PROC_EVENT_NONE:
        case PROC_EVENT_UID:
        case PROC_EVENT_GID:
        case PROC_EVENT_SID:
        case PROC_EVENT_PTRACE:
        case PROC_EVENT_COMM:
            assert(1);
            break;
    }

    switch (baton->operation) {
        case UPDATE_PARENT_EXE:
            baton->parent_pid = 
                          nlcn_msg->nl_body.proc_ev.event_data.fork.parent_pid;
            baton->parent_proc_exe = bformat("/proc/%d/exe",
                                             baton->parent_pid);
            baton->parent_proc_cmdline = bformat("/proc/%d/cmdline",
                                                 baton->parent_pid);
            baton->child_pid =
                           nlcn_msg->nl_body.proc_ev.event_data.fork.child_pid;
            baton->child_proc_exe = bformat("/proc/%d/exe",
                                            baton->child_pid);
            baton->child_proc_cmdline = bformat("/proc/%d/cmdline",
                                                baton->parent_pid);
            uv_fs_readlink(loop,
                           (uv_fs_t *)baton,
                           bdatae(baton->parent_proc_exe, "<out of memory>"),
                           on_fs_event_processed);
            break;

        case UPDATE_PROCESS_EXE:
            baton->process_pid =
                         nlcn_msg->nl_body.proc_ev.event_data.exec.process_pid;
            baton->process_proc_exe = bformat("/proc/%d/exe",
                                              baton->process_pid);
            baton->process_proc_cmdline = bformat("/proc/%d/cmdline",
                                                  baton->process_pid);
            uv_fs_readlink(loop,
                           (uv_fs_t *)baton,
                           bdatae(baton->process_proc_exe, "<out of memory>"),
                           on_fs_event_processed);
            break;

        case UPDATE_CHILD_EXE:
        case UPDATE_PARENT_CMDLINE_OPEN:
        case UPDATE_PARENT_CMDLINE_READ:
        case UPDATE_PARENT_CMDLINE_CLOSE:
        case UPDATE_CHILD_CMDLINE_OPEN:
        case UPDATE_CHILD_CMDLINE_READ:
        case UPDATE_CHILD_CMDLINE_CLOSE:
        case UPDATE_PROCESS_CMDLINE_OPEN:
        case UPDATE_PROCESS_CMDLINE_READ:
        case UPDATE_PROCESS_CMDLINE_CLOSE:
            assert(1);
            break;
    }
}

void append_bstring_with_null_delimited_cstr(bstring *output,
                                             char *buffer,
                                             int size) {
    bstring b0 = bfromcstralloc(size, "");
    struct bstrList *lines;
    if (!(*output)) {
        *output = bfromcstralloc(size * 2, "");
    }
    if (BSTR_ERR != bassignblk(b0, buffer, size)) {
        if (NULL != (lines = bsplit(b0, '\0'))) {
            for (int i = 0; i < lines->qty; i++) {
                bcatcstr(*output, bdata(lines->entry[i]));
                bcatcstr(*output, " ");
            }
            btrimws(*output);
            bstrListDestroy(lines);
        }                    
    }
    bdestroy(b0);
}

int get_is_result_available(proc_event_baton_t *baton) {
    bool is_result_available = false;
    switch(baton->operation) {
        case UPDATE_PARENT_EXE:
        case UPDATE_CHILD_EXE:
        case UPDATE_PROCESS_EXE:
            is_result_available = (baton->req.result != -1) &&
                                  (baton->req.ptr);
            break;

        case UPDATE_PARENT_CMDLINE_READ:
        case UPDATE_CHILD_CMDLINE_READ:
        case UPDATE_PROCESS_CMDLINE_READ:
            is_result_available = (baton->req.result != -1);
            break;

        case UPDATE_PARENT_CMDLINE_OPEN:
        case UPDATE_PARENT_CMDLINE_CLOSE:
        case UPDATE_CHILD_CMDLINE_OPEN:
        case UPDATE_CHILD_CMDLINE_CLOSE:
        case UPDATE_PROCESS_CMDLINE_OPEN:
        case UPDATE_PROCESS_CMDLINE_CLOSE:
            break;
    }
    return is_result_available;
}

void update_baton_fields(proc_event_baton_t *baton) {
    bool is_result_available = get_is_result_available(baton);
    switch (baton->operation) {
        case UPDATE_PARENT_EXE:
            baton->parent_exe = is_result_available ?
                            bfromcstr(baton->req.ptr) : bfromcstr("<unknown>");
            break;
        case UPDATE_CHILD_EXE:
            baton->child_exe = is_result_available ?
                            bfromcstr(baton->req.ptr) : bfromcstr("<unknown>");
            break;
        case UPDATE_PROCESS_EXE:
            baton->process_exe = is_result_available ?
                            bfromcstr(baton->req.ptr) : bfromcstr("<unknown>");
            break;
        case UPDATE_PARENT_CMDLINE_READ:
            if (is_result_available) {
                append_bstring_with_null_delimited_cstr(
                    &(baton->parent_cmdline),
                    baton->buffer,
                    baton->req.result);
            } else {
                baton->parent_cmdline = bfromcstr("<unknown>");
            }
            break;
        case UPDATE_CHILD_CMDLINE_READ:
            if (is_result_available) {
                append_bstring_with_null_delimited_cstr(
                    &(baton->child_cmdline),
                    baton->buffer,
                    baton->req.result);
            } else {
                baton->child_cmdline = bfromcstr("<unknown>");
            }
            break;
        case UPDATE_PROCESS_CMDLINE_READ:
            if (is_result_available) {
                append_bstring_with_null_delimited_cstr(
                    &(baton->process_cmdline),
                    baton->buffer,
                    baton->req.result);
            } else {
                baton->process_cmdline = bfromcstr("<unknown>");
            }
            break;

        case UPDATE_PARENT_CMDLINE_OPEN:
        case UPDATE_PARENT_CMDLINE_CLOSE:
        case UPDATE_CHILD_CMDLINE_OPEN:
        case UPDATE_CHILD_CMDLINE_CLOSE:
        case UPDATE_PROCESS_CMDLINE_OPEN:
        case UPDATE_PROCESS_CMDLINE_CLOSE:
            break;
    }
}

void update_baton_operation(proc_event_baton_t **baton_pp) {
    proc_event_baton_t *baton_p = *baton_pp;
    switch(baton_p->operation) {
        case UPDATE_PARENT_EXE:
            baton_p->operation = UPDATE_PARENT_CMDLINE_OPEN;
            break;

        case UPDATE_PARENT_CMDLINE_OPEN:
            if (baton_p->req.result < 0) {
                on_finished_processing_event(baton_pp);
            } else {
                baton_p->operation = UPDATE_PARENT_CMDLINE_READ;
                baton_p->open_fh = baton_p->req.result;
            }
            break;

        case UPDATE_PARENT_CMDLINE_READ:
            if (baton_p->req.result > 0) {
                baton_p->operation = UPDATE_PARENT_CMDLINE_READ;
            } else {
                baton_p->operation = UPDATE_PARENT_CMDLINE_CLOSE;
            }
            break;

        case UPDATE_PARENT_CMDLINE_CLOSE:
            baton_p->operation = UPDATE_CHILD_EXE;
            break;

        case UPDATE_CHILD_EXE:
            baton_p->operation = UPDATE_CHILD_CMDLINE_OPEN;
            break;
            
        case UPDATE_CHILD_CMDLINE_OPEN:
            if (baton_p->req.result < 0) {
                on_finished_processing_event(baton_pp);
            } else {
                baton_p->operation = UPDATE_CHILD_CMDLINE_READ;
                baton_p->open_fh = baton_p->req.result;
            }
            break;

        case UPDATE_CHILD_CMDLINE_READ:
            if (baton_p->req.result > 0) {
                baton_p->operation = UPDATE_CHILD_CMDLINE_READ;
            } else {
                baton_p->operation = UPDATE_CHILD_CMDLINE_CLOSE;
            }
            break;

        case UPDATE_CHILD_CMDLINE_CLOSE:
            on_finished_processing_event(baton_pp);
            break;

        case UPDATE_PROCESS_EXE:
            baton_p->operation = UPDATE_PROCESS_CMDLINE_OPEN;
            break;

        case UPDATE_PROCESS_CMDLINE_OPEN:
            if (baton_p->req.result < 0) {
                on_finished_processing_event(baton_pp);
            } else {
                baton_p->operation = UPDATE_PROCESS_CMDLINE_READ;
                baton_p->open_fh = baton_p->req.result;
            }
            break;

        case UPDATE_PROCESS_CMDLINE_READ:
            if (baton_p->req.result > 0) {
                baton_p->operation = UPDATE_PROCESS_CMDLINE_READ;
            } else {
                baton_p->operation = UPDATE_PROCESS_CMDLINE_CLOSE;
            }
            break;

        case UPDATE_PROCESS_CMDLINE_CLOSE:
            on_finished_processing_event(baton_pp);
            break;
    }  
}

void kick_fs_event_fsm(proc_event_baton_t **baton_pp) {    
    update_baton_operation(baton_pp);
    if (!(*baton_pp)) {
        return;
    }
    proc_event_baton_t *baton_p = *baton_pp;
    switch(baton_p->operation) {
        case UPDATE_PARENT_CMDLINE_OPEN:
            uv_fs_open(
                loop,
                (uv_fs_t *)baton_p,
                bdatae(baton_p->parent_proc_cmdline, "<out of memory>"),
                O_RDONLY,
                0,
                on_fs_event_processed);
            break;

        case UPDATE_CHILD_CMDLINE_OPEN:
            uv_fs_open(
                loop,
                (uv_fs_t *)baton_p,
                bdatae(baton_p->child_proc_cmdline, "<out of memory>"),
                O_RDONLY,
                0,
                on_fs_event_processed);
            break;

        case UPDATE_PROCESS_CMDLINE_OPEN:
            uv_fs_open(
                loop,
                (uv_fs_t *)baton_p,
                bdatae(baton_p->process_proc_cmdline, "<out of memory>"),
                O_RDONLY,
                0,
                on_fs_event_processed);                  
            break;

        case UPDATE_PARENT_CMDLINE_READ:
        case UPDATE_CHILD_CMDLINE_READ:
        case UPDATE_PROCESS_CMDLINE_READ:
            uv_fs_read(
                loop,
                (uv_fs_t *)baton_p,
                baton_p->req.result,
                baton_p->buffer,
                sizeof(baton_p->buffer),
                -1,
                on_fs_event_processed);  
            break;

        case UPDATE_PARENT_CMDLINE_CLOSE:
        case UPDATE_CHILD_CMDLINE_CLOSE:
        case UPDATE_PROCESS_CMDLINE_CLOSE:
            uv_fs_close(
                loop,
                (uv_fs_t *)baton_p,
                baton_p->open_fh,
                on_fs_event_processed);
            break;

        case UPDATE_CHILD_EXE:
            uv_fs_readlink(
                loop,
                (uv_fs_t *)baton_p,
                bdatae(baton_p->child_proc_exe, "<out of memory>"),
                on_fs_event_processed);
            break;

        case UPDATE_PARENT_EXE:
        case UPDATE_PROCESS_EXE:
            assert(1);
            break;
    }
}

void on_fs_event_processed(uv_fs_t *req) {    
    proc_event_baton_t *baton = (proc_event_baton_t *)req->data;   
    update_baton_fields(baton); 
    uv_fs_req_cleanup(req);
    kick_fs_event_fsm(&baton);
}

void on_finished_processing_event(proc_event_baton_t **baton_pp) {
    proc_event_baton_t *baton_p = *baton_pp;
    switch(baton_p->event) {
        case FORK:
            printf("fork. parent (exe=%s) (cmdline=%s) (pid=%d) -> child (exe=%s) (cmdline=%s) (pid=%d)\n",
                   bdatae(baton_p->parent_exe, "<out of memory>"),
                   bdatae(baton_p->parent_cmdline, "<out of memory>"),
                   baton_p->parent_pid,
                   bdatae(baton_p->child_exe, "<out of memory>"),
                   bdatae(baton_p->child_cmdline, "<out of memory>"),
                   baton_p->child_pid);
            break;

        case EXEC:
            printf("exec. process (exe=%s) (cmdline=%s) (pid=%d)\n",
                   bdatae(baton_p->process_exe, "<out of memory>"),
                   bdatae(baton_p->process_cmdline, "<out of memory>"),
                   baton_p->process_pid);
            break;

        case EXIT:
            printf("exit. process (exe=%s) (cmdline=%s) (pid=%d)\n",
                   bdatae(baton_p->process_exe, "<out of memory>"),
                   bdatae(baton_p->process_cmdline, "<out of memory>"),
                   baton_p->process_pid);
            break;
    }
    
    bdestroy(baton_p->parent_proc_exe);
    bdestroy(baton_p->parent_exe);
    bdestroy(baton_p->parent_proc_cmdline);
    bdestroy(baton_p->parent_cmdline);
    bdestroy(baton_p->child_proc_exe);
    bdestroy(baton_p->child_exe);
    bdestroy(baton_p->child_proc_cmdline);
    bdestroy(baton_p->child_cmdline);    
    bdestroy(baton_p->process_proc_exe);
    bdestroy(baton_p->process_exe);
    bdestroy(baton_p->process_proc_cmdline);
    bdestroy(baton_p->process_cmdline);
    g_slice_free(proc_event_baton_t, *baton_pp);
    *baton_pp = NULL;
}
