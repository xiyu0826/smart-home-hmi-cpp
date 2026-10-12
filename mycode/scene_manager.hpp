#pragma once
// scene_manager.hpp —— 场景配置唯一数据源
//
// 原 C 版存在两份场景数据：apply_scene 读硬编码 g_scenes[]、设置页改 g_scene_config[]，
// 导致网页修改的设置对"一键场景"不生效。这里统一为单一 SceneManager，
// 触摸屏与网页 /api/scene_cfg 读写同一份 std::array<SceneConfig, 5>。

#include "scene_config.hpp"
#include <array>
#include <string>

namespace smarthome {

// 场景英文名 → 场景 ID（HTTP /api/scene 与桥接分发共用）
inline int scene_name_to_id(const std::string &name) {
    if (name == "sleep") return SCENE_SLEEP;
    if (name == "movie") return SCENE_MOVIE;
    if (name == "away")  return SCENE_AWAY;
    if (name == "home")  return SCENE_HOME;
    if (name == "off")   return SCENE_POWER_OFF;
    return -1;
}

class SceneManager {
public:
    static SceneManager &instance() {
        static SceneManager m;
        return m;
    }

    // 填充 5 个场景的默认配置（程序启动时调用一次）
    void init();

    SceneConfig &config(int scene) { return configs_[scene]; }
    const SceneConfig &config(int scene) const { return configs_[scene]; }

    const char *name(int scene) const { return names_[scene]; }

    static constexpr int COUNT = SCENE_COUNT;

private:
    SceneManager() = default;
    SceneManager(const SceneManager &) = delete;
    SceneManager &operator=(const SceneManager &) = delete;

    std::array<SceneConfig, SCENE_COUNT> configs_{};
    static constexpr const char *names_[SCENE_COUNT] = {
        "睡眠模式", "影院模式", "离家模式", "回家模式", "一键断电",
    };
};

} // namespace smarthome
