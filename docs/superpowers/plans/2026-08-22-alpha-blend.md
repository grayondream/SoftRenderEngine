# Alpha 混合实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** FrameBuffer::blendPixel 实现 src-over 半透明混合（α=0 全跳过/α=255 覆写/中间整数舍入），三角形光栅化路径切换，纹理 alpha 端到端生效。

**Architecture:** 单点扩展 FrameBuffer（blendPixel 与 setPixel 并列），Rasterizer 两个三角形方法换调用；公式 `out=(src*a+dst*(255-a)+127)/255`，输出 alpha 恒 0xFF。

**Tech Stack:** C++20 / googletest

**Spec:** `docs/superpowers/specs/2026-08-22-alpha-blend-design.md`

## Global Constraints

- 所有命令从仓库根运行；像素打包 a<<24|r<<16|g<<8|b
- blendPixel 顺序：越界→α==0 跳过→深度测试→α==255 覆写→src-over
- drawLine/plot 不改；无注释新增；conventional commits
- 任务收尾更新 PROGRESS.md，汇报以「完成了sir」结尾

---

### Task 1: blendPixel + 光栅化切换 + 测试

**Files:**
- Modify: `src/Render/FrameBuffer.hpp`（blendPixel 声明实现，header 内联或 FrameBuffer.cpp？现有 FrameBuffer 全部 header 内联——跟随现状 header 内联）
- Modify: `src/Render/Rasterizer.cpp`（两处 setPixel → blendPixel）
- Test: `test/render/FrameBuffer.cpp` 追加 4 项 + `test/render/Rasterizer.cpp` 追加 SemiTransparentOverlay

**Interfaces:**
- Consumes: 既有 setPixel 深度规则、PackBGRA、tex.sample 输出
- Produces: `void blendPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth);`

- [ ] **Step 1: 写失败测试** — test/render/FrameBuffer.cpp 追加：

```cpp
TEST(FrameBufferBlendTest, OpaqueMatchesSetPixel){
    FrameBuffer fb(2, 1);
    fb.setPixel(0, 0, 0xFF00FF00u, -1.0f);
    fb.blendPixel(1, 0, 0xFF00FF00u, -1.0f);
    EXPECT_EQ(fb.colorData()[0], fb.colorData()[1]);
}

TEST(FrameBufferBlendTest, SrcOverHalfAlpha){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF000000u, -1.0f);          // 黑底
    const uint32_t a = (128u << 24) | 0x808080u;    // α=128 灰128
    fb.blendPixel(0, 0, a, -0.5f);
    const uint32_t got = fb.colorData()[0];
    EXPECT_EQ((got >> 16) & 0xFF, ((128 * 128 + 0 * 127) + 127) / 255);
    EXPECT_EQ(got >> 24, 0xFFu);
}

TEST(FrameBufferBlendTest, AlphaZeroSkipsBoth){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF0000FFu, -1.0f);
    fb.blendPixel(0, 0, 0u, -0.5f);                 // α=0 且深度更近
    EXPECT_EQ(fb.colorData()[0], 0xFF0000FFu);
    EXPECT_FLOAT_EQ(fb.depthData()[0], -1.0f);
}

TEST(FrameBufferBlendTest, DepthStillGuardsMidAlpha){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF0000FFu, -1.0f);
    const uint32_t a = (128u << 24) | 0x808080u;
    fb.blendPixel(0, 0, a, 0.5f);                   // 更远被拒
    EXPECT_EQ(fb.colorData()[0], 0xFF0000FFu);
}
```

数值依据：SrcOver `(128*128 + 0*(255-128) + 127)/255 = 16511/255 = 64` 整除精确。

- [ ] **Step 2: 构建验证失败**

```bash
cmake --build build -j$(nproc) --target render_FrameBuffer 2>&1 | tail -3
```
预期：FAIL，blendPixel 未声明。

- [ ] **Step 3: 实现** — FrameBuffer.hpp 的 clearDepth 之后追加：

