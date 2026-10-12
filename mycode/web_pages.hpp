#pragma once
// web_pages.hpp —— Web 控制页面提供者
//
// 浏览器访问 http://localhost:8080 时，HTTP 服务器返回此页面。
// HTML/CSS/JS 与 C 版保持一致（响应式布局 + 2 秒轮询 /api/status + 设备控制）。

namespace smarthome {

// 返回 Web 控制页面 HTML（内含 CSS 与 JavaScript）
const char *web_get_page();

} // namespace smarthome
