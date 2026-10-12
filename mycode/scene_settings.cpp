#include "scene_settings.hpp"
#include "smart_home.hpp"
#include <cstdio>

namespace smarthome {

namespace {

/* ==================== 回调宏：直接写当前场景配置 ==================== */

// 注意：LVGL 9.0 的 lv_event_get_target() 返回 void*，C 语言能隐式转成 lv_obj_t*，
// C++ 不允许，所以这里取值前都加 (lv_obj_t *) 显式转换（与文件末尾那几处一致）。

#define MAKE_SLIDER_CB(name, field, fmt)                                        \
    static void name(lv_event_t *e) {                                           \
        int v = (int)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));   \
        SceneSettings::current_config().field = v;                              \
        lv_obj_t *lbl = (lv_obj_t *)lv_event_get_user_data(e);                  \
        if (lbl) { char buf[32]; snprintf(buf, sizeof(buf), fmt, v);            \
                   lv_label_set_text(lbl, buf); }                               \
    }

#define MAKE_DD_CB(name, field)                                                 \
    static void name(lv_event_t *e) {                                           \
        SceneSettings::current_config().field =                                 \
            (int)lv_dropdown_get_selected((lv_obj_t *)lv_event_get_target(e));  \
    }

#define MAKE_SW_CB(name, field)                                                 \
    static void name(lv_event_t *e) {                                           \
        SceneSettings::current_config().field =                                 \
            lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED); \
    }

// 客厅
MAKE_SLIDER_CB(s_lr_bright_cb, lr_light_brightness, "%d%%")
MAKE_SLIDER_CB(s_lr_ac_temp_cb, lr_ac_temp, "%d C")
MAKE_DD_CB(s_lr_ac_mode_cb, lr_ac_mode)
MAKE_DD_CB(s_lr_curtain_cb, lr_curtain_state)
MAKE_SLIDER_CB(s_lr_tv_vol_cb, lr_tv_volume, "%d")
MAKE_SLIDER_CB(s_lr_spk_vol_cb, lr_speaker_volume, "%d")
MAKE_DD_CB(s_lr_purifier_cb, lr_purifier_mode)
MAKE_DD_CB(s_lr_strip_cb, lr_strip_color)
MAKE_SW_CB(s_lr_light_sw_cb, lr_light_on)
MAKE_SW_CB(s_lr_ac_sw_cb, lr_ac_on)
MAKE_SW_CB(s_lr_strip_sw_cb, lr_strip_on)
MAKE_SW_CB(s_lr_tv_sw_cb, lr_tv_on)
MAKE_SW_CB(s_lr_robot_sw_cb, lr_robot_on)
MAKE_SW_CB(s_lr_purifier_sw_cb, lr_purifier_on)
MAKE_SW_CB(s_lr_spk_sw_cb, lr_speaker_on)

// 卧室
MAKE_SLIDER_CB(s_br_bright_cb, br_light_brightness, "%d%%")
MAKE_SLIDER_CB(s_br_ac_temp_cb, br_ac_temp, "%d C")
MAKE_DD_CB(s_br_ac_mode_cb, br_ac_mode)
MAKE_DD_CB(s_br_curtain_cb, br_curtain_state)
MAKE_SLIDER_CB(s_br_spk_vol_cb, br_speaker_volume, "%d")
MAKE_DD_CB(s_br_humidifier_cb, br_humidifier_level)
MAKE_DD_CB(s_br_strip_cb, br_strip_color)
MAKE_SW_CB(s_br_light_sw_cb, br_light_on)
MAKE_SW_CB(s_br_ac_sw_cb, br_ac_on)
MAKE_SW_CB(s_br_strip_sw_cb, br_strip_on)
MAKE_SW_CB(s_br_spk_sw_cb, br_speaker_on)
MAKE_SW_CB(s_br_humidifier_sw_cb, br_humidifier_on)
MAKE_SW_CB(s_br_closet_sw_cb, br_closet_light_on)
MAKE_SW_CB(s_br_aroma_sw_cb, br_aroma_on)

// 厨房
MAKE_SW_CB(s_kt_light_cb, kt_light_on)
MAKE_SW_CB(s_kt_outlet_cb, kt_outlet_on)
MAKE_SW_CB(s_kt_fan_cb, kt_fan_on)
MAKE_SW_CB(s_kt_fridge_cb, kt_fridge_on)
MAKE_SW_CB(s_kt_microwave_cb, kt_microwave_on)
MAKE_SW_CB(s_kt_dishwasher_cb, kt_dishwasher_on)
MAKE_SW_CB(s_kt_oven_cb, kt_oven_on)
MAKE_SW_CB(s_kt_purifier_water_cb, kt_purifier_water_on)

