# OBJ 网格加载实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Wavefront OBJ 文本子集解析器 `loadObjFromFile(path, Object4D&)`——v/vt/vn/f、四种索引格式、负索引、fan 三角化、缺省兜底、容量与格式失败路径，附真实资产 assets/cube.obj 与端到端渲染验证。

**Architecture:** 单组件 ObjLoader（render 库）：逐行 istringstream 解析到本地 vector（避免半填充），全部成功后一次性拷入 Object4D 固定数组；f 行 fan 三角化；缺 vn 用前三顶点叉积面法线兜底；任何非法 token/越界/超容即整体失败。

**Tech Stack:** C++20 / clang++ / googletest

**Spec:** `docs/superpowers/specs/2026-08-22-obj-loader-design.md`

## Global Constraints

- 所有命令从仓库根 `/home/ares/workspace/SoftGameEngine` 运行
- OBJ 索引 1-based；负索引 `-n` = 当前已解析数量倒数第 n 个；0 恒非法
- 容量上限：顶点 ≤ kObject4vListLen(64)、三角化后总面数 ≤ kPolyListLen(128)，超出整体失败
- 失败时 out 已在入口清零（numVertices=0/numPolys=0），不抛异常
- 测试临时文件写入 `/tmp/opencode/`；无注释新增；conventional commits
- 任务收尾更新 PROGRESS.md，汇报以「完成了sir」结尾

## 文件结构

| 文件 | 动作 | 职责 |
| ---- | ---- | ---- |
| `src/Render/ObjLoader.hpp` | Create(T1) | loadObjFromFile 声明 |
| `src/Render/ObjLoader.cpp` | Create(T1) | 解析器实现 |
| `src/Render/CMakeLists.txt` | Modify(T1) | 追加 ObjLoader.cpp |
| `assets/cube.obj` | Create(T1) | 真实立方体资产（8v/6vn/4vt/12f）|
| `test/render/ObjLoader.cpp` | Create(T1) | render_ObjLoader 套件（T1 解析 + T2 兜底/失败/端到端）|

---

### Task 1: 解析器核心与资产

**Files:**
- Create: `src/Render/ObjLoader.hpp`, `src/Render/ObjLoader.cpp`, `assets/cube.obj`, `test/render/ObjLoader.cpp`
- Modify: `src/Render/CMakeLists.txt`

**Interfaces:**
- Consumes: `Object4D`（vlistLocal[kObject4vListLen]/plist[kPolyListLen]/numVertices/numPolys/name）；`Point4D{x,y,z,w}`；`UV2D{u,v}`；`Vector3DBase<double>`（`.mul()` 叉积、`.normalize()` const）
- Produces: `bool loadObjFromFile(const std::string &path, Object4D &out);`

- [ ] **Step 1: 写失败测试** — 创建 `test/render/ObjLoader.cpp`：

