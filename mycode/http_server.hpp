#pragma once
// http_server.hpp —— HTTP 服务器（RAII + std::thread + epoll 多路复用）
//
// 用 std::thread 替代 pthread，用 std::atomic 管理运行标志，析构自动 stop。
// 相比 C 版修复了：客户端 socket 非阻塞（配合 EPOLLET）、recv EAGAIN 用 break 而非 continue、
// 删除未使用的 EpollClient/HttpResponse/g_clients 死代码。

#include <atomic>
#include <string>
#include <thread>

namespace smarthome {

constexpr int HTTP_PORT          = 8080;
constexpr int HTTP_BACKLOG       = 128;
constexpr int HTTP_BUFFER_SIZE   = 4096;
constexpr int MAX_EPOLL_EVENTS   = 64;

// HTTP 请求（用 std::string 取代定长 char 数组）
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    int content_length = 0;
};

class HttpServer {
public:
    static HttpServer &instance() {
        static HttpServer s;
        return s;
    }
    ~HttpServer();

    bool start();                 // 启动服务器线程
    void stop();                  // 停止并等待线程退出（幂等）
    bool is_running() const { return running_.load(); }

    // 发送响应（供 http_handler 调用）
    static void send_html(int fd, const char *html);
    static void send_json(int fd, const char *json);
    static void send_error(int fd, int status_code, const char *msg);

private:
    HttpServer() = default;
    HttpServer(const HttpServer &) = delete;
    HttpServer &operator=(const HttpServer &) = delete;

    void thread_main();

    std::thread thread_;
    std::atomic<bool> running_{false};
    int server_fd_ = -1;
    int epoll_fd_ = -1;
};

} // namespace smarthome
