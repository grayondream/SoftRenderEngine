# 纹理映射实施计划（透视校正 UV 插值 + 纹理采样）

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为软渲染器加入透视校正 UV 插值、stb_image 加载、最近邻/双线性采样与 Repeat/Clamp 寻址，并在旋转立方体 Demo 上贴图验收。

**Architecture:** 方案 A——`ScreenVertex` 增加 `u/v` 字段；新增 `Texture` 类（内存构造 + stb_image 文件加载）；`Rasterizer::drawTriangleTextured` 镜像 `drawTriangleSolid` 的重心坐标骨架，仅把着色端换成「透视校正 UV → tex.sample → setPixel」。不改 flat 路径，零回归风险。

**Tech Stack:** C++20 / clang++ / CMake 3.20+ / googletest release-1.12.1（FetchContent 已配置）/ stb_image.h（vendor 单头库）

**Spec:** `docs/superpowers/specs/2026-08-22-texture-mapping-design.md`

## Global Constraints

- 编译器 clang++，C++20；构建目录 `build`；所有命令从仓库根 `/home/ares/workspace/SoftGameEngine` 运行
- 新增测试文件后必须先 `cmake -B build -S .` 重新配置（test 目标由 `file(GLOB_RECURSE)` 在 configure 时展开）
- 测试目标命名规则：`test/render/Texture.cpp` → 目标 `render_Texture`，二进制 `./build/test/render_Texture`
- 像素打包格式全项目统一：`a<<24 | r<<16 | g<<8 | b`
- 光栅化采样点是**像素中心 `(x+0.5, y+0.5)`**（见 `src/Render/Rasterizer.cpp:69`），所有测试期望值按此推导
- 代码风格：无注释新增（镜像既有代码的结构性注释除外）、无异常（错误返回空对象/布尔）、conventional commits
- 任务收尾更新 `PROGRESS.md`（Markdown 表格），回复以「完成了sir」结尾

## 文件结构

| 文件 | 动作 | 职责 |
| ---- | ---- | ---- |
| `third_party/stb/stb_image.h` | Create(Task 3) | vendor 图像解码单头库 |
| `src/Render/Texture.hpp` | Create(Task 1) | TextureFilter/TextureWrap 枚举 + Texture 类接口 |
| `src/Render/Texture.cpp` | Create(Task 1) | 内存构造/采样(Nearest→Bilinear)/loadFromFile |
| `src/Render/CMakeLists.txt` | Modify(Task 1) | SGE_RENDER_SRC_FILES 追加 Texture.cpp |
| `src/include.cmake` | Modify(Task 3) | 全局 include 追加 third_party |
| `test/render/Texture.cpp` | Create(Task 1) | render_Texture 测试套件 |
| `test/assets/test_4x4.png` | Create(Task 3) | 4×4 RGBA 测试资产（脚本生成后提交） |
| `src/Render/Rasterizer.hpp` | Modify(Task 4/5) | ScreenVertex.u/v；drawTriangleTextured 声明 |
| `src/Render/GeoObject/PolyF4D.hpp` | Modify(Task 4) | UV2D 结构 + uvlist[3] |
| `src/Render/Pipeline.cpp` | Modify(Task 4) | projectObject UV 透传 |
| `test/render/PipelineTest.cpp` | Modify(Task 4) | UvPassthrough 测试 |
| `src/Render/Rasterizer.cpp` | Modify(Task 5) | drawTriangleTextured 实现 |
| `test/render/Rasterizer.cpp` | Modify(Task 5) | PerspectiveCorrectNotAffine / QuadDiagonalUvContinuity |
| `src/Application.hpp` / `src/Application.cpp` | Modify(Task 6) | 棋盘格纹理成员 + MakeCube UV + 贴图渲染 |

---

### Task 1: Texture 类骨架（内存构造 + Nearest 采样 + Repeat/Clamp）

**Files:**
- Create: `src/Render/Texture.hpp`
- Create: `src/Render/Texture.cpp`
- Create: `test/render/Texture.cpp`
- Modify: `src/Render/CMakeLists.txt`

**Interfaces:**
- Consumes: 无（独立新组件）
- Produces（后续任务依赖的精确签名）:
  - `enum class TextureFilter{ Nearest, Bilinear };`
  - `enum class TextureWrap{ Repeat, Clamp };`
  - `Texture() = default;`（空纹理 w=h=0）
  - `Texture(std::size_t w, std::size_t h, const uint32_t *bgra);`（bgra 可为 nullptr）
  - `static Texture loadFromFile(const std::string &path);`（本任务仅声明，Task 3 实现）
  - `uint32_t sample(double u, double v, TextureFilter filter = TextureFilter::Nearest, TextureWrap wrap = TextureWrap::Repeat) const;`（Task 2 将默认值改为 Bilinear）
  - `std::size_t width() const; std::size_t height() const;`

- [ ] **Step 1: 写失败测试**

创建 `test/render/Texture.cpp`：

