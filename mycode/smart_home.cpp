#include "smart_home.hpp"
#include "livingroom.hpp"
#include "bedroom.hpp"
#include "kitchen.hpp"
#include "bathroom.hpp"
#include "scene_manager.hpp"
#include "scene_settings.hpp"
#include "../lvgl/src/libs/freetype/lv_freetype.h"
#include <cstdlib>
#include <ctime>

namespace smarthome {

/* ==================== 内部辅助 ==================== */

// 应用场景到全部 4 个房间（唯一数据源：SceneManager）
static void apply_scene_all(int scene, const char *msg) {
    const SceneConfig &cfg = SceneManager::instance().config(scene);
    LivingRoom::instance().apply_scene(cfg);
    Bedroom::instance().apply_scene(cfg);
    Kitchen::instance().apply_scene(cfg);
    Bathroom::instance().apply_scene(cfg);
    SmartHome::instance().show_toast(msg);
}

// 创建房间卡片按钮
static lv_obj_t *create_room_btn(lv_obj_t *parent, const char *name, int color, lv_event_cb_t cb) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 220, 260);
    lv_obj_set_style_radius(card, 14, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, 10, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0xaaaaaa), 0);

    lv_obj_t *name_label = lv_label_create(card);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_font(name_label, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xffffff), 0);
    lv_obj_align(name_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, cb, LV_EVENT_CLICKED, nullptr);
    return card;
}

// 创建场景按钮
static lv_obj_t *create_scene_btn(lv_obj_t *parent, const char *name, int bg_color, lv_event_cb_t cb) {
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 110, 46);
    lv_obj_set_style_radius(btn, 23, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color), 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_font(label, SmartHome::instance().font_chinese(), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    return btn;
}

/* ==================== UI 工具函数 ==================== */

lv_obj_t *SmartHome::create_device_card(lv_obj_t *parent, const char *title,
                                        int w, int h, lv_obj_t **out_title) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_pad_all(card, 12, 0);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xd0d0d0), 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xfafafa), 0);

    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, font_chinese_, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), 0);
    lv_obj_set_pos(title_label, 15, 8);

    if (out_title) *out_title = title_label;
    return card;
}

