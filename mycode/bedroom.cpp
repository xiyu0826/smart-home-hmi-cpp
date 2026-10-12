#include "bedroom.hpp"
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

void Bedroom::init() {
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xf0f2f5), 0);
    create_top_bar(screen, "卧室");

    lv_obj_t *cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 980, 520);
    lv_obj_set_pos(cont, 22, 60);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // ========== 卧室主灯 ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "卧室主灯", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_LIGHT, lv_color_hex(0xf1c40f), 61);

        light_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "亮度", POS_CTRL_LBL_X, 58);
        light_brightness_.create(card, POS_SLIDER_X, 66, 170, 10, POS_VAL_X, 56);
        light_ig_.create(card, POS_IND_X, 98, 22, 22, light_brightness_.value(), 100, 4,
                         lv_color_hex(0xf1c40f), lv_color_hex(0xdddddd), IMG_BULB);

        light_sw_.on_changed([this](bool on) {
            if (on) { if (light_brightness_.value() == 0) light_brightness_.set_value(80); }
            else light_brightness_.set_value(0);
            light_ig_.update(light_brightness_.value(), 100);
        });
        light_brightness_.on_changed([this](int v) {
            light_sw_.set(v > 0);
            light_ig_.update(v, 100);
        });
    }

    // ========== 空调 ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "空调", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_AC, lv_color_hex(0x3498db), 61);

        ac_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "温度", POS_CTRL_LBL_X, 58);
        ac_temp_.create(card, POS_SLIDER_X, 66, 170, 10, POS_VAL_X, 56);
        ac_ig_.create(card, POS_IND_X, 100, 18, 18, ac_temp_.value(), 30, 5,
                      lv_color_hex(0x3498db), lv_color_hex(0xdddddd), IMG_SNOWFLAKE);
        add_ctrl_label(card, "模式", 210, 100);
        ac_mode_.create(card, 255, 96, 110, 30);

        ac_temp_.on_changed([this](int v) { ac_ig_.update(v, 30); });
    }

    // ========== 窗帘 ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "窗帘", 460, CARD_H_DROPDOWN, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 20);
        SmartHome::instance().add_device_icon(card, IMG_CURTAIN, lv_color_hex(0x9b59b6), 41);
        add_ctrl_label(card, "状态", POS_CTRL_LBL_X, 65);
        curtain_.create(card, POS_DD_X, 60, 130, 30);
    }

    // ========== 灯带 ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "灯带", 460, CARD_H_DROPDOWN, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_STRIP, lv_color_hex(0xe91e63), 41);
        strip_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "颜色", POS_CTRL_LBL_X, 65);
        strip_color_.create(card, POS_DD_X, 60, 100, 30);
    }

    // ========== 智能音箱 ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "智能音箱", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_SPEAKER, lv_color_hex(0xe67e22), 61);

        speaker_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "音量", POS_CTRL_LBL_X, 58);
        speaker_vol_.create(card, POS_SLIDER_X, 66, 170, 10, POS_VAL_X, 56);
        speaker_ig_.create(card, POS_IND_X, 98, 18, 18, speaker_vol_.value(), 100, 5,
                           lv_color_hex(0xe67e22), lv_color_hex(0xdddddd), IMG_SPEAKER);

        speaker_vol_.on_changed([this](int v) { speaker_ig_.update(v, 100); });
    }

    // ========== 加湿器（开关 + 档位下拉） ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "加湿器", 460, CARD_H_SLIDER, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 12);
        SmartHome::instance().add_device_icon(card, IMG_HUMIDIFIER, lv_color_hex(0x00bcd4), 61);

        humidifier_sw_.create(card, POS_SWITCH_X, 12);
        add_ctrl_label(card, "档位", POS_CTRL_LBL_X, 80);
        humidifier_level_.create(card, POS_DD_X, 75, 100, 30);
    }

    // ========== 衣帽间灯（仅开关） ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "衣帽间灯", 460, CARD_H_SWITCH, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 35);
        SmartHome::instance().add_device_icon(card, IMG_CLOSET, lv_color_hex(0xff9800), 31);
        closet_sw_.create(card, POS_SWITCH_X, 33);
    }

    // ========== 香薰机（仅开关） ==========
    {
        lv_obj_t *t;
        lv_obj_t *card = SmartHome::instance().create_device_card(cont, "香薰机", 460, CARD_H_SWITCH, &t);
        lv_obj_set_pos(t, POS_TITLE_X, 35);
        SmartHome::instance().add_device_icon(card, IMG_AROMA, lv_color_hex(0x8bc34a), 31);
        aroma_sw_.create(card, POS_SWITCH_X, 33);
    }
}