```cpp
#include "Texture.hpp"
#include <gtest/gtest.h>

namespace{
constexpr uint32_t kBlack = 0xFF000000u;
constexpr uint32_t kRed   = 0xFFFF0000u;
constexpr uint32_t kGreen = 0xFF00FF00u;
constexpr uint32_t kBlue  = 0xFF0000FFu;
constexpr uint32_t kWhite = 0xFFFFFFFFu;
constexpr uint32_t kGray(uint32_t v){
    return 0xFF000000u | (v << 16) | (v << 8) | v;
}
}

TEST(TextureTest, EmptyReturnsBlack){
    Texture t{};
    EXPECT_EQ(t.width(), 0u);
    EXPECT_EQ(t.height(), 0u);
    EXPECT_EQ(t.sample(0.5, 0.5), kBlack);
    EXPECT_EQ(t.sample(0.5, 0.5, TextureFilter::Nearest), kBlack);
    EXPECT_EQ(t.sample(0.5, 0.5, TextureFilter::Bilinear), kBlack);
}

TEST(TextureTest, MemoryConstructorStoresPixels){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);
    ASSERT_EQ(t.width(), 2u);
    ASSERT_EQ(t.height(), 2u);

    EXPECT_EQ(t.sample(0.24, 0.24, TextureFilter::Nearest), kRed);
    EXPECT_EQ(t.sample(0.75, 0.26, TextureFilter::Nearest), kGreen);
    EXPECT_EQ(t.sample(0.25, 0.76, TextureFilter::Nearest), kBlue);
    EXPECT_EQ(t.sample(0.75, 0.75, TextureFilter::Nearest), kWhite);
}

TEST(TextureTest, WrapRepeat){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(-0.25, 0.26, TextureFilter::Nearest, TextureWrap::Repeat), kGreen);
    EXPECT_EQ(t.sample( 1.25, 0.26, TextureFilter::Nearest, TextureWrap::Repeat), kRed);
    EXPECT_EQ(t.sample( 0.26,-0.25, TextureFilter::Nearest, TextureWrap::Repeat), kBlue);
}

TEST(TextureTest, WrapClamp){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(-0.25, 0.26, TextureFilter::Nearest, TextureWrap::Clamp), kRed);
    EXPECT_EQ(t.sample( 1.25, 0.26, TextureFilter::Nearest, TextureWrap::Clamp), kGreen);
    EXPECT_EQ(t.sample( 0.26, 1.25, TextureFilter::Nearest, TextureWrap::Clamp), kBlue);
    EXPECT_EQ(t.sample( 1.25, 1.25, TextureFilter::Nearest, TextureWrap::Clamp), kWhite);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

数值依据（2×2 纹理，nearest：`tx=floor(u*2)`）：`(0.24,0.24)`→texel(0,0)；`(0.75,0.26)`→tx=floor(1.5)=1,ty=0；`(0.25,0.76)`→ty=1。Repeat 取模 `((t%size)+size)%size`：`-0.25`→tx=-1→1；`1.25`→tx=2→0；`v=-0.25`→ty=1。

- [ ] **Step 2: 配置并构建，验证失败**

```bash
cmake -B build -S . && cmake --build build -j$(nproc) --target render_Texture
```

预期：FAIL，报 `Texture.hpp` 文件不存在（目标尚未可编译）。

- [ ] **Step 3: 最小实现**

创建 `src/Render/Texture.hpp`：

```cpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class TextureFilter{
    Nearest,
    Bilinear,
};

enum class TextureWrap{
    Repeat,
    Clamp,
};

class Texture{
public:
    Texture() = default;

    Texture(std::size_t w, std::size_t h, const uint32_t *bgra);

    static Texture loadFromFile(const std::string &path);

    uint32_t sample(double u, double v,
                    TextureFilter filter = TextureFilter::Nearest,
                    TextureWrap wrap = TextureWrap::Repeat) const;

    std::size_t width() const{ return m_w; }
    std::size_t height() const{ return m_h; }

private:
    uint32_t fetchTexel(int tx, int ty, TextureWrap wrap) const;

    std::size_t m_w{};
    std::size_t m_h{};
    std::vector<uint32_t> m_pixels{};
};
```

创建 `src/Render/Texture.cpp`：

```cpp
#include "Texture.hpp"
#include <algorithm>
#include <cmath>

namespace{
int WrapTexel(int t, int maxIdx, TextureWrap wrap){
    if(wrap == TextureWrap::Clamp){
        return std::clamp(t, 0, maxIdx);
    }
    const int size = maxIdx + 1;
    return ((t % size) + size) % size;
}
}

Texture::Texture(std::size_t w, std::size_t h, const uint32_t *bgra)
    : m_w(w), m_h(h){
    if(bgra){
        m_pixels.assign(bgra, bgra + w * h);
    }
}

uint32_t Texture::fetchTexel(int tx, int ty, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;
    const int cx = WrapTexel(tx, static_cast<int>(m_w) - 1, wrap);
    const int cy = WrapTexel(ty, static_cast<int>(m_h) - 1, wrap);
    return m_pixels[static_cast<std::size_t>(cy) * m_w + static_cast<std::size_t>(cx)];
}

