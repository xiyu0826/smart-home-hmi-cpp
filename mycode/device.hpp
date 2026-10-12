#pragma once
// device.hpp —— 设备控件封装（开关 / 滑块 / 下拉框 / 档位指示灯）
//
// 这些类把原 C 版里大量重复的"控件 + 回调 + 状态"模式封装成对象。
// 关键设计：设备的"初始配置"在构造时设定，create() 用**当前状态**创建控件，
// 因此场景在房间初始化前应用也能被正确反映（apply_scene 先改状态，init 再据此建控件）。
// 程序化赋值（set / set_value / set_selected）不触发回调，沿用 LVGL 的
// "lv_slider_set_value 不触发 VALUE_CHANGED"语义，保证联动不会死循环。

#include "../lvgl/lvgl.h"
#include "lv_bind.hpp"
#include "smart_home.hpp"
#include <cstdio>
#include <functional>
#include <string>

namespace smarthome {

// 注意：本工程用的 LVGL 9.0 里 lv_event_get_target() 声明为 `void *lv_event_get_target(...)`。
// C 语言能把它隐式转成 lv_obj_t*，C++ 不允许，所以下面每个取 target 的地方都显式转换。

/* ==================== 档位指示灯组 ==================== */

class IndicatorGroup {
public:
    static constexpr int MAX_INDICATORS = 8;

    // 创建指示灯组。img_path 非空用图片版，否则用圆点版。
    // （修复原 C 版用 tmp[0]!=NULL 判断导致"圆点回退恒为死代码"的 bug）
    void create(lv_obj_t *parent, int x, int y, int w, int h,
                int value, int max, int count,
                lv_color_t on, lv_color_t off, const char *img_path) {
        count_ = count;
        on_color_ = on;
        off_color_ = off;
        is_image_ = (img_path != nullptr);

        int gap = 3;
        for (int i = 0; i < count_; i++) {
            if (is_image_) {
                lv_obj_t *img = lv_image_create(parent);
                lv_image_set_src(img, img_path);
                lv_obj_set_size(img, w, h);
                lv_obj_set_pos(img, x + i * (w + gap), y);
                lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
                indicators_[i] = img;
            } else {
                lv_obj_t *dot = lv_obj_create(parent);
                lv_obj_set_size(dot, w, h);
                lv_obj_set_pos(dot, x + i * (w + gap), y);
                lv_obj_set_style_radius(dot, w / 2, 0);
                lv_obj_set_style_border_width(dot, 0, 0);
                lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
                indicators_[i] = dot;
            }
        }
        update(value, max);
    }

    void update(int value, int max) {
        if (count_ <= 0) return;
        int step = max / count_;
        if (step <= 0) step = 1;
        for (int i = 0; i < count_; i++) {
            int threshold = (i + 1) * step;
            bool lit = (value >= threshold);
            if (is_image_) {
                lv_obj_set_style_opa(indicators_[i], lit ? LV_OPA_COVER : LV_OPA_30, 0);
            } else {
                lv_obj_set_style_bg_color(indicators_[i], lit ? on_color_ : off_color_, 0);
            }
        }
    }

private:
    lv_obj_t *indicators_[MAX_INDICATORS] = {};
    int count_ = 0;
    bool is_image_ = false;
    lv_color_t on_color_{};
    lv_color_t off_color_{};
};

/* ==================== 开关设备 ==================== */

class SwitchDevice : public EventTarget<SwitchDevice> {
public:
    explicit SwitchDevice(bool initial = false) : on_(initial) {}

    void create(lv_obj_t *card, int x, int y, int w = 60, int h = 30) {
        sw_ = lv_switch_create(card);
        lv_obj_set_pos(sw_, x, y);
        lv_obj_set_size(sw_, w, h);
        if (on_) lv_obj_add_state(sw_, LV_STATE_CHECKED);
        bind_event(sw_, LV_EVENT_VALUE_CHANGED);
    }

    // 程序赋值（不触发回调）
    void set(bool on) {
        on_ = on;
        if (sw_) {
            if (on) lv_obj_add_state(sw_, LV_STATE_CHECKED);
            else    lv_obj_remove_state(sw_, LV_STATE_CHECKED);
        }
    }

    bool get() const { return on_; }

    // 联动回调（开关状态改变时触发，可空）
    void on_changed(std::function<void(bool)> cb) { cb_ = std::move(cb); }

