# Alpha 混合设计（src-over 半透明片元）

日期：2026-08-22
状态：已确认（用户授权连续执行模式）
前置：纹理映射特性已完成（tex.sample 输出含 alpha 通道的打包像素）

## 目标

FrameBuffer 支持半透明片元的 src-over 混合：纹理 alpha 生效于贴图三角形路径，透明区域不污染深度缓冲，为粒子/玻璃类渲染铺路。

## 非目标（YAGNI）

- 排序透明队列、premultiplied alpha、additive/乘法等其余混合模式
- drawLine/wireframe 路径（调试语义保持硬覆写）

## 方案要点

### FrameBuffer 扩展

```cpp
void blendPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth);
```

行为（按序）：
1. 越界直接返回；
2. `alpha == 0`：完全跳过（不写颜色**也不写深度**——全透明片元不应遮挡）；
3. 深度测试失败返回（与 setPixel 同规则 `depth >= m_depth[idx]` 拒绝）；
4. `alpha == 255`：等同 setPixel 直接覆写；
5. 中间 alpha：RGB 三通道按整数 src-over 混合 `out = (src*a + dst*(255-a) + 127) / 255`（确定性舍入）；**输出 alpha 恒 0xFF**（画布不透明语义），深度照常写入。

### Rasterizer 集成

`drawTriangleSolid` / `drawTriangleTextured` 的 `setPixel(...)` 调用改为 `blendPixel(...)`——flat 色路径 alpha 恒 255 行为不变（零回归），textured 路径纹理 alpha 开始生效。`plot()`（线框）不动。

## 测试策略（扩充 test/render/FrameBuffer.cpp 或新套件）

| 测试 | 验证点 |
| ---- | ---- |
| Blend.OpaqueMatchesSetPixel | α=255 时与 setPixel 输出逐字节一致 |
| Blend.SrcOverHalfAlpha | 黑底 + 灰128 α=128 → 各通道 ≈64（公式精确值 (128*128+0*127+127)/255=(16383+127)/255=64.46→64）|
| Blend.AlphaZeroSkipsBoth | α=0 后颜色不变且深度仍为 FLT_MAX（后续不透明片元可覆盖同像素）|
| Blend.DepthStillGuardsMidAlpha | 中间 α 片元深度更远时被拒绝 |
| Textured.SemiTransparentOverlay | 两三角形叠加：底层红 + 上层绿 α≈128 → 混合色数值断言 |
| 回归 | 全部既有套件通过 |

## 验收标准

1. 全部测试绿；2. dummy 冒烟无 crash；3. 布光立方体成像与基线一致（checkerboard 不透明 → 视觉零变化）。
