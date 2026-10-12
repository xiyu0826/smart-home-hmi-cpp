#include "http_handler.hpp"
#include "http_bridge.hpp"
#include "http_state.hpp"
#include "scene_manager.hpp"
#include "smart_home.hpp"
#include "web_pages.hpp"
#include <cstdio>
#include <cstring>

namespace smarthome {

/* ==================== JSON 解析（用 std::string，线程安全） ==================== */

static int parse_json_value(const std::string &body, const std::string &key) {
    std::string search = "\"" + key + "\":";
    std::size_t pos = body.find(search);
    if (pos == std::string::npos) return -1;
    pos += search.size();
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t')) pos++;

    if (pos < body.size() && body[pos] >= '0' && body[pos] <= '9') {
        int value = 0;
        while (pos < body.size() && body[pos] >= '0' && body[pos] <= '9') {
            value = value * 10 + (body[pos] - '0');
            pos++;
        }
        return value;
    }
    if (body.compare(pos, 4, "true") == 0) return 1;
    if (body.compare(pos, 5, "false") == 0) return 0;
    if (body.compare(pos, 2, "on") == 0) return 1;
    if (body.compare(pos, 3, "off") == 0) return 0;
    return -1;
}

static std::string parse_json_string(const std::string &body, const std::string &key) {
    std::string search = "\"" + key + "\":\"";
    std::size_t pos = body.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    std::string result;
    while (pos < body.size() && body[pos] != '"') {
        result += body[pos];
        pos++;
    }
    return result;
}

/* ==================== 状态 JSON 生成（读 HttpState 快照） ==================== */

static std::string livingroom_json() {
    const HttpLR &r = HttpState::instance().lr;
    char buf[512];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":{\"on\":%d,\"brightness\":%d},\"ac\":{\"on\":%d,\"temp\":%d,\"mode\":%d},"
        "\"curtain\":%d,\"strip\":{\"on\":%d,\"color\":%d},\"tv\":{\"on\":%d,\"volume\":%d},"
        "\"robot\":%d,\"purifier\":{\"on\":%d,\"mode\":%d},\"speaker\":{\"on\":%d,\"volume\":%d}}",
        r.light_on, r.light_brightness, r.ac_on, r.ac_temp, r.ac_mode,
        r.curtain_state, r.strip_on, r.strip_color, r.tv_on, r.tv_volume,
        r.robot_on, r.purifier_on, r.purifier_mode, r.speaker_on, r.speaker_volume);
    return buf;
}

static std::string bedroom_json() {
    const HttpBR &r = HttpState::instance().br;
    char buf[512];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":{\"on\":%d,\"brightness\":%d},\"ac\":{\"on\":%d,\"temp\":%d,\"mode\":%d},"
        "\"curtain\":%d,\"strip\":{\"on\":%d,\"color\":%d},\"speaker\":{\"on\":%d,\"volume\":%d},"
        "\"humidifier\":{\"on\":%d,\"level\":%d},\"closet\":%d,\"aroma\":%d}",
        r.light_on, r.light_brightness, r.ac_on, r.ac_temp, r.ac_mode,
        r.curtain_state, r.strip_on, r.strip_color, r.speaker_on, r.speaker_volume,
        r.humidifier_on, r.humidifier_level, r.closet_light_on, r.aroma_on);
    return buf;
}

static std::string kitchen_json() {
    const HttpKT &r = HttpState::instance().kt;
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":%d,\"outlet\":%d,\"fan\":%d,\"fridge\":%d,"
        "\"microwave\":%d,\"dishwasher\":%d,\"oven\":%d,\"purifier\":%d}",
        r.light_on, r.outlet_on, r.fan_on, r.fridge_on,
        r.microwave_on, r.dishwasher_on, r.oven_on, r.purifier_water_on);
    return buf;
}

