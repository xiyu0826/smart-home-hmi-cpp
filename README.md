# 智能家居触控中控（LVGL 9 + C++17）

基于 **LVGL 9** 与 **C++17** 开发的嵌入式智能家居触控中控。四个房间共 30 个可控设备与
2 块温湿度仪表盘，支持触控屏本地控制与浏览器远程控制，两端状态实时双向同步。

同一套代码支持两种后端：PC 上用 SDL2 窗口模拟调试，开发板上走 Framebuffer + evdev。

本项目是对前一版 C 实现的 C++17 重构——功能、场景与接口保持一致，用类与命名空间、
RAII、`std::thread`/`std::mutex` 替代了原版的裸全局变量、`pthread` 与手写定长缓冲区。

---

## 功能

- **四个房间**：客厅 8 设备、卧室 8 设备、厨房 8 设备、卫生间 6 设备
- **5 种一键场景**：睡眠 / 影院 / 离家 / 回家 / 断电，另有独立页面可视化自定义每个场景的预设值
- **Web 远程控制**：内嵌 HTTP 服务器，手机或 PC 浏览器即可控制全部设备
- **双向状态同步**：网页改则触控屏变，触控屏改则网页变
- **全中文界面**：FreeType 动态加载中文字体，加载失败自动降级，不崩溃
- **实时时钟与温湿度**：顶部时钟每秒刷新，客厅 / 卧室双区温湿度仪表盘
- **页面懒加载**：房间页面按需创建，切换前不占内存
- **开关与滑块互锁**：关灯则亮度归零，滑块拖到 0 则自动关灯，且不会回调死循环

## 目录结构

```
.
├── CMakeLists.txt          # 构建脚本（C++17；顶部含交叉编译配置段）
├── main.cpp                # 程序入口：显示初始化 → 主界面 → HTTP 线程 → 主循环
├── lv_conf.h               # LVGL 配置（SDL / FreeType / 文件系统）
├── mycode/                 # 业务代码（C++，11 个 .cpp + 17 个 .hpp）
│   ├── lv_bind.hpp         # 回调桥接（CRTP EventTarget + user_data 回传 this）
│   ├── device.hpp          # 设备控件封装（开关 / 滑块 / 下拉框 / 指示灯）
│   ├── room.hpp            # Room 抽象基类 + 房间共享 UI 辅助
│   ├── scene_config.hpp    # SceneConfig 结构 + SceneId 枚举
│   ├── scene_manager.*     # 场景唯一数据源（单例）
│   ├── scene_settings.*    # 场景自定义设置页面
│   ├── command_queue.hpp   # 模板环形队列 RingBuffer<T>
│   ├── http_state.hpp      # HTTP 状态快照结构
│   ├── http_server.*       # HTTP 服务器（epoll + std::thread）
│   ├── http_handler.*      # 路由分发 + JSON API + 设备控制
│   ├── http_bridge.*       # 线程安全命令队列（Web ↔ LVGL 双向同步）
│   ├── web_pages.*         # 内嵌 Web 控制页面（HTML 用 raw string literal）
│   ├── smart_home.*        # 主界面 + 共享资源 + UI 工具函数
│   └── livingroom/bedroom/kitchen/bathroom.*   # 四个房间
├── images/                 # 28 个 48×48 BMP 设备图标
├── tools/
│   └── gen_icons.c         # 图标批量生成程序（gcc gen_icons.c -o gen_icons -lm）
├── scripts/
│   └── fetch_deps.sh       # 拉取 LVGL 与 FreeType 源码
└── docs/
    └── screenshots/
```

## 依赖

| 依赖 | 版本 | 用途 |
|---|---|---|
| LVGL | **9.0.0** | GUI 框架 |
| FreeType | **2.13.3** | 中文矢量字体渲染 |
| CMake | 3.10+ | 构建 |
| g++ | 支持 C++17 | 编译 |
| SDL2 / SDL2_image | 发行版自带 | PC 模拟显示 |

LVGL 与 FreeType 的源码**不在本仓库中**（第三方代码，合计 140 MB 以上）。执行下面一条
命令即可自动拉取到正确位置：