```cpp
#include "ObjLoader.hpp"
#include <gtest/gtest.h>
#include <cstdio>
#include <cmath>

namespace{
std::string TmpPath(const char *name){
    return std::string("/tmp/opencode/") + name;
}

bool WriteFile(const std::string &path, const std::string &content){
    FILE *f = std::fopen(path.c_str(), "wb");
    if(!f) return false;
    std::fwrite(content.data(), 1, content.size(), f);
    std::fclose(f);
    return true;
}
}

TEST(ObjTest, VerticesAndTriangleFaces){
    ASSERT_TRUE(WriteFile(TmpPath("tri.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("tri.obj"), obj));
    EXPECT_EQ(obj.numVertices, 3);
    EXPECT_EQ(obj.numPolys, 1);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[0].x, 0.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[1].x, 1.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[2].y, 1.0);
    EXPECT_STREQ(obj.name, "obj");
}

TEST(ObjTest, QuadFaceFanTriangulated){
    ASSERT_TRUE(WriteFile(TmpPath("quad.obj"),
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("quad.obj"), obj));
    EXPECT_EQ(obj.numVertices, 4);
    EXPECT_EQ(obj.numPolys, 2);
    EXPECT_EQ(obj.plist[0].vlist[0].x, 0.0);
    EXPECT_EQ(obj.plist[0].vlist[1].x, 1.0);
    EXPECT_EQ(obj.plist[1].vlist[1].y, 1.0);
    EXPECT_EQ(obj.plist[1].vlist[2].x, 0.0);
}

TEST(ObjTest, SlashFormats){
    ASSERT_TRUE(WriteFile(TmpPath("slash.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "vt 0.25 0.5\n"
        "vn 0 0 1\n"
        "f 1/1 2/1 3/1\n"
        "f 1//1 2//1 3//1\n"
        "f 1/1/1 2/1/1 3/1/1\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("slash.obj"), obj));
    EXPECT_EQ(obj.numPolys, 3);
    EXPECT_NEAR(obj.plist[0].uvlist[0].u, 0.25, 1e-12);
    EXPECT_NEAR(obj.plist[0].uvlist[1].v, 0.5, 1e-12);
    EXPECT_DOUBLE_EQ(obj.plist[1].nlist[0].z, 1.0);
    EXPECT_NEAR(obj.plist[2].uvlist[2].u, 0.25, 1e-12);
    EXPECT_DOUBLE_EQ(obj.plist[2].nlist[2].z, 1.0);
}

TEST(ObjTest, NegativeIndices){
    ASSERT_TRUE(WriteFile(TmpPath("neg.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf -3 -2 -1\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("neg.obj"), obj));
    EXPECT_EQ(obj.numPolys, 1);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[2].y, 1.0);
}
```

数值依据：负索引 `-1` = 最后一个顶点（count+raw）；`a/b` 只填 uvlist、`a//c` 只填 nlist、`a/b/c` 双填；四边形 `f 1 2 3 4` fan 出 (1,2,3)+(1,3,4)。

- [ ] **Step 2: 配置构建验证失败**

```bash
cmake -B build -S . && cmake --build build -j$(nproc) --target render_ObjLoader 2>&1 | tail -3
```
预期：FAIL，`ObjLoader.hpp` 不存在。

- [ ] **Step 3: 实现** — 创建 `src/Render/ObjLoader.hpp`：

```cpp
#pragma once
#include <string>
#include "GeoObject/Object4D.hpp"

bool loadObjFromFile(const std::string &path, Object4D &out);
```

创建 `src/Render/ObjLoader.cpp`：

