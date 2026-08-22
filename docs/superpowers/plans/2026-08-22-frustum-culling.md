# 完整视锥裁剪实施计划（六平面 Sutherland-Hodgman）

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将管线裁剪从单一近平面扩展为 clip 空间六平面（near/far/left/right/bottom/top）Sutherland-Hodgman 裁剪，属性精确插值，出画开销归零。

**Architecture:** 方案 A——`Pipeline.cpp` 内以 `FrustumPlane{a,b,c,d}` 统一表示半平面（内侧 `a·x+b·y+c·z+d·w ≥ 0`），`ClipPolygon` 单平面 S-H 循环六次，复用既有 `LerpClip` 全 12 属性分量插值，末次 fan 三角化。删除 `clipNearPlane` 双轨，`projectObject` 改调 `clipTriangle`。

**Tech Stack:** C++20 / clang++ / CMake / googletest release-1.12.1

**Spec:** `docs/superpowers/specs/2026-08-22-frustum-culling-design.md`

## Global Constraints

- 所有命令从仓库根 `/home/ares/workspace/SoftGameEngine` 运行
- clip 空间内点条件：六平面距离全部 `>= 0`（含等号边界）
- `LerpClip` 已插值 x/y/z/w/u/v/nx/ny/nz/wx/wy/wz 全部 12 分量——本特性不得改动其签名与公式
- 严格 TDD；无异常风格；不加注释（简报既有除外）；conventional commits
- 任务收尾更新 `PROGRESS.md`，回复以「完成了sir」结尾

## 文件结构

| 文件 | 动作 | 职责 |
| ---- | ---- | ---- |
| `src/Render/Pipeline.hpp` | Modify(T1) | clipNearPlane 声明 → clipTriangle |
| `src/Render/Pipeline.cpp` | Modify(T1) | FrustumPlane/PlaneDist/ClipPolygon/clipTriangle；删 clipNearPlane/NearDistance |
| `test/render/PipelineTest.cpp` | Modify(T1/T2) | 既有裁剪测试改造 + Frustum 系列 |
| `src/Application.cpp` | 不改动 | 经 projectObject 自动切换 |

---

### Task 1: clipTriangle 六平面重构与测试改造

**Files:**
- Modify: `src/Render/Pipeline.hpp`
- Modify: `src/Render/Pipeline.cpp`
- Test: `test/render/PipelineTest.cpp`

**Interfaces:**
- Consumes: 既有 `LerpClip(const ScreenVertex&, const ScreenVertex&, double t)`（匿名 namespace，全分量插值）；`ScreenTriangle{v[3]}`
- Produces:
  - `std::vector<ScreenTriangle> clipTriangle(const ScreenVertex (&tri)[3]);` —— 六平面裁剪唯一入口
  - 删除：`clipNearPlane`、`NearDistance`（LerpClip 保留）
  - `projectObject` 内部改调 `clipTriangle`，对外签名不变

- [ ] **Step 1: 写失败测试** — `test/render/PipelineTest.cpp` 改造：

(a) 既有 4 个裁剪测试整体替换（FullyInsideUnchanged / OneVertexBehindSplitsIntoTwo / FullyBehindDropped / ClipInterpolatesUv / ClipInterpolatesNormalAndWorld 五个删除），替换为：

