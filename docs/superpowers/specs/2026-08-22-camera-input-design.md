# 相机控制与输入设计（自由飞行相机 + 键盘轮询）

日期：2026-08-22
状态：已确认（用户授权连续执行模式）
前置：光照/视锥裁剪特性已完成；`Window::processEvent` 事件管道仅处理 QUIT

## 目标

为引擎加入自由飞行相机：键盘控制移动与转向，Demo 从固定机位变为可交互；输入→相机变换的核心逻辑为纯函数、可脱离 SDL 单测。

## 非目标（YAGNI）

- 鼠标捕获/视角跟随、滚轮变焦
- WindowEventType/listener 接口改造（保持事件管道不动）
- 惯性、碰撞、地形贴合
- 手柄输入

## 方案选择记录

| 方案 | 说明 | 结论 |
| ---- | ---- | ---- |
| A：SDL_GetKeyboardState 每帧轮询 | 不动事件管道；轮询→InputState 为薄适配层，「InputState+dt→Camera」为纯函数单测核心 | **采用** |
| B：扩展 WindowEventType+listener 签名带键值 | 接口侵入大，回调风暴 | 弃 |

## §1 Camera 组件

新文件 `src/Render/Camera.hpp/cpp`（render 库）：

```cpp
struct InputState{
    bool w{}, a{}, s{}, d{};        // 前后左右
    bool r{}, f{};                  // 升降
    bool left{}, right{};           // 偏航
    bool up{}, down{};              // 俯仰
};

class Camera{
public:
    Vector3DBase<double> position{0, 2, -6};
    double yaw{0.0};       // 绕 Y 偏航(弧度)，yaw=0 朝 +Z
    double pitch{-0.3217505543966422};  // 俯仰(弧度)，初始精确对准原点

    Vector3DBase<double> forward() const;
    Vector3DBase<double> right() const;
    Matrix4DBase<double> viewMatrix() const;
};

void update(Camera &cam, const InputState &in, double dt);
```

### 数学约定

- `forward = (sinYaw·cosPitch, sinPitch, cosYaw·cosPitch)` —— yaw=0 时朝 `+Z`，与 demo 初始机位 `(0,2,-6)→origin` 的视线方向一致（pitch 初值 `asin(-1/√10)` 使首帧成像与布光版逐像素一致）
- `right = normalize(cross(forward, {0,1,0}))` —— 恒水平且垂直于 forward；yaw=0 时为 `(-1,0,0)`，与既有渲染约定（lookAt 相机右方 = 世界 -X）吻合
- `viewMatrix = lookAt(position, position + forward, {0,1,0})`
- pitch 运行时 clamp 到 `(-π/2+0.01, π/2-0.01)` 防万向锁

### 输入映射（update 纯函数）

- 平移速度 `MOVE_SPEED = 3.0` 单位/秒；转向速度 `TURN_SPEED = 1.5` 弧度/秒
- `W/S`: ±forward·speed·dt；`A/D`: ∓right·speed·dt；`R/F`: ±{0,1,0}·speed·dt
- `←/→`: yaw ∓ turn·dt；`↑/↓`: pitch ± turn·dt（先加后 clamp）

## §2 Application 集成

- 成员追加 `Camera m_camera{};`（默认值即初始位姿）
- `run()` 循环内 processEvent 之后：

```cpp
int kbCount = 0;
const Uint8 *kb = SDL_GetKeyboardState(&kbCount);
InputState in{};
if(kbCount > 0){
    in.w = kb[SDL_SCANCODE_W] != 0;   in.s = kb[SDL_SCANCODE_S] != 0;
    in.a = kb[SDL_SCANCODE_A] != 0;   in.d = kb[SDL_SCANCODE_D] != 0;
    in.r = kb[SDL_SCANCODE_R] != 0;   in.f = kb[SDL_SCANCODE_F] != 0;
    in.left = kb[SDL_SCANCODE_LEFT] != 0;  in.right = kb[SDL_SCANCODE_RIGHT] != 0;
    in.up = kb[SDL_SCANCODE_UP] != 0;      in.down = kb[SDL_SCANCODE_DOWN] != 0;
}
SGE::Render::update(m_camera, in, frameDtSeconds);
```

帧间 dt 由现有 high_resolution_clock 计算（FPS 统计同源），首帧 dt 取 0。

- `RenderCube()` 中 `view = SGE::Math::lookAt(...)` 替换为 `auto view = m_camera.viewMatrix();`；ShadingContext 的 viewPos 改用 `m_camera.position`
- dummy driver 下键盘状态恒零 → 相机静止 → 成像应与布光版基线**逐字节一致**（视觉回归验证点）

## §3 测试策略与验收

新增 `test/render/Camera.cpp`（目标 render_Camera）：

| 测试 | 验证点 |
| ---- | ---- |
| Camera.ForwardYawZeroFacesPlusZ | yaw=pitch=0 → forward=(0,0,1) |
| Camera.ForwardPitchTiltsTowardY | pitch=π/2 → forward≈(0,1,0)；pitch=-π/2 → ≈(0,-1,0) |
| Camera.RightIsHorizontalPerpendicular | yaw=π/2 → right≈(0,0,1)；right·forward=0 且 right.y=0 |
| Camera.ViewMatrixMatchesLookAt | viewMatrix 与 lookAt(pos,pos+f,{0,1,0}) 逐元素一致 |
| Camera.UpdateMovesAlongForward | 仅 W、dt=1 → position += 3·forward |
| Camera.UpdateStrafesAlongRight | 仅 D → position -= 3·right（D 向右 = -right 方向×速度？约定 D=+right·speed·dt，以实现为准在 plan 定稿）|
| Camera.UpdateYawTurnAndPitchClamp | ←→ 转 yaw 数值；连发 ↑ 后 pitch 不超上界 |

回归：全部既有套件照常通过。

### 验收标准

1. 全部测试绿（既有 + 新增 ~7 项）；
2. dummy driver 冒烟无 crash；
3. 单帧成像与布光版基线**逐字节一致**（初始位姿精确对准 + 键盘全零）。
