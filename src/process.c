#include "duckhunter.h"

void process_fork_event(uv_work_t *req) {
    fork_event_baton *baton = (fork_event_baton *)req->data;
    char buffer[1024];
    memset(buffer, 0, 1024);

    bstring parent_exe;
    bstring parent_proc_exe = bformat("/proc/%d/exe", baton->parent_pid);
    if (readlink((char *)parent_proc_exe->data, buffer, 1024) > -1) {
        parent_exe = bfromcstr(buffer);
    } else {
        parent_exe = bfromcstr("<unknown>");
    }

    bstring child_exe;
    bstring child_proc_exe = bformat("/proc/%d/exe", baton->child_pid);
    if (readlink((char *)child_proc_exe->data, buffer, 1024) > -1) {
        child_exe = bfromcstr(buffer);
    } else {
        child_exe = bfromcstr("<unknown>");
    }

    printf("fork. parent (%s) (%d) -> child (%s) (%d)\n",
           bdatae(parent_exe, "<out of memory>"),
           baton->parent_pid,
           bdatae(child_exe, "<out of memory>"),
           baton->child_pid);

    bdestroy(parent_exe);
    bdestroy(parent_proc_exe);
    bdestroy(child_exe);
    bdestroy(child_proc_exe);
}

void cleanup_fork_event(uv_work_t *req, int status) {
    fork_event_baton *baton = (fork_event_baton *)req->data;
    free(baton);
}
