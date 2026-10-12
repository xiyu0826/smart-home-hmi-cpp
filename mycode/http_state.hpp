#pragma once
// http_state.hpp —— HTTP 状态快照结构体
//
// 这些是各房间 sync_to_http() 填充、HTTP 线程读取的设备状态快照。
// 字段命名与 JSON 键名、HTTP 设备名保持一致，保证 Web 接口兼容。

namespace smarthome {

// 客厅设备状态（15 字段）
struct HttpLR {
    int light_on        = 0;
    int light_brightness = 80;
    int ac_on           = 0;
    int ac_temp         = 22;
    int ac_mode         = 0;
    int curtain_state   = 0;
    int strip_on        = 0;
    int strip_color     = 0;
    int tv_on           = 0;
    int tv_volume       = 50;
    int robot_on        = 0;
    int purifier_on     = 0;
    int purifier_mode   = 0;
    int speaker_on      = 0;
    int speaker_volume  = 40;
};

// 卧室设备状态（14 字段）
struct HttpBR {
    int light_on        = 1;
    int light_brightness = 60;
    int ac_on           = 1;
    int ac_temp         = 24;
    int ac_mode         = 0;
    int curtain_state   = 0;
    int strip_on        = 0;
    int strip_color     = 0;
    int speaker_on      = 0;
    int speaker_volume  = 40;
    int humidifier_on   = 0;
    int humidifier_level = 0;
    int closet_light_on = 0;
    int aroma_on        = 0;
};

// 厨房设备状态（8 字段）
struct HttpKT {
    int light_on        = 0;
    int outlet_on       = 0;
    int fan_on          = 0;
    int fridge_on       = 1;
    int microwave_on    = 0;
    int dishwasher_on   = 0;
    int oven_on         = 0;
    int purifier_water_on = 0;
};

// 卫生间设备状态（7 字段）
struct HttpBH {
    int light_on        = 0;
    int fan_on          = 0;
    int heater_on       = 0;
    int heater_temp     = 40;
    int bath_heater_on  = 0;
    int mirror_light_on = 0;
    int smart_toilet_on = 0;
};

// HTTP 状态快照的全局持有者（单例）。
// UI 线程通过各房间 sync_to_http() 写入，HTTP 线程读取以生成 JSON 响应。
// 字段均为 int，单次读写原子，采用与原 C 版一致的"最终一致"模型（不加锁）。
class HttpState {
public:
    static HttpState &instance() {
        static HttpState s;
        return s;
    }

    HttpLR lr;
    HttpBR br;
    HttpKT kt;
    HttpBH bh;
    int sensors[4] = {26, 55, 24, 50}; // 客厅温湿、卧室温湿

private:
    HttpState() = default;
    HttpState(const HttpState &) = delete;
    HttpState &operator=(const HttpState &) = delete;
};

} // namespace smarthome
