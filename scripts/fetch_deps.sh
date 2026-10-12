#!/usr/bin/env bash
# 拉取本项目依赖的第三方源码到仓库根目录。
#
# 用法：  ./scripts/fetch_deps.sh
#
# 会得到：
#   ./lvgl/                 LVGL 9.0.0
#   ./freetype-2.13.3/      FreeType 2.13.3
#
# 这两个目录已在 .gitignore 中排除，不需要也不应该提交。

set -euo pipefail

LVGL_VER="9.0.0"
FREETYPE_VER="2.13.3"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

fetch() {
    local url="$1" out="$2"
    echo "  -> $url"
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --connect-timeout 20 -o "$out" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=3 -O "$out" "$url"
    else
        echo "错误：需要 curl 或 wget，请先安装其一。" >&2
        exit 1
    fi
}

echo "==> 检查 LVGL v${LVGL_VER}"
if [ -d lvgl ]; then
    echo "  已存在 ./lvgl，跳过"
else
    fetch "https://github.com/lvgl/lvgl/archive/refs/tags/v${LVGL_VER}.tar.gz" "$TMP/lvgl.tar.gz"
    tar -xzf "$TMP/lvgl.tar.gz" -C "$TMP"
    mv "$TMP/lvgl-${LVGL_VER}" lvgl
    echo "  完成：./lvgl ($(cat lvgl/library.json | grep -o '"version": *"[^"]*"' | head -1))"
fi

echo "==> 检查 FreeType ${FREETYPE_VER}"
if [ -d "freetype-${FREETYPE_VER}" ]; then
    echo "  已存在 ./freetype-${FREETYPE_VER}，跳过"
else
    if ! fetch "https://download.savannah.gnu.org/releases/freetype/freetype-${FREETYPE_VER}.tar.gz" \
               "$TMP/freetype.tar.gz" 2>/dev/null; then
        echo "  Savannah 源不可用，改用 GitHub 镜像"
        fetch "https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.tar.gz" \
              "$TMP/freetype.tar.gz"
        tar -xzf "$TMP/freetype.tar.gz" -C "$TMP"
        mv "$TMP/freetype-VER-2-13-3" "freetype-${FREETYPE_VER}"
        echo "  完成：./freetype-${FREETYPE_VER}"
    else
        tar -xzf "$TMP/freetype.tar.gz" -C "$ROOT"
        echo "  完成：./freetype-${FREETYPE_VER}"
    fi
fi

echo
echo "依赖就绪。"
echo
echo "注意：仓库不包含中文字体文件。"
echo "请自备一个支持中文的字体放到项目根目录并命名为 simhei.ttf，"
echo "否则程序可正常运行，但中文会显示为方块。详见 README。"
