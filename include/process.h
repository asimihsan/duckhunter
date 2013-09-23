#ifndef PROCESS_H_
#define PROCESS_H_

typedef enum {
    FORK,
    EXEC,
    EXIT
} event_t;
typedef enum {
    UPDATE_PARENT_EXE,
    UPDATE_PARENT_CMDLINE_OPEN,
    UPDATE_PARENT_CMDLINE_READ,
    UPDATE_PARENT_CMDLINE_CLOSE,

    UPDATE_CHILD_EXE,
    UPDATE_CHILD_CMDLINE_OPEN,
    UPDATE_CHILD_CMDLINE_READ,
    UPDATE_CHILD_CMDLINE_CLOSE,

    UPDATE_PROCESS_EXE,
    UPDATE_PROCESS_CMDLINE_OPEN,
    UPDATE_PROCESS_CMDLINE_READ,
    UPDATE_PROCESS_CMDLINE_CLOSE
} operation_t;

typedef struct {
    uv_fs_t req;
    event_t event;
    operation_t operation;

    char buffer[1024];
    int open_fh;

    int parent_pid;
    bstring parent_proc_exe;
    bstring parent_exe;
    bstring parent_proc_cmdline;
    bstring parent_cmdline;

    int child_pid;
    bstring child_proc_exe;
    bstring child_exe;    
    bstring child_proc_cmdline;
    bstring child_cmdline;

    int process_pid;
    bstring process_proc_exe;
    bstring process_exe;
    bstring process_proc_cmdline;
    bstring process_cmdline;
} proc_event_baton_t;

void start_processing_event(nlcn_ev_msg * nlcn_msg);
void on_fs_event_processed(uv_fs_t *req);
void on_finished_processing_event(proc_event_baton_t **baton_pp);

void append_bstring_with_null_delimited_cstr(bstring *output,
                                             char *buffer,
                                             int size);
int get_is_result_available(proc_event_baton_t *baton);
void update_baton_fields(proc_event_baton_t *baton);
void kick_fs_event_fsm(proc_event_baton_t **baton_pp);
void update_baton_operation(proc_event_baton_t **baton_pp);

#endif