uint32_t Texture::sample(double u, double v,
                         TextureFilter /*filter*/, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;

    const int tx = static_cast<int>(std::floor(u * static_cast<double>(m_w)));
    const int ty = static_cast<int>(std::floor(v * static_cast<double>(m_h)));
    return fetchTexel(tx, ty, wrap);
}
```

说明：本任务 sample 一律走 Nearest 行为，`filter` 参数以 `/*filter*/` 抑制未用告警；Task 2 恢复参数名并加 Bilinear 分支，同时把默认值改为 `Bilinear`（对齐 spec §1）。`loadFromFile` 仅声明无定义，无调用点故不产生链接需求。

修改 `src/Render/CMakeLists.txt` 的源文件列表：

```cmake
set(SGE_RENDER_SRC_FILES
    ${SGE_RENDER_DIR}/Rasterizer.cpp
    ${SGE_RENDER_DIR}/Pipeline.cpp
    ${SGE_RENDER_DIR}/Texture.cpp
)
```

- [ ] **Step 4: 构建并运行，验证通过**

```bash
cmake --build build -j$(nproc) --target render_Texture && ./build/test/render_Texture
```

预期：5 项测试全部 PASS。

- [ ] **Step 5: 回归既有套件**

```bash
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```

预期：无 FAIL 输出。

- [ ] **Step 6: 提交**

```bash
git add src/Render/Texture.hpp src/Render/Texture.cpp src/Render/CMakeLists.txt test/render/Texture.cpp
git commit -m "feat(render): Texture class with memory constructor and nearest sampling"
```

---

### Task 2: 双线性过滤

**Files:**
- Modify: `src/Render/Texture.cpp`（sample 增加 Bilinear 分支、恢复 filter 参数名、默认值改 Bilinear）
- Modify: `src/Render/Texture.hpp`（默认参数改 `TextureFilter::Bilinear`）

**Interfaces:**
- Consumes: Task 1 的 `fetchTexel(int,int,TextureWrap)` 与 `TextureFilter/TextureWrap`
- Produces: `sample()` 完整语义——Bilinear 用半像素偏移 `fx = u*w - 0.5`，对 `(floor(fx), floor(fy))` 与 `(+1,+1)` 四 texel 按 `ax/ay` 小数部分加权，通道用 `std::lround` 舍入；wrap 应用于全部 4 个 texel 坐标

- [ ] **Step 1: 写失败测试（追加到 `test/render/Texture.cpp`，放在 main 之前）**

```cpp
TEST(TextureTest, BilinearExactAtTexelCenter){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(0.25, 0.25, TextureFilter::Bilinear), kRed);
    EXPECT_EQ(t.sample(0.75, 0.25, TextureFilter::Bilinear), kGreen);
}

TEST(TextureTest, BilinearBlendGrayLevels){
    const uint32_t px[4] = {kGray(100), kGray(200), kGray(0), kGray(200)};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(0.375, 0.25, TextureFilter::Bilinear), kGray(125));
    EXPECT_EQ(t.sample(0.5,   0.5,  TextureFilter::Bilinear), kGray(125));
}
```

数值依据：`fx=0.375*2-0.5=0.25, fy=0`（二进制精确）→ 行混合 `100*0.75+200*0.25=125`，行下权重 0。中心 `(0.5,0.5)`：top=`100*0.5+200*0.5=150`，bot=`0*0.5+200*0.5=100`，结果 `125`。texel 中心处 ax/ay=0，精确还原单 texel。

- [ ] **Step 2: 构建，验证失败**

```bash
cmake --build build -j$(nproc) --target render_Texture && ./build/test/render_Texture
```

预期：FAIL——两个 Bilinear 测试得到 nearest 结果（如 `kGray(100)`/`kGray(150)` 等）而非 125。

- [ ] **Step 3: 实现 Bilinear 分支**

`src/Render/Texture.cpp` 匿名 namespace 内追加通道混合辅助：

```cpp
uint8_t BlendChannel(uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3,
                     double ax, double ay, int shift){
    const double top = static_cast<double>((c0 >> shift) & 0xFF) * (1.0 - ax)
                     + static_cast<double>((c1 >> shift) & 0xFF) * ax;
    const double bot = static_cast<double>((c2 >> shift) & 0xFF) * (1.0 - ax)
                     + static_cast<double>((c3 >> shift) & 0xFF) * ax;
    return static_cast<uint8_t>(std::lround(top * (1.0 - ay) + bot * ay));
}
```

`sample()` 整体替换为：

```cpp
uint32_t Texture::sample(double u, double v,
                         TextureFilter filter, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;

    if(filter == TextureFilter::Nearest){
        const int tx = static_cast<int>(std::floor(u * static_cast<double>(m_w)));
        const int ty = static_cast<int>(std::floor(v * static_cast<double>(m_h)));
        return fetchTexel(tx, ty, wrap);
    }

    const double fx = u * static_cast<double>(m_w) - 0.5;
    const double fy = v * static_cast<double>(m_h) - 0.5;
    const int x0 = static_cast<int>(std::floor(fx));
    const int y0 = static_cast<int>(std::floor(fy));
    const double ax = fx - static_cast<double>(x0);
    const double ay = fy - static_cast<double>(y0);

    const uint32_t c00 = fetchTexel(x0,     y0,     wrap);
    const uint32_t c10 = fetchTexel(x0 + 1, y0,     wrap);
    const uint32_t c01 = fetchTexel(x0,     y0 + 1, wrap);
    const uint32_t c11 = fetchTexel(x0 + 1, y0 + 1, wrap);

    auto pack = [](uint8_t a, uint8_t r, uint8_t g, uint8_t b) -> uint32_t {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
                static_cast<uint32_t>(b);
    };
    return pack(BlendChannel(c00,c10,c01,c11,ax,ay,24),
                BlendChannel(c00,c10,c01,c11,ax,ay,16),
                BlendChannel(c00,c10,c01,c11,ax,ay,8),
                BlendChannel(c00,c10,c01,c11,ax,ay,0));
}
```

同步把 `src/Render/Texture.hpp` 中默认参数改为：

```cpp
                    TextureFilter filter = TextureFilter::Bilinear,
