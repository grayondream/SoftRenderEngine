# 阴影特性设计（方向光 shadow mapping）

日期：2026-08-23
状态：已确认（用户授权连续执行模式）

## 方案

经典两遍 shadow mapping，仅支持单一 key 方向光：

**组件**
1. `Render/Shadow.hpp`：
   - `struct ShadowData{ const FrameBuffer *depth; Matrix4DBase<double> lightViewProj; double bias{0.005}; }`
   - `Matrix4DBase<double> directionalLightVP(dir, center, extent)`：光源 eye=center−normalize(dir)*extent*2，lookAt+正交投影 ±extent
2. `Rasterizer::drawTriangleDepth(a,b,c)`：仅深度写入的光栅化变体（重心覆盖判定与既有路径一致），用于 shadow pass
3. `ShadingContext` 增加 `const ShadowData *shadow{}`
4. `Light::shade(..., double shadowFactor = 1.0)`：漫反射+高光项乘 factor，ambient 不乘（签名向后兼容）
5. 主循环像素级：Pw 经 lightViewProj → NDC xy∈[-1,1]→texel 采样深度 dLight；NDC z 映射 [0,1] 后比较 `zMain > dLight + bias` → factor=0

**Shadow pass 流程**（调用方负责）：FrameBuffer(512,512).clearDepth() → 对每三角形 projectObject(lightVP) → drawTriangleDepth。

## 测试

1. LightVPLookAtCenter：光源 VP 把 center 映射至 NDC≈原点
2. DepthPassOcclusion：两三角形沿光轴前后排布 → 深度图近处值 < 远处
3. ShadowedPixelDarkens：遮挡场景中阴影区像素亮度 == 仅 ambient；无遮挡区正常光照
4. NoShadowContextUnchanged：shadow=nullptr 时输出与基线一致
