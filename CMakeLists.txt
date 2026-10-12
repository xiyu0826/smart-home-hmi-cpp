cmake_minimum_required(VERSION 3.10)

# # 交叉编译配置（如需编译到 RK3568 开发板，取消注释并指定工具链路径）
# set(CMAKE_SYSTEM_NAME Linux)
# set(CMAKE_SYSTEM_PROCESSOR arm)
# set(CMAKE_C_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
# set(CMAKE_CXX_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

project(lvgl)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)          # 本项目业务代码用 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

link_directories(./)
include_directories(
    freetype-2.13.3/include/freetype
    freetype-2.13.3/include/
)

add_subdirectory(freetype-2.13.3)
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} PRIVATE freetype-2.13.3/include/freetype)

# aux_source_directory 会收集 .c 与 .cpp，业务代码全部为 .cpp
aux_source_directory(mycode SRC_FILE)
include_directories(mycode)
add_executable(main main.cpp ${SRC_FILE})

# SDL2 显示驱动（PC 模拟用）
find_package(SDL2)
link_libraries(SDL2_image)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
set(SDL2_LIBRARIES "-L ${SDL2_LIBDIR} -lSDL2")

target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${Libdrm_LIBRARIES} m pthread freetype SDL2)
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)
