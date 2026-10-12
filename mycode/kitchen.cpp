#include "kitchen.hpp"
#include "http_state.hpp"
#include <cstdio>

namespace smarthome {

void Kitchen::init() {
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xf0f2f5), 0);
    create_top_bar(screen, "厨房");

    lv_obj_t *cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 980, 520);
    lv_obj_set_pos(cont, 22, 60);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 厨房全部是纯开关设备，用 lambda 统一创建（初始状态由成员默认值决定）
    auto add_switch = [&](const char *title, const char *img, lv_color_t color,
                          SwitchDevice &sw) {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, title, 460, CARD_H_SWITCH, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 35);
        SmartHome::instance().add_device_icon(card, img, color, 31);
        sw.create(card, POS_SWITCH_X, 33);
    };

    add_switch("厨房灯", IMG_KITCHEN_LIGHT, lv_color_hex(0xf1c40f), light_sw_);
    add_switch("制冰机", IMG_OUTLET, lv_color_hex(0x00bcd4), outlet_sw_);
    add_switch("油烟机", IMG_FAN, lv_color_hex(0x95a5a6), fan_sw_);
    add_switch("冰箱", IMG_FRIDGE, lv_color_hex(0x3498db), fridge_sw_);
    add_switch("微波炉", IMG_MICROWAVE, lv_color_hex(0xff9800), microwave_sw_);
    add_switch("洗碗机", IMG_DISHWASHER, lv_color_hex(0x9b59b6), dishwasher_sw_);
    add_switch("烤箱", IMG_OVEN, lv_color_hex(0xe74c3c), oven_sw_);
    add_switch("净水器", IMG_WATER, lv_color_hex(0x1abc9c), purifier_water_sw_);
}

void Kitchen::apply_scene(const SceneConfig &cfg) {
    light_sw_.set(cfg.kt_light_on);
    outlet_sw_.set(cfg.kt_outlet_on);
    fan_sw_.set(cfg.kt_fan_on);
    fridge_sw_.set(cfg.kt_fridge_on);
    microwave_sw_.set(cfg.kt_microwave_on);
    dishwasher_sw_.set(cfg.kt_dishwasher_on);
    oven_sw_.set(cfg.kt_oven_on);
    purifier_water_sw_.set(cfg.kt_purifier_water_on);
}

void Kitchen::web_control(const std::string &device, const std::string &action, int /*value*/) {
    SwitchDevice *sw = nullptr;
    if (device == "kt_light")          sw = &light_sw_;
    else if (device == "outlet")       sw = &outlet_sw_;
    else if (device == "kt_fan")       sw = &fan_sw_;
    else if (device == "fridge")       sw = &fridge_sw_;
    else if (device == "microwave")    sw = &microwave_sw_;
    else if (device == "dishwasher")   sw = &dishwasher_sw_;
    else if (device == "oven")         sw = &oven_sw_;
    else if (device == "kt_purifier")  sw = &purifier_water_sw_;
    if (!sw) return;

    if (action == "on") sw->set(true);
    else if (action == "off") sw->set(false);
}

void Kitchen::get_json(std::string &out) const {
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":%d,\"outlet\":%d,\"fan\":%d,\"fridge\":%d,"
        "\"microwave\":%d,\"dishwasher\":%d,\"oven\":%d,\"purifier\":%d}",
        light_sw_.get(), outlet_sw_.get(), fan_sw_.get(), fridge_sw_.get(),
        microwave_sw_.get(), dishwasher_sw_.get(), oven_sw_.get(), purifier_water_sw_.get());
    out = buf;
}

void Kitchen::sync_to_http() {
    HttpState &s = HttpState::instance();
    s.kt.light_on = light_sw_.get();
    s.kt.outlet_on = outlet_sw_.get();
    s.kt.fan_on = fan_sw_.get();
    s.kt.fridge_on = fridge_sw_.get();
    s.kt.microwave_on = microwave_sw_.get();
    s.kt.dishwasher_on = dishwasher_sw_.get();
    s.kt.oven_on = oven_sw_.get();
    s.kt.purifier_water_on = purifier_water_sw_.get();
}

} // namespace smarthome