```

- [ ] **Step 4: 构建并运行，验证通过**

```bash
cmake --build build -j$(nproc) --target render_Texture && ./build/test/render_Texture
```

预期：7 项测试全部 PASS。

- [ ] **Step 5: 提交**

```bash
git add src/Render/Texture.hpp src/Render/Texture.cpp test/render/Texture.cpp
git commit -m "feat(render): bilinear texture filtering with half-pixel offsets"
```

---

### Task 3: stb_image 接入与 loadFromFile

**Files:**
- Create: `third_party/stb/stb_image.h`（下载 vendor）
- Create: `test/assets/test_4x4.png`（脚本生成后提交）
- Modify: `src/include.cmake`（追加 include 路径）
- Modify: `src/Render/Texture.cpp`（实现 loadFromFile）
- Test: `test/render/Texture.cpp` 追加 2 个测试

**Interfaces:**
- Consumes: Task 1 的 `Texture(std::size_t,std::size_t,const uint32_t*)` 构造与空纹理语义
- Produces: `static Texture loadFromFile(const std::string&)`——失败返回 `w=h=0` 空纹理；成功时像素为 RGBA→BGRA 打包

- [ ] **Step 1: vendor stb_image.h 并接入构建**

```bash
mkdir -p third_party/stb
curl -sL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -o third_party/stb/stb_image.h
head -20 third_party/stb/stb_image.h
```

验证输出包含 `stb_image - v2.x` 与 `#define STBI_VERSION`。

`src/include.cmake` 末尾追加：

```cmake
include_directories(${CMAKE_SOURCE_DIR}/third_party)
```

- [ ] **Step 2: 生成测试 PNG 资产**

```bash
mkdir -p test/assets && python3 - <<'PYEOF'
import zlib, struct
W = H = 4
rows = []
for y in range(H):
    row = bytearray([0])
    for x in range(W):
        row += bytes([x*60, y*60, 128, 255])
    rows.append(bytes(row))
raw = b''.join(rows)
def chunk(tag, data):
    c = tag + data
    return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c))
png  = b'\x89PNG\r\n\x1a\n'
png += chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 6, 0, 0, 0))
png += chunk(b'IDAT', zlib.compress(raw))
png += chunk(b'IEND', b'')
open('test/assets/test_4x4.png','wb').write(png)
print('written', len(png), 'bytes')
PYEOF
```

像素约定：RGBA8（color type 6），pixel(x,y) = (r=x*60, g=y*60, b=128, a=255)。

- [ ] **Step 3: 写失败测试（追加到 main 前）**

```cpp
TEST(TextureTest, LoadFromFile){
    Texture t = Texture::loadFromFile("test/assets/test_4x4.png");
    ASSERT_EQ(t.width(), 4u);
    ASSERT_EQ(t.height(), 4u);

    for(int y = 0; y < 4; y++){
        for(int x = 0; x < 4; x++){
            const uint32_t expect = 0xFF000000u
                                  | (static_cast<uint32_t>(x*60) << 16)
                                  | (static_cast<uint32_t>(y*60) << 8)
                                  | 128u;
            const uint32_t got = t.sample((x + 0.5) / 4.0, (y + 0.5) / 4.0,
                                          TextureFilter::Nearest, TextureWrap::Clamp);
            EXPECT_EQ(got, expect) << "x=" << x << " y=" << y;
        }
    }
}

TEST(TextureTest, LoadFromFileMissingGivesEmpty){
    Texture t = Texture::loadFromFile("no_such_file.png");
    EXPECT_EQ(t.width(), 0u);
    EXPECT_EQ(t.sample(0.5, 0.5), kBlack);
}
```

注意：必须从仓库根目录运行二进制（相对路径 `test/assets/...`）。

- [ ] **Step 4: 构建，验证失败**

```bash
cmake -B build -S . && cmake --build build -j$(nproc) --target render_Texture
```

预期：链接失败——`loadFromFile` 无定义（undefined reference）。

- [ ] **Step 5: 实现 loadFromFile**