static std::string bathroom_json() {
    const HttpBH &r = HttpState::instance().bh;
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "{\"light\":%d,\"fan\":%d,\"heater\":{\"on\":%d,\"temp\":%d},"
        "\"bath_heater\":%d,\"mirror\":%d,\"toilet\":%d}",
        r.light_on, r.fan_on, r.heater_on, r.heater_temp,
        r.bath_heater_on, r.mirror_light_on, r.smart_toilet_on);
    return buf;
}

static std::string all_status_json() {
    HttpState &s = HttpState::instance();
    char buf[2048];
    std::snprintf(buf, sizeof(buf),
        "{\"sensors\":{\"lr_temp\":%d,\"lr_humi\":%d,\"br_temp\":%d,\"br_humi\":%d},"
        "\"livingroom\":%s,\"bedroom\":%s,\"kitchen\":%s,\"bathroom\":%s}",
        s.sensors[0], s.sensors[1], s.sensors[2], s.sensors[3],
        livingroom_json().c_str(), bedroom_json().c_str(),
        kitchen_json().c_str(), bathroom_json().c_str());
    return buf;
}

// 单个场景配置 JSON（字段名与 SceneConfig 一致，网页 JS 按名访问）
static void scene_config_json_one(const SceneConfig &c, const char *name, char *buf, std::size_t size) {
    std::snprintf(buf, size,
        "{\"name\":\"%s\","
        "\"lr_light_on\":%d,\"lr_light_brightness\":%d,\"lr_ac_on\":%d,\"lr_ac_temp\":%d,\"lr_ac_mode\":%d,"
        "\"lr_curtain_state\":%d,\"lr_strip_on\":%d,\"lr_strip_color\":%d,"
        "\"lr_tv_on\":%d,\"lr_tv_volume\":%d,\"lr_robot_on\":%d,"
        "\"lr_purifier_on\":%d,\"lr_purifier_mode\":%d,\"lr_speaker_on\":%d,\"lr_speaker_volume\":%d,"
        "\"br_light_on\":%d,\"br_light_brightness\":%d,\"br_ac_on\":%d,\"br_ac_temp\":%d,\"br_ac_mode\":%d,"
        "\"br_curtain_state\":%d,\"br_strip_on\":%d,\"br_strip_color\":%d,"
        "\"br_speaker_on\":%d,\"br_speaker_volume\":%d,"
        "\"br_humidifier_on\":%d,\"br_humidifier_level\":%d,"
        "\"br_closet_light_on\":%d,\"br_aroma_on\":%d,"
        "\"bh_heater_temp\":%d,"
        "\"kt_light_on\":%d,\"kt_outlet_on\":%d,\"kt_fan_on\":%d,\"kt_fridge_on\":%d,"
        "\"kt_microwave_on\":%d,\"kt_dishwasher_on\":%d,\"kt_oven_on\":%d,\"kt_purifier_water_on\":%d,"
        "\"bh_light_on\":%d,\"bh_fan_on\":%d,\"bh_heater_on\":%d,"
        "\"bh_bath_heater_on\":%d,\"bh_mirror_light_on\":%d,\"bh_smart_toilet_on\":%d}",
        name,
        c.lr_light_on, c.lr_light_brightness, c.lr_ac_on, c.lr_ac_temp, c.lr_ac_mode,
        c.lr_curtain_state, c.lr_strip_on, c.lr_strip_color,
        c.lr_tv_on, c.lr_tv_volume, c.lr_robot_on,
        c.lr_purifier_on, c.lr_purifier_mode, c.lr_speaker_on, c.lr_speaker_volume,
        c.br_light_on, c.br_light_brightness, c.br_ac_on, c.br_ac_temp, c.br_ac_mode,
        c.br_curtain_state, c.br_strip_on, c.br_strip_color,
        c.br_speaker_on, c.br_speaker_volume,
        c.br_humidifier_on, c.br_humidifier_level,
        c.br_closet_light_on, c.br_aroma_on,
        c.bh_heater_temp,
        c.kt_light_on, c.kt_outlet_on, c.kt_fan_on, c.kt_fridge_on,
        c.kt_microwave_on, c.kt_dishwasher_on, c.kt_oven_on, c.kt_purifier_water_on,
        c.bh_light_on, c.bh_fan_on, c.bh_heater_on,
        c.bh_bath_heater_on, c.bh_mirror_light_on, c.bh_smart_toilet_on);
}