```cpp
TEST(PipelineClipTest, FrustumFullyInsideKeepsOne){
    ScreenVertex tri[3]{};
    tri[0].x = -1;  tri[0].y = -1;   tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 1;   tri[1].y = -1;   tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 0;   tri[2].y = 1.5;  tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 1u);
}

TEST(PipelineClipTest, FrustumNearBehindDropped){
    ScreenVertex tri[3]{};
    for(int i = 0; i < 3; i++){
        tri[i].z = -2.0f; tri[i].w = 0.5f;
    }
    auto out = Pipeline::clipTriangle(tri);
    EXPECT_EQ(out.size(), 0u);
}

TEST(PipelineClipTest, FrustumOneVertexNearSplits){
    ScreenVertex tri[3]{};
    tri[0].x = -0.7; tri[0].y = -1;  tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 0.7;  tri[1].y = -1;  tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 0;    tri[2].y = 2;   tri[2].z = -0.5f; tri[2].w = -2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumLeftPlaneClips){
    ScreenVertex tri[3]{};
    tri[0].x = -3;  tri[0].y = 0;    tri[0].z = -0.5f; tri[0].w = 1;
    tri[1].x = 1;   tri[1].y = -1.5; tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 1;   tri[2].y = 1.5;  tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumCrossCornerSplits){
    ScreenVertex tri[3]{};
    tri[0].x = -2;  tri[0].y = -0.5; tri[0].z = -2; tri[0].w = 1;
    tri[1].x = 1;   tri[1].y = -1.5; tri[1].z = 1;  tri[1].w = 2;
    tri[2].x = 1;   tri[2].y = 1.5;  tri[2].z = 1;  tri[2].w = 2;

    auto out = Pipeline::clipTriangle(tri);
    ASSERT_FALSE(out.empty());

    auto inAll = [](const ScreenVertex &v){
        const double d[6] = {v.z+v.w, v.w-v.z, v.x+v.w, v.w-v.x, v.y+v.w, v.w-v.y};
        for(int i = 0; i < 6; i++){
            if(d[i] < -1e-9) return false;
        }
        return true;
    };
    std::size_t count = 0;
    for(auto &t : out){
        for(int i = 0; i < 3; i++){
            EXPECT_TRUE(inAll(t.v[i]));
            count++;
        }
    }
    EXPECT_LE(count, 9u);
}

TEST(PipelineClipTest, FrustumFarPlaneCulls){
    ScreenVertex tri[3]{};
    for(int i = 0; i < 3; i++){
        tri[i].z = 3.0f; tri[i].w = 1.0f;
    }
    auto out = Pipeline::clipTriangle(tri);
    EXPECT_EQ(out.size(), 0u);
}
```

数值依据：FullyInside 各点 `|x|,|y| ≤ 2`、`z+w=1.5`、`w-z=2.5` 全过；NearBehind 沿用语义；OneVertexNear 的近平面交点 `I12(t=0.375)=(0.4375,0.125,w=0.5)` 与 `I20(t=0.625)=(-0.4375,0.125,w=0.5)` 对左右面最紧余量 `±0.0625`，四边形过全部六面故恰 2 个三角形；LeftPlane 交点 `(-1.4,±0.6,w=1.4)` 恰在 left 边界内 → 2 个；CrossCorner 顶点 0 同时越 near(`z+w=-1`)与 left(`x+w=-1`)，仅作性质断言（多边形顶点上限 3+平面数）；Far `w-z=-2` 全出。

(b) 追加属性插值测试（合并原 UV 与法线两组验证，同 OneVertexNear 几何）：

