#ifndef PROCESS_H_
#define PROCESS_H_

typedef struct {
    uv_work_t req;
    int parent_pid;
    int child_pid;
} fork_event_baton;

void process_fork_event(uv_work_t *req);
void cleanup_fork_event(uv_work_t *req, int status);

#endif
