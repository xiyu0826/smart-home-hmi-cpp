#pragma once
// scene_config.hpp —— 场景枚举与单个场景的完整设备配置
//
// 一个 SceneConfig 记录 4 个房间共 51 个字段的预设状态。
// 这是场景唯一数据源：触摸屏"一键场景"按钮与网页 /api/scene_cfg 读写同一份数据，
// 修复了原 C 版 apply_scene 用硬编码 g_scenes[]、而设置页改 g_scene_config[] 导致的
// "改了设置却对一键场景不生效"的 bug。

namespace smarthome {

// 场景编号
enum SceneId : int {
    SCENE_SLEEP     = 0,  // 睡眠模式
    SCENE_MOVIE     = 1,  // 影院模式
    SCENE_AWAY      = 2,  // 离家模式
    SCENE_HOME      = 3,  // 回家模式
    SCENE_POWER_OFF = 4,  // 断电模式
    SCENE_COUNT     = 5,
};

// 单个场景的完整配置（字段命名沿用 C 版，保证场景配置 API 兼容）
struct SceneConfig {
    // ========== 客厅 ==========
    bool lr_light_on        = false;
    int  lr_light_brightness = 0;
    bool lr_ac_on           = false;
    int  lr_ac_temp         = 22;
    int  lr_ac_mode         = 0;
    int  lr_curtain_state   = 0;
    bool lr_strip_on        = false;
    int  lr_strip_color     = 0;
    bool lr_tv_on           = false;
    int  lr_tv_volume       = 50;
    bool lr_robot_on        = false;
    bool lr_purifier_on     = false;
    int  lr_purifier_mode   = 0;
    bool lr_speaker_on      = false;
    int  lr_speaker_volume  = 40;

    // ========== 卧室 ==========
    bool br_light_on        = false;
    int  br_light_brightness = 0;
    bool br_ac_on           = false;
    int  br_ac_temp         = 24;
    int  br_ac_mode         = 0;
    int  br_curtain_state   = 0;
    bool br_strip_on        = false;
    int  br_strip_color     = 0;
    bool br_speaker_on      = false;
    int  br_speaker_volume  = 40;
    bool br_humidifier_on   = false;
    int  br_humidifier_level = 0;
    bool br_closet_light_on = false;
    bool br_aroma_on        = false;

    // ========== 厨房 ==========
    bool kt_light_on        = false;
    bool kt_outlet_on       = false;
    bool kt_fan_on          = false;
    bool kt_fridge_on       = true;
    bool kt_microwave_on    = false;
    bool kt_dishwasher_on   = false;
    bool kt_oven_on         = false;
    bool kt_purifier_water_on = false;

    // ========== 卫生间 ==========
    bool bh_light_on        = false;
    bool bh_fan_on          = false;
    bool bh_heater_on       = false;
    int  bh_heater_temp     = 40;
    bool bh_bath_heater_on  = false;
    bool bh_mirror_light_on = false;
    bool bh_smart_toilet_on = false;
};

} // namespace smarthome
