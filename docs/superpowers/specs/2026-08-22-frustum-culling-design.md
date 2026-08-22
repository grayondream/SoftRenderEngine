# 完整视锥裁剪设计（clip 空间六平面 Sutherland-Hodgman）

日期：2026-08-22
状态：已确认（方案 A）
前置：光照特性已完成（ScreenVertex 含 u/v/n*/w* 全属性、LerpClip 全分量插值）

## 目标

将管线裁剪从单一近平面扩展为完整视锥六平面裁剪：大三角形越出屏幕的开销归零，far 平面正确剔除，所有顶点属性在裁剪边界处精确插值。

## 非目标（YAGNI）

- 光栅化器 scissor 双保险（setPixel 边界防御已存在且零成本，保留即可）
- guard-band、多线程 tile 划分
- 物体级包围球预剔除（后续优化项）

## 方案选择记录

| 方案 | 说明 | 结论 |
| ---- | ---- | ---- |
| A：clip 空间多平面 Sutherland-Hodgman 泛化 | 六平面依次裁剪多边形，fan 三角化 | **采用**：一次做对 GPU 式裁剪，far 顺带解决，裁后 bbox 天然在屏内 |
| B：光栅化器 scissor bbox 钳制 | 仅钳制像素循环范围 | 弃：far 不解决、屏外插值数值不稳、治标 |

## §1 裁剪管线重构

### 统一表示

clip 空间半平面 `plane=(a,b,c,d)`，内侧条件 `a·x + b·y + c·z + d·w ≥ 0`。六平面：

```cpp
struct FrustumPlane{ double a, b, c, d; };
static const FrustumPlane kFrustumPlanes[6] = {
    {0,0,1,1},    // near:  z+w >= 0
    {0,0,-1,1},   // far:   w-z >= 0
    {1,0,0,1},    // left:  x+w >= 0
    {-1,0,0,1},   // right: w-x >= 0
    {0,1,0,1},    // bottom:y+w >= 0
    {0,-1,0,1},   // top:   w-y >= 0
};
```

### 新入口（Pipeline.cpp）

```cpp
double PlaneDist(const FrustumPlane &pl, const ScreenVertex &v){
    return pl.a*v.x + pl.b*v.y + pl.c*v.z + pl.d*v.w;
}

std::vector<ScreenVertex> ClipPolygon(const std::vector<ScreenVertex> &poly,
                                      const FrustumPlane &pl);
// Sutherland-Hodgman 单平面：curIn 入队；跨界 t = dCur/(dCur-dNxt) 调 LerpClip（全 12 属性分量复用）

std::vector<ScreenTriangle> clipTriangle(const ScreenVertex (&tri)[3]);
// poly = tri → 依序过 6 平面（poly 空提前退出）→ fan 三角化输出 0..n 个
```

### 既有代码处置

- `clipNearPlane` **删除**（避免双轨）；`projectObject` 改调 `clipTriangle`
- 既有 5 个裁剪测试几何有顶点越出左右平面（如 `x=4 > w=2`），六面全裁下会被二次裁剪破坏断言——测试几何收缩进侧边界并重算交点数值（实施计划给出精确值），各测试验证意图不变

## §2 测试策略与验收

| 测试 | 验证点 |
| ---- | ---- |
| Frustum.FullyInsideKeepsOne | 屏内三角形原样输出 1 个 |
| Frustum.NearBehindDropped | 近面之后全弃 |
| Frustum.OneVertexNearSplits | 近面裁剪 1→2 |
| Frustum.LeftPlaneClips | 左越界顶点被裁 1→2 |
| Frustum.CrossCornerSplits | 角部双平面相交 1→3（五顶点多边形）|
| Frustum.FarPlaneCulls | 远面之外全弃 |
| Frustum.ClipInterpolatesAttributes | 裁剪交点上 uv/法线/世界坐标按 t 精确 |
| Raster.OffscreenZeroPixels | 完全出画三角形零像素写入 |
| 回归 | projectObject 全链路既有套件 + Demo 冒烟 + 单帧视觉 |

### 验收标准

1. 全部测试绿（既有 + 新增）；
2. dummy driver 冒烟无 crash；
3. 布光立方体成像与上一版一致（demo 物体本就在视锥内，视觉应零变化）。