// 卫生间
MAKE_SLIDER_CB(s_bh_heater_temp_cb, bh_heater_temp, "%d C")
MAKE_SW_CB(s_bh_light_cb, bh_light_on)
MAKE_SW_CB(s_bh_fan_cb, bh_fan_on)
MAKE_SW_CB(s_bh_heater_cb, bh_heater_on)
MAKE_SW_CB(s_bh_bath_heater_cb, bh_bath_heater_on)
MAKE_SW_CB(s_bh_mirror_cb, bh_mirror_light_on)
MAKE_SW_CB(s_bh_toilet_cb, bh_smart_toilet_on)

#undef MAKE_SLIDER_CB
#undef MAKE_DD_CB
#undef MAKE_SW_CB

/* ==================== 设置页辅助行构造函数 ==================== */

static lv_obj_t *add_switch_row(lv_obj_t *parent, int y, const char *label,
                                bool on, lv_event_cb_t cb) {
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x444444), 0);
    lv_obj_set_pos(lbl, 15, y + 5);

    lv_obj_t *sw = lv_switch_create(parent);
    lv_obj_set_pos(sw, 170, y + 3);
    lv_obj_set_size(sw, 55, 28);
    if (on) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, cb, LV_EVENT_VALUE_CHANGED, nullptr);
    return sw;
}

static lv_obj_t *add_slider_row(lv_obj_t *parent, int y, const char *label,
                                int min, int max, int val,
                                lv_obj_t **val_label_out, lv_event_cb_t cb, const char *fmt) {
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x444444), 0);
    lv_obj_set_pos(lbl, 15, y + 4);

    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 170, 8);
    lv_obj_set_pos(slider, 170, y + 10);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    lv_obj_t *vlbl = lv_label_create(parent);
    lv_label_set_text_fmt(vlbl, fmt, val);
    lv_obj_set_style_text_font(vlbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(vlbl, lv_color_hex(0x555555), 0);
    lv_obj_set_pos(vlbl, 360, y + 4);
    if (val_label_out) *val_label_out = vlbl;

    lv_obj_add_event_cb(slider, cb, LV_EVENT_VALUE_CHANGED, vlbl); // label 作 user_data
    return slider;
}

static lv_obj_t *add_dropdown_row(lv_obj_t *parent, int y, const char *label,
                                  const char *options, int sel, lv_event_cb_t cb) {
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x444444), 0);
    lv_obj_set_pos(lbl, 15, y + 2);

    lv_obj_t *dd = lv_dropdown_create(parent);
    lv_obj_set_pos(dd, 170, y);
    lv_obj_set_size(dd, 120, 30);
    lv_dropdown_set_options(dd, options);
    lv_dropdown_set_selected(dd, sel);
    lv_obj_set_style_text_font(dd, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_pad_top(dd, 1, 0);
    lv_obj_set_style_pad_bottom(dd, 7, 0);
    lv_obj_set_style_pad_left(dd, 20, 0);
    lv_obj_set_style_pad_right(dd, 4, 0);
    lv_dropdown_set_symbol(dd, " ");
    lv_obj_add_event_cb(dd, cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(dd, [](lv_event_t *e) {
        lv_obj_t *list = lv_dropdown_get_list((lv_obj_t *)lv_event_get_target(e));
        if (list) lv_obj_set_style_text_font(list, SmartHome::instance().font_chinese(), 0);
    }, LV_EVENT_CLICKED, nullptr);
    return dd;
}

static void settings_back_cb(lv_event_t *) {
    lv_screen_load(SmartHome::instance().main_screen());
}

// 场景选择：切换场景时回填所有控件
static void scene_select_cb(lv_event_t *e) {
    int idx = (int)lv_dropdown_get_selected((lv_obj_t *)lv_event_get_target(e));
    if (idx < 0 || idx >= SceneManager::COUNT) return;
    SceneSettings::set_current_scene_idx(idx);
    SceneSettings::instance().load_config_into_controls();
}

} // anonymous namespace