void Bedroom::apply_scene(const SceneConfig &cfg) {
    light_sw_.set(cfg.br_light_on);
    light_brightness_.set_value(cfg.br_light_brightness, false);
    light_ig_.update(cfg.br_light_brightness, 100);

    ac_sw_.set(cfg.br_ac_on);
    ac_temp_.set_value(cfg.br_ac_temp, false);
    ac_ig_.update(cfg.br_ac_temp, 30);
    ac_mode_.set_selected(cfg.br_ac_mode);

    curtain_.set_selected(cfg.br_curtain_state);

    strip_sw_.set(cfg.br_strip_on);
    strip_color_.set_selected(cfg.br_strip_color);

    speaker_sw_.set(cfg.br_speaker_on);
    speaker_vol_.set_value(cfg.br_speaker_volume, false);
    speaker_ig_.update(cfg.br_speaker_volume, 100);

    humidifier_sw_.set(cfg.br_humidifier_on);
    humidifier_level_.set_selected(cfg.br_humidifier_level);

    closet_sw_.set(cfg.br_closet_light_on);
    aroma_sw_.set(cfg.br_aroma_on);
}

void Bedroom::web_control(const std::string &device, const std::string &action, int value) {
    if (device == "br_light") {
        if (action == "on") { light_sw_.set(true); light_brightness_.set_value(value > 0 ? value : 80, true); }
        else if (action == "off") { light_sw_.set(false); light_brightness_.set_value(0, true); }
        else if (value >= 0) { light_sw_.set(value > 0); light_brightness_.set_value(value, true); }
        light_ig_.update(light_brightness_.value(), 100);
    } else if (device == "br_ac") {
        if (action == "on") ac_sw_.set(true);
        else if (action == "off") ac_sw_.set(false);
        if (value >= 16 && value <= 30) { ac_temp_.set_value(value, true); ac_ig_.update(value, 30); }
        else if (value >= 0 && value <= 2) ac_mode_.set_selected(value);
    } else if (device == "br_curtain") {
        if (value >= 0 && value <= 3) curtain_.set_selected(value);
    } else if (device == "br_strip") {
        if (action == "on") strip_sw_.set(true);
        else if (action == "off") strip_sw_.set(false);
        if (value >= 0 && value <= 2) strip_color_.set_selected(value);
    } else if (device == "br_speaker") {
        if (action == "on") speaker_sw_.set(true);
        else if (action == "off") speaker_sw_.set(false);
        if (value >= 0 && value <= 100) { speaker_vol_.set_value(value, true); speaker_ig_.update(value, 100); }
    } else if (device == "humidifier") {
        if (action == "on") humidifier_sw_.set(true);
        else if (action == "off") humidifier_sw_.set(false);
        if (value >= 0 && value <= 2) humidifier_level_.set_selected(value);
    } else if (device == "closet") {
        if (action == "on") closet_sw_.set(true);
        else if (action == "off") closet_sw_.set(false);
    } else if (device == "aroma") {
        if (action == "on") aroma_sw_.set(true);
        else if (action == "off") aroma_sw_.set(false);
    }
}

void Bedroom::get_json(std::string &out) const {
    char buf[512];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":{\"on\":%d,\"brightness\":%d},"
        "\"ac\":{\"on\":%d,\"temp\":%d,\"mode\":%d},"
        "\"curtain\":%d,"
        "\"strip\":{\"on\":%d,\"color\":%d},"
        "\"speaker\":{\"on\":%d,\"volume\":%d},"
        "\"humidifier\":{\"on\":%d,\"level\":%d},"
        "\"closet\":%d,"
        "\"aroma\":%d}",
        light_sw_.get(), light_brightness_.value(),
        ac_sw_.get(), ac_temp_.value(), ac_mode_.selected(),
        curtain_.selected(),
        strip_sw_.get(), strip_color_.selected(),
        speaker_sw_.get(), speaker_vol_.value(),
        humidifier_sw_.get(), humidifier_level_.selected(),
        closet_sw_.get(), aroma_sw_.get());
    out = buf;
}

void Bedroom::sync_to_http() {
    HttpState &s = HttpState::instance();
    s.br.light_on = light_sw_.get();
    s.br.light_brightness = light_brightness_.value();
    s.br.ac_on = ac_sw_.get();
    s.br.ac_temp = ac_temp_.value();
    s.br.ac_mode = ac_mode_.selected();
    s.br.curtain_state = curtain_.selected();
    s.br.strip_on = strip_sw_.get();
    s.br.strip_color = strip_color_.selected();
    s.br.speaker_on = speaker_sw_.get();
    s.br.speaker_volume = speaker_vol_.value();
    s.br.humidifier_on = humidifier_sw_.get();
    s.br.humidifier_level = humidifier_level_.selected();
    s.br.closet_light_on = closet_sw_.get();
    s.br.aroma_on = aroma_sw_.get();
}

} // namespace smarthome