    void on_event(lv_event_t *e) {
        lv_obj_t *tgt = static_cast<lv_obj_t *>(lv_event_get_target(e));
        on_ = lv_obj_has_state(tgt, LV_STATE_CHECKED);
        if (cb_) cb_(on_);
    }

private:
    bool on_ = false;
    lv_obj_t *sw_ = nullptr;
    std::function<void(bool)> cb_;
};

/* ==================== 滑块设备 ==================== */

class SliderDevice : public EventTarget<SliderDevice> {
public:
    SliderDevice(int min, int max, int value, const char *fmt)
        : min_(min), max_(max), value_(value), fmt_(fmt) {}

    // 创建滑块 + 数值标签（用当前 value_，而非硬编码初始值）
    void create(lv_obj_t *card, int x, int y, int w, int h, int label_x, int label_y) {
        slider_ = lv_slider_create(card);
        lv_obj_set_size(slider_, w, h);
        lv_obj_set_pos(slider_, x, y);
        lv_slider_set_range(slider_, min_, max_);
        lv_slider_set_value(slider_, value_, LV_ANIM_OFF);
        bind_event(slider_, LV_EVENT_VALUE_CHANGED);

        label_ = lv_label_create(card);
        lv_obj_set_style_text_font(label_, &lv_font_montserrat_18, 0);
        lv_obj_set_pos(label_, label_x, label_y);
        update_label();
    }

    // 程序赋值（不触发回调）
    void set_value(int v, bool anim = false) {
        value_ = v;
        if (slider_) lv_slider_set_value(slider_, v, anim ? LV_ANIM_ON : LV_ANIM_OFF);
        update_label();
    }

    int value() const { return value_; }
    lv_obj_t *label() const { return label_; }

    void on_changed(std::function<void(int)> cb) { cb_ = std::move(cb); }

    void on_event(lv_event_t *e) {
        lv_obj_t *tgt = static_cast<lv_obj_t *>(lv_event_get_target(e));
        value_ = static_cast<int>(lv_slider_get_value(tgt));
        update_label();
        if (cb_) cb_(value_);
    }

private:
    void update_label() {
        if (!label_) return;
        char buf[32];
        std::snprintf(buf, sizeof(buf), fmt_.c_str(), value_);
        lv_label_set_text(label_, buf);
    }

    int value_;
    int min_;
    int max_;
    std::string fmt_;
    lv_obj_t *slider_ = nullptr;
    lv_obj_t *label_ = nullptr;
    std::function<void(int)> cb_;
};

/* ==================== 下拉框设备 ==================== */

class DropdownDevice : public EventTarget<DropdownDevice> {
public:
    DropdownDevice(const char *options, int selected, int pad_left = 23)
        : options_(options), selected_(selected), pad_left_(pad_left) {}

    void create(lv_obj_t *card, int x, int y, int w, int h) {
        dd_ = lv_dropdown_create(card);
        lv_obj_set_pos(dd_, x, y);
        lv_obj_set_size(dd_, w, h);
        lv_dropdown_set_options(dd_, options_);
        lv_dropdown_set_selected(dd_, selected_);
        lv_obj_set_style_text_font(dd_, SmartHome::instance().font_chinese(), 0);
        lv_obj_set_style_pad_top(dd_, 1, 0);
        lv_obj_set_style_pad_bottom(dd_, 7, 0);
        lv_obj_set_style_pad_left(dd_, pad_left_, 0);
        lv_obj_set_style_pad_right(dd_, 4, 0);
        lv_dropdown_set_symbol(dd_, " ");
        bind_event(dd_, LV_EVENT_VALUE_CHANGED);
        bind_event(dd_, LV_EVENT_CLICKED); // 展开时设置列表字体
    }

    void set_selected(int idx) {
        selected_ = idx;
        if (dd_) lv_dropdown_set_selected(dd_, idx);
    }

    int selected() const { return selected_; }

    void on_changed(std::function<void(int)> cb) { cb_ = std::move(cb); }

    void on_event(lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t *tgt = static_cast<lv_obj_t *>(lv_event_get_target(e));
        if (code == LV_EVENT_VALUE_CHANGED) {
            selected_ = static_cast<int>(lv_dropdown_get_selected(tgt));
            if (cb_) cb_(selected_);
        } else if (code == LV_EVENT_CLICKED) {
            // 下拉列表是独立对象，需额外设置中文字体
            lv_obj_t *list = lv_dropdown_get_list(tgt);
            if (list) lv_obj_set_style_text_font(list, SmartHome::instance().font_chinese(), 0);
        }
    }

private:
    int selected_;
    const char *options_;
    int pad_left_;
    lv_obj_t *dd_ = nullptr;
    std::function<void(int)> cb_;
};

} // namespace smarthome