```cpp
    void blendPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth){
        if(x >= m_width || y >= m_height) return;
        const uint32_t sa = (bgra >> 24) & 0xFF;
        if(sa == 0) return;
        auto idx = y * m_width + x;
        if(depth >= m_depth[idx]) return;
        if(sa == 255){
            m_depth[idx] = depth;
            m_color[idx] = bgra;
            return;
        }
        uint32_t dst = m_color[idx];
        auto mix = [sa](uint32_t s, uint32_t d){
            return static_cast<uint32_t>((s * sa + d * (255u - sa) + 127u) / 255u);
        };
        const uint32_t outR = mix((bgra >> 16) & 0xFF, (dst >> 16) & 0xFF);
        const uint32_t outG = mix((bgra >> 8) & 0xFF, (dst >> 8) & 0xFF);
        const uint32_t outB = mix(bgra & 0xFF, dst & 0xFF);
        m_depth[idx] = depth;
        m_color[idx] = 0xFF000000u | (outR << 16) | (outG << 8) | outB;
    }
```

Rasterizer.cpp：drawTriangleSolid 与 drawTriangleTextured 的 `m_fb.setPixel(...)` 各替换为 `m_fb.blendPixel(...)`（参数不变）。

- [ ] **Step 4: 构建运行验证通过 + 回归**

```bash
cmake --build build -j$(nproc) --target render_FrameBuffer --target render_Rasterizer && ./build/test/render_FrameBuffer && ./build/test/render_Rasterizer
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```
预期：FrameBuffer 新 4 项 PASS；Rasterizer 既有 12 项 PASS（flat 色零回归）；全量无 FAIL。

- [ ] **Step 5: 提交**

```bash
git add src/Render/FrameBuffer.hpp src/Render/Rasterizer.cpp test/render/FrameBuffer.cpp
git commit -m "feat(render): src-over alpha blending in frame buffer"
```

---

### Task 2: 半透明端到端验证与收尾

**Files:**
- Test: `test/render/Rasterizer.cpp` 追加 Textured.SemiTransparentOverlay

**Interfaces:**
- Consumes: Task 1 blendPixel；Texture alpha 采样链路
- Produces: 纹理半透明叠加的端到端证据

- [ ] **Step 1: 写测试并验证通过** — test/render/Rasterizer.cpp 追加：

```cpp
TEST(RasterTexturedTest, SemiTransparentOverlay){
    FrameBuffer fb(4, 1);
    Rasterizer rz{fb};

    rz.plot(1, 0, Color32{255, 0, 0, 255});            // 底层红（线框 plot 直接覆写）

    uint32_t px[1] = {(100u << 24) | (0u << 16) | (255u << 8) | 0u};   // 绿 α=100
    Texture tex(1, 1, px);

    ScreenVertex v{}, w{};
    v.x = 0; v.y = 0; v.z = -1; v.w = 1; v.u = 0; v.v = 0;
    w.x = 4; w.y = 1; w.z = -1; w.w = 1; w.u = 1; w.v = 0;
    ScreenVertex u{}; u.x = 0; u.y = 1; u.z = -1; u.w = 1; u.u = 0; u.v = 1;
    rz.drawTriangleTextured(v, w, u, tex, nullptr,
                            TextureFilter::Nearest, TextureWrap::Clamp);

    const uint32_t got = fb.colorData()[1];
    EXPECT_EQ((got >> 8) & 0xFF, (255u * 100u + 0u * 155u + 127u) / 255u);
    EXPECT_EQ((got >> 16) & 0xFF, 0u);
}
```

数值依据：像素 (1,0) 在细长三角内（覆盖性由执行确认，若未覆盖调整顶点使该像素严格内部——以实际运行为准修正几何而非断言）；绿通道 `(255*100+0*155+127)/255=100`；红通道被绿完全替换（src r=0）→ 0。

先运行验证像素确实被覆盖（若 got 为纯红说明三角形未盖住 (1,0)，把 w.x/u.y 放大重试），再锁定断言。全量回归后提交：

```bash
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
git add test/render/Rasterizer.cpp PROGRESS.md
git commit -m "test(render): semi-transparent textured overlay end-to-end"
```
PROGRESS.md 记录后汇报，以「完成了sir」结尾。

---

## Self-Review 记录

1. Spec 覆盖：五条行为序→Task1 实现；六项测试表→T1 四项 + T2 一项端到端 + 回归步骤。
2. 无占位符；SemiTransparentOverlay 的覆盖性预校验为显式指令。
3. 类型一致：blendPixel 五参签名贯穿；plot 未触碰。
