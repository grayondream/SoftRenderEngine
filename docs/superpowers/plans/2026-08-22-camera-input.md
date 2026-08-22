# 相机控制与输入实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 自由飞行相机（WASD/RF 平移 + 方向键转向），键盘 SDL_GetKeyboardState 轮询驱动，核心逻辑纯函数可单测。

**Architecture:** 新增 `SGE::Render::Camera`（position/yaw/pitch + forward/right/viewMatrix）与 `update(cam, InputState, dt)` 纯函数；Application 每帧轮询键盘状态填充 InputState 并 update，RenderCube 改用 `m_camera.viewMatrix()` 与 `m_camera.position` 作视点。

**Tech Stack:** C++20 / clang++ / CMake / googletest / SDL2 keyboard state

**Spec:** `docs/superpowers/specs/2026-08-22-camera-input-design.md`

## Global Constraints

- 所有命令从仓库根 `/home/ares/workspace/SoftGameEngine` 运行
- forward = `(sinYaw·cosPitch, sinPitch, cosYaw·cosPitch)`；yaw=0 朝 +Z；初始 pitch=`-0.3217505543966422`（=asin(-1/√10)，精确对准原点）
- right = normalize(cross(forward, {0,1,0}))（Vector3DBase 的 `.mul()` 为叉积）；恒水平
- MOVE_SPEED=3.0 单位/秒、TURN_SPEED=1.5 弧度/秒；pitch 运行时 clamp ±(π/2−0.01)
- 按键约定：W/S ±forward、A −right/D +right、R/F ±worldUp、← yaw+ / → yaw−、↑ pitch+ / ↓ pitch−
- 向量加法/标量乘不假设存在于 Vector3DBase——Camera.cpp 内用局部分量 helper（Add）
- 无异常风格、不加注释、conventional commits；测试目标命名 test/render/Camera.cpp → render_Camera
- 任务收尾更新 PROGRESS.md，汇报以「完成了sir」结尾

## 文件结构

| 文件 | 动作 | 职责 |
| ---- | ---- | ---- |
| `src/Render/Camera.hpp` | Create(T1) | InputState/Camera/update 声明（SGE::Render）|
| `src/Render/Camera.cpp` | Create(T1) | 实现 |
| `src/Render/CMakeLists.txt` | Modify(T1) | 追加 Camera.cpp |
| `test/render/Camera.cpp` | Create(T1) | render_Camera 套件 7 项 |
| `src/Application.hpp/.cpp` | Modify(T2) | m_camera 成员、键盘轮询、RenderCube 改造 |

---

### Task 1: Camera 组件

**Files:**
- Create: `src/Render/Camera.hpp`, `src/Render/Camera.cpp`, `test/render/Camera.cpp`
- Modify: `src/Render/CMakeLists.txt`

**Interfaces:**
- Consumes: `Vector3DBase<double>`（`.mul()` 叉积、`.normalize()` const 版、构造 `{x,y,z}`）；`SGE::Math::lookAt(eye,center,up)->Matrix4DBase<double>`；`Matrix4DBase` 四层索引 `[0][0][r][c]`
- Produces（T2 依赖的精确签名）:
  - `struct InputState{ bool w{},a{},s{},d{}; bool r{},f{}; bool left{},right{}; bool up{},down{}; };`（命名空间 SGE::Render）
  - `class Camera{ public: Vector3DBase<double> position{0,2,-6}; double yaw{0}; double pitch{-0.3217505543966422}; Vector3DBase<double> forward() const; Vector3DBase<double> right() const; Matrix4DBase<double> viewMatrix() const; };`
  - `void update(Camera&, const InputState&, double dt);`

- [ ] **Step 1: 写失败测试** — 创建 `test/render/Camera.cpp`：

