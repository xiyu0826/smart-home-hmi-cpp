#pragma once
// bathroom.hpp —— 卫生间房间（6 设备，热水器带水温滑块）

#include "room.hpp"
#include "device.hpp"

namespace smarthome {

class Bathroom : public Room {
public:
    static Bathroom &instance() {
        static Bathroom b;
        return b;
    }

    void init() override;
    void apply_scene(const SceneConfig &cfg) override;
    void web_control(const std::string &device, const std::string &action, int value) override;
    void get_json(std::string &out) const override;
    void sync_to_http() override;

private:
    Bathroom() = default;
    Bathroom(const Bathroom &) = delete;
    Bathroom &operator=(const Bathroom &) = delete;

    SwitchDevice light_sw_{false};
    SwitchDevice fan_sw_{false};
    // 热水器（开关 + 水温滑块）
    SwitchDevice heater_sw_{false};
    SliderDevice heater_temp_{30, 60, 40, "%d C"};
    IndicatorGroup heater_ig_;
    SwitchDevice bath_heater_sw_{false};
    SwitchDevice mirror_light_sw_{false};
    SwitchDevice smart_toilet_sw_{false};
};

} // namespace smarthome