static std::string scene_cfg_json() {
    std::string out = "[";
    char buf[4096];
    for (int i = 0; i < SceneManager::COUNT; i++) {
        scene_config_json_one(SceneManager::instance().config(i),
                              SceneManager::instance().name(i), buf, sizeof(buf));
        if (i > 0) out += ",";
        out += buf;
    }
    out += "]";
    return out;
}

/* ==================== 设备控制（写 HttpState 快照） ==================== */

static bool control_device(const std::string &device, const std::string &action, int value) {
    HttpState &s = HttpState::instance();

    // ---- 客厅 ----
    if (device == "light") {
        if (action == "on") { s.lr.light_on = 1; s.lr.light_brightness = (value > 0) ? value : 80; }
        else if (action == "off") { s.lr.light_on = 0; s.lr.light_brightness = 0; }
        else if (value >= 0) { s.lr.light_on = (value > 0); s.lr.light_brightness = value; }
    } else if (device == "ac") {
        if (action == "on") s.lr.ac_on = 1;
        else if (action == "off") s.lr.ac_on = 0;
        if (value >= 16 && value <= 30) s.lr.ac_temp = value;
        else if (value >= 0 && value <= 2) s.lr.ac_mode = value;
    } else if (device == "curtain") {
        if (value >= 0 && value <= 3) s.lr.curtain_state = value;
    } else if (device == "strip") {
        if (action == "on") s.lr.strip_on = 1;
        else if (action == "off") s.lr.strip_on = 0;
        if (value >= 0 && value <= 2) s.lr.strip_color = value;
    } else if (device == "tv") {
        if (action == "on") s.lr.tv_on = 1;
        else if (action == "off") s.lr.tv_on = 0;
        if (value >= 0 && value <= 100) s.lr.tv_volume = value;
    } else if (device == "robot") {
        if (action == "on") s.lr.robot_on = 1;
        else if (action == "off") s.lr.robot_on = 0;
    } else if (device == "purifier") {
        if (action == "on") s.lr.purifier_on = 1;
        else if (action == "off") s.lr.purifier_on = 0;
        if (value >= 0 && value <= 2) s.lr.purifier_mode = value;
    } else if (device == "speaker") {
        if (action == "on") s.lr.speaker_on = 1;
        else if (action == "off") s.lr.speaker_on = 0;
        if (value >= 0 && value <= 100) s.lr.speaker_volume = value;
    }
    // ---- 卧室 ----
    else if (device == "br_light") {
        if (action == "on") { s.br.light_on = 1; s.br.light_brightness = (value > 0) ? value : 80; }
        else if (action == "off") { s.br.light_on = 0; s.br.light_brightness = 0; }
        else if (value >= 0) { s.br.light_on = (value > 0); s.br.light_brightness = value; }
    } else if (device == "br_ac") {
        if (action == "on") s.br.ac_on = 1;
        else if (action == "off") s.br.ac_on = 0;
        if (value >= 16 && value <= 30) s.br.ac_temp = value;
        else if (value >= 0 && value <= 2) s.br.ac_mode = value;
    } else if (device == "br_curtain") {
        if (value >= 0 && value <= 3) s.br.curtain_state = value;
    } else if (device == "br_strip") {
        if (action == "on") s.br.strip_on = 1;
        else if (action == "off") s.br.strip_on = 0;
        if (value >= 0 && value <= 2) s.br.strip_color = value;
    } else if (device == "br_speaker") {
        if (action == "on") s.br.speaker_on = 1;
        else if (action == "off") s.br.speaker_on = 0;
        if (value >= 0 && value <= 100) s.br.speaker_volume = value;
    } else if (device == "humidifier") {
        if (action == "on") s.br.humidifier_on = 1;
        else if (action == "off") s.br.humidifier_on = 0;
        if (value >= 0 && value <= 2) s.br.humidifier_level = value;
    } else if (device == "closet") {
        if (action == "on") s.br.closet_light_on = 1;
        else if (action == "off") s.br.closet_light_on = 0;
    } else if (device == "aroma") {
        if (action == "on") s.br.aroma_on = 1;
        else if (action == "off") s.br.aroma_on = 0;
    }
    // ---- 厨房 ----
    else if (device == "kt_light") {
        if (action == "on") s.kt.light_on = 1; else if (action == "off") s.kt.light_on = 0;
    } else if (device == "outlet") {
        if (action == "on") s.kt.outlet_on = 1; else if (action == "off") s.kt.outlet_on = 0;
    } else if (device == "kt_fan") {
        if (action == "on") s.kt.fan_on = 1; else if (action == "off") s.kt.fan_on = 0;
    } else if (device == "fridge") {
        if (action == "on") s.kt.fridge_on = 1; else if (action == "off") s.kt.fridge_on = 0;
    } else if (device == "microwave") {
        if (action == "on") s.kt.microwave_on = 1; else if (action == "off") s.kt.microwave_on = 0;
    } else if (device == "dishwasher") {
        if (action == "on") s.kt.dishwasher_on = 1; else if (action == "off") s.kt.dishwasher_on = 0;
    } else if (device == "oven") {
        if (action == "on") s.kt.oven_on = 1; else if (action == "off") s.kt.oven_on = 0;
    } else if (device == "kt_purifier") {
        if (action == "on") s.kt.purifier_water_on = 1; else if (action == "off") s.kt.purifier_water_on = 0;
    }
    // ---- 卫生间 ----
    else if (device == "bh_light") {
        if (action == "on") s.bh.light_on = 1; else if (action == "off") s.bh.light_on = 0;
    } else if (device == "bh_fan") {
        if (action == "on") s.bh.fan_on = 1; else if (action == "off") s.bh.fan_on = 0;
    } else if (device == "heater") {
        if (action == "on") s.bh.heater_on = 1;
        else if (action == "off") s.bh.heater_on = 0;
        if (value >= 30 && value <= 60) s.bh.heater_temp = value;
    } else if (device == "bath_heater") {
        if (action == "on") s.bh.bath_heater_on = 1; else if (action == "off") s.bh.bath_heater_on = 0;
    } else if (device == "mirror") {
        if (action == "on") s.bh.mirror_light_on = 1; else if (action == "off") s.bh.mirror_light_on = 0;
    } else if (device == "toilet") {
        if (action == "on") s.bh.smart_toilet_on = 1; else if (action == "off") s.bh.smart_toilet_on = 0;
    } else {
        return false; // 未知设备
    }
    return true;
}

