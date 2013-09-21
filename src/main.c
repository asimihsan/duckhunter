/*!
 * duckhunter: src/main.c
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

#include "duckhunter.h"

uv_loop_t *loop;
connector_context_t *context;

void on_sigint(uv_signal_t *handle, int signum) {
    uv_poll_stop(&(context->poll_handle));
    destroy_connector_context(context);
    uv_stop(loop);
}

int main() {
    int rc = EXIT_SUCCESS;
    int conn_sock;
    uv_signal_t signal;

    loop = uv_default_loop();
    uv_signal_init(loop, &signal);
    uv_signal_start(&signal, on_sigint, SIGINT);
    conn_sock = create_connector_socket();
    set_socket_filter(conn_sock);
    context = create_connector_context(conn_sock);

    select_proc_connector(conn_sock);
    if (send_subscribe_message(conn_sock) == -1) {
        perror("failed to subscribe to connector socket!");
        rc = EXIT_FAILURE;
        goto EXIT_LABEL;
    }

    uv_poll_start(&(context->poll_handle), UV_READABLE,
                  handle_connector_event);
    uv_run(loop, UV_RUN_DEFAULT);

    if (send_unsubscribe_message(conn_sock) == -1) {
        perror("failed to unsubscribe from connector socket!");
        rc = EXIT_FAILURE;
        goto EXIT_LABEL;
    }

EXIT_LABEL:
    printf("exiting\n");
    close(conn_sock);
    return rc;
}