// 把当前场景配置回填到所有控件（成员函数，可访问私有控件指针）
void SceneSettings::load_config_into_controls() {
    const SceneConfig &c = current_config();

    auto set_sw = [](lv_obj_t *sw, bool on) {
        if (!sw) return;
        if (on) lv_obj_add_state(sw, LV_STATE_CHECKED);
        else    lv_obj_remove_state(sw, LV_STATE_CHECKED);
    };
    auto set_slider = [](lv_obj_t *sl, lv_obj_t *lb, int val, const char *fmt) {
        if (!sl) return;
        lv_slider_set_value(sl, val, LV_ANIM_OFF);
        if (lb) { char buf[32]; snprintf(buf, sizeof(buf), fmt, val); lv_label_set_text(lb, buf); }
    };
    auto set_dd = [](lv_obj_t *dd, int val) {
        if (dd) lv_dropdown_set_selected(dd, val);
    };

    // 客厅
    set_sw(s_lr_light_sw_, c.lr_light_on);
    set_slider(s_lr_bright_slider_, s_lr_bright_label_, c.lr_light_brightness, "%d%%");
    set_sw(s_lr_ac_sw_, c.lr_ac_on);
    set_slider(s_lr_ac_temp_slider_, s_lr_ac_temp_label_, c.lr_ac_temp, "%d C");
    set_dd(s_lr_ac_mode_dd_, c.lr_ac_mode);
    set_dd(s_lr_curtain_dd_, c.lr_curtain_state);
    set_sw(s_lr_strip_sw_, c.lr_strip_on);
    set_dd(s_lr_strip_dd_, c.lr_strip_color);
    set_sw(s_lr_tv_sw_, c.lr_tv_on);
    set_slider(s_lr_tv_vol_slider_, s_lr_tv_vol_label_, c.lr_tv_volume, "%d");
    set_sw(s_lr_robot_sw_, c.lr_robot_on);
    set_sw(s_lr_purifier_sw_, c.lr_purifier_on);
    set_dd(s_lr_purifier_dd_, c.lr_purifier_mode);
    set_sw(s_lr_spk_sw_, c.lr_speaker_on);
    set_slider(s_lr_spk_vol_slider_, s_lr_spk_vol_label_, c.lr_speaker_volume, "%d");

    // 卧室
    set_sw(s_br_light_sw_, c.br_light_on);
    set_slider(s_br_bright_slider_, s_br_bright_label_, c.br_light_brightness, "%d%%");
    set_sw(s_br_ac_sw_, c.br_ac_on);
    set_slider(s_br_ac_temp_slider_, s_br_ac_temp_label_, c.br_ac_temp, "%d C");
    set_dd(s_br_ac_mode_dd_, c.br_ac_mode);
    set_dd(s_br_curtain_dd_, c.br_curtain_state);
    set_sw(s_br_strip_sw_, c.br_strip_on);
    set_dd(s_br_strip_dd_, c.br_strip_color);
    set_sw(s_br_spk_sw_, c.br_speaker_on);
    set_slider(s_br_spk_vol_slider_, s_br_spk_vol_label_, c.br_speaker_volume, "%d");
    set_sw(s_br_humidifier_sw_, c.br_humidifier_on);
    set_dd(s_br_humidifier_dd_, c.br_humidifier_level);
    set_sw(s_br_closet_sw_, c.br_closet_light_on);
    set_sw(s_br_aroma_sw_, c.br_aroma_on);

    // 厨房
    set_sw(s_kt_light_sw_, c.kt_light_on);
    set_sw(s_kt_outlet_sw_, c.kt_outlet_on);
    set_sw(s_kt_fan_sw_, c.kt_fan_on);
    set_sw(s_kt_fridge_sw_, c.kt_fridge_on);
    set_sw(s_kt_microwave_sw_, c.kt_microwave_on);
    set_sw(s_kt_dishwasher_sw_, c.kt_dishwasher_on);
    set_sw(s_kt_oven_sw_, c.kt_oven_on);
    set_sw(s_kt_purifier_sw_, c.kt_purifier_water_on);

    // 卫生间
    set_slider(s_bh_heater_temp_slider_, s_bh_heater_temp_label_, c.bh_heater_temp, "%d C");
    set_sw(s_bh_light_sw_, c.bh_light_on);
    set_sw(s_bh_fan_sw_, c.bh_fan_on);
    set_sw(s_bh_heater_sw_, c.bh_heater_on);
    set_sw(s_bh_bath_heater_sw_, c.bh_bath_heater_on);
    set_sw(s_bh_mirror_sw_, c.bh_mirror_light_on);
    set_sw(s_bh_toilet_sw_, c.bh_smart_toilet_on);
}

/* ==================== 打开设置页面 ==================== */