`src/Render/Texture.cpp` 的 include 区末尾追加：

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```

类外追加实现：

```cpp
Texture Texture::loadFromFile(const std::string &path){
    int w = 0, h = 0, n = 0;
    stbi_uc *data = stbi_load(path.c_str(), &w, &h, &n, 4);
    if(!data || w <= 0 || h <= 0){
        if(data) stbi_image_free(data);
        return Texture{};
    }

    std::vector<uint32_t> pixels(static_cast<std::size_t>(w) * h);
    for(std::size_t i = 0; i < pixels.size(); i++){
        pixels[i] = (static_cast<uint32_t>(data[i*4+3]) << 24) |
                    (static_cast<uint32_t>(data[i*4+0]) << 16) |
                    (static_cast<uint32_t>(data[i*4+1]) << 8)  |
                     static_cast<uint32_t>(data[i*4+2]);
    }
    stbi_image_free(data);
    return Texture(static_cast<std::size_t>(w), static_cast<std::size_t>(h), pixels.data());
}
```

- [ ] **Step 6: 构建并从仓库根运行，验证通过**

```bash
cmake --build build -j$(nproc) --target render_Texture && ./build/test/render_Texture
```

预期：9 项测试全部 PASS。

- [ ] **Step 7: 提交（含 vendor 头与二进制资产）**

```bash
git add third_party/stb/stb_image.h src/include.cmake src/Render/Texture.cpp test/render/Texture.cpp test/assets/test_4x4.png
git commit -m "feat(render): texture loading via vendored stb_image with PNG test asset"
```

---

### Task 4: ScreenVertex/PolyF4D 增加 UV 并透传管线

**Files:**
- Modify: `src/Render/Rasterizer.hpp`（ScreenVertex 加字段）
- Modify: `src/Render/GeoObject/PolyF4D.hpp`（UV2D + uvlist）
- Modify: `src/Render/Pipeline.cpp`（projectObject 透传，约 76-81 行顶点填充处）
- Test: Modify `test/render/PipelineTest.cpp`

**Interfaces:**
- Consumes: 既有 `PolyF4D::vlist[kTriangleVerticesNumber]`（常量 `kTriangleVerticesNumber`=3 已在该头文件定义）；`projectObject` 顶点循环内已有的 `sv[i].color = poly.color;` 锚点
- Produces（Task 5/6 依赖）:
  - `struct ScreenVertex{ double x; double y; float z; float w; float u = 0; float v = 0; Color32 color{}; };`
  - `struct UV2D{ double u{}; double v{}; };` 与 `PolyF4D::uvlist[kTriangleVerticesNumber]{}`
  - 不变量：projectObject 对 UV 只透传不变换

- [ ] **Step 1: 写失败测试（追加到 PipelineTest.cpp 的 BehindCameraProducesNothing 之后）**

```cpp
TEST(PipelineProjectTest, UvPassthrough){
    Object4D obj{};
    obj.numVertices = 4;
    Point4D v[4] = {{-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1}};
    for(int i = 0; i < 4; i++) obj.vlistLocal[i] = v[i];
    obj.numPolys = 1;
    obj.plist[0].vlist[0] = v[0];
    obj.plist[0].vlist[1] = v[3];
    obj.plist[0].vlist[2] = v[2];
    obj.plist[0].uvlist[0] = {0.0, 0.0};
    obj.plist[0].uvlist[1] = {0.25, 0.5};
    obj.plist[0].uvlist[2] = {1.0, 1.0};

    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0, 0, -5}, Vector3DBase<double>{0, 0, 0},
        Vector3DBase<double>{0, 1, 0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto mvp = proj.mul(view);

    auto tris = Pipeline::projectObject(obj, mvp, 800, 600);
    ASSERT_EQ(tris.size(), 1u);
    EXPECT_FLOAT_EQ(tris[0].v[0].u, 0.0f);
    EXPECT_FLOAT_EQ(tris[0].v[0].v, 0.0f);
    EXPECT_FLOAT_EQ(tris[0].v[1].u, 0.25f);
    EXPECT_FLOAT_EQ(tris[0].v[1].v, 0.5f);
    EXPECT_FLOAT_EQ(tris[0].v[2].u, 1.0f);
    EXPECT_FLOAT_EQ(tris[0].v[2].v, 1.0f);
}
```

绕序依据：front 面 `(v0,v3,v2)` 是 Demo 中已验证可见的外法线面表（MakeCube faces[0] 同款）。

- [ ] **Step 2: 构建验证失败**

```bash
cmake --build build -j$(nproc) --target render_PipelineTest 2>&1 | tail -5
```

预期：编译错误——`uvlist`/`.u/.v` 字段不存在。

- [ ] **Step 3: 实现**

`src/Render/GeoObject/PolyF4D.hpp`（`kTriangleVerticesNumber` 定义之后、`class PolyF4D` 之前插入结构，类内追加成员）：

```cpp
struct UV2D{
    double u{};
    double v{};
};
```

```cpp
    Point4D tvlist[kTriangleVerticesNumber];
    UV2D uvlist[kTriangleVerticesNumber]{};
```

`src/Render/Rasterizer.hpp` 的 `ScreenVertex` 替换为：

```cpp
struct ScreenVertex{
    double x = 0;
    double y = 0;
    float z = 0;
    float w = 1;
    float u = 0;
    float v = 0;
    Color32 color{};
};
```

`src/Render/Pipeline.cpp` 的 `projectObject` 顶点循环中，在 `sv[i].color = poly.color;` 之后追加：

```cpp
            sv[i].u = static_cast<float>(poly.uvlist[i].u);
            sv[i].v = static_cast<float>(poly.uvlist[i].v);
