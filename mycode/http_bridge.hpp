#pragma once
// http_bridge.hpp —— HTTP 线程与 UI 线程的桥接（命令队列 + 双向同步）
//
// 用模板环形队列 RingBuffer<HttpCmd> + std::mutex 替代 pthread 环形队列。
// HTTP 线程 push 命令，主循环 process_all 取出分发到各房间 web_control，
// sync_lvgl_to_http 把 UI 状态同步回 HttpState 供网页查询。

#include <string>

#include "command_queue.hpp"

namespace smarthome {

// HTTP 命令（队列元素）
struct HttpCmd {
    std::string device;
    std::string action;
    int value = 0;
};

class HttpBridge {
public:
    static HttpBridge &instance() {
        static HttpBridge b;
        return b;
    }

    // HTTP 线程调用：把控制命令放入队列
    void push(const std::string &device, const std::string &action, int value);

    // 主线程调用：取出并处理所有待处理命令（按前缀分发到各房间）
    void process_all();

    // 主线程调用：把 UI 状态同步到 HttpState（节流，每 2 次调用同步一次）
    void sync_lvgl_to_http();

private:
    HttpBridge() = default;
    HttpBridge(const HttpBridge &) = delete;
    HttpBridge &operator=(const HttpBridge &) = delete;

    RingBuffer<HttpCmd, 64> queue_;
    int sync_counter_ = 0;
};

} // namespace smarthome
