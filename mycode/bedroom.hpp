#pragma once
// bedroom.hpp —— 卧室房间（8 设备）

#include "room.hpp"
#include "device.hpp"

namespace smarthome {

class Bedroom : public Room {
public:
    static Bedroom &instance() {
        static Bedroom b;
        return b;
    }

    void init() override;
    void apply_scene(const SceneConfig &cfg) override;
    void web_control(const std::string &device, const std::string &action, int value) override;
    void get_json(std::string &out) const override;
    void sync_to_http() override;

private:
    Bedroom() = default;
    Bedroom(const Bedroom &) = delete;
    Bedroom &operator=(const Bedroom &) = delete;

    // 主灯
    SwitchDevice light_sw_{true};
    SliderDevice light_brightness_{0, 100, 60, "%d%%"};
    IndicatorGroup light_ig_;
    // 空调
    SwitchDevice ac_sw_{true};
    SliderDevice ac_temp_{16, 30, 24, "%d C"};
    DropdownDevice ac_mode_{"制冷\n制热\n送风", 0, 23};
    IndicatorGroup ac_ig_;
    // 窗帘
    DropdownDevice curtain_{"全开\n关左\n关右\n全关", 0, 28};
    // 灯带
    SwitchDevice strip_sw_{false};
    DropdownDevice strip_color_{"暖光\n白光\n彩光", 0, 23};
    // 智能音箱
    SwitchDevice speaker_sw_{false};
    SliderDevice speaker_vol_{0, 100, 40, "%d"};
    IndicatorGroup speaker_ig_;
    // 加湿器
    SwitchDevice humidifier_sw_{false};
    DropdownDevice humidifier_level_{"低\n中\n高", 0, 23};
    // 衣帽间灯
    SwitchDevice closet_sw_{false};
    // 香薰机
    SwitchDevice aroma_sw_{false};
};

} // namespace smarthome
