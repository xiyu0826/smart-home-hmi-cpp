#pragma once
// room.hpp —— 房间抽象基类 + 房间共享 UI 辅助
//
// 四个房间（客厅/卧室/厨房/卫生间）共享统一的 5 个操作接口，
// 便于 SmartHome 的场景分发与 HttpBridge 的命令分发统一调用。
// 顶部栏、返回回调等重复代码抽取为共享函数，消除原 C 版的四处重复。

#include "../lvgl/lvgl.h"
#include "scene_config.hpp"
#include "smart_home.hpp"
#include <string>

namespace smarthome {

class Room {
public:
    virtual ~Room() = default;

    lv_obj_t *screen = nullptr;

    // 懒加载：首次进入时才创建界面
    void ensure_init() {
        if (screen == nullptr) init();
    }

    // 创建房间界面（含顶部栏 + 设备卡片）
    virtual void init() = 0;
    // 应用场景配置到本房间（直接赋值状态再同步 UI，不走回调）
    virtual void apply_scene(const SceneConfig &cfg) = 0;
    // 处理来自 HTTP 的设备控制命令（由 HttpBridge 在主循环分发）
    virtual void web_control(const std::string &device, const std::string &action, int value) = 0;
    // 生成本房间设备状态的 JSON（供 HTTP 接口返回）
    virtual void get_json(std::string &out) const = 0;
    // 把本房间状态同步到 HTTP 快照结构体（供网页轮询）
    virtual void sync_to_http() = 0;
};

// 返回主界面回调（四个房间共用）
inline void back_to_main_cb(lv_event_t * /*e*/) {
    lv_screen_load(SmartHome::instance().main_screen());
}

// 创建房间顶部栏（返回按钮 + 居中标题），四个房间共用
inline lv_obj_t *create_top_bar(lv_obj_t *parent, const char *title) {
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 1024, 56);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x2c3e50), 0);
    lv_obj_set_pos(bar, 0, 0);

    lv_obj_t *btn = lv_button_create(bar);
    lv_obj_set_size(btn, 70, 36);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(btn, back_to_main_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "返回");
    lv_obj_set_style_text_font(btn_label, SmartHome::instance().font_chinese(), 0);
    lv_obj_center(btn_label);

    lv_obj_t *title_label = lv_label_create(bar);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), 0);
    lv_obj_center(title_label);

    return bar;
}

} // namespace smarthome