```bash
./scripts/fetch_deps.sh
```

> 拉下来的目录已在 `.gitignore` 中排除，不要提交。

## 编译与运行

```bash
sudo apt-get install build-essential cmake libsdl2-dev libsdl2-image-dev

./scripts/fetch_deps.sh

mkdir build && cd build
cmake ..
make -j4
cd ..                 # 必须回到项目根目录再启动
./bin/main
```

启动后会弹出一个 1024×600 的 SDL 窗口，同时监听 `:8080`。
浏览器打开 `http://localhost:8080` 即可进入 Web 控制界面。

> **为什么必须回到项目根目录启动**：字体与图片路径按相对路径解析
> （见「中文字体」一节），工作目录不对会加载不到资源。

### 交叉编译到开发板

`CMakeLists.txt` 顶部有一段交叉编译配置，取消注释并填入工具链路径即可：

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER   .../aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER .../aarch64-linux-gnu-g++)
```

板上运行需要把显示后端从 SDL 切到 Framebuffer / DRM，并配置触摸设备节点
（`main.cpp` 中三种后端的代码都已保留，改 `lv_conf.h` 的开关即可）。

## 架构

```
应用层   main.cpp → SmartHome → 4 个 Room 子类
         (业务逻辑、UI、场景分发、状态管理)
------------------------------------------------------
字体层   FreeType 加载中文字体 → lv_font_t
------------------------------------------------------
输入层   触摸屏 / 鼠标 → evdev / SDL → LVGL indev
------------------------------------------------------
显示层   LVGL 渲染 → SDL 窗口（PC）/ /dev/fb0（开发板）
```

启动流程：

```
main()
  ├─ lv_init()                       初始化 LVGL 内核
  ├─ lv_linux_disp_init()            创建显示后端（SDL / FBDEV / DRM）
  ├─ SmartHome::instance().init()    加载字体 + 场景默认值 + 创建主界面
  ├─ HttpServer::instance().start()  启动 HTTP 服务器线程
  └─ while (running) {
         lv_timer_handler();                定时器 → 触摸 → 重绘
         HttpBridge::process_all();         HTTP 命令 → UI
         HttpBridge::sync_lvgl_to_http();   UI 状态 → HTTP 快照
         usleep(5000);                      5ms，约 200Hz
     }
```

## 核心设计

### 1. 回调桥接：让 C 回调调用 C++ 成员函数

LVGL 的事件回调是 C 函数指针 `void(*)(lv_event_t*)`，非静态成员函数带隐式 `this`，
签名不兼容。用 **CRTP 基类 + user_data 回传 this** 解决：

```cpp
template <typename Derived>
class EventTarget {
protected:
    static void event_cb(lv_event_t *e) {
        auto *self = static_cast<Derived *>(lv_event_get_user_data(e));
        if (self) self->on_event(e);
    }
    void bind_event(lv_obj_t *obj, lv_event_code_t code) {
        lv_obj_add_event_cb(obj, &EventTarget<Derived>::event_cb, code, this);
    }
};
```

`SwitchDevice`、`SliderDevice`、`DropdownDevice` 各自继承 `EventTarget<自身>` 并实现
`on_event`，`bind_event()` 自动把 `this` 塞进 `user_data`。每个控件可多实例、无全局状态。

### 2. 设备对象化

原版每个"开关回调 = 读 CHECKED 写状态变量"重复了约 20 次。重构后封装为四个类：

| 类 | 状态 | 控件 | 联动 |
|---|---|---|---|
| `SwitchDevice` | `bool` | `lv_switch` | `std::function<void(bool)>` |
| `SliderDevice` | `int value/min/max` | `lv_slider` + label | `std::function<void(int)>` |
| `DropdownDevice` | `int selected` | `lv_dropdown` | `std::function<void(int)>` |
| `IndicatorGroup` | 指示灯数组 | 圆点 / 图片 | — |

灯与亮度互锁用 `std::function` 回调链表达；程序化赋值（`set` / `set_value` /
`set_selected`）**不触发回调**，沿用 LVGL"`lv_slider_set_value` 不触发
`VALUE_CHANGED`"的语义，因此联动不会死循环。

### 3. 场景唯一数据源

原版 `apply_scene` 读硬编码表，而设置页改的是另一份配置数组，导致"网页改的设置对一键
场景不生效"。重构后统一为 `SceneManager` 单例，触控屏与 `/api/scene_cfg` 读写同一份数据。

### 4. Room 抽象基类与懒加载

```cpp
class Room {
public:
    void ensure_init() { if (screen == nullptr) init(); }   // 懒加载
    virtual void init() = 0;
    virtual void apply_scene(const SceneConfig &cfg) = 0;
    virtual void web_control(const std::string &device,
                             const std::string &action, int value) = 0;
    virtual void get_json(std::string &out) const = 0;
    virtual void sync_to_http() = 0;
};
```

四个房间各为单例，顶部栏与返回逻辑抽取为共享函数，消除原版四处重复。

### 5. 网络层

`epoll`（ET 模式）+ 非阻塞 socket + `std::thread`，命令经由线程安全环形队列
（`RingBuffer<T>` + `std::mutex`）传回主线程，避免跨线程直接操作 LVGL 对象。

**HTTP 接口：**

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/` | Web 控制页面 |
| GET | `/api/status` | 全部设备状态 JSON |
| GET | `/api/sensor` | 温湿度数据 |
| GET | `/api/<room>` | 单房间状态 |
| POST | `/api/device` | 控制设备 `{device, action, value}` |
| POST | `/api/scene` | 执行场景 `{scene: "sleep"}` |
| GET / POST | `/api/scene_cfg` | 场景预设值读写 |