```

- [ ] **Step 4: 构建运行，验证通过 + 回归**

```bash
cmake --build build -j$(nproc) --target render_PipelineTest && ./build/test/render_PipelineTest
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```

预期：PipelineTest 7 项 PASS；回归无 FAIL。

- [ ] **Step 5: 提交**

```bash
git add src/Render/Rasterizer.hpp src/Render/GeoObject/PolyF4D.hpp src/Render/Pipeline.cpp test/render/PipelineTest.cpp
git commit -m "feat(render): UV attributes through ScreenVertex, PolyF4D and projectObject"
```

---

### Task 5: drawTriangleTextured 透视校正贴图光栅化

**Files:**
- Modify: `src/Render/Rasterizer.hpp`（include Texture.hpp + 方法声明）
- Modify: `src/Render/Rasterizer.cpp`（实现）
- Test: Modify `test/render/Rasterizer.cpp`

**Interfaces:**
- Consumes: Task 1-2 `Texture::sample(double,double,TextureFilter,TextureWrap)->uint32_t`；Task 4 `ScreenVertex::u/v`；既有匿名 namespace 的 `EdgeFunction/IsTopLeftEdge` 与 `drawTriangleSolid` 骨架
- Produces: `void drawTriangleTextured(const ScreenVertex&, const ScreenVertex&, const ScreenVertex&, const Texture&, TextureFilter filter = TextureFilter::Bilinear, TextureWrap wrap = TextureWrap::Repeat);`——深度语义与 solid 一致（clip-space w 校正，z-buffer 存校正后深度）

- [ ] **Step 1: 写失败测试（追加到 test/render/Rasterizer.cpp 末尾 main 之前；文件顶部补 `#include "Texture.hpp"`）**

```cpp
TEST(RasterTexturedTest, PerspectiveCorrectNotAffine){
    FrameBuffer fb(24, 24);
    Rasterizer rz{fb};

    uint32_t px[8];
    for(int j = 0; j < 8; j++){
        px[j] = 0xFF000000u | (static_cast<uint32_t>(j * 30) << 16);
    }
    Texture tex(8, 1, px);

    Color32 white{255, 255, 255, 255};
    ScreenVertex a{}, b{}, c{};
    a.x = 2;  a.y = 2;  a.w = 2; a.u = 0; a.z = -2; a.color = white;
    b.x = 18; b.y = 2;  b.w = 2; b.u = 1; b.z = -2; b.color = white;
    c.x = 2;  c.y = 18; c.w = 1; c.u = 0; c.z = -1; c.color = white;

    rz.drawTriangleTextured(a, b, c, tex, TextureFilter::Nearest, TextureWrap::Clamp);

    // 像素(6,6)采样点(6.5,6.5)：lambda=(0.4375, 0.28125, 0.28125)
    // 透视校正 u = (0.28125/2)/(0.4375/2 + 0.28125/2 + 0.28125/1) = 9/41 ≈ 0.2195 -> texel1 -> r=30
    // 仿射插值 u = 0.28125 -> texel2 -> r=60（错误实现会得此值）
    const uint32_t got = fb.colorData()[6 * 24 + 6];
    EXPECT_EQ(got, 0xFF001E00u);
}

TEST(RasterTexturedTest, QuadDiagonalUvContinuity){
    FrameBuffer fb(14, 14);
    Rasterizer rz{fb};

    std::vector<uint32_t> px(200);
    for(int j = 0; j < 200; j++){
        px[j] = 0xFF000000u | (static_cast<uint32_t>(j) << 16);
    }
    Texture tex(200, 1, px.data());

    Color32 white{255, 255, 255, 255};
    ScreenVertex A{}, B{}, C{}, D{};
    A.x = 2;  A.y = 2;  A.w = 1; A.u = 0; A.v = 0; A.z = -1; A.color = white;
    B.x = 10; B.y = 2;  B.w = 1; B.u = 1; B.v = 0; B.z = -1; B.color = white;
    C.x = 10; C.y = 10; C.w = 1; C.u = 1; C.v = 1; C.z = -1; C.color = white;
    D.x = 2;  D.y = 10; D.w = 1; D.u = 0; D.v = 1; D.z = -1; D.color = white;

    rz.drawTriangleTextured(A, B, C, tex, TextureFilter::Nearest, TextureWrap::Clamp);
    rz.drawTriangleTextured(A, C, D, tex, TextureFilter::Nearest, TextureWrap::Clamp);

    // w=1 时透视校正退化为仿射：u(px) = ((px+0.5)-2)/8，r = floor(u*200)
    // 探针取 u*200=x.5 形式避开 floor 边界；三个探针分居对角线 AC 两侧
    auto expectR = [](int p){
        const double u = ((p + 0.5) - 2.0) / 8.0;
        return static_cast<int>(std::floor(u * 200.0));
    };
    const int probes[][2] = {{3, 5}, {7, 6}, {5, 8}};
    for(auto &pr : probes){
        const uint32_t got = fb.colorData()[pr[1] * 14 + pr[0]];
        EXPECT_EQ(static_cast<int>((got >> 16) & 0xFF), expectR(pr[0]))
            << "px=" << pr[0] << " py=" << pr[1];
    }
}
```

