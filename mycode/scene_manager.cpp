#include "scene_manager.hpp"

namespace smarthome {

// 5 个场景的默认设备预设（字段含义见 scene_config.hpp）。
// 关键差异：睡眠开卧室空调 26° + 加湿器 + 热水器 42°；影院开客厅灯 25% + 电视 + 音箱；
// 离家仅冰箱开；回家开客厅灯 80% + 空调 23° + 窗帘全开 + 厨房多数开 + 热水器 42°；断电全关（含冰箱）。
void SceneManager::init() {
    // ---- 场景 0：睡眠模式 ----
    {
        SceneConfig &c = configs_[SCENE_SLEEP];
        c.lr_light_on = false;        c.lr_light_brightness = 0;
        c.lr_ac_on = false;           c.lr_ac_temp = 22; c.lr_ac_mode = 0;
        c.lr_curtain_state = 3;
        c.lr_strip_on = false;        c.lr_strip_color = 0;
        c.lr_tv_on = false;           c.lr_tv_volume = 50;
        c.lr_robot_on = false;
        c.lr_purifier_on = true;      c.lr_purifier_mode = 0;
        c.lr_speaker_on = false;      c.lr_speaker_volume = 40;
        c.br_light_on = false;        c.br_light_brightness = 0;
        c.br_ac_on = true;            c.br_ac_temp = 26; c.br_ac_mode = 0;
        c.br_curtain_state = 3;
        c.br_strip_on = false;        c.br_strip_color = 0;
        c.br_speaker_on = false;      c.br_speaker_volume = 40;
        c.br_humidifier_on = true;    c.br_humidifier_level = 0;
        c.br_closet_light_on = false; c.br_aroma_on = false;
        c.kt_light_on = false;        c.kt_outlet_on = true; c.kt_fan_on = false;
        c.kt_fridge_on = true;        c.kt_microwave_on = false; c.kt_dishwasher_on = false;
        c.kt_oven_on = false;         c.kt_purifier_water_on = true;
        c.bh_light_on = false;        c.bh_fan_on = false;
        c.bh_heater_on = true;        c.bh_heater_temp = 42;
        c.bh_bath_heater_on = false;  c.bh_mirror_light_on = false; c.bh_smart_toilet_on = false;
    }

    // ---- 场景 1：影院模式 ----
    {
        SceneConfig &c = configs_[SCENE_MOVIE];
        c.lr_light_on = true;         c.lr_light_brightness = 25;
        c.lr_ac_on = true;            c.lr_ac_temp = 24; c.lr_ac_mode = 0;
        c.lr_curtain_state = 3;
        c.lr_strip_on = false;        c.lr_strip_color = 0;
        c.lr_tv_on = true;            c.lr_tv_volume = 45;
        c.lr_robot_on = false;
        c.lr_purifier_on = false;     c.lr_purifier_mode = 0;
        c.lr_speaker_on = true;       c.lr_speaker_volume = 40;
        c.br_light_on = false;        c.br_light_brightness = 0;
        c.br_ac_on = false;           c.br_ac_temp = 24; c.br_ac_mode = 0;
        c.br_curtain_state = 3;
        c.br_strip_on = false;        c.br_strip_color = 0;
        c.br_speaker_on = false;      c.br_speaker_volume = 40;
        c.br_humidifier_on = false;   c.br_humidifier_level = 0;
        c.br_closet_light_on = false; c.br_aroma_on = false;
        c.kt_light_on = false;        c.kt_outlet_on = false; c.kt_fan_on = false;
        c.kt_fridge_on = true;        c.kt_microwave_on = false; c.kt_dishwasher_on = false;
        c.kt_oven_on = false;         c.kt_purifier_water_on = true;
        c.bh_light_on = false;        c.bh_fan_on = false;
        c.bh_heater_on = false;       c.bh_heater_temp = 40;
        c.bh_bath_heater_on = false;  c.bh_mirror_light_on = false; c.bh_smart_toilet_on = false;
    }

    // ---- 场景 2：离家模式 ----
    {
        SceneConfig &c = configs_[SCENE_AWAY];
        c.lr_light_on = false;        c.lr_light_brightness = 0;
        c.lr_ac_on = false;           c.lr_ac_temp = 25; c.lr_ac_mode = 0;
        c.lr_curtain_state = 3;
        c.lr_strip_on = false;        c.lr_strip_color = 0;
        c.lr_tv_on = false;           c.lr_tv_volume = 50;
        c.lr_robot_on = false;
        c.lr_purifier_on = false;     c.lr_purifier_mode = 0;
        c.lr_speaker_on = false;      c.lr_speaker_volume = 40;
        c.br_light_on = false;        c.br_light_brightness = 0;
        c.br_ac_on = false;           c.br_ac_temp = 26; c.br_ac_mode = 0;
        c.br_curtain_state = 3;
        c.br_strip_on = false;        c.br_strip_color = 0;
        c.br_speaker_on = false;      c.br_speaker_volume = 40;
        c.br_humidifier_on = false;   c.br_humidifier_level = 0;
        c.br_closet_light_on = false; c.br_aroma_on = false;
        c.kt_light_on = false;        c.kt_outlet_on = false; c.kt_fan_on = false;
        c.kt_fridge_on = true;        c.kt_microwave_on = false; c.kt_dishwasher_on = false;
        c.kt_oven_on = false;         c.kt_purifier_water_on = false;
        c.bh_light_on = false;        c.bh_fan_on = false;
        c.bh_heater_on = false;       c.bh_heater_temp = 40;
        c.bh_bath_heater_on = false;  c.bh_mirror_light_on = false; c.bh_smart_toilet_on = false;
    }

    // ---- 场景 3：回家模式 ----
    {
        SceneConfig &c = configs_[SCENE_HOME];
        c.lr_light_on = true;         c.lr_light_brightness = 80;
        c.lr_ac_on = true;            c.lr_ac_temp = 23; c.lr_ac_mode = 0;
        c.lr_curtain_state = 0;
        c.lr_strip_on = true;         c.lr_strip_color = 0;
        c.lr_tv_on = false;           c.lr_tv_volume = 50;
        c.lr_robot_on = false;
        c.lr_purifier_on = true;      c.lr_purifier_mode = 0;
        c.lr_speaker_on = false;      c.lr_speaker_volume = 40;
        c.br_light_on = false;        c.br_light_brightness = 0;
        c.br_ac_on = false;           c.br_ac_temp = 24; c.br_ac_mode = 0;
        c.br_curtain_state = 3;
        c.br_strip_on = false;        c.br_strip_color = 0;
        c.br_speaker_on = false;      c.br_speaker_volume = 40;
        c.br_humidifier_on = false;   c.br_humidifier_level = 0;
        c.br_closet_light_on = false; c.br_aroma_on = false;
        c.kt_light_on = true;         c.kt_outlet_on = true; c.kt_fan_on = false;
        c.kt_fridge_on = true;        c.kt_microwave_on = false; c.kt_dishwasher_on = false;
        c.kt_oven_on = false;         c.kt_purifier_water_on = true;
        c.bh_light_on = false;        c.bh_fan_on = false;
        c.bh_heater_on = true;        c.bh_heater_temp = 42;
        c.bh_bath_heater_on = false;  c.bh_mirror_light_on = false; c.bh_smart_toilet_on = false;
    }

    // ---- 场景 4：断电模式（全关，含冰箱） ----
    {
        SceneConfig &c = configs_[SCENE_POWER_OFF];
        c.lr_light_on = false;        c.lr_light_brightness = 0;
        c.lr_ac_on = false;           c.lr_ac_temp = 22; c.lr_ac_mode = 0;
        c.lr_curtain_state = 3;
        c.lr_strip_on = false;        c.lr_strip_color = 0;
        c.lr_tv_on = false;           c.lr_tv_volume = 50;
        c.lr_robot_on = false;
        c.lr_purifier_on = false;     c.lr_purifier_mode = 0;
        c.lr_speaker_on = false;      c.lr_speaker_volume = 40;
        c.br_light_on = false;        c.br_light_brightness = 0;
        c.br_ac_on = false;           c.br_ac_temp = 24; c.br_ac_mode = 0;
        c.br_curtain_state = 3;
        c.br_strip_on = false;        c.br_strip_color = 0;
        c.br_speaker_on = false;      c.br_speaker_volume = 40;
        c.br_humidifier_on = false;   c.br_humidifier_level = 0;
        c.br_closet_light_on = false; c.br_aroma_on = false;
        c.kt_light_on = false;        c.kt_outlet_on = false; c.kt_fan_on = false;
        c.kt_fridge_on = false;       c.kt_microwave_on = false; c.kt_dishwasher_on = false;
        c.kt_oven_on = false;         c.kt_purifier_water_on = false;
        c.bh_light_on = false;        c.bh_fan_on = false;
        c.bh_heater_on = false;       c.bh_heater_temp = 40;
        c.bh_bath_heater_on = false;  c.bh_mirror_light_on = false; c.bh_smart_toilet_on = false;
    }
}

} // namespace smarthome
