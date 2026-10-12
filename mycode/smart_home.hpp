#pragma once
// smart_home.hpp —— 主界面与共享资源（字体、主屏、运行标志、UI 工具函数）
//
// SmartHome 是全局唯一的主界面实例（Meyers 单例），持有 FreeType 中文字体、
// 主屏幕、运行标志，并提供设备卡片 / 图标 / Toast 等 UI 工具函数。
// 各房间类和设备类通过 SmartHome::instance() 访问字体与工具。

#include "../lvgl/lvgl.h"
#include <atomic>

namespace smarthome {

/* ==================== 字体与资源路径 ==================== */

// 中文字体文件：默认在程序运行目录下查找 simhei.ttf。
// 需要绝对路径时，编译期用 -DFONT_PATH=\"/abs/path/xxx.ttf\" 覆盖。
#ifndef FONT_PATH
#define FONT_PATH "simhei.ttf"
#endif

inline constexpr int FONT_SIZE = 22;

/* ==================== 卡片布局常量 ==================== */

inline constexpr int CARD_H_SWITCH   = 110;  // 纯开关卡片高
inline constexpr int CARD_H_DROPDOWN = 130;  // 开关+下拉框卡片高
inline constexpr int CARD_H_SLIDER   = 170;  // 开关+滑块卡片高

inline constexpr int POS_ICON_X    = 15;   // 图标 X
inline constexpr int POS_TITLE_X   = 75;   // 标题 X
inline constexpr int POS_SWITCH_X  = 340;  // 开关 X
inline constexpr int POS_CTRL_LBL_X = 75;  // 滑块/下拉标签 X
inline constexpr int POS_SLIDER_X  = 130;  // 滑块 X
inline constexpr int POS_DD_X      = 130;  // 下拉框 X
inline constexpr int POS_VAL_X     = 310;  // 数值标签 X
inline constexpr int POS_IND_X     = 75;   // 指示灯起始 X

inline constexpr int ICON_W = 48;  // 图标宽
inline constexpr int ICON_H = 48;  // 图标高

/* ==================== 设备图片路径 ==================== */

// LVGL 文件系统盘符 'A' 映射到工作目录（见 lv_conf.h 的 LV_FS_STDIO_LETTER），
// 因此这里用相对路径，从项目根目录启动程序即可。编译期可用
// -DIMG_DIR=\"A:/abs/path/images/\" 覆盖。
#ifndef IMG_DIR
#define IMG_DIR "A:./images/"
#endif

// 客厅
#define IMG_LIGHT     IMG_DIR "light.bmp"
#define IMG_AC        IMG_DIR "ac.bmp"
#define IMG_CURTAIN   IMG_DIR "curtain.bmp"
#define IMG_STRIP     IMG_DIR "strip.bmp"
#define IMG_TV        IMG_DIR "tv.bmp"
#define IMG_ROBOT     IMG_DIR "robot.bmp"
#define IMG_PURIFIER  IMG_DIR "purifier.bmp"
#define IMG_SPEAKER   IMG_DIR "speaker.bmp"
// 卧室
#define IMG_HUMIDIFIER IMG_DIR "humidifier.bmp"
#define IMG_CLOSET    IMG_DIR "closet.bmp"
#define IMG_AROMA     IMG_DIR "aroma.bmp"
// 厨房
#define IMG_KITCHEN_LIGHT IMG_DIR "kitchen_light.bmp"
#define IMG_OUTLET    IMG_DIR "outlet.bmp"
#define IMG_FAN       IMG_DIR "fan.bmp"
#define IMG_FRIDGE    IMG_DIR "fridge.bmp"
#define IMG_MICROWAVE IMG_DIR "microwave.bmp"
#define IMG_DISHWASHER IMG_DIR "dishwasher.bmp"
#define IMG_OVEN      IMG_DIR "oven.bmp"
#define IMG_WATER     IMG_DIR "water.bmp"
// 卫生间
#define IMG_BATH_LIGHT IMG_DIR "bath_light.bmp"
#define IMG_EXHAUST   IMG_DIR "exhaust.bmp"
#define IMG_HEATER    IMG_DIR "heater.bmp"
#define IMG_BATH_HEATER IMG_DIR "bath_heater.bmp"
#define IMG_MIRROR    IMG_DIR "mirror.bmp"
#define IMG_TOILET    IMG_DIR "toilet.bmp"
// 指示灯
#define IMG_BULB      IMG_DIR "bulb.bmp"
#define IMG_SNOWFLAKE IMG_DIR "snowflake.bmp"
#define IMG_FIRE      IMG_DIR "fire.bmp"

/* ==================== SmartHome 单例 ==================== */

class SmartHome {
public:
    static SmartHome &instance() {
        static SmartHome s;
        return s;
    }

    // 初始化主界面（加载字体 + 创建全部 UI + 显示首页）
    void init();

    // ---- 共享资源访问 ----
    lv_font_t *font_chinese() const { return font_chinese_; }
    lv_obj_t  *main_screen() const { return main_screen_; }
    bool       running() const { return running_.load(); }
    void       set_running(bool v) { running_.store(v); }

    // ---- UI 工具函数 ----
    // 创建设备卡片容器（圆角背景 + 边框 + 中文标题）
    lv_obj_t *create_device_card(lv_obj_t *parent, const char *title,
                                 int w, int h, lv_obj_t **out_title = nullptr);
    // 在卡片上添加图标（图片加载失败时露出彩色占位底板）
    lv_obj_t *add_device_icon(lv_obj_t *card, const char *img_path,
                              lv_color_t placeholder_color, int y);
    // 显示 Toast 提示（1.5s 自动消失）
    void show_toast(const char *msg);

    // 传感器模拟数据（客厅温湿度 + 卧室温湿度）
    void get_sensors(int *lr_t, int *lr_h, int *br_t, int *br_h) const;

private:
    SmartHome() = default;
    SmartHome(const SmartHome &) = delete;
    SmartHome &operator=(const SmartHome &) = delete;

    // ---- 静态回调（LVGL 的 C 回调入口，定义在 smart_home.cpp）----
    static void clock_timer_cb(lv_timer_t *);
    static void sensor_timer_cb(lv_timer_t *);
    static void toast_delete_cb(lv_timer_t *);
    static void exit_app_cb(lv_event_t *);
    static void open_scene_settings_cb(lv_event_t *);
    static void scene_sleep_cb(lv_event_t *);
    static void scene_movie_cb(lv_event_t *);
    static void scene_away_cb(lv_event_t *);
    static void scene_home_cb(lv_event_t *);
    static void scene_power_off_cb(lv_event_t *);
    static void goto_livingroom_cb(lv_event_t *);
    static void goto_bedroom_cb(lv_event_t *);
    static void goto_kitchen_cb(lv_event_t *);
    static void goto_bathroom_cb(lv_event_t *);

    // ---- 状态 ----
    lv_font_t *font_chinese_ = nullptr;
    lv_obj_t  *main_screen_ = nullptr;
    std::atomic<bool> running_{true};

    lv_obj_t *clock_label_ = nullptr;
    lv_obj_t *lr_temp_l_ = nullptr, *lr_humi_l_ = nullptr;
    lv_obj_t *br_temp_l_ = nullptr, *br_humi_l_ = nullptr;
    lv_obj_t *toast_obj_ = nullptr;
    lv_timer_t *toast_timer_ = nullptr;

    // 传感器模拟值
    int sensor_st_ = 26, sensor_sh_ = 55, sensor_bt_ = 24, sensor_bh_ = 50;
};

} // namespace smarthome
