#pragma once
// kitchen.hpp —— 厨房房间（8 设备，全开关）

#include "room.hpp"
#include "device.hpp"

namespace smarthome {

class Kitchen : public Room {
public:
    static Kitchen &instance() {
        static Kitchen k;
        return k;
    }

    void init() override;
    void apply_scene(const SceneConfig &cfg) override;
    void web_control(const std::string &device, const std::string &action, int value) override;
    void get_json(std::string &out) const override;
    void sync_to_http() override;

private:
    Kitchen() = default;
    Kitchen(const Kitchen &) = delete;
    Kitchen &operator=(const Kitchen &) = delete;

    SwitchDevice light_sw_{false};
    SwitchDevice outlet_sw_{false};
    SwitchDevice fan_sw_{false};
    SwitchDevice fridge_sw_{true};
    SwitchDevice microwave_sw_{false};
    SwitchDevice dishwasher_sw_{false};
    SwitchDevice oven_sw_{false};
    SwitchDevice purifier_water_sw_{false};
};

} // namespace smarthome