```cpp
TEST(PipelineClipTest, FrustumClipInterpolatesAttributes){
    ScreenVertex tri[3]{};
    tri[0].x = -0.7; tri[0].y = -1; tri[0].z = -0.5f; tri[0].w = 2;
    tri[0].u = 0; tri[0].v = 0;
    tri[0].nx = 0; tri[0].ny = 0; tri[0].nz = -1;
    tri[0].wx = 1; tri[0].wy = 2; tri[0].wz = 3;
    tri[1].x = 0.7; tri[1].y = -1; tri[1].z = -0.5f; tri[1].w = 2;
    tri[1].u = 1; tri[1].v = 0;
    tri[1].nx = 1; tri[1].ny = 1; tri[1].nz = 1;
    tri[1].wx = 5; tri[1].wy = 6; tri[1].wz = 7;
    tri[2].x = 0; tri[2].y = 2; tri[2].z = -0.5f; tri[2].w = -2;
    tri[2].u = 0; tri[2].v = 1;
    tri[2].nx = 0; tri[2].ny = 1; tri[2].nz = 0;
    tri[2].wx = 9; tri[2].wy = 10; tri[2].wz = 11;

    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);

    int foundA = 0, foundB = 0;
    for(auto &t : out){
        for(int i = 0; i < 3; i++){
            const ScreenVertex &v = t.v[i];
            if(std::fabs(v.u - 0.625) < 1e-9 && std::fabs(v.v - 0.375) < 1e-9 &&
               std::fabs(v.nx - 0.625) < 1e-9 && std::fabs(v.ny - 1.0) < 1e-9 &&
               std::fabs(v.nz - 0.625) < 1e-9 &&
               std::fabs(v.wx - 6.5) < 1e-9 && std::fabs(v.wy - 7.5) < 1e-9 &&
               std::fabs(v.wz - 8.5) < 1e-9) foundA++;
            if(std::fabs(v.u) < 1e-9 && std::fabs(v.v - 0.375) < 1e-9 &&
               std::fabs(v.nx) < 1e-9 && std::fabs(v.ny - 0.375) < 1e-9 &&
               std::fabs(v.nz + 0.625) < 1e-9 &&
               std::fabs(v.wx - 4.0) < 1e-9 && std::fabs(v.wy - 5.0) < 1e-9 &&
               std::fabs(v.wz - 6.0) < 1e-9) foundB++;
        }
    }
    EXPECT_EQ(foundA, 2);
    EXPECT_EQ(foundB, 1);
}
```

数值依据：near 平面交点 `I12 = lerp(v1,v2,0.375)`：uv `(0.625,0.375)`、法线 `(0.625,1,0.625)`、世界 `(6.5,7.5,8.5)`——为扇形三角化共享顶点出现 2 次；`I20 = lerp(v2,v0,0.625)`：uv `(0,0.375)`、法线 `(0,0.375,-0.625)`、世界 `(4,5,6)` 出现 1 次。全部 t 为二进制精确分数，1e-9 容差安全。

- [ ] **Step 2: 构建验证失败**

```bash
cmake --build build -j$(nproc) --target render_PipelineTest 2>&1 | tail -3
```
预期：编译错误——`clipTriangle` 未声明。

- [ ] **Step 3: 实现**

`src/Render/Pipeline.hpp`：`clipNearPlane` 声明替换为：

```cpp
std::vector<ScreenTriangle> clipTriangle(const ScreenVertex (&tri)[3]);
```

`src/Render/Pipeline.cpp`：删除 `NearDistance` 与 `clipNearPlane` 整体，替换为：

```cpp
namespace{
struct FrustumPlane{ double a, b, c, d; };

double PlaneDist(const FrustumPlane &pl, const ScreenVertex &v){
    return pl.a*v.x + pl.b*v.y + pl.c*v.z + pl.d*v.w;
}

std::vector<ScreenVertex> ClipPolygon(const std::vector<ScreenVertex> &poly,
                                      const FrustumPlane &pl){
    std::vector<ScreenVertex> out{};
    for(std::size_t i = 0; i < poly.size(); i++){
        const ScreenVertex &cur = poly[i];
        const ScreenVertex &nxt = poly[(i+1)%poly.size()];
        const double dCur = PlaneDist(pl, cur);
        const double dNxt = PlaneDist(pl, nxt);
        const bool curIn = dCur >= 0;
        const bool nxtIn = dNxt >= 0;
        if(curIn) out.push_back(cur);
        if(curIn != nxtIn){
            const double t = dCur / (dCur - dNxt);
            out.push_back(LerpClip(cur, nxt, t));
        }
    }
    return out;
}
}

std::vector<ScreenTriangle> clipTriangle(const ScreenVertex (&tri)[3]){
    static const FrustumPlane kPlanes[6] = {
        {0,0,1,1}, {0,0,-1,1}, {1,0,0,1}, {-1,0,0,1}, {0,1,0,1}, {0,-1,0,1}};
    std::vector<ScreenVertex> poly{tri[0], tri[1], tri[2]};
    for(const auto &pl : kPlanes){
        if(poly.empty()) break;
        poly = ClipPolygon(poly, pl);
    }

    std::vector<ScreenTriangle> res{};
    for(std::size_t i = 1; i + 1 < poly.size(); i++){
        res.push_back(ScreenTriangle{poly[0], poly[i], poly[i+1]});
    }
    return res;
}
```