```cpp
#include "Camera.hpp"
#include <gtest/gtest.h>
#include <cmath>

namespace{
bool VecNear(const Vector3DBase<double> &v, double x, double y, double z, double eps = 1e-12){
    return std::fabs(v.x-x) < eps && std::fabs(v.y-y) < eps && std::fabs(v.z-z) < eps;
}
}

TEST(CameraTest, ForwardYawZeroFacesPlusZ){
    SGE::Render::Camera c{};
    c.yaw = 0; c.pitch = 0;
    EXPECT_TRUE(VecNear(c.forward(), 0, 0, 1));
}

TEST(CameraTest, ForwardPitchTiltsTowardY){
    SGE::Render::Camera c{};
    c.yaw = 0; c.pitch = 3.14159265358979 / 2;
    EXPECT_TRUE(VecNear(c.forward(), 0, 1, 0, 1e-9));
    c.pitch = -3.14159265358979 / 2;
    EXPECT_TRUE(VecNear(c.forward(), 0, -1, 0, 1e-9));
}

TEST(CameraTest, RightIsHorizontalPerpendicular){
    SGE::Render::Camera c{};
    c.yaw = 3.14159265358979 / 2; c.pitch = 0;
    EXPECT_TRUE(VecNear(c.right(), 0, 0, 1, 1e-9));

    c.yaw = 0.7; c.pitch = 0.3;
    const auto r = c.right();
    EXPECT_LT(std::fabs(r.y), 1e-12);
    const auto f = c.forward();
    EXPECT_NEAR(r.dot(f), 0.0, 1e-12);
}

TEST(CameraTest, ViewMatrixMatchesLookAt){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0.5, -2, 3};
    c.yaw = 0.7; c.pitch = -0.4;
    const auto f = c.forward();
    auto expected = SGE::Math::lookAt(c.position,
        Vector3DBase<double>{c.position.x + f.x, c.position.y + f.y, c.position.z + f.z},
        Vector3DBase<double>{0, 1, 0});
    auto actual = c.viewMatrix();
    for(int r = 0; r < 4; r++){
        for(int cc = 0; cc < 4; cc++){
            EXPECT_NEAR(actual[0][0][r][cc], expected[0][0][r][cc], 1e-9) << r << "," << cc;
        }
    }
}

TEST(CameraTest, UpdateMovesAlongForward){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    c.yaw = 0; c.pitch = 0;
    SGE::Render::InputState in{};
    in.w = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_TRUE(VecNear(c.position, 0, 0, 3));
}

TEST(CameraTest, UpdateStrafesAlongRight){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    c.yaw = 0; c.pitch = 0;
    SGE::Render::InputState in{};
    in.d = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_TRUE(VecNear(c.position, -3, 0, 0));
}

TEST(CameraTest, UpdateTurnAndPitchClamp){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    SGE::Render::InputState in{};
    in.left = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_NEAR(c.yaw, -1.5, 1e-12);

    in.left = false; in.up = true;
    for(int i = 0; i < 10; i++){
        SGE::Render::update(c, in, 1.0);
    }
    EXPECT_NEAR(c.pitch, 3.14159265358979 / 2 - 0.01, 1e-12);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

数值依据：D 键沿 right 移动，yaw=0 时 right=(-1,0,0) 故位移 (-3,0,0)；pitch 上限 π/2−0.01≈1.5608，连发 10×1.5 rad 触顶。

- [ ] **Step 2: 配置构建验证失败**

```bash
cmake -B build -S . && cmake --build build -j$(nproc) --target render_Camera 2>&1 | tail -3
```
预期：FAIL，`Camera.hpp` 不存在。

- [ ] **Step 3: 实现** — 创建 `src/Render/Camera.hpp`：

```cpp
#pragma once
#include "Transform.hpp"

namespace SGE::Render{

struct InputState{
    bool w{}, a{}, s{}, d{};
    bool r{}, f{};
    bool left{}, right{};
    bool up{}, down{};
};

class Camera{
public:
    Vector3DBase<double> position{0, 2, -6};
    double yaw{0.0};
    double pitch{-0.3217505543966422};

    Vector3DBase<double> forward() const;
    Vector3DBase<double> right() const;
    Matrix4DBase<double> viewMatrix() const;
};

void update(Camera &cam, const InputState &in, double dt);

}
```

创建 `src/Render/Camera.cpp`：

```cpp
#include "Camera.hpp"
#include <algorithm>
#include <cmath>

