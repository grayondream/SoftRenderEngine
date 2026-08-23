# 雾效特性设计

日期：2026-08-23
状态：已确认（用户授权连续执行模式）

## 方案

线性雾（distance fog），参数挂载于既有 `ShadingContext`：

```cpp
struct FogParams{
    double start{};
    double end{};
    ColorFlt color{0.5f, 0.5f, 0.5f};
};
// ShadingContext 增加:
const FogParams *fog{};   // nullptr = 关闭（默认零破坏）
```

**计算**（`drawTriangleTextured` 像素循环内、光照着色后、blendPixel 前）：

- 距离 `d = |P - viewPos|`（P 为已透传的世界坐标 wx,wy,wz）
- `f = clamp((d - start) / (end - start), 0, 1)`
- 通道插值 `out = shaded + (fog - shaded) * f`（float 域，α 不变恒 255）

**范围**：textured 路径生效；solid/line 路径不受影响（备案后续可扩展）。

## 测试

1. FogNullptrIsNoOp：fog 缺省时输出与基线逐位一致
2. FogFarConvergesToColor：end=2 且物体距相机 10 → 像素 == 雾色
3. FogNearUnaffected：start=100 → 像素 == 无雾基线值

Demo（Application.cpp）启用温和雾（start=8,end=25,灰蓝）作视觉验收；golden 场景保持无雾不变。