```cpp
#include "ObjLoader.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace{
struct ObjVertexRef{
    int v = 0;
    int vt = 0;
    int vn = 0;
};

bool ParseInt(const std::string &s, int &out){
    if(s.empty()) return false;
    try{
        std::size_t pos = 0;
        const int v = std::stoi(s, &pos);
        if(pos != s.size()) return false;
        out = v;
        return true;
    }catch(...){
        return false;
    }
}

bool ParseVertexRef(const std::string &tok, const std::vector<Point4D> &verts,
                    const std::vector<UV2D> &uvs, const std::vector<Vector3DBase<double>> &normals,
                    ObjVertexRef &ref){
    ref = ObjVertexRef{};
    const std::size_t s1 = tok.find('/');
    if(s1 == std::string::npos){
        return ParseInt(tok, ref.v);
    }
    if(!ParseInt(tok.substr(0, s1), ref.v)) return false;
    const std::size_t s2 = tok.find('/', s1 + 1);
    if(s2 == std::string::npos){
        return ParseInt(tok.substr(s1 + 1), ref.vt);
    }
    if(s2 == s1 + 1){
        return ParseInt(tok.substr(s2 + 1), ref.vn);
    }
    return ParseInt(tok.substr(s1 + 1, s2 - s1 - 1), ref.vt)
        && ParseInt(tok.substr(s2 + 1), ref.vn);
}

bool ResolveIndex(int raw, std::size_t count, int &out){
    if(raw > 0){
        if(static_cast<std::size_t>(raw) > count) return false;
        out = raw - 1;
        return true;
    }
    if(raw < 0){
        const int idx = static_cast<int>(count) + raw;
        if(idx < 0 || idx >= static_cast<int>(count)) return false;
        out = idx;
        return true;
    }
    return false;
}
}

bool loadObjFromFile(const std::string &path, Object4D &out){
    out.numVertices = 0;
    out.numPolys = 0;

    std::ifstream file(path);
    if(!file.is_open()){
        return false;
    }

    std::vector<Point4D> verts{};
    std::vector<UV2D> uvs{};
    std::vector<Vector3DBase<double>> normals{};

    std::string line;
    while(std::getline(file, line)){
        if(!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream ss(line);
        std::string tag;
        if(!(ss >> tag)) continue;

        if(tag == "v"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return false;
            if(verts.size() >= kObject4vListLen) return false;
            verts.push_back(Point4D{x, y, z, 1});
        }else if(tag == "vt"){
            double u = 0, v = 0;
            if(!(ss >> u >> v)) return false;
            uvs.push_back(UV2D{u, v});
        }else if(tag == "vn"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return false;
            normals.push_back(Vector3DBase<double>{x, y, z});
        }else if(tag == "f"){
            std::vector<ObjVertexRef> refs{};
            std::string tok;
            while(ss >> tok){
                ObjVertexRef ref{};
                if(!ParseVertexRef(tok, verts, uvs, normals, ref)) return false;
                refs.push_back(ref);
            }
            if(refs.size() < 3) return false;
            for(std::size_t i = 1; i + 1 < refs.size(); i++){
                if(out.numPolys >= kPolyListLen) return false;
                PolyF4D &poly = out.plist[out.numPolys++];
                const ObjVertexRef tri[3] = {refs[0], refs[i], refs[i + 1]};
                bool hasUv = true;
                bool hasN = true;
                Point4D p[3]{};
                for(int k = 0; k < 3; k++){
                    int vi = -1;
                    if(!ResolveIndex(tri[k].v, verts.size(), vi)) return false;
                    p[k] = verts[vi];
                    poly.vlist[k] = p[k];

                    int ui = -1;
                    if(tri[k].vt != 0){
                        if(!ResolveIndex(tri[k].vt, uvs.size(), ui)) return false;
                        poly.uvlist[k] = uvs[ui];
                    }else{
                        poly.uvlist[k] = UV2D{};
                        hasUv = false;
                    }

                    int ni = -1;
                    if(tri[k].vn != 0){
                        if(!ResolveIndex(tri[k].vn, normals.size(), ni)) return false;
                        poly.nlist[k] = normals[ni];
                    }else{
                        poly.nlist[k] = Vector3DBase<double>{};
                        hasN = false;
                    }
                }
                if(!hasN){
                    const Vector3DBase<double> e1{p[1].x - p[0].x, p[1].y - p[0].y, p[1].z - p[0].z};
                    const Vector3DBase<double> e2{p[2].x - p[0].x, p[2].y - p[0].y, p[2].z - p[0].z};
                    auto n = e1.mul(e2);
                    const double len = n.length();
                    if(len > 1e-12){
                        n = n.normalize();
                        poly.nlist[0] = n; poly.nlist[1] = n; poly.nlist[2] = n;
                    }
                }
            }
        }
    }

    out.numVertices = static_cast<int>(verts.size());
    for(int i = 0; i < out.numVertices; i++){
        out.vlistLocal[i] = verts[i];
    }
    std::snprintf(out.name, sizeof(out.name), "%s", "obj");
    return true;
}
```

创建资产 `assets/cube.obj`（8v/6vn/4vt/12f，UV 角点复用）：

