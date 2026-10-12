// main.cpp —— 智能家居控制中心主程序入口（C++ 版）
//
// 流程：lv_init → 显示驱动初始化(SDL) → SmartHome 主界面 → HTTP 服务器线程 → 主循环

#include "lvgl/lvgl.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "mycode/smart_home.hpp"
#include "mycode/http_server.hpp"
#include "mycode/http_bridge.hpp"

// 获取环境变量，不存在则返回默认值（C++ 标准写法，替代 GNU 的 `?:` 扩展）
static const char *getenv_default(const char *name, const char *dflt) {
    const char *v = getenv(name);
    return v ? v : dflt;
}

// 显示驱动初始化：当前 lv_conf.h 启用 SDL（PC 模拟），其余分支保留供切换
#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void) {
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, device);
    lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void) {
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();
    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void) {
    const int width = std::atoi(getenv_default("LV_SDL_VIDEO_WIDTH", "1024"));
    const int height = std::atoi(getenv_default("LV_SDL_VIDEO_HEIGHT", "600"));
    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

int main(void) {
    // 1. 初始化 LVGL
    lv_init();

    // 2. 初始化显示驱动（SDL 窗口）
    lv_linux_disp_init();

    // 3. 初始化智能家居主界面（字体 + UI + 场景默认值）
    smarthome::SmartHome::instance().init();

    // 4. 启动 HTTP 服务器线程（后台运行）
    smarthome::HttpServer::instance().start();

    std::printf("\n=========================================\n");
    std::printf("  智能家居控制中心 - HTTP服务已启动\n");
    std::printf("  打开浏览器访问: http://localhost:8080\n");
    std::printf("=========================================\n\n");

    // 5. 主循环：刷新 UI + 处理 HTTP 命令 + 同步状态到 HTTP
    while (smarthome::SmartHome::instance().running()) {
        lv_timer_handler();                            // LVGL 定时器/动画/重绘
        smarthome::HttpBridge::instance().process_all();        // HTTP → UI 命令
        smarthome::HttpBridge::instance().sync_lvgl_to_http();  // UI → HTTP 状态
        usleep(5000);                                  // 5ms，约 200 次/秒
    }

    // 6. 退出：停止 HTTP 服务器（HttpServer 析构也会兜底 stop）
    smarthome::HttpServer::instance().stop();

    return 0;
}
