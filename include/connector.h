/*!
 * duckhunter: include/connector.h.c
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

#ifndef CONNECTOR_H_
#define CONNECTOR_H_

int create_connector_socket();
void select_proc_connector(int conn_sock);
void set_socket_filter();
int send_connector_message(int conn_sock, bool enable);
int send_subscribe_message(int conn_sock);
int send_unsubscribe_message(int conn_sock);
void handle_connector_event(uv_poll_t *req, int status, int events);

connector_context_t *create_connector_context(int conn_sock);
void connector_close_cb(uv_handle_t *handle);
void destroy_connector_context(connector_context_t *context);

#endif