// 应用场景：从 SceneManager（唯一数据源）写到 HttpState 快照
static bool apply_scene(const std::string &scene) {
    int id = scene_name_to_id(scene);
    if (id < 0) return false;

    const SceneConfig &c = SceneManager::instance().config(id);
    HttpState &s = HttpState::instance();

    s.lr.light_on = c.lr_light_on;
    s.lr.light_brightness = c.lr_light_brightness;
    s.lr.ac_on = c.lr_ac_on;
    s.lr.ac_temp = c.lr_ac_temp;
    s.lr.ac_mode = c.lr_ac_mode;
    s.lr.curtain_state = c.lr_curtain_state;
    s.lr.strip_on = c.lr_strip_on;
    s.lr.strip_color = c.lr_strip_color;
    s.lr.tv_on = c.lr_tv_on;
    s.lr.tv_volume = c.lr_tv_volume;
    s.lr.robot_on = c.lr_robot_on;
    s.lr.purifier_on = c.lr_purifier_on;
    s.lr.purifier_mode = c.lr_purifier_mode;
    s.lr.speaker_on = c.lr_speaker_on;
    s.lr.speaker_volume = c.lr_speaker_volume;

    s.br.light_on = c.br_light_on;
    s.br.light_brightness = c.br_light_brightness;
    s.br.ac_on = c.br_ac_on;
    s.br.ac_temp = c.br_ac_temp;
    s.br.ac_mode = c.br_ac_mode;
    s.br.curtain_state = c.br_curtain_state;
    s.br.strip_on = c.br_strip_on;
    s.br.strip_color = c.br_strip_color;
    s.br.speaker_on = c.br_speaker_on;
    s.br.speaker_volume = c.br_speaker_volume;
    s.br.humidifier_on = c.br_humidifier_on;
    s.br.humidifier_level = c.br_humidifier_level;
    s.br.closet_light_on = c.br_closet_light_on;
    s.br.aroma_on = c.br_aroma_on;

    s.kt.light_on = c.kt_light_on;
    s.kt.outlet_on = c.kt_outlet_on;
    s.kt.fan_on = c.kt_fan_on;
    s.kt.fridge_on = c.kt_fridge_on;
    s.kt.microwave_on = c.kt_microwave_on;
    s.kt.dishwasher_on = c.kt_dishwasher_on;
    s.kt.oven_on = c.kt_oven_on;
    s.kt.purifier_water_on = c.kt_purifier_water_on;

    s.bh.light_on = c.bh_light_on;
    s.bh.fan_on = c.bh_fan_on;
    s.bh.heater_on = c.bh_heater_on;
    s.bh.heater_temp = c.bh_heater_temp;
    s.bh.bath_heater_on = c.bh_bath_heater_on;
    s.bh.mirror_light_on = c.bh_mirror_light_on;
    s.bh.smart_toilet_on = c.bh_smart_toilet_on;
    return true;
}