namespace SGE::Render{

namespace{
constexpr double kMoveSpeed = 3.0;
constexpr double kTurnSpeed = 1.5;
constexpr double kPitchLimit = 3.14159265358979323846 / 2 - 0.01;

Vector3DBase<double> Add(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x + b.x, a.y + b.y, a.z + b.z};
}
}

Vector3DBase<double> Camera::forward() const{
    return Vector3DBase<double>{
        std::sin(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::cos(yaw) * std::cos(pitch)};
}

Vector3DBase<double> Camera::right() const{
    return forward().mul(Vector3DBase<double>{0, 1, 0}).normalize();
}

Matrix4DBase<double> Camera::viewMatrix() const{
    return SGE::Math::lookAt(position,
                             Add(position, forward()),
                             Vector3DBase<double>{0, 1, 0});
}

void update(Camera &cam, const InputState &in, double dt){
    cam.yaw += ((in.left ? 1.0 : 0.0) - (in.right ? 1.0 : 0.0)) * kTurnSpeed * dt;
    cam.pitch += ((in.up ? 1.0 : 0.0) - (in.down ? 1.0 : 0.0)) * kTurnSpeed * dt;
    cam.pitch = std::clamp(cam.pitch, -kPitchLimit, kPitchLimit);

    if(in.w || in.s || in.a || in.d || in.r || in.f){
        const auto fwd = cam.forward();
        const auto rgt = cam.right();
        double dx = 0, dy = 0, dz = 0;
        if(in.w){ dx += fwd.x; dy += fwd.y; dz += fwd.z; }
        if(in.s){ dx -= fwd.x; dy -= fwd.y; dz -= fwd.z; }
        if(in.a){ dx -= rgt.x; dy -= rgt.y; dz -= rgt.z; }
        if(in.d){ dx += rgt.x; dy += rgt.y; dz += rgt.z; }
        if(in.r){ dy += 1.0; }
        if(in.f){ dy -= 1.0; }
        cam.position = Add(cam.position,
            Vector3DBase<double>{dx * kMoveSpeed * dt, dy * kMoveSpeed * dt, dz * kMoveSpeed * dt});
    }
}

}
```

修改 `src/Render/CMakeLists.txt` 源列表追加 `${SGE_RENDER_DIR}/Camera.cpp`。

- [ ] **Step 4: 构建运行验证通过**

```bash
cmake --build build -j$(nproc) --target render_Camera && ./build/test/render_Camera
```
预期：7 项 PASS。

- [ ] **Step 5: 回归 + 提交**

```bash
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
git add src/Render/Camera.hpp src/Render/Camera.cpp src/Render/CMakeLists.txt test/render/Camera.cpp
git commit -m "feat(render): free-fly camera with pure-function input update"
```

---

### Task 2: Application 集成与视觉一致性验收

**Files:**
- Modify: `src/Application.hpp`（成员 + include）
- Modify: `src/Application.cpp`（键盘轮询 + RenderCube 改造）
- Test: 冒烟 + PPM 逐字节对比（仓库外 /tmp/opencode）

**Interfaces:**
- Consumes: Task 1 全部产物；既有布光 Demo 渲染链路；`SDL_GetKeyboardState`/`SDL_SCANCODE_*`
- Produces: 可交互相机 Demo；dummy 下成像与基线逐字节一致

- [ ] **Step 1: 生成基线 PPM**（改动前）

```bash
clang++ -std=c++20 -I src -I src/math -I src/math/Matrix -I src/math/Primitive \
  -I src/math/Util -I src/math/Vector -I src/math/Geometry -I src/math/Math \
  -I src/Render -I src/Render/GeoObject \
  /tmp/opencode/cube_lit_smoke.cpp build/src/Render/librender.a build/src/math/libmath.a \
  -luuid -o /tmp/opencode/cube_cam_base && /tmp/opencode/cube_cam_base && mv /tmp/opencode/cube_lit.ppm /tmp/opencode/cube_cam_base.ppm