数值依据：探针 (3,5)：u=0.1875→37.5→37；(7,6)：u=0.6875→137.5→137；(5,8)：u=0.4375→87.5→87。两三角形对同一平面 UV 函数独立重建，跨对角线一致即连续性成立。

- [ ] **Step 2: 构建验证失败**

```bash
cmake --build build -j$(nproc) --target render_Rasterizer 2>&1 | tail -5
```

预期：编译错误——`drawTriangleTextured` 未声明。

- [ ] **Step 3: 实现完整函数体**

`src/Render/Rasterizer.hpp`：顶部 `#include "FrameBuffer.hpp"` 后追加 `#include "Texture.hpp"`；类内 `drawTriangleSolid` 声明后追加：

```cpp
    void drawTriangleTextured(const ScreenVertex &v0, const ScreenVertex &v1,
                              const ScreenVertex &v2, const Texture &tex,
                              TextureFilter filter = TextureFilter::Bilinear,
                              TextureWrap wrap = TextureWrap::Repeat);
```

`src/Render/Rasterizer.cpp` 末尾追加完整实现：

```cpp
void Rasterizer::drawTriangleTextured(const ScreenVertex &v0, const ScreenVertex &v1,
                                      const ScreenVertex &v2, const Texture &tex,
                                      TextureFilter filter, TextureWrap wrap){
    double area = EdgeFunction(v0.x,v0.y, v1.x,v1.y, v2.x,v2.y);
    if(area == 0) return;
    double invArea = 1.0 / area;

    double minX = std::min({v0.x, v1.x, v2.x});
    double maxX = std::max({v0.x, v1.x, v2.x});
    double minY = std::min({v0.y, v1.y, v2.y});
    double maxY = std::max({v0.y, v1.y, v2.y});
    int x0 = std::max(0, static_cast<int>(std::floor(minX)));
    int y0 = std::max(0, static_cast<int>(std::floor(minY)));
    int x1 = std::min(static_cast<int>(m_fb.width()) - 1,  static_cast<int>(std::ceil(maxX)));
    int y1 = std::min(static_cast<int>(m_fb.height()) - 1, static_cast<int>(std::ceil(maxY)));

    bool tl0 = IsTopLeftEdge(v1.x,v1.y, v2.x,v2.y);
    bool tl1 = IsTopLeftEdge(v2.x,v2.y, v0.x,v0.y);
    bool tl2 = IsTopLeftEdge(v0.x,v0.y, v1.x,v1.y);

    constexpr double eps = 1e-9;

    for(int y = y0; y <= y1; y++){
        for(int x = x0; x <= x1; x++){
            double px = x + 0.5, py = y + 0.5;
            double w0 = EdgeFunction(v1.x,v1.y, v2.x,v2.y, px,py) * invArea;
            double w1 = EdgeFunction(v2.x,v2.y, v0.x,v0.y, px,py) * invArea;
            double w2 = EdgeFunction(v0.x,v0.y, v1.x,v1.y, px,py) * invArea;

            auto inside = [&](double w, bool topLeft){
                return w > eps || (topLeft && w >= -eps);
            };
            if(!inside(w0, tl0) || !inside(w1, tl1) || !inside(w2, tl2)) continue;

            double iw = w0/v0.w + w1/v1.w + w2/v2.w;
            if(iw <= 0) continue;
            float zNdc = static_cast<float>(
                (w0*v0.z/v0.w + w1*v1.z/v1.w + w2*v2.z/v2.w) / iw);
            float uPix = static_cast<float>(
                (w0*v0.u/v0.w + w1*v1.u/v1.w + w2*v2.u/v2.w) / iw);
            float vPix = static_cast<float>(
                (w0*v0.v/v0.w + w1*v1.v/v1.w + w2*v2.v/v2.w) / iw);

            m_fb.setPixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y),
                          tex.sample(uPix, vPix, filter, wrap), zNdc);
        }
    }
}
```

- [ ] **Step 4: 构建运行，验证通过 + 全量回归**

```bash
cmake --build build -j$(nproc) --target render_Rasterizer && ./build/test/render_Rasterizer
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```

预期：Rasterizer 11 项 PASS；回归无 FAIL。

- [ ] **Step 5: 提交**

```bash
git add src/Render/Rasterizer.hpp src/Render/Rasterizer.cpp test/render/Rasterizer.cpp
git commit -m "feat(render): perspective-correct textured triangle rasterization"
```

---

### Task 6: Demo 贴图立方体 + 冒烟 + 视觉验收

**Files:**
- Modify: `src/Application.hpp`（m_checker 成员 + include）
- Modify: `src/Application.cpp`（棋盘格纹理、MakeCube UV、RenderCube 改贴图路径）
- Test: 冒烟 + 单帧视觉程序（仓库外 /tmp/opencode）

**Interfaces:**
- Consumes: Task 3 `Texture::loadFromFile/内存构造`；Task 5 `drawTriangleTextured`；Task 4 UV 透传；既有 `MakeCube()`（faces 表 `{a,b,c}` 对，i 偶数为 tri1 `{a,b,c}`、i+1 为 tri2 `{a,c,b'}` 型共享结构）
- Produces: 可运行的贴图旋转立方体 Demo

