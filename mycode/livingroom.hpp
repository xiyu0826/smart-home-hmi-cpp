#pragma once
// livingroom.hpp —— 客厅房间（8 设备，最复杂，作为其余房间的样板）

#include "room.hpp"
#include "device.hpp"

namespace smarthome {

class LivingRoom : public Room {
public:
    static LivingRoom &instance() {
        static LivingRoom r;
        return r;
    }

    void init() override;
    void apply_scene(const SceneConfig &cfg) override;
    void web_control(const std::string &device, const std::string &action, int value) override;
    void get_json(std::string &out) const override;
    void sync_to_http() override;

private:
    LivingRoom() = default;
    LivingRoom(const LivingRoom &) = delete;
    LivingRoom &operator=(const LivingRoom &) = delete;

    // 主灯
    SwitchDevice light_sw_{false};
    SliderDevice light_brightness_{0, 100, 80, "%d%%"};
    IndicatorGroup light_ig_;
    // 空调
    SwitchDevice ac_sw_{false};
    SliderDevice ac_temp_{16, 30, 22, "%d C"};
    DropdownDevice ac_mode_{"制冷\n制热\n送风", 0, 23};
    IndicatorGroup ac_ig_;
    // 窗帘
    DropdownDevice curtain_{"全开\n关左\n关右\n全关", 0, 28};
    // 灯带
    SwitchDevice strip_sw_{false};
    DropdownDevice strip_color_{"暖光\n白光\n彩光", 0, 23};
    // 电视
    SwitchDevice tv_sw_{false};
    SliderDevice tv_vol_{0, 100, 50, "%d"};
    IndicatorGroup tv_ig_;
    // 扫地机器人
    SwitchDevice robot_{false};
    // 空气净化器
    SwitchDevice purifier_sw_{false};
    DropdownDevice purifier_mode_{"自动\n睡眠\n强力", 0, 18};
    // 智能音箱
    SwitchDevice speaker_sw_{false};
    SliderDevice speaker_vol_{0, 100, 40, "%d"};
    IndicatorGroup speaker_ig_;
};

} // namespace smarthome
