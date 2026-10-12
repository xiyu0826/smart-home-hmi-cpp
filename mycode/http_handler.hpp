#pragma once
// http_handler.hpp —— HTTP 请求处理（路由 + JSON + 设备控制）

#include "http_server.hpp"

namespace smarthome {

// HTTP 请求主分发函数（由 http_server 调用），根据 method 分发 GET/POST
void http_handle_request(int client_fd, HttpRequest *req);

} // namespace smarthome
