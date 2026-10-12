#include "http_bridge.hpp"
#include "http_state.hpp"
#include "scene_manager.hpp"
#include "smart_home.hpp"
#include "livingroom.hpp"
#include "bedroom.hpp"
#include "kitchen.hpp"
#include "bathroom.hpp"

namespace smarthome {

// C++17 无 std::string::starts_with，用 rfind 模拟
static bool starts_with(const std::string &s, const std::string &prefix) {
    return s.rfind(prefix, 0) == 0;
}

void HttpBridge::push(const std::string &device, const std::string &action, int value) {
    queue_.push(HttpCmd{device, action, value});
}

void HttpBridge::process_all() {
    // 按值 pop（RingBuffer 内部已按值返回，避免解锁后解引用的 TOCTOU）
    while (auto cmd = queue_.pop()) {
        const std::string &dev = cmd->device;
        const std::string &act = cmd->action;
        int val = cmd->value;

        if (dev == "scene") {
            int id = scene_name_to_id(act);
            if (id >= 0) {
                const SceneConfig &cfg = SceneManager::instance().config(id);
                LivingRoom::instance().apply_scene(cfg);
                Bedroom::instance().apply_scene(cfg);
                Kitchen::instance().apply_scene(cfg);
                Bathroom::instance().apply_scene(cfg);
            }
        } else if (starts_with(dev, "br_") || dev == "humidifier" || dev == "closet" || dev == "aroma") {
            Bedroom::instance().web_control(dev, act, val);
        } else if (starts_with(dev, "kt_") || dev == "outlet" || dev == "fridge" ||
                   dev == "microwave" || dev == "dishwasher" || dev == "oven") {
            Kitchen::instance().web_control(dev, act, val);
        } else if (starts_with(dev, "bh_") || dev == "heater" || dev == "bath_heater" ||
                   dev == "mirror" || dev == "toilet") {
            Bathroom::instance().web_control(dev, act, val);
        } else {
            LivingRoom::instance().web_control(dev, act, val);
        }
    }
}

void HttpBridge::sync_lvgl_to_http() {
    sync_counter_++;
    if (sync_counter_ % 2 != 0) return; // 节流：每 2 次调用同步一次

    HttpState &s = HttpState::instance();
    SmartHome::instance().get_sensors(&s.sensors[0], &s.sensors[1], &s.sensors[2], &s.sensors[3]);
    LivingRoom::instance().sync_to_http();
    Bedroom::instance().sync_to_http();
    Kitchen::instance().sync_to_http();
    Bathroom::instance().sync_to_http();
}

} // namespace smarthome