注意：`LerpClip` 保持在匿名 namespace 且定义于 `ClipPolygon` 之前（现文件顺序天然满足）；`projectObject` 内 `auto clipped = clipNearPlane(sv);` 改为 `auto clipped = clipTriangle(sv);`。

- [ ] **Step 4: 构建运行验证通过 + 全量回归**

```bash
cmake --build build -j$(nproc) --target render_PipelineTest && ./build/test/render_PipelineTest
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```
预期：PipelineTest 11 项 PASS（CubeFrontFaceProjectsCentered / BehindCameraProducesNothing / UvPassthrough / NormalPassthrough 4 项经 projectObject 自动切换无需改动 + 新 Frustum 系列 7 项）；回归无 FAIL。

- [ ] **Step 5: 提交**

```bash
git add src/Render/Pipeline.hpp src/Render/Pipeline.cpp test/render/PipelineTest.cpp
git commit -m "feat(render): full six-plane frustum clipping"
```

---

### Task 2: 回归验证与视觉一致性验收

**Files:**
- Test: 冒烟 + 单帧视觉程序（仓库外 /tmp/opencode）

**Interfaces:**
- Consumes: Task 1 `clipTriangle`；既有布光 Demo 全链路
- Produces: 视锥裁剪落地后 Demo 成像一致性证据

- [ ] **Step 1: 全量构建 + 回归**

```bash
cmake --build build -j$(nproc)
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```
预期：构建零错误；14 套件无 FAIL。

- [ ] **Step 2: dummy 冒烟**

```bash
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
```
预期：无 crash/assert（退出挂起为备案缺陷）。

- [ ] **Step 3: 单帧视觉一致性** — 复制 `/tmp/opencode/cube_lit_smoke.cpp` 为 `/tmp/opencode/cube_frustum_smoke.cpp` 编译运行（命令同上一特性：clang++ -I 各 include 目录 + librender.a/libmath.a -luuid），输出 PPM 转 PNG。

预期成像：与光照版本 cube_lit.png 一致——front 面最亮、侧面暗部、暖色点光、棋盘格透视无扭曲（demo 物体本就在视锥内，视觉应零变化；统计值 non_black≈42083 允许 ±1% 浮点抖动）。

- [ ] **Step 4: 提交 PROGRESS.md**

```bash
git add PROGRESS.md
git commit -m "docs: frustum culling progress"
```
若 Step 1-3 无代码变更则本步仅更新 PROGRESS.md（可与 Task 1 提交合并处理，由实现者按实际 diff 判断；无代码变化时不强制空提交）。

汇报以「完成了sir」结尾。

---

## Self-Review 记录

1. **Spec 覆盖**：§1 FrustumPlane/PlaneDist/ClipPolygon/clipTriangle/projectObject 切换/clipNearPlane 删除→T1；§2 测试表 8 项→T1（7 项 PipelineTest：FullyInside/NearBehind/OneVertexNear/LeftPlane/CrossCorner/FarPlane/ClipInterpolatesAttributes）+T2（Raster.OffscreenZeroPixels 语义由 FarPlaneCulls/NearBehindDropped 的空输出 + demo 零像素路径覆盖；spec 该行意图「完全出画零像素写入」由空裁剪输出直接保证）+ 回归行→T2。
2. **占位符扫描**：全部步骤含完整代码与推导数值；无 TBD。
3. **类型一致性**：`clipTriangle` 签名贯穿 T1 声明/实现/测试；`LerpClip` 未改动；既有 4 个 projectObject 测试无需改动。
4. **风险预控**：CrossCorner 采用性质断言规避角部退化（交点恰在边界导致重复顶点的 S-H 已知情况）；既有测试几何越出侧平面的问题已在设计期识别并重算全部数值。