lv_obj_t *SmartHome::add_device_icon(lv_obj_t *card, const char *img_path,
                                     lv_color_t placeholder_color, int y) {
    lv_obj_t *bg = lv_obj_create(card);
    lv_obj_set_size(bg, ICON_W, ICON_H);
    lv_obj_set_pos(bg, POS_ICON_X, y);
    lv_obj_set_style_bg_color(bg, placeholder_color, 0);
    lv_obj_set_style_radius(bg, 12, 0);
    lv_obj_set_style_border_width(bg, 2, 0);
    lv_obj_set_style_border_color(bg, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(bg, LV_OPA_30, 0);
    lv_obj_set_scrollbar_mode(bg, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *hi = lv_obj_create(bg);
    lv_obj_set_size(hi, 14, 14);
    lv_obj_set_style_bg_color(hi, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(hi, LV_OPA_50, 0);
    lv_obj_set_style_radius(hi, 7, 0);
    lv_obj_set_style_border_width(hi, 0, 0);
    lv_obj_center(hi);

    lv_obj_t *icon = lv_image_create(card);
    lv_image_set_src(icon, img_path);
    lv_obj_set_pos(icon, POS_ICON_X, y);
    lv_obj_set_size(icon, ICON_W, ICON_H);
    lv_obj_set_scrollbar_mode(icon, LV_SCROLLBAR_MODE_OFF);

    return bg;
}

void SmartHome::show_toast(const char *msg) {
    if (toast_timer_) { lv_timer_del(toast_timer_); toast_timer_ = nullptr; }
    if (toast_obj_)   { lv_obj_delete(toast_obj_); toast_obj_ = nullptr; }

    toast_obj_ = lv_obj_create(lv_layer_top());
    lv_obj_set_size(toast_obj_, 240, 52);
    lv_obj_set_style_bg_color(toast_obj_, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(toast_obj_, LV_OPA_90, 0);
    lv_obj_set_style_radius(toast_obj_, 10, 0);
    lv_obj_set_style_border_width(toast_obj_, 0, 0);
    lv_obj_set_scrollbar_mode(toast_obj_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(toast_obj_, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t *label = lv_label_create(toast_obj_);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_font(label, font_chinese_, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_center(label);

    toast_timer_ = lv_timer_create(toast_delete_cb, 1500, nullptr);
}

void SmartHome::get_sensors(int *lr_t, int *lr_h, int *br_t, int *br_h) const {
    *lr_t = sensor_st_; *lr_h = sensor_sh_; *br_t = sensor_bt_; *br_h = sensor_bh_;
}

/* ==================== 静态回调 ==================== */

void SmartHome::clock_timer_cb(lv_timer_t *) {
    SmartHome &self = instance();
    std::time_t now = std::time(nullptr);
    struct tm *tm_info = std::localtime(&now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M", tm_info);
    lv_label_set_text(self.clock_label_, buf);
}

void SmartHome::sensor_timer_cb(lv_timer_t *) {
    SmartHome &self = instance();
    self.sensor_st_ += (rand() % 3) - 1; if (self.sensor_st_ < 20) self.sensor_st_ = 20; if (self.sensor_st_ > 32) self.sensor_st_ = 32;
    self.sensor_sh_ += (rand() % 5) - 2; if (self.sensor_sh_ < 30) self.sensor_sh_ = 30; if (self.sensor_sh_ > 80) self.sensor_sh_ = 80;
    self.sensor_bt_ += (rand() % 3) - 1; if (self.sensor_bt_ < 18) self.sensor_bt_ = 18; if (self.sensor_bt_ > 30) self.sensor_bt_ = 30;
    self.sensor_bh_ += (rand() % 5) - 2; if (self.sensor_bh_ < 30) self.sensor_bh_ = 30; if (self.sensor_bh_ > 80) self.sensor_bh_ = 80;
    lv_label_set_text_fmt(self.lr_temp_l_, "%d C", self.sensor_st_);
    lv_label_set_text_fmt(self.lr_humi_l_, "%d%%", self.sensor_sh_);
    lv_label_set_text_fmt(self.br_temp_l_, "%d C", self.sensor_bt_);
    lv_label_set_text_fmt(self.br_humi_l_, "%d%%", self.sensor_bh_);
}

void SmartHome::toast_delete_cb(lv_timer_t *) {
    SmartHome &self = instance();
    if (self.toast_obj_) { lv_obj_delete(self.toast_obj_); self.toast_obj_ = nullptr; }
    self.toast_timer_ = nullptr;
}

void SmartHome::exit_app_cb(lv_event_t *) {
    instance().show_toast("正在退出...");
    instance().set_running(false);
}

void SmartHome::open_scene_settings_cb(lv_event_t *) {
    SceneSettings::instance().open();
}

void SmartHome::scene_sleep_cb(lv_event_t *)     { apply_scene_all(SCENE_SLEEP, "睡眠模式已启动"); }
void SmartHome::scene_movie_cb(lv_event_t *)     { apply_scene_all(SCENE_MOVIE, "影院模式已启动"); }
void SmartHome::scene_away_cb(lv_event_t *)      { apply_scene_all(SCENE_AWAY, "离家模式已启动"); }
void SmartHome::scene_home_cb(lv_event_t *)      { apply_scene_all(SCENE_HOME, "回家模式已启动"); }
void SmartHome::scene_power_off_cb(lv_event_t *) { apply_scene_all(SCENE_POWER_OFF, "已全部断电"); }

void SmartHome::goto_livingroom_cb(lv_event_t *) {
    LivingRoom::instance().ensure_init();
    lv_screen_load(LivingRoom::instance().screen);
}
void SmartHome::goto_bedroom_cb(lv_event_t *) {
    Bedroom::instance().ensure_init();
    lv_screen_load(Bedroom::instance().screen);
}
void SmartHome::goto_kitchen_cb(lv_event_t *) {
    Kitchen::instance().ensure_init();
    lv_screen_load(Kitchen::instance().screen);
}
void SmartHome::goto_bathroom_cb(lv_event_t *) {
    Bathroom::instance().ensure_init();
    lv_screen_load(Bathroom::instance().screen);
}

/* ==================== 主界面初始化 ==================== */

void SmartHome::init() {
    // 1. 加载 FreeType 中文字体
    font_chinese_ = lv_freetype_font_create(FONT_PATH, LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                            FONT_SIZE, LV_FREETYPE_FONT_STYLE_NORMAL);
    if (font_chinese_ == nullptr) {
        LV_LOG_ERROR("FreeType font create failed: %s", FONT_PATH);
        font_chinese_ = (lv_font_t *)&lv_font_montserrat_20; // 回退，防崩溃
    }

    // 2. 初始化场景默认配置
    SceneManager::instance().init();

    // 3. 创建主屏幕
    main_screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(main_screen_, lv_color_hex(0xf0f2f5), 0);

    // 4. 顶部状态栏
    lv_obj_t *top_bar = lv_obj_create(main_screen_);
    lv_obj_set_size(top_bar, 1024, 56);
    lv_obj_set_style_radius(top_bar, 0, 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_pad_all(top_bar, 0, 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x2c3e50), 0);
    lv_obj_set_pos(top_bar, 0, 0);

    lv_obj_t *title = lv_label_create(top_bar);
    lv_label_set_text(title, "智能家居控制中心");
    lv_obj_set_style_text_font(title, font_chinese_, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    clock_label_ = lv_label_create(top_bar);
    lv_label_set_text(clock_label_, "--:--");
    lv_obj_set_style_text_font(clock_label_, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(clock_label_, lv_color_hex(0xbdc3c7), 0);
    lv_obj_align(clock_label_, LV_ALIGN_RIGHT_MID, -100, 0);
    lv_timer_create(clock_timer_cb, 1000, nullptr);

    lv_obj_t *exit_btn = lv_button_create(top_bar);
    lv_obj_set_size(exit_btn, 60, 32);
    lv_obj_set_style_radius(exit_btn, 6, 0);
    lv_obj_set_style_bg_color(exit_btn, lv_color_hex(0xc0392b), 0);
    lv_obj_set_style_border_width(exit_btn, 0, 0);
    lv_obj_align(exit_btn, LV_ALIGN_RIGHT_MID, -15, 0);
    lv_obj_add_event_cb(exit_btn, exit_app_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *exit_label = lv_label_create(exit_btn);
    lv_label_set_text(exit_label, "退出");
    lv_obj_set_style_text_font(exit_label, font_chinese_, 0);
    lv_obj_set_style_text_color(exit_label, lv_color_hex(0xffffff), 0);
    lv_obj_center(exit_label);

    // 5. 环境仪表盘
    lv_obj_t *dash = lv_obj_create(main_screen_);
    lv_obj_set_size(dash, 980, 52);
    lv_obj_set_pos(dash, 22, 62);
    lv_obj_set_style_bg_opa(dash, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dash, 0, 0);
    lv_obj_set_style_pad_all(dash, 0, 0);
    lv_obj_set_flex_flow(dash, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dash, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    auto make_dash = [&](const char *room, lv_color_t bg, lv_color_t title_color,
                         lv_obj_t **temp_out, lv_obj_t **humi_out, const char *temp_txt, const char *humi_txt) {
        lv_obj_t *d = lv_obj_create(dash);
        lv_obj_set_size(d, 300, 46);
        lv_obj_set_style_radius(d, 8, 0);
        lv_obj_set_style_bg_color(d, bg, 0);
        lv_obj_set_style_border_width(d, 0, 0);
        lv_obj_set_style_pad_all(d, 0, 0);
        { lv_obj_t *l = lv_label_create(d); lv_label_set_text(l, room);
          lv_obj_set_style_text_font(l, font_chinese_, 0); lv_obj_set_style_text_color(l, title_color, 0);
          lv_obj_align(l, LV_ALIGN_LEFT_MID, 10, 0); }
        { lv_obj_t *l = lv_label_create(d); lv_label_set_text(l, "温");
          lv_obj_set_style_text_font(l, font_chinese_, 0); lv_obj_set_style_text_color(l, lv_color_hex(0x555555), 0);
          lv_obj_align(l, LV_ALIGN_CENTER, -50, 0); }
        *temp_out = lv_label_create(d);
        lv_label_set_text(*temp_out, temp_txt);
        lv_obj_set_style_text_font(*temp_out, &lv_font_montserrat_22, 0);
        lv_obj_set_style_text_color(*temp_out, lv_color_hex(0xd84315), 0);
        lv_obj_align(*temp_out, LV_ALIGN_CENTER, -10, 0);
        { lv_obj_t *l = lv_label_create(d); lv_label_set_text(l, "湿");
          lv_obj_set_style_text_font(l, font_chinese_, 0); lv_obj_set_style_text_color(l, lv_color_hex(0x555555), 0);
          lv_obj_align(l, LV_ALIGN_CENTER, 35, 0); }
        *humi_out = lv_label_create(d);
        lv_label_set_text(*humi_out, humi_txt);
        lv_obj_set_style_text_font(*humi_out, &lv_font_montserrat_22, 0);
        lv_obj_set_style_text_color(*humi_out, lv_color_hex(0x1565c0), 0);
        lv_obj_align(*humi_out, LV_ALIGN_CENTER, 70, 0);
    };
    make_dash("客厅", lv_color_hex(0xe8f5e9), lv_color_hex(0x2e7d32), &lr_temp_l_, &lr_humi_l_, "26 C", "55%");
    make_dash("卧室", lv_color_hex(0xede7f6), lv_color_hex(0x4527a0), &br_temp_l_, &br_humi_l_, "24 C", "50%");
    lv_timer_create(sensor_timer_cb, 3000, nullptr);

    // 6. 房间选择区域
    lv_obj_t *room_cont = lv_obj_create(main_screen_);
    lv_obj_set_size(room_cont, 980, 280);
    lv_obj_set_pos(room_cont, 22, 120);
    lv_obj_set_style_bg_opa(room_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(room_cont, 0, 0);
    lv_obj_set_style_pad_all(room_cont, 0, 0);
    lv_obj_set_flex_flow(room_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(room_cont, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    create_room_btn(room_cont, "客  厅", 0x3498db, goto_livingroom_cb);
    create_room_btn(room_cont, "卧  室", 0x9b59b6, goto_bedroom_cb);
    create_room_btn(room_cont, "厨  房", 0xe67e22, goto_kitchen_cb);
    create_room_btn(room_cont, "卫生间", 0x1abc9c, goto_bathroom_cb);

    // 7. 场景按钮区域
    lv_obj_t *scene_cont = lv_obj_create(main_screen_);
    lv_obj_set_size(scene_cont, 980, 70);
    lv_obj_set_pos(scene_cont, 22, 430);
    lv_obj_set_style_bg_opa(scene_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scene_cont, 0, 0);
    lv_obj_set_style_pad_all(scene_cont, 0, 0);
    lv_obj_set_flex_flow(scene_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scene_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *scene_tip = lv_label_create(scene_cont);
    lv_label_set_text(scene_tip, "一键场景");
    lv_obj_set_style_text_font(scene_tip, font_chinese_, 0);
    lv_obj_set_style_text_color(scene_tip, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_right(scene_tip, 15, 0);

    create_scene_btn(scene_cont, "睡眠", 0x34495e, scene_sleep_cb);
    create_scene_btn(scene_cont, "影院", 0x8e44ad, scene_movie_cb);
    create_scene_btn(scene_cont, "离家", 0x95a5a6, scene_away_cb);
    create_scene_btn(scene_cont, "回家", 0x27ae60, scene_home_cb);
    create_scene_btn(scene_cont, "断电", 0xc0392b, scene_power_off_cb);
    create_scene_btn(scene_cont, "设置", 0xf39c12, open_scene_settings_cb);

    // 8. 底部提示
    lv_obj_t *hint = lv_label_create(main_screen_);
    lv_label_set_text(hint, "点击房间卡片进入");
    lv_obj_set_style_text_font(hint, font_chinese_, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x999999), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_screen_load(main_screen_);
}

} // namespace smarthome
