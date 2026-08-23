# 防御性加固批次实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 清偿四项备案防御缺陷（Light range NaN / Texture nullptr UB / Camera dt 无界 / Testing 目录噪声），每项配判别测试。

**Architecture:** 三处生产代码各 1-2 行防御 + 对应测试；单任务批次。

**Spec:** `docs/superpowers/specs/2026-08-22-hardening-design.md`

## Global Constraints

- 所有命令从仓库根运行；无异常风格；conventional commits
- EXPECT_DEATH 需要 gtest 死亡测试支持（默认可用）；assert 在 NDEBUG 未定义时生效（当前构建无 -DNDEBUG ✓）

---

### Task 1: 四项加固与测试

**Files:**
- Modify: `src/Render/Light.cpp`、`src/Render/Texture.cpp`、`src/Render/Camera.cpp`、`.gitignore`
- Test: `test/render/Light.cpp`、`test/render/Camera.cpp`

**Interfaces:**
- Produces: 行为加固，既有签名全部不变

- [ ] **Step 1: 写失败测试**

test/render/Light.cpp 追加：

```cpp
TEST(LightTest, ZeroRangePointLightIgnored){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    PointLight pl{};
    pl.position = Vector3DBase<double>{0, 0, -5};
    pl.color = ColorFlt{1.0f, 1.0f, 1.0f};
    pl.range = 0.0;
    rig.point.push_back(pl);

    const uint32_t out = shade(rig, Color32{128,128,128,255},
                               Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                               Vector3DBase<double>{0,0,-5});
    EXPECT_EQ(out, 0xFF000000u);
}
```

test/render/Camera.cpp 追加：

```cpp
TEST(CameraTest, UpdateClampsLargeDt){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    SGE::Render::InputState in{};
    in.w = true;
    SGE::Render::update(c, in, 100.0);
    EXPECT_TRUE(VecNear(c.position, 0, 0, 0.3));

    SGE::Render::Camera c2{};
    c2.position = Vector3DBase<double>{0, 0, 0};
    SGE::Render::InputState neg{};
    neg.w = true;
    SGE::Render::update(c2, neg, -5.0);
    EXPECT_TRUE(VecNear(c2.position, 0, 0, 0));
}
```

```cpp
TEST(TextureDeathTest, NullptrNonZeroAsserts){
    EXPECT_DEATH(Texture t(4, 4, nullptr), "");
}
```
（放 test/render/Texture.cpp，include 区确认 `<gtest/gtest.h>` 已有；死亡测试套件名须为 `*DeathTest` 后缀组——gtest 约定 TEST 宏第二参即可。）

- [ ] **Step 2: 构建运行验证失败**（ZeroRange 输出不确定值或崩溃；ClampsLargeDt 得 (0,0,300)；Nullptr 用例不死）

- [ ] **Step 3: 实现**
- Light.cpp 点光循环首行：`if(pl.range <= 0) continue;`
- Texture.cpp 构造函数体首行：`assert(bgra != nullptr || (w == 0 && h == 0));`（补 `#include <cassert>`）
- Camera.cpp update 首行：`dt = std::clamp(dt, 0.0, 0.1);`（`<algorithm>` 已有）
- .gitignore 追加一行 `Testing/`

- [ ] **Step 4: 构建运行验证通过 + 全量回归 + 冒烟 + 提交**

```bash
cmake --build build -j$(nproc)
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
git add src/Render/Light.cpp src/Render/Texture.cpp src/Render/Camera.cpp .gitignore test/render/Light.cpp test/render/Camera.cpp test/render/Texture.cpp PROGRESS.md
git commit -m "fix(render): defensive hardening batch (light range, texture ctor, camera dt)"
```
PROGRESS.md 记录后汇报，以「完成了sir」结尾。
