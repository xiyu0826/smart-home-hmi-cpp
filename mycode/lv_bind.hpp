#pragma once
// lv_bind.hpp —— 把 LVGL 的 C 回调桥接到 C++ 成员方法
//
// LVGL 的事件回调类型是 C 函数指针 `lv_event_cb_t = void(*)(lv_event_t*)`，
// 非静态成员函数因含隐式 this 无法直接用作回调。这里的 CRTP 基类通过
// `lv_obj_add_event_cb` 的 user_data 参数回传 this，在 static 回调里恢复并
// 转调成员方法 `on_event`，从而让每个类都能安全地把自身方法绑到控件上。

#include "../lvgl/lvgl.h"

namespace smarthome {

// 事件桥接：继承 `EventTarget<Derived>` 并实现 `void on_event(lv_event_t*)`。
// 注意：这是模板静态转调，不是虚函数覆盖（基类里没有 on_event），
// 所以子类实现 on_event 时**不要**写 override，否则报 "marked override, but does not override"。
template <typename Derived>
class EventTarget {
protected:
    // C 回调入口：从 user_data 恢复 this 后转调成员方法。
    static void event_cb(lv_event_t *e) {
        auto *self = static_cast<Derived *>(lv_event_get_user_data(e));
        if (self) self->on_event(e);
    }

    // 绑定事件，自动把 this 作为 user_data 传入。
    void bind_event(lv_obj_t *obj, lv_event_code_t code) {
        lv_obj_add_event_cb(obj, &EventTarget<Derived>::event_cb, code, this);
    }
};

// 定时器桥接：继承 `TimerTarget<Derived>` 并实现 `void on_timer(lv_timer_t*)`。
template <typename Derived>
class TimerTarget {
protected:
    static void timer_cb(lv_timer_t *t) {
        auto *self = static_cast<Derived *>(lv_timer_get_user_data(t));
        if (self) self->on_timer(t);
    }

    lv_timer_t *create_timer(uint32_t period_ms) {
        return lv_timer_create(&TimerTarget<Derived>::timer_cb, period_ms, this);
    }
};

} // namespace smarthome