void SceneSettings::open() {
    if (settings_screen_) { lv_obj_delete(settings_screen_); settings_screen_ = nullptr; }

    settings_screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(settings_screen_, lv_color_hex(0xf0f2f5), 0);

    // 顶部栏
    lv_obj_t *bar = lv_obj_create(settings_screen_);
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
    lv_obj_add_event_cb(btn, settings_back_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *bl = lv_label_create(btn);
    lv_label_set_text(bl, "返回");
    lv_obj_set_style_text_font(bl, SmartHome::instance().font_chinese(), 0);
    lv_obj_center(bl);

    lv_obj_t *title = lv_label_create(bar);
    lv_label_set_text(title, "场景自定义设置");
    lv_obj_set_style_text_font(title, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_center(title);

    // 滚动区
    lv_obj_t *scroll = lv_obj_create(settings_screen_);
    lv_obj_set_size(scroll, 996, 536);
    lv_obj_set_pos(scroll, 14, 60);
    lv_obj_set_style_bg_opa(scroll, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scroll, 0, 0);
    lv_obj_set_scroll_dir(scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll, LV_SCROLLBAR_MODE_ON);

    lv_obj_t *inner = lv_obj_create(scroll);
    lv_obj_set_size(inner, 960, 1700);
    lv_obj_set_style_bg_opa(inner, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(inner, 0, 0);
    lv_obj_set_style_pad_all(inner, 0, 0);

    const SceneConfig &c = SceneManager::instance().config(0); // 默认睡眠

    // 场景选择下拉框
    int y = 5;
    lv_obj_t *l = lv_label_create(inner);
    lv_label_set_text(l, "选择场景：");
    lv_obj_set_style_text_font(l, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_pos(l, 5, y + 2);

    scene_dd_ = lv_dropdown_create(inner);
    lv_obj_set_pos(scene_dd_, 110, y);
    lv_obj_set_size(scene_dd_, 150, 34);
    lv_dropdown_set_options(scene_dd_, "睡眠模式\n影院模式\n离家模式\n回家模式\n一键断电");
    lv_dropdown_set_selected(scene_dd_, 0);
    lv_obj_set_style_text_font(scene_dd_, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_pad_top(scene_dd_, 1, 0);
    lv_obj_set_style_pad_bottom(scene_dd_, 7, 0);
    lv_obj_set_style_pad_left(scene_dd_, 23, 0);
    lv_obj_set_style_pad_right(scene_dd_, 4, 0);
    lv_dropdown_set_symbol(scene_dd_, " ");
    lv_obj_add_event_cb(scene_dd_, scene_select_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(scene_dd_, [](lv_event_t *e) {
        lv_obj_t *list = lv_dropdown_get_list((lv_obj_t *)lv_event_get_target(e));
        if (list) lv_obj_set_style_text_font(list, SmartHome::instance().font_chinese(), 0);
    }, LV_EVENT_CLICKED, nullptr);

    // 客厅卡片
    y = 45;
    lv_obj_t *lr_card = SmartHome::instance().create_device_card(inner, "客  厅", 940, 560, nullptr);
    lv_obj_set_pos(lr_card, 5, y);
    int ry = 40;
    s_lr_light_sw_       = add_switch_row(lr_card, ry, "主灯", c.lr_light_on, s_lr_light_sw_cb); ry += 32;
    s_lr_bright_slider_  = add_slider_row(lr_card, ry, "亮度", 0, 100, c.lr_light_brightness, &s_lr_bright_label_, s_lr_bright_cb, "%d%%"); ry += 36;
    s_lr_ac_sw_          = add_switch_row(lr_card, ry, "空调", c.lr_ac_on, s_lr_ac_sw_cb); ry += 32;
    s_lr_ac_temp_slider_ = add_slider_row(lr_card, ry, "温度", 16, 30, c.lr_ac_temp, &s_lr_ac_temp_label_, s_lr_ac_temp_cb, "%d C"); ry += 36;
    s_lr_ac_mode_dd_     = add_dropdown_row(lr_card, ry, "模式", "制冷\n制热\n送风", c.lr_ac_mode, s_lr_ac_mode_cb); ry += 34;
    s_lr_curtain_dd_     = add_dropdown_row(lr_card, ry, "窗帘", "全开\n关左\n关右\n全关", c.lr_curtain_state, s_lr_curtain_cb); ry += 34;
    s_lr_strip_sw_       = add_switch_row(lr_card, ry, "灯带", c.lr_strip_on, s_lr_strip_sw_cb); ry += 32;
    s_lr_strip_dd_       = add_dropdown_row(lr_card, ry, "颜色", "暖光\n白光\n彩光", c.lr_strip_color, s_lr_strip_cb); ry += 34;
    s_lr_tv_sw_          = add_switch_row(lr_card, ry, "电视", c.lr_tv_on, s_lr_tv_sw_cb); ry += 32;
    s_lr_tv_vol_slider_  = add_slider_row(lr_card, ry, "音量", 0, 100, c.lr_tv_volume, &s_lr_tv_vol_label_, s_lr_tv_vol_cb, "%d"); ry += 36;
    s_lr_robot_sw_       = add_switch_row(lr_card, ry, "扫地机器人", c.lr_robot_on, s_lr_robot_sw_cb); ry += 32;
    s_lr_purifier_sw_    = add_switch_row(lr_card, ry, "空气净化器", c.lr_purifier_on, s_lr_purifier_sw_cb); ry += 32;
    s_lr_purifier_dd_    = add_dropdown_row(lr_card, ry, "模式", "自动\n睡眠\n强力", c.lr_purifier_mode, s_lr_purifier_cb); ry += 34;
    s_lr_spk_sw_         = add_switch_row(lr_card, ry, "智能音箱", c.lr_speaker_on, s_lr_spk_sw_cb); ry += 32;
    s_lr_spk_vol_slider_ = add_slider_row(lr_card, ry, "音量", 0, 100, c.lr_speaker_volume, &s_lr_spk_vol_label_, s_lr_spk_vol_cb, "%d"); ry += 36;

    // 卧室卡片
    y = 620;
    lv_obj_t *br_card = SmartHome::instance().create_device_card(inner, "卧  室", 940, 480, nullptr);
    lv_obj_set_pos(br_card, 5, y);
    ry = 40;
    s_br_light_sw_      = add_switch_row(br_card, ry, "主灯", c.br_light_on, s_br_light_sw_cb); ry += 32;
    s_br_bright_slider_ = add_slider_row(br_card, ry, "亮度", 0, 100, c.br_light_brightness, &s_br_bright_label_, s_br_bright_cb, "%d%%"); ry += 36;
    s_br_ac_sw_         = add_switch_row(br_card, ry, "空调", c.br_ac_on, s_br_ac_sw_cb); ry += 32;
    s_br_ac_temp_slider_ = add_slider_row(br_card, ry, "温度", 16, 30, c.br_ac_temp, &s_br_ac_temp_label_, s_br_ac_temp_cb, "%d C"); ry += 36;
    s_br_ac_mode_dd_    = add_dropdown_row(br_card, ry, "模式", "制冷\n制热\n送风", c.br_ac_mode, s_br_ac_mode_cb); ry += 34;
    s_br_curtain_dd_    = add_dropdown_row(br_card, ry, "窗帘", "全开\n关左\n关右\n全关", c.br_curtain_state, s_br_curtain_cb); ry += 34;
    s_br_spk_sw_        = add_switch_row(br_card, ry, "智能音箱", c.br_speaker_on, s_br_spk_sw_cb); ry += 32;
    s_br_spk_vol_slider_ = add_slider_row(br_card, ry, "音量", 0, 100, c.br_speaker_volume, &s_br_spk_vol_label_, s_br_spk_vol_cb, "%d"); ry += 36;
    s_br_strip_sw_      = add_switch_row(br_card, ry, "灯带", c.br_strip_on, s_br_strip_sw_cb); ry += 32;
    s_br_strip_dd_      = add_dropdown_row(br_card, ry, "颜色", "暖光\n白光\n彩光", c.br_strip_color, s_br_strip_cb); ry += 34;
    s_br_humidifier_sw_ = add_switch_row(br_card, ry, "加湿器", c.br_humidifier_on, s_br_humidifier_sw_cb); ry += 32;
    s_br_humidifier_dd_ = add_dropdown_row(br_card, ry, "档位", "低\n中\n高", c.br_humidifier_level, s_br_humidifier_cb); ry += 34;
    s_br_closet_sw_     = add_switch_row(br_card, ry, "衣帽间灯", c.br_closet_light_on, s_br_closet_sw_cb); ry += 32;
    s_br_aroma_sw_      = add_switch_row(br_card, ry, "香薰机", c.br_aroma_on, s_br_aroma_sw_cb); ry += 32;

    // 厨房卡片（两列开关）
    y = 1115;
    lv_obj_t *kt_card = SmartHome::instance().create_device_card(inner, "厨  房", 940, 190, nullptr);
    lv_obj_set_pos(kt_card, 5, y);
    auto kt_sw = [&](int col, int row, const char *label, lv_obj_t *&sw_ptr, bool init_on, lv_event_cb_t cb) {
        lv_obj_t *lbl = lv_label_create(kt_card);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_font(lbl, SmartHome::instance().font_chinese(), 0);
        lv_obj_set_pos(lbl, 30 + col * 450, 40 + row * 40);
        sw_ptr = lv_switch_create(kt_card);
        lv_obj_set_pos(sw_ptr, 150 + col * 450, 38 + row * 40);
        lv_obj_set_size(sw_ptr, 55, 28);
        if (init_on) lv_obj_add_state(sw_ptr, LV_STATE_CHECKED);
        lv_obj_add_event_cb(sw_ptr, cb, LV_EVENT_VALUE_CHANGED, nullptr);
    };
    kt_sw(0, 0, "厨房灯", s_kt_light_sw_, c.kt_light_on, s_kt_light_cb);
    kt_sw(0, 1, "油烟机", s_kt_fan_sw_, c.kt_fan_on, s_kt_fan_cb);
    kt_sw(0, 2, "微波炉", s_kt_microwave_sw_, c.kt_microwave_on, s_kt_microwave_cb);
    kt_sw(0, 3, "烤箱", s_kt_oven_sw_, c.kt_oven_on, s_kt_oven_cb);
    kt_sw(1, 0, "制冰机", s_kt_outlet_sw_, c.kt_outlet_on, s_kt_outlet_cb);
    kt_sw(1, 1, "冰箱", s_kt_fridge_sw_, c.kt_fridge_on, s_kt_fridge_cb);
    kt_sw(1, 2, "洗碗机", s_kt_dishwasher_sw_, c.kt_dishwasher_on, s_kt_dishwasher_cb);
    kt_sw(1, 3, "净水器", s_kt_purifier_sw_, c.kt_purifier_water_on, s_kt_purifier_water_cb);

    // 卫生间卡片（滑块 + 两列开关）
    y = 1320;
    lv_obj_t *bh_card = SmartHome::instance().create_device_card(inner, "卫生间", 940, 210, nullptr);
    lv_obj_set_pos(bh_card, 5, y);
    int by = 30;
    s_bh_heater_temp_slider_ = add_slider_row(bh_card, by, "热水器水温", 30, 60, c.bh_heater_temp, &s_bh_heater_temp_label_, s_bh_heater_temp_cb, "%d C"); by += 40;
    auto bh_sw = [&](int col, int row, const char *label, lv_obj_t *&sw_ptr, bool init_on, lv_event_cb_t cb) {
        lv_obj_t *lbl = lv_label_create(bh_card);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_font(lbl, SmartHome::instance().font_chinese(), 0);
        lv_obj_set_pos(lbl, 30 + col * 450, by + row * 38);
        sw_ptr = lv_switch_create(bh_card);
        lv_obj_set_pos(sw_ptr, 150 + col * 450, by - 2 + row * 38);
        lv_obj_set_size(sw_ptr, 55, 28);
        if (init_on) lv_obj_add_state(sw_ptr, LV_STATE_CHECKED);
        lv_obj_add_event_cb(sw_ptr, cb, LV_EVENT_VALUE_CHANGED, nullptr);
    };
    bh_sw(0, 0, "卫生间灯", s_bh_light_sw_, c.bh_light_on, s_bh_light_cb);
    bh_sw(0, 1, "排风扇", s_bh_fan_sw_, c.bh_fan_on, s_bh_fan_cb);
    bh_sw(0, 2, "热水器", s_bh_heater_sw_, c.bh_heater_on, s_bh_heater_cb);
    bh_sw(1, 0, "浴霸", s_bh_bath_heater_sw_, c.bh_bath_heater_on, s_bh_bath_heater_cb);
    bh_sw(1, 1, "镜前灯", s_bh_mirror_sw_, c.bh_mirror_light_on, s_bh_mirror_cb);
    bh_sw(1, 2, "智能马桶", s_bh_toilet_sw_, c.bh_smart_toilet_on, s_bh_toilet_cb);

    // 底部提示
    lv_obj_t *hint = lv_label_create(inner);
    lv_label_set_text(hint, "修改即时生效，返回即保存");
    lv_obj_set_style_text_font(hint, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x999999), 0);
    lv_obj_set_pos(hint, 10, 1550);

    lv_screen_load(settings_screen_);
}

} // namespace smarthome
