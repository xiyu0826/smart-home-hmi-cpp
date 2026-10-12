#include "http_server.hpp"
#include "http_handler.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace smarthome {

/* ==================== 工具 ==================== */

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* ==================== 响应发送 ==================== */

static void send_response(int client_fd, int status_code, const char *status_msg,
                          const char *content_type, const char *body, int body_len) {
    char header[512];
    int header_len = std::snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache, no-store, must-revalidate\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        status_code, status_msg, content_type, body_len);

    if (send(client_fd, header, header_len, 0) < 0) return;

    if (body && body_len > 0) {
        int sent = 0;
        while (sent < body_len) {
            int n = send(client_fd, body + sent, body_len - sent, 0);
            if (n < 0) break;
            sent += n;
        }
    }
}

void HttpServer::send_html(int fd, const char *html) {
    send_response(fd, 200, "OK", "text/html; charset=utf-8", html, std::strlen(html));
}

void HttpServer::send_json(int fd, const char *json) {
    send_response(fd, 200, "OK", "application/json; charset=utf-8", json, std::strlen(json));
}

void HttpServer::send_error(int fd, int status_code, const char *msg) {
    char body[256];
    std::snprintf(body, sizeof(body), "{\"error\":%d,\"message\":\"%s\"}", status_code, msg);
    send_response(fd, status_code, msg, "application/json; charset=utf-8", body, std::strlen(body));
}

/* ==================== 请求解析 ==================== */

// 解析请求行 "METHOD path?query HTTP/1.1"，返回 method/path/query
static bool parse_request_line(const char *buf, HttpRequest *req) {
    // 找第一行
    const char *line_end = std::strstr(buf, "\r\n");
    if (!line_end) line_end = buf + std::strlen(buf);

    std::string line(buf, line_end - buf);
    std::size_t sp1 = line.find(' ');
    std::size_t sp2 = (sp1 == std::string::npos) ? std::string::npos : line.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return false;

    req->method = line.substr(0, sp1);
    std::string path_query = line.substr(sp1 + 1, sp2 - sp1 - 1);

    std::size_t q = path_query.find('?');
    if (q != std::string::npos) {
        req->path = path_query.substr(0, q);
        req->query = path_query.substr(q + 1);
    } else {
        req->path = path_query;
        req->query.clear();
    }
    return true;
}

static void handle_client_request(int client_fd, const char *buf, int len) {
    HttpRequest req;
    if (!parse_request_line(buf, &req)) {
        HttpServer::send_error(client_fd, 400, "Bad Request");
        return;
    }

    // 找 header 结束，解析 Content-Length
    const char *header_end = std::strstr(buf, "\r\n\r\n");
    const char *cl = std::strstr(buf, "Content-Length:");
    if (cl) req.content_length = std::atoi(cl + 15);

    if (header_end) {
        int header_len = header_end - buf + 4;
        if (header_len < len) {
            int body_len = len - header_len;
            req.body.assign(header_end + 4, body_len);
        }
    }

    http_handle_request(client_fd, &req);
}

/* ==================== 连接与数据处理 ==================== */

static void handle_new_connection(int epoll_fd, int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) return;

    int opt = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    set_nonblocking(client_fd); // 修复：客户端 fd 也设非阻塞，配合 EPOLLET

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        close(client_fd);
        return;
    }

    std::printf("[HTTP] New client: %s:%d (fd=%d)\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
}

static void handle_client_data(int epoll_fd, int client_fd) {
    char buf[HTTP_BUFFER_SIZE];
    int total = 0;

    while (true) {
        int n = recv(client_fd, buf + total, sizeof(buf) - 1 - total, 0);
        if (n < 0 && errno == EAGAIN) {
            break; // 修复：非阻塞模式下没数据，跳出等待下次事件（原 continue 会忙循环）
        }
        if (n <= 0) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
            close(client_fd);
            return;
        }
        total += n;
        buf[total] = '\0';

        const char *body_start = std::strstr(buf, "\r\n\r\n");
        if (body_start) {
            const char *cl = std::strstr(buf, "Content-Length:");
            int content_length = cl ? std::atoi(cl + 15) : 0;
            int header_len = body_start - buf + 4;
            int body_len = total - header_len;

            while (body_len < content_length) {
                if (total >= (int)sizeof(buf) - 1) break;
                n = recv(client_fd, buf + total, sizeof(buf) - 1 - total, 0);
                if (n < 0 && errno == EAGAIN) break;
                if (n <= 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                    close(client_fd);
                    return;
                }
                total += n;
                body_len += n;
                buf[total] = '\0';
            }
            break;
        }

        if (total >= (int)sizeof(buf) - 1) break;
    }

    handle_client_request(client_fd, buf, total);
}

/* ==================== 线程主函数 ==================== */

void HttpServer::thread_main() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) { perror("socket failed"); return; }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(HTTP_PORT);

    if (bind(server_fd_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd_);
        server_fd_ = -1;
        return;
    }
    if (listen(server_fd_, HTTP_BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd_);
        server_fd_ = -1;
        return;
    }
    set_nonblocking(server_fd_);

    epoll_fd_ = epoll_create(MAX_EPOLL_EVENTS);
    if (epoll_fd_ < 0) {
        perror("epoll_create failed");
        close(server_fd_);
        server_fd_ = -1;
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) < 0) {
        close(epoll_fd_);
        close(server_fd_);
        epoll_fd_ = -1;
        server_fd_ = -1;
        return;
    }

    std::printf("[HTTP] Server listening on port %d\n", HTTP_PORT);

    struct epoll_event events[MAX_EPOLL_EVENTS];
    while (running_.load()) {
        int nfds = epoll_wait(epoll_fd_, events, MAX_EPOLL_EVENTS, 100);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            if (fd == server_fd_) {
                handle_new_connection(epoll_fd_, fd);
            } else if (events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
                handle_client_data(epoll_fd_, fd);
            }
        }
    }

    std::printf("[HTTP] Server shutting down...\n");
    close(epoll_fd_);
    close(server_fd_);
    epoll_fd_ = -1;
    server_fd_ = -1;
}

/* ==================== 公共 API ==================== */

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    if (running_.load()) return true;
    running_.store(true);
    thread_ = std::thread(&HttpServer::thread_main, this);
    std::printf("[HTTP] Server thread created\n");
    return true;
}

void HttpServer::stop() {
    if (!running_.exchange(false)) return; // 幂等
    if (thread_.joinable()) thread_.join();
    std::printf("[HTTP] Server stopped\n");
}

} // namespace smarthome
