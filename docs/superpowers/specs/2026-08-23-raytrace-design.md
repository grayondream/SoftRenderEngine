# 光线追踪特性设计

日期：2026-08-23
状态：已确认（用户授权连续执行模式）

## 方案

新增独立 CPU 光线追踪器 `SGE::Render::RayTracer`，与既有光栅管线共存：

**核心组件**（`Render/RayTrace.hpp/.cpp`）
```cpp
struct RaySphere{ Vector3DBase<double> center; double radius; Color32 albedo; float reflectivity; };
struct RayTriangle{ Vector3DBase<double> a,b,c; Color32 albedo; float reflectivity; };
struct RayScene{ std::vector<RaySphere> spheres; std::vector<RayTriangle> triangles; };

struct RayTraceOptions{
    int maxDepth{3};        // 反射弹射上限
    Color32 background{};   // 未命中背景色
};

class RayTracer{
public:
    RayTracer(FrameBuffer &fb);
    void render(const RayScene &scene, const SGE::Render::Camera &camera,
                const LightingRig &rig, const RayTraceOptions &opt = {});
};
```

**算法**
- 相机：复用 Camera forward/right + perspective 视角参数（fovY=π/3），逐像素发射主射线
- 求交：球（二次方程）+ 三角形（Möller–Trumbore，双面）
- 着色：命中点法线 → **复用既有 `LightingRig`/`shade()`** 做直接光照；阴影射线检测任一求交体阻挡则 shadowFactor=0
- 反射：reflectivity>0 时按 maxDepth 迭代递减能量（attenuation *= reflectivity）
- 输出：blendPixel 写入 FrameBuffer（α=255），可继续走后处理/上屏链路

## 测试

| 测试 | 验证点 |
| ---- | ---- |
| SphereHitAndMiss | 中心命中/擦边未命中距离正确性 |
| TriangleHitMissParallel | MT 算法命中/未命中/平行退化 |
| ShadowRayBlocked | 两球排列，后方球在阴影区仅 ambient |
| MirrorReflection | reflectivity=1 平面镜映出另一球颜色 |
| EndToEndImage | 小图渲染非空且背景区域 == background |