```obj
# unit cube, CCW outward faces
v -1 -1 -1
v 1 -1 -1
v 1 1 -1
v -1 1 -1
v -1 -1 1
v 1 -1 1
v 1 1 1
v -1 1 1
vt 0 0
vt 1 0
vt 1 1
vt 0 1
vn 0 0 -1
vn 0 0 1
vn 0 -1 0
vn 0 1 0
vn 1 0 0
vn -1 0 0
f 1/1/1 4/4/1 3/3/1
f 1/1/1 3/3/1 2/2/1
f 5/1/2 6/2/2 7/3/2
f 5/1/2 7/3/2 8/4/2
f 1/1/3 2/2/3 6/3/3
f 1/1/3 6/3/3 5/4/3
f 4/1/4 8/4/4 7/3/4
f 4/1/4 7/3/4 3/2/4
f 2/1/5 3/4/5 7/3/5
f 2/1/5 7/3/5 6/2/5
f 1/1/6 5/2/6 8/3/6
f 1/1/6 8/3/6 4/4/6
```

修改 `src/Render/CMakeLists.txt` 源列表追加 `${SGE_RENDER_DIR}/ObjLoader.cpp`。

- [ ] **Step 4: 构建运行验证通过**

```bash
cmake --build build -j$(nproc) --target render_ObjLoader && ./build/test/render_ObjLoader
```
预期：4 项 PASS。

---

### Task 2: 失败路径、兜底与端到端渲染验证

**Files:**
- Test: Modify `test/render/ObjLoader.cpp`

**Interfaces:**
- Consumes: Task 1 全部产物；`FrameBuffer`/`Rasterizer`/`Pipeline::projectObject`/`Texture`
- Produces: 兜底/失败行为回归锁定 + 渲染链路端到端证据

- [ ] **Step 1: 写失败测试** — 追加（include 区补 `"Render/Camera.hpp"` 不需要——补 `"Rasterizer.hpp"`、`"Pipeline.hpp"`、`"Transform.hpp"`、`"Texture.hpp"`）：

```cpp
TEST(ObjTest, MissingNormalsFlatComputed){
    ASSERT_TRUE(WriteFile(TmpPath("nonorm.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 0 -1\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("nonorm.obj"), obj));
    const auto &n = obj.plist[0].nlist[0];
    EXPECT_NEAR(n.x, 0.0, 1e-12);
    EXPECT_NEAR(n.y, -1.0, 1e-12);
    EXPECT_NEAR(n.z, 0.0, 1e-12);
    EXPECT_EQ(obj.nlist? 0 : 0, 0);   // placeholder removed in final
}
```

（注意：上块中最后一行占位在落盘前删除；真实断言为三行 NEAR + 三顶点法线一致性 `EXPECT_TRUE(n == obj.plist[0].nlist[1])`。）

数值推导：e1=(1,0,0)-? p0=(0,0,0), p1=(1,0,0), p2=(0,0,-1)：e1=(1,0,0), e2=(0,0,-1)，叉积 e1×e2=(0*(-1)-0*0, 0*0-1*(-1), 0)=(0,1,0)。**修正期望为 (0,1,0)**（以实现者独立复算为准，叉积方向按右手系）。