### 6. RAII

`HttpServer` 析构自动 `stop()`，epoll fd 与 socket 由成员管理，线程退出统一清理，
异常路径也不会漏掉资源释放。

## 相比前一版 C 实现修复的问题

1. **客户端 socket 未设非阻塞，配合 `EPOLLET`** —— 慢客户端会阻塞整个服务器线程
2. **`recv` 返回 `EAGAIN` 时用 `continue`** —— 造成忙循环，改为 `break` 等下次事件
3. **桥接队列解锁后解引用内部指针** —— TOCTOU 竞态，改为出队时按值拷出
4. **两套场景数据源不同步** —— 统一为 `SceneManager`
5. **`parse_json_string` 使用 `static` 返回缓冲** —— 非线程安全，改用 `std::string`
6. **图片回退判定恒为真** —— 按路径是否为空判定
7. **场景在房间初始化前应用会丢失** —— 设备对象构造时定初值
8. **死代码** —— 删除未使用的客户端结构与全局数组

## 中文字体

仓库**不包含中文字体文件**。SimHei（中易黑体）等常见中文字体受版权保护，不适合随
开源仓库分发。

自备任一个支持中文的字体即可：

- Windows：`C:\Windows\Fonts\` 下的 `simhei.ttf` / `msyh.ttc`
- Ubuntu：`/usr/share/fonts/` 下的 `NotoSansCJK*.ttc` / `wqy-*.ttc`
- 开源字体：Noto Sans SC、Source Han Sans（OFL 许可，可自由使用）

放到项目根目录并命名为 `simhei.ttf` 即可。字体路径由 `mycode/smart_home.hpp` 的
`FONT_PATH` 宏控制，也可在编译期覆盖：

```bash
cmake -DFONT_PATH="/abs/path/to/font.ttf" ..
```

**没有字体程序也能正常启动**——中文会显示为方块，字体加载失败会自动降级为内置的
Montserrat 字体。

## 已知限制

- 温湿度为随机游走模拟值，未接入真实传感器
- HTTP 接口偏 RPC 风格（如 `/api/device` 动作型接口），未严格遵循 REST 语义
- 交叉编译配置默认注释，需手动开启

## 已验证环境

- 编译与调试：Ubuntu（x86_64），g++（C++17），CMake 3.10+，`libsdl2-dev` + `libsdl2-image-dev`
- 目标平台：RK3568（aarch64），Linux，Framebuffer + evdev