```
（若 cube_lit_smoke.cpp 已不存在，从 git 历史 Task 5 报告中的程序骨架重建；输出文件名按其 main 内硬编码为准调整 mv 目标。）

- [ ] **Step 2: Application 改造**

`src/Application.hpp` include 区追加 `#include "Render/Camera.hpp"`；私有成员追加：

```cpp
    SGE::Render::Camera m_camera{};
```

`src/Application.cpp`：
- include 区追加 `#include <SDL2/SDL.h>`
- `run()` 中 `const std::chrono::high_resolution_clock::time_point pt = ...` 行后追加：

```cpp
    auto lastFrameTime = pt;
```

循环内 `m_pwindow->processEvent();` 之后插入：

```cpp
        auto frameNow = std::chrono::high_resolution_clock::now();
        const double frameDt = std::chrono::duration<double>(frameNow - lastFrameTime).count();
        lastFrameTime = frameNow;

        int kbCount = 0;
        const Uint8 *kb = SDL_GetKeyboardState(&kbCount);
        InputState in{};
        if(kb && kbCount > 0){
            in.w = kb[SDL_SCANCODE_W] != 0;      in.s = kb[SDL_SCANCODE_S] != 0;
            in.a = kb[SDL_SCANCODE_A] != 0;      in.d = kb[SDL_SCANCODE_D] != 0;
            in.r = kb[SDL_SCANCODE_R] != 0;      in.f = kb[SDL_SCANCODE_F] != 0;
            in.left = kb[SDL_SCANCODE_LEFT] != 0;   in.right = kb[SDL_SCANCODE_RIGHT] != 0;
            in.up = kb[SDL_SCANCODE_UP] != 0;       in.down = kb[SDL_SCANCODE_DOWN] != 0;
        }
        SGE::Render::update(m_camera, in, frameDt);
```

`RenderCube()` 中：

```cpp
    auto view = SGE::Math::lookAt(Vector3DBase<double>{0, 2, -6},
                                  Vector3DBase<double>{0, 0, 0},
                                  Vector3DBase<double>{0, 1, 0});
```

替换为：

```cpp
    auto view = m_camera.viewMatrix();
```

且 `ShadingContext shading{&rig, Vector3DBase<double>{0, 2, -6}};` 替换为：

```cpp
    ShadingContext shading{&rig, m_camera.position};
```

- [ ] **Step 3: 全量构建 + 回归 + 冒烟**

```bash
cmake --build build -j$(nproc)
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
```
预期：零错误、无 FAIL、冒烟无 crash。

- [ ] **Step 4: PPM 逐字节视觉回归** — 复制 `/tmp/opencode/cube_lit_smoke.cpp` 为 `/tmp/opencode/cube_cam_smoke.cpp`，仅改两处：view 构造改为等价 Camera 默认位姿（`SGE::Render::Camera cam{}; auto view = cam.viewMatrix();`）、viewPos 用 `cam.position`；编译运行输出新 PPM 后：

```bash
cmp /tmp/opencode/cube_cam_base.ppm /tmp/opencode/<新PPM路径>
```
预期：逐字节一致（exit 0）。不一致则停止排查（不得以"接近"放行）。

- [ ] **Step 5: 提交 + PROGRESS.md**

```bash
git add src/Application.hpp src/Application.cpp
git commit -m "feat(demo): interactive free-fly camera via keyboard polling"
```
PROGRESS.md 追加记录；汇报以「完成了sir」结尾。

---

## Self-Review 记录

1. **Spec 覆盖**：§1 Camera/InputState/update 数学→T1；§2 集成/dt/键盘映射→T2；§3 七项测试→T1；视觉逐字节验收→T2。
2. **占位符扫描**：无 TBD；全部代码块完整。
3. **类型一致性**：`update(Camera&, const InputState&, double)` 贯穿 T1/T2；`Camera` 位于 `SGE::Render` 命名空间，T2 成员声明用全限定名。
4. **风险预控**：向量加法 API 不确定性→Add helper；dummy 键盘全零→静止相机保证逐字节可比。