/* ==================== 路由处理 ==================== */

static void handle_get(int client_fd, const std::string &path) {
    if (path == "/" || path == "/index.html") {
        HttpServer::send_html(client_fd, web_get_page());
    } else if (path == "/api/sensor") {
        int lt, lh, bt, bh;
        SmartHome::instance().get_sensors(&lt, &lh, &bt, &bh);
        char json[256];
        std::snprintf(json, sizeof(json),
            "{\"lr_temp\":%d,\"lr_humi\":%d,\"br_temp\":%d,\"br_humi\":%d}", lt, lh, bt, bh);
        HttpServer::send_json(client_fd, json);
    } else if (path == "/api/status") {
        std::string json = all_status_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else if (path == "/api/livingroom") {
        std::string json = livingroom_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else if (path == "/api/bedroom") {
        std::string json = bedroom_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else if (path == "/api/kitchen") {
        std::string json = kitchen_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else if (path == "/api/bathroom") {
        std::string json = bathroom_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else if (path == "/api/scene_cfg") {
        std::string json = scene_cfg_json();
        HttpServer::send_json(client_fd, json.c_str());
    } else {
        HttpServer::send_error(client_fd, 404, "Not Found");
    }
}

static void handle_post(int client_fd, const std::string &path, const std::string &body) {
    if (path == "/api/device") {
        std::string device = parse_json_string(body, "device");
        std::string action = parse_json_string(body, "action");
        int value = parse_json_value(body, "value");

        if (!device.empty() && !action.empty()) {
            if (control_device(device, action, value)) {
                HttpBridge::instance().push(device, action, value);
                HttpServer::send_json(client_fd, "{\"result\":\"ok\"}");
            } else {
                HttpServer::send_error(client_fd, 400, "Unknown device or action");
            }
        } else {
            HttpServer::send_error(client_fd, 400, "Invalid request");
        }
    } else if (path == "/api/scene") {
        std::string scene = parse_json_string(body, "scene");
        if (!scene.empty()) {
            if (apply_scene(scene)) {
                HttpBridge::instance().push("scene", scene, 0);
                HttpServer::send_json(client_fd, "{\"result\":\"ok\"}");
            } else {
                HttpServer::send_error(client_fd, 400, "Unknown scene");
            }
        } else {
            HttpServer::send_error(client_fd, 400, "Invalid request");
        }
    } else if (path == "/api/scene_cfg") {
        int idx = parse_json_value(body, "scene");
        if (idx >= 0 && idx < SceneManager::COUNT) {
            SceneConfig &c = SceneManager::instance().config(idx);
            int v;
#define UPDATE_FIELD(field) v = parse_json_value(body, #field); if (v >= 0) c.field = v;
            UPDATE_FIELD(lr_light_on); UPDATE_FIELD(lr_light_brightness);
            UPDATE_FIELD(lr_ac_on); UPDATE_FIELD(lr_ac_temp); UPDATE_FIELD(lr_ac_mode);
            UPDATE_FIELD(lr_curtain_state); UPDATE_FIELD(lr_strip_on); UPDATE_FIELD(lr_strip_color);
            UPDATE_FIELD(lr_tv_on); UPDATE_FIELD(lr_tv_volume); UPDATE_FIELD(lr_robot_on);
            UPDATE_FIELD(lr_purifier_on); UPDATE_FIELD(lr_purifier_mode);
            UPDATE_FIELD(lr_speaker_on); UPDATE_FIELD(lr_speaker_volume);
            UPDATE_FIELD(br_light_on); UPDATE_FIELD(br_light_brightness);
            UPDATE_FIELD(br_ac_on); UPDATE_FIELD(br_ac_temp); UPDATE_FIELD(br_ac_mode);
            UPDATE_FIELD(br_curtain_state); UPDATE_FIELD(br_strip_on); UPDATE_FIELD(br_strip_color);
            UPDATE_FIELD(br_speaker_on); UPDATE_FIELD(br_speaker_volume);
            UPDATE_FIELD(br_humidifier_on); UPDATE_FIELD(br_humidifier_level);
            UPDATE_FIELD(br_closet_light_on); UPDATE_FIELD(br_aroma_on);
            UPDATE_FIELD(kt_light_on); UPDATE_FIELD(kt_outlet_on); UPDATE_FIELD(kt_fan_on);
            UPDATE_FIELD(kt_fridge_on); UPDATE_FIELD(kt_microwave_on); UPDATE_FIELD(kt_dishwasher_on);
            UPDATE_FIELD(kt_oven_on); UPDATE_FIELD(kt_purifier_water_on);
            UPDATE_FIELD(bh_light_on); UPDATE_FIELD(bh_fan_on); UPDATE_FIELD(bh_heater_on);
            UPDATE_FIELD(bh_bath_heater_on); UPDATE_FIELD(bh_mirror_light_on); UPDATE_FIELD(bh_smart_toilet_on);
            UPDATE_FIELD(bh_heater_temp);
#undef UPDATE_FIELD
            HttpServer::send_json(client_fd, "{\"result\":\"ok\"}");
        } else {
            HttpServer::send_error(client_fd, 400, "Bad scene index");
        }
    } else {
        HttpServer::send_error(client_fd, 404, "Not Found");
    }
}

void http_handle_request(int client_fd, HttpRequest *req) {
    std::printf("[HTTP] %s %s\n", req->method.c_str(), req->path.c_str());

    if (req->method == "GET") {
        handle_get(client_fd, req->path);
    } else if (req->method == "POST") {
        handle_post(client_fd, req->path, req->body);
    } else {
        HttpServer::send_error(client_fd, 405, "Method Not Allowed");
    }
}

} // namespace smarthome
