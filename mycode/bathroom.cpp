#include "bathroom.hpp"
#include "http_state.hpp"
#include <cstdio>

namespace smarthome {

static lv_obj_t *add_ctrl_label(lv_obj_t *card, const char *text, int x, int y) {
    lv_obj_t *l = lv_label_create(card);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_pos(l, x, y);
    return l;
}

void Bathroom::init() {
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xf0f2f5), 0);
    create_top_bar(screen, "卫生间");

    lv_obj_t *cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 980, 520);
    lv_obj_set_pos(cont, 22, 60);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    auto add_switch = [&](const char *title, const char *img, lv_color_t color,
                          SwitchDevice &sw) {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, title, 460, CARD_H_SWITCH, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 35);
        SmartHome::instance().add_device_icon(card, img, color, 31);
        sw.create(card, POS_SWITCH_X, 33);
    };

    // 卫生间灯 / 排风扇
    add_switch("卫生间灯", IMG_BATH_LIGHT, lv_color_hex(0xf1c40f), light_sw_);
    add_switch("排风扇", IMG_EXHAUST, lv_color_hex(0x95a5a6), fan_sw_);

    // ========== 热水器（开关 + 水温滑块） ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "热水器", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_HEATER, lv_color_hex(0xe74c3c), 61);

        heater_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "水温", POS_CTRL_LBL_X, 58);
        heater_temp_.create(card, POS_SLIDER_X, 66, 170, 10, POS_VAL_X, 56);
        heater_ig_.create(card, POS_IND_X, 98, 18, 18, heater_temp_.value(), 60, 5,
                          lv_color_hex(0xe74c3c), lv_color_hex(0xdddddd), IMG_FIRE);

        heater_temp_.on_changed([this](int v) { heater_ig_.update(v, 60); });
    }

    // ========== 浴霸（仅开关，用高卡片） ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "浴霸", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 35);
        SmartHome::instance().add_device_icon(card, IMG_BATH_HEATER, lv_color_hex(0xff5722), 61);
        bath_heater_sw_.create(card, POS_SWITCH_X, 33);
    }

    // 镜前灯 / 智能马桶
    add_switch("镜前灯", IMG_MIRROR, lv_color_hex(0x8bc34a), mirror_light_sw_);
    add_switch("智能马桶", IMG_TOILET, lv_color_hex(0x607d8b), smart_toilet_sw_);
}

void Bathroom::apply_scene(const SceneConfig &cfg) {
    light_sw_.set(cfg.bh_light_on);
    fan_sw_.set(cfg.bh_fan_on);
    heater_sw_.set(cfg.bh_heater_on);
    heater_temp_.set_value(cfg.bh_heater_temp, false);
    heater_ig_.update(cfg.bh_heater_temp, 60);
    bath_heater_sw_.set(cfg.bh_bath_heater_on);
    mirror_light_sw_.set(cfg.bh_mirror_light_on);
    smart_toilet_sw_.set(cfg.bh_smart_toilet_on);
}

void Bathroom::web_control(const std::string &device, const std::string &action, int value) {
    if (device == "bh_light") {
        if (action == "on") light_sw_.set(true);
        else if (action == "off") light_sw_.set(false);
    } else if (device == "bh_fan") {
        if (action == "on") fan_sw_.set(true);
        else if (action == "off") fan_sw_.set(false);
    } else if (device == "heater") {
        if (action == "on") heater_sw_.set(true);
        else if (action == "off") heater_sw_.set(false);
        if (value >= 30 && value <= 60) {
            heater_temp_.set_value(value, true);
            heater_ig_.update(value, 60);
        }
    } else if (device == "bath_heater") {
        if (action == "on") bath_heater_sw_.set(true);
        else if (action == "off") bath_heater_sw_.set(false);
    } else if (device == "mirror") {
        if (action == "on") mirror_light_sw_.set(true);
        else if (action == "off") mirror_light_sw_.set(false);
    } else if (device == "toilet") {
        if (action == "on") smart_toilet_sw_.set(true);
        else if (action == "off") smart_toilet_sw_.set(false);
    }
}

void Bathroom::get_json(std::string &out) const {
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":%d,\"fan\":%d,\"heater\":{\"on\":%d,\"temp\":%d},"
        "\"bath_heater\":%d,\"mirror\":%d,\"toilet\":%d}",
        light_sw_.get(), fan_sw_.get(),
        heater_sw_.get(), heater_temp_.value(),
        bath_heater_sw_.get(), mirror_light_sw_.get(), smart_toilet_sw_.get());
    out = buf;
}

void Bathroom::sync_to_http() {
    HttpState &s = HttpState::instance();
    s.bh.light_on = light_sw_.get();
    s.bh.fan_on = fan_sw_.get();
    s.bh.heater_on = heater_sw_.get();
    s.bh.heater_temp = heater_temp_.value();
    s.bh.bath_heater_on = bath_heater_sw_.get();
    s.bh.mirror_light_on = mirror_light_sw_.get();
    s.bh.smart_toilet_on = smart_toilet_sw_.get();
}

} // namespace smarthome