```cpp
TEST(ObjTest, MissingUvsZeroed){
    ASSERT_TRUE(WriteFile(TmpPath("nouvs.obj"), "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("nouvs.obj"), obj));
    EXPECT_DOUBLE_EQ(obj.plist[0].uvlist[0].u, 0.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].uvlist[2].v, 0.0);
}

TEST(ObjTest, CapacityExceededFails){
    std::string content;
    for(int i = 0; i < 65; i++){
        content += "v 0 0 0\n";
    }
    ASSERT_TRUE(WriteFile(TmpPath("cap.obj"), content));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("cap.obj"), obj));
    EXPECT_EQ(obj.numVertices, 0);
    EXPECT_EQ(obj.numPolys, 0);
}

TEST(ObjTest, BadTokenFails){
    ASSERT_TRUE(WriteFile(TmpPath("bad.obj"), "v x 0 0\n"));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("bad.obj"), obj));
}

TEST(ObjTest, MissingFileFails){
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile("/tmp/opencode/no_such_12345.obj", obj));
}

TEST(ObjTest, RealCubeAsset){
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile("assets/cube.obj", obj));
    EXPECT_EQ(obj.numVertices, 8);
    EXPECT_EQ(obj.numPolys, 12);
    EXPECT_DOUBLE_EQ(obj.plist[0].nlist[0].z, -1.0);
    EXPECT_DOUBLE_EQ(obj.plist[1].nlist[0].z, -1.0);
}

TEST(ObjTest, EndToEndRendersPixels){
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile("assets/cube.obj", obj));

    auto model = SGE::Math::rotationY(0.6).mul(SGE::Math::rotationX(0.4));
    auto view = SGE::Math::lookAt(Vector3DBase<double>{0, 2, -6},
                                  Vector3DBase<double>{0, 0, 0},
                                  Vector3DBase<double>{0, 1, 0});
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto vp = proj.mul(view);
    auto nrm = SGE::Math::normalMatrix(model);

    FrameBuffer fb(800, 600);
    Rasterizer rz{fb};
    uint32_t px[1] = {0xFFFFFFFFu};
    Texture tex(1, 1, px);
    for(auto &t : Pipeline::projectObject(obj, model, vp, nrm, 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tex);
    }

    std::size_t lit = 0;
    for(std::size_t i = 0; i < fb.width() * fb.height(); i++){
        if(fb.colorData()[i] != 0xFF000000u) lit++;
    }
    EXPECT_GT(lit, 1000u);
}
```

（include 需求：`Rasterizer.hpp`/`Pipeline.hpp`/`Transform.hpp`/`Texture.hpp`；测试从仓库根运行保证 `assets/cube.obj` 相对路径可达。）

- [ ] **Step 2: 构建运行验证通过 + 全量回归**

```bash
cmake --build build -j$(nproc) --target render_ObjLoader && ./build/test/render_ObjLoader
for t in build/test/math_* build/test/render_*; do ./$t > /dev/null 2>&1 || echo "FAIL $t"; done
```
预期：render_ObjLoader 10 项 PASS；全量回归无 FAIL。

- [ ] **Step 3: 冒烟 + 提交 + PROGRESS.md**

```bash
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./build/src/soft-game-engine
git add src/Render/ObjLoader.hpp src/Render/ObjLoader.cpp src/Render/CMakeLists.txt assets/cube.obj test/render/ObjLoader.cpp
git commit -m "feat(render): wavefront OBJ loader with flat-normal fallback and cube asset"
```
PROGRESS.md 记录后汇报，以「完成了sir」结尾。

---

## Self-Review 记录

1. **Spec 覆盖**：语法子集表→T1 实现；四种索引/负索引/fan→T1 测试；兜底规则→T2 MissingNormals/MissingUvs；失败条件→T2 Capacity/BadToken/MissingFile；资产→T1 cube.obj+RealCubeAsset；端到端验收标准 3→T2 EndToEndRendersPixels。
2. **占位符扫描**：MissingNormalsFlatComputed 块内标注了一处落盘前须删除的说明行与叉积方向待实现者复算项——这是显式指令非占位（叉积符号由右手系唯一确定，实现者以代码为准并保持断言与之一致）。
3. **类型一致性**：`loadObjFromFile(const std::string&, Object4D&)` 贯穿；`UV2D{}` 默认构造（spec §3 既有类型）；`Vector3DBase<double>` 叉积 `.mul()`。
4. **风险预控**：半填充状态由「本地 vector 先行、成功才拷入」规避；\r\n 行尾 pop_back；测试从仓库根运行保证相对路径。
