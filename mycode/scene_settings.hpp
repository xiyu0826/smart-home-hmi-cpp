#pragma once
// scene_settings.hpp —— 场景自定义设置页面
//
// 可视化配置 5 个场景下每个设备的预设状态。所有编辑直接写入
// SceneManager::instance().config(scene)，与"一键场景"按钮共用同一份数据。

#include "../lvgl/lvgl.h"
#include "scene_manager.hpp"

namespace smarthome {

class SceneSettings {
public:
    static SceneSettings &instance() {
        static SceneSettings s;
        return s;
    }

    // 打开（或重建）场景设置页面
    void open();

    // 把当前场景配置回填到所有控件
    // （场景切换回调是本文件里的自由函数，访问不到 private，所以放在 public）
    void load_config_into_controls();

    // 当前正在编辑的场景索引（回调宏通过它定位 SceneConfig）
    static int current_scene_idx() { return instance().current_scene_idx_; }
    static void set_current_scene_idx(int i) { instance().current_scene_idx_ = i; }
    // 当前场景配置的便捷引用
    static SceneConfig &current_config() {
        return SceneManager::instance().config(instance().current_scene_idx_);
    }

private:
    SceneSettings() = default;
    SceneSettings(const SceneSettings &) = delete;
    SceneSettings &operator=(const SceneSettings &) = delete;

    lv_obj_t *settings_screen_ = nullptr;
    int current_scene_idx_ = 0;
    lv_obj_t *scene_dd_ = nullptr;

    // 客厅控件
    lv_obj_t *s_lr_light_sw_ = nullptr;
    lv_obj_t *s_lr_bright_slider_ = nullptr, *s_lr_bright_label_ = nullptr;
    lv_obj_t *s_lr_ac_sw_ = nullptr;
    lv_obj_t *s_lr_ac_temp_slider_ = nullptr, *s_lr_ac_temp_label_ = nullptr;
    lv_obj_t *s_lr_ac_mode_dd_ = nullptr;
    lv_obj_t *s_lr_curtain_dd_ = nullptr;
    lv_obj_t *s_lr_strip_sw_ = nullptr, *s_lr_strip_dd_ = nullptr;
    lv_obj_t *s_lr_tv_sw_ = nullptr;
    lv_obj_t *s_lr_tv_vol_slider_ = nullptr, *s_lr_tv_vol_label_ = nullptr;
    lv_obj_t *s_lr_robot_sw_ = nullptr;
    lv_obj_t *s_lr_purifier_sw_ = nullptr, *s_lr_purifier_dd_ = nullptr;
    lv_obj_t *s_lr_spk_sw_ = nullptr;
    lv_obj_t *s_lr_spk_vol_slider_ = nullptr, *s_lr_spk_vol_label_ = nullptr;

    // 卧室控件
    lv_obj_t *s_br_light_sw_ = nullptr;
    lv_obj_t *s_br_bright_slider_ = nullptr, *s_br_bright_label_ = nullptr;
    lv_obj_t *s_br_ac_sw_ = nullptr;
    lv_obj_t *s_br_ac_temp_slider_ = nullptr, *s_br_ac_temp_label_ = nullptr;
    lv_obj_t *s_br_ac_mode_dd_ = nullptr;
    lv_obj_t *s_br_curtain_dd_ = nullptr;
    lv_obj_t *s_br_strip_sw_ = nullptr, *s_br_strip_dd_ = nullptr;
    lv_obj_t *s_br_spk_sw_ = nullptr;
    lv_obj_t *s_br_spk_vol_slider_ = nullptr, *s_br_spk_vol_label_ = nullptr;
    lv_obj_t *s_br_humidifier_sw_ = nullptr, *s_br_humidifier_dd_ = nullptr;
    lv_obj_t *s_br_closet_sw_ = nullptr;
    lv_obj_t *s_br_aroma_sw_ = nullptr;

    // 厨房控件
    lv_obj_t *s_kt_light_sw_ = nullptr, *s_kt_outlet_sw_ = nullptr;
    lv_obj_t *s_kt_fan_sw_ = nullptr, *s_kt_fridge_sw_ = nullptr;
    lv_obj_t *s_kt_microwave_sw_ = nullptr, *s_kt_dishwasher_sw_ = nullptr;
    lv_obj_t *s_kt_oven_sw_ = nullptr, *s_kt_purifier_sw_ = nullptr;

    // 卫生间控件
    lv_obj_t *s_bh_light_sw_ = nullptr, *s_bh_fan_sw_ = nullptr;
    lv_obj_t *s_bh_heater_sw_ = nullptr;
    lv_obj_t *s_bh_heater_temp_slider_ = nullptr, *s_bh_heater_temp_label_ = nullptr;
    lv_obj_t *s_bh_bath_heater_sw_ = nullptr;
    lv_obj_t *s_bh_mirror_sw_ = nullptr, *s_bh_toilet_sw_ = nullptr;
};

} // namespace smarthome