- [ ] **Step 1: Application.hpp 成员**

include 区追加 `#include "Render/Texture.hpp"`；私有成员区追加：

```cpp
    Texture m_checker{};
```

- [ ] **Step 2: Application.cpp 改造**

include 区追加 `#include "Render/Texture.hpp"` 与 `#include <vector>`。

匿名 namespace 内（MakeCube 之后）追加：

```cpp
Texture MakeCheckerTexture(){
    constexpr std::size_t kSide = 8;
    std::vector<uint32_t> px(kSide * kSide);
    for(std::size_t y = 0; y < kSide; y++){
        for(std::size_t x = 0; x < kSide; x++){
            px[y * kSide + x] = (((x >> 1) + (y >> 1)) % 2 == 0)
                              ? 0xFFFFFFFFu : 0xFF202020u;
        }
    }
    return Texture(kSide, kSide, px.data());
}
```

`MakeCube()` 的 `for(i<12)` 面表赋值循环之后追加（每面四角 A(0,0) B(1,0) C(1,1) D(0,1)，对角线 AC 拆分：tri1=A,B,C；tri2=A,C,D）：

```cpp
    for(int i = 0;i < 12;i += 2){
        cube.plist[i].uvlist[0]   = {0, 0};
        cube.plist[i].uvlist[1]   = {1, 0};
        cube.plist[i].uvlist[2]   = {1, 1};
        cube.plist[i+1].uvlist[0] = {0, 0};
        cube.plist[i+1].uvlist[1] = {1, 1};
        cube.plist[i+1].uvlist[2] = {0, 1};
    }
```

`Application::initalize` 中 `m_cube = MakeCube();` 之后追加：

```cpp
    m_checker = MakeCheckerTexture();
```

`Application::RenderCube` 中替换绘制调用：

```cpp
    for(auto &t : Pipeline::projectObject(m_cube, mvp, 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], m_checker);
    }
```

- [ ] **Step 3: 全量构建 + 回归**

```bash
cmake --build build -j$(nproc)
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```

预期：构建零错误；回归无 FAIL。

- [ ] **Step 4: dummy 冒烟**

```bash
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
```

预期：输出 `Quit Normally` 或 timeout 杀死且无 crash/assert。

- [ ] **Step 5: 单帧视觉验收**

写 `/tmp/opencode/cube_tex_smoke.cpp`（与 Task 4 会话版 cube_smoke 相同骨架：MakeCube 含 uvlist 赋值同 Step 2 规则 + `MakeCheckerTexture` 同款 + `projectObject` + `drawTriangleTextured` + PPM 导出 + 非黑像素/颜色种类统计），编译运行：

```bash
clang++ -std=c++20 -I src -I src/math -I src/math/Matrix -I src/math/Primitive \
  -I src/math/Util -I src/math/Vector -I src/math/Geometry -I src/math/Math \
  -I src/Render -I src/Render/GeoObject \
  /tmp/opencode/cube_tex_smoke.cpp build/src/Render/librender.a build/src/math/libmath.a \
  -luuid -o /tmp/opencode/cube_tex_smoke && /tmp/opencode/cube_tex_smoke
```

再用会话中已有 python 片段把 PPM 转 PNG 后目视检查：立方体表面呈现 2×2 黑白棋盘格、近大远小处格纹无剪切扭曲、两面色块边界干净。

预期统计量级：可见三角形 4、非黑像素 ~40k、颜色种类 ≥3（黑白格 + 抗锯齿无、背景黑除外即黑白两色 + 可能的边缘混色）。

- [ ] **Step 6: 提交 + 更新 PROGRESS.md**

```bash
git add src/Application.hpp src/Application.cpp
git commit -m "feat(demo): textured rotating cube via checkerboard texture"
```

`PROGRESS.md` 表格追加一行（时间/日志=完成纹理映射/任务类型=功能/内容=Texture+UV 插值+贴图 Demo/备注=spec 与 plan 路径、测试计数）。

最终汇报以「完成了sir」结尾。

---

## Self-Review 记录

1. **Spec 覆盖**：§1 Texture/枚举/空语义→Task 1-3；§2 ScreenVertex/公式/drawTriangleTextured→Task 4-5；§3 数据流/Demo/测试策略→Task 4-6；CMake/include 路径→Task 1/3；PNG 资产→Task 3。无遗漏。
2. **占位符扫描**：所有步骤含完整代码/命令/数值；Task 1 的 `loadFromFile` 仅声明属任务间接口约定（Task 3 实现），非占位。
3. **类型一致性**：`sample` 默认参数 Task 1 为 Nearest、Task 2 起 Bilinear（计划内显式注明）；`fetchTexel(int,int,TextureWrap)` 私有签名贯穿 Task 1-3；`UV2D{double,double}` 与 `sv.u/v(float)` 转换点唯一（projectObject cast）。
4. **采样点陷阱**：所有光栅化断言按 `(x+0.5,y+0.5)` 中心采样推导（PerspectiveCorrectNotAffine λ=(112,72,72)/256@P(6.5,6.5) 已手工验算）。
