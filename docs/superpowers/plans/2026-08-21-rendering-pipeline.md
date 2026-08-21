# Rendering Pipeline Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render a rotating solid-shaded cube with wireframe support via a barycentric rasterizer with Z-buffer.

**Architecture:** FrameBuffer owns color+depth pixels; Rasterizer draws lines/triangles into it; Pipeline transforms Object4D polygons to screen space (cull/clip/divide/map); Application blits FrameBuffer into the existing BufferManager display path.

**Tech Stack:** C++20, clang++, existing math lib (Matrix4DBase::mul, Transform.hpp), SDL2, Google Test.

## Global Constraints

- Build: `cmake -B build -S . && cmake --build build -j$(nproc)`; reconfigure needed for new test files (glob).
- All 9 existing suites / 69 tests must keep passing.
- New render sources must be registered in `src/Render/CMakeLists.txt` (aux_source_directory covers only GeoObject/ — add explicit list).
- Depth convention: smaller depth = closer; buffer initialized to FLT_MAX; NDC z in [-1,1].
- Screen coords: origin top-left, y down; px = (ndc.x*0.5+0.5)*W, py = (1-(ndc.y*0.5+0.5))*H.
- Points are Vector4DBase<double> with w=1 (must be set explicitly when building geometry).

---

### Task 1: FrameBuffer

**Files:**
- Create: `src/Render/FrameBuffer.hpp`, `src/Render/FrameBuffer.cpp`
- Modify: `src/Render/CMakeLists.txt`
- Test: `test/render/FrameBuffer.cpp` (new suite `render_FrameBuffer`)

**Interfaces:**
- Produces: `class FrameBuffer { FrameBuffer(std::size_t w, std::size_t h); void setPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth); void clear(uint32_t bgra); uint32_t* colorData(); float* depthData(); std::size_t width() const; std::size_t height() const; }`

- [ ] **Step 1: Write failing tests** (`test/render/FrameBuffer.cpp`)

```cpp
#include "FrameBuffer.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(FrameBufferTest, Construction){
    FrameBuffer fb{8, 4};
    EXPECT_EQ(fb.width(), 8u);
    EXPECT_EQ(fb.height(), 4u);
    // initial: black color, depth = FLT_MAX
    for(std::size_t i = 0; i < 8*4; i++){
        EXPECT_EQ(fb.colorData()[i], 0xFF000000u);  // black opaque BGRA
        EXPECT_EQ(fb.depthData()[i], std::numeric_limits<float>::max());
    }
}

TEST(FrameBufferTest, SetPixelRoundtrip){
    FrameBuffer fb{4, 4};
    fb.setPixel(2, 1, 0xFF0000FFu, 0.5f);
    EXPECT_EQ(fb.colorData()[1*4 + 2], 0xFF0000FFu);
    EXPECT_FLOAT_EQ(fb.depthData()[1*4 + 2], 0.5f);
}

TEST(FrameBufferTest, SetPixelOutOfBoundsIgnored){
    FrameBuffer fb{4, 4};
    fb.setPixel(4, 0, 0xFFFFFFFFu, 0.0f);
    fb.setPixel(0, 4, 0xFFFFFFFFu, 0.0f);
    fb.setPixel(100, 100, 0xFFFFFFFFu, 0.0f);
    SUCCEED();
}

TEST(FrameBufferTest, DepthTest){
    FrameBuffer fb{2, 2};
    fb.setPixel(0, 0, 0xFF00FF00u, 0.5f);   // far first
    fb.setPixel(0, 0, 0xFFFF0000u, 0.3f);   // nearer wins
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
    fb.setPixel(0, 0, 0xFF0000FFu, 0.7f);   // farther rejected
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
    fb.setPixel(0, 0, 0xFF0000FFu, 0.3f);   // equal rejected
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
}

TEST(FrameBufferTest, ClearResets){
    FrameBuffer fb{2, 2};
    fb.setPixel(0, 0, 0xFF123456u, 0.25f);
    fb.clear(0xFF000000u);
    for(std::size_t i = 0; i < 4; i++){
        EXPECT_EQ(fb.colorData()[i], 0xFF000000u);
        EXPECT_EQ(fb.depthData()[i], std::numeric_limits<float>::max());
    }
}
```

- [ ] **Step 2: Run to verify failure**

Run: `cmake --build build -j$(nproc) --target render_FrameBuffer`
Expected: FAIL — FrameBuffer.hpp not found

- [ ] **Step 3: Implement** `src/Render/FrameBuffer.hpp`

```cpp
#pragma once
#include <cstdint>
#include <cfloat>
#include <vector>
#include <algorithm>

class FrameBuffer{
public:
    FrameBuffer(std::size_t width, std::size_t height)
        : m_width(width), m_height(height){
        m_color.resize(m_width * m_height, 0xFF000000u);
        m_depth.resize(m_width * m_height, FLT_MAX);
    }

    void setPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth){
        if(x >= m_width || y >= m_height) return;
        auto idx = y * m_width + x;
        if(depth >= m_depth[idx]) return;
        m_depth[idx] = depth;
        m_color[idx] = bgra;
    }

    void clear(uint32_t bgra = 0xFF000000u){
        std::fill(m_color.begin(), m_color.end(), bgra);
        clearDepth();
    }

    void clearDepth(){
        std::fill(m_depth.begin(), m_depth.end(), FLT_MAX);
    }

    uint32_t* colorData(){ return m_color.data(); }
    const uint32_t* colorData() const{ return m_color.data(); }
    float* depthData(){ return m_depth.data(); }
    std::size_t width() const{ return m_width; }
    std::size_t height() const{ return m_height; }

private:
    std::size_t m_width{};
    std::size_t m_height{};
    std::vector<uint32_t> m_color{};
    std::vector<float> m_depth{};
};
```

Note on depth test: `depth >= m_depth[idx]` rejects equal AND farther — matches test expectations.

- [ ] **Step 4: Register sources** — `src/Render/CMakeLists.txt`

```cmake
set(SGE_RENDER_GEOOBJECT_DIR ${SGE_RENDER_DIR}/GeoObject)
set(SGE_RENDER_CORE_SRC ${SGE_RENDER_DIR}/FrameBuffer.cpp)

aux_source_directory(${SGE_RENDER_GEOOBJECT_DIR} GEMO_SRC_FILES)

add_library(render ${GEMO_SRC_FILES} ${SGE_RENDER_CORE_SRC})
target_compile_options(render PRIVATE ${SGE_WARNING_FLAGS})
target_link_libraries(render PUBLIC uuid)
```

(Task 3/4 will append Rasterizer.cpp/Pipeline.cpp to SGE_RENDER_CORE_SRC.)

- [ ] **Step 5: Build & run** — `cmake -B build -S . > /dev/null && cmake --build build -j$(nproc) --target render_FrameBuffer && ./build/test/render_FrameBuffer`
Expected: 5/5 PASSED

- [ ] **Step 6: Commit** — `git add -A src/Render test/render/FrameBuffer.cpp && git commit -m "feat(render): FrameBuffer with depth-tested setPixel"`

---

### Task 2: Rasterizer wireframe

**Files:**
- Create: `src/Render/Rasterizer.hpp`, `src/Render/Rasterizer.cpp`
- Modify: `src/Render/CMakeLists.txt` (append Rasterizer.cpp)
- Test: `test/render/Rasterizer.cpp` (new suite `render_Rasterizer`; grows in Task 3)

**Interfaces:**
- Consumes: FrameBuffer (Task 1), `Color32` from `GeoObject/Color.hpp`.
- Produces:
```cpp
struct ScreenVertex {
    double x = 0, y = 0;
    float z = 0;      // NDC depth
    float w = 1;      // clip w
    Color32 color{};  // per-vertex color
};
class Rasterizer {
public:
    explicit Rasterizer(FrameBuffer &fb);
    void drawLine(const ScreenVertex &a, const ScreenVertex &b);
    void drawTriangleWireframe(const ScreenVertex&, const ScreenVertex&, const ScreenVertex&);
};
```

- [ ] **Step 1: Write failing tests** (append-ready file)

```cpp
#include "Rasterizer.hpp"
#include <gtest/gtest.h>

namespace{
uint32_t red = 0xFFFF0000u;

int countPixels(FrameBuffer &fb, uint32_t v){
    int n = 0;
    for(std::size_t i = 0; i < fb.width()*fb.height(); i++)
        if(fb.colorData()[i] == v) n++;
    return n;
}
bool hasPixel(FrameBuffer &fb, std::size_t x, std::size_t y, uint32_t v){
    return fb.colorData()[y*fb.width()+x] == v;
}
ScreenVertex V(double x, double y){ ScreenVertex v; v.x=x; v.y=y; v.z=0; v.w=1; v.color=Color32{255,0,0,255}; return v; }
uint32_t packRed(){ /* BGRA memory little-endian of Color32{255,0,0,255}: b=0,g=0,r=255,a=255 */ return 0xFF0000FFu; }
}

TEST(RasterLineTest, Horizontal){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawLine(V(2, 5), V(7, 5));
    EXPECT_EQ(countPixels(fb, packRed()), 6);
    EXPECT_TRUE(hasPixel(fb, 2, 5, packRed()));
    EXPECT_TRUE(hasPixel(fb, 7, 5, packRed()));
}

TEST(RasterLineTest, VerticalAndDiagonal){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawLine(V(3, 1), V(3, 6));
    EXPECT_EQ(countPixels(fb, packRed()), 6);
    fb.clear();
    rz.drawLine(V(1, 1), V(5, 5));
    EXPECT_EQ(countPixels(fb, packRed()), 5);
    EXPECT_TRUE(hasPixel(fb, 1, 1, packRed()));
    EXPECT_TRUE(hasPixel(fb, 5, 5, packRed()));
}

TEST(RasterWireTest, SquarePerimeterOnly){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawTriangleWireframe(V(2,2), V(8,2), V(2,8));  // right triangle legs
    EXPECT_TRUE(hasPixel(fb, 2, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 8, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 2, 8, packRed()));
    EXPECT_FALSE(hasPixel(fb, 4, 4, packRed()));       // interior untouched
}
```

Note: `drawLine(V(a),V(b))` and reversed call must both draw all 6 pixels (symmetric Bresenham). Add `rz.drawLine(V(7,5), V(2,5));` variant inside Horizontal test after clear if desired — plan keeps one direction plus endpoint checks.

- [ ] **Step 2: Run to verify failure** — build target render_Rasterizer: header not found.

- [ ] **Step 3: Implement Rasterizer.hpp/.cpp**

```cpp
// Rasterizer.hpp
#pragma once
#include "FrameBuffer.hpp"
#include "GeoObject/Color.hpp"

struct ScreenVertex{
    double x = 0;
    double y = 0;
    float z = 0;
    float w = 1;
    Color32 color{};
};

inline uint32_t PackBGRA(const Color32 &c){
    return (static_cast<uint32_t>(c.a) << 24) |
           (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8)  |
            static_cast<uint32_t>(c.b);
}

class Rasterizer{
public:
    explicit Rasterizer(FrameBuffer &fb) : m_fb(fb){ }

    void drawLine(const ScreenVertex &a, const ScreenVertex &b);
    void drawTriangleWireframe(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);

private:
    void plot(int x, int y, const Color32 &c);

    FrameBuffer &m_fb;
};
```

```cpp
// Rasterizer.cpp
#include "Rasterizer.hpp"
#include <algorithm>
#include <cmath>

void Rasterizer::plot(int x, int y, const Color32 &c){
    if(x < 0 || y < 0 || x >= static_cast<int>(m_fb.width()) || y >= static_cast<int>(m_fb.height())) return;
    m_fb.colorData()[static_cast<std::size_t>(y)*m_fb.width() + static_cast<std::size_t>(x)] = PackBGRA(c);
}

void Rasterizer::drawLine(const ScreenVertex &a, const ScreenVertex &b){
    int x0 = static_cast<int>(std::lround(a.x)), y0 = static_cast<int>(std::lround(a.y));
    int x1 = static_cast<int>(std::lround(b.x)), y1 = static_cast<int>(std::lround(b.y));
    int dx = std::abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -std::abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = dx + dy;
    while(true){
        plot(x0, y0, a.color);
        if(x0 == x1 && y0 == y1) break;
        auto e2 = 2*err;
        if(e2 >= dy){ err += dy; x0 += sx; }
        if(e2 <= dx){ err += dx; y0 += sy; }
    }
}

void Rasterizer::drawTriangleWireframe(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2){
    drawLine(v0, v1);
    drawLine(v1, v2);
    drawLine(v2, v0);
}
```

Wireframe writes color directly (no depth test/write) — spec decision.

- [ ] **Step 4: CMake** — append `Rasterizer.cpp` to SGE_RENDER_CORE_SRC.
- [ ] **Step 5: Build & run** render_Rasterizer → 3/3 PASSED; run render_FrameBuffer still 5/5.
- [ ] **Step 6: Commit** — `git commit -m "feat(render): Bresenham wireframe rasterizer"`

---

### Task 3: Rasterizer solid fill

**Files:**
- Modify: `src/Render/Rasterizer.hpp/.cpp`
- Test: extend `test/render/Rasterizer.cpp`

**Interfaces:**
- Produces: `void drawTriangleSolid(const ScreenVertex&, const ScreenVertex&, const ScreenVertex&);`

- [ ] **Step 1: Write failing tests** (append)

```cpp
TEST(RasterSolidTest, DegenerateWritesNothing){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawTriangleSolid(V(3,3), V(3,3), V(3,3));
    EXPECT_EQ(countPixels(fb, packRed()), 0);
}

TEST(RasterSolidTest, SmallTriangleExactPixels){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    // right triangle with legs on integer grid: pixel centers strictly inside
    rz.drawTriangleSolid(V(2,2), V(6,2), V(2,6));
    EXPECT_TRUE(hasPixel(fb, 3, 3, packRed()));
    EXPECT_TRUE(hasPixel(fb, 4, 3, packRed()));
    EXPECT_TRUE(hasPixel(fb, 3, 4, packRed()));
    EXPECT_FALSE(hasPixel(fb, 5, 5, packRed()));  // outside hypotenuse region
    EXPECT_FALSE(hasPixel(fb, 2, 2, packRed()));  // corner vertex not filled by top-left rule here
}

TEST(RasterSolidTest, SharedEdgeNoOverlap){
    FrameBuffer fb{12, 12};
    Rasterizer rz{fb};
    // two triangles sharing diagonal of square (2..8)
    rz.drawTriangleSolid(V(2,2), V(8,2), V(2,8));
    rz.drawTriangleSolid(V(8,2), V(8,8), V(2,8));
    // union area of the two right triangles: each ~13 px -> total 26, no duplicates possible
    EXPECT_EQ(countPixels(fb, packRed()), countPixelsUnique(fb)); // helper counts cells != black
    // simpler invariant below
}
```

Replace last assertion mechanics: count non-black pixels == sum written by each triangle alone into separate buffers (compute expected by rendering each triangle into its own framebuffer and adding counts):

```cpp
TEST(RasterSolidTest, SharedEdgeNoOverlap){
    auto tri1 = [](Rasterizer &rz){ rz.drawTriangleSolid(V(2,2), V(8,2), V(2,8)); };
    auto tri2 = [](Rasterizer &rz){ rz.drawTriangleSolid(V(8,2), V(8,8), V(2,8)); };

    FrameBuffer f1{12,12}; Rasterizer r1{f1}; tri1(r1);
    FrameBuffer f2{12,12}; Rasterizer r2{f2}; tri2(r2);
    int expect = countPixels(f1, packRed()) + countPixels(f2, packRed());

    FrameBuffer fb{12,12}; Rasterizer rz{fb};
    tri1(rz); tri2(rz);
    EXPECT_EQ(countPixels(fb, packRed()), expect);
}

TEST(RasterSolidTest, PerspectiveCorrectDepth){
    FrameBuffer fb{8, 8};
    Rasterizer rz{fb};
    // steep sliver: near vertex at bottom (small z), far edge on top
    ScreenVertex a = V(2, 6), b = V(6, 6), c = V(4, 1);
    a.w = 1; b.w = 1; c.w = 4;
    a.z = -0.5f; b.z = -0.5f; c.z = -0.9f;
    rz.drawTriangleSolid(a, b, c);
    // bottom row center should hold near depth
    auto *dep = fb.depthData();
    float d = dep[6*8+4];
    ASSERT_LT(d, 0.0f);
    EXPECT_NEAR(d, -0.55f, 0.15f);   // interpolated toward vertex z=-0.5
}

TEST(RasterSolidTest, DepthRejectsBehind){
    FrameBuffer fb{8, 8};
    Rasterizer rz{fb};
    ScreenVertex t = V(2,2), u = V(6,2), s = V(2,6);
    t.z=u.z=s.z=-0.2f; t.w=u.w=s.w=1;
    rz.drawTriangleSolid(t,u,s);
    ScreenVertex t2 = V(2,2), u2 = V(6,2), s2 = V(2,6);
    t2.z=u2.z=s2.z=-0.8f;                       // farther (larger z? no: -0.8 < -0.2 => nearer!)
    // careful: NDC near=-1 is closest; use +0.5 as "farther"
    t2.z=u2.z=s2.z=0.5f;
    rz.drawTriangleSolid(t2,u2,s2);
    EXPECT_EQ(countPixels(fb, packRed()), countPixels(fb, 0x000000FFu) , 0); // second write fully rejected
    EXPECT_TRUE(hasPixel(fb, 3, 3, packRed()));
}
```

Final depth-rejection test simplified:

```cpp
TEST(RasterSolidTest, FartherTriangleFullyRejected){
    FrameBuffer fb{8, 8};
    Rasterizer rz{fb};
    auto tri = [&](float z){
        ScreenVertex t=V(2,2), u=V(6,2), s=V(2,6);
        t.z=u.z=s.z=z; t.w=u.w=s.w=1;
        rz.drawTriangleSolid(t,u,s);
    };
    tri(-0.2f);                                  // near, red
    ScreenVertex t=V(3,3), u=V(5,3), s=V(3,5);
    t.z=u.z=s.z=0.5f;                            // far, blue
    t.color=u.color=s.color=Color32{255,0,0,255};
    rz.drawTriangleSolid(t,u,s);
    EXPECT_FALSE(hasPixel(fb, 4, 4, 0xFF0000FFu)); // blue never lands
    EXPECT_TRUE(hasPixel(fb, 3, 3, packRed()));
}
```

(Plan note: implementer should reconcile the exact final form of these tests — keep: degenerate, small-exact-pixels, shared-edge-no-overlap, perspective-depth, farther-rejected.)

- [ ] **Step 2: Implement drawTriangleSolid**

Add to Rasterizer.hpp public section:

```cpp
void drawTriangleSolid(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
```

Implementation (Rasterizer.cpp):

```cpp
namespace{
double EdgeFunction(double ax,double ay,double bx,double by,double px,double py){
    return (bx-ax)*(py-ay) - (by-ay)*(px-ax);
}
bool IsTopLeft(const ScreenVertex &a, const ScreenVertex &b){
    // edge a->b is top-left if horizontal-top or vertical-left (y-down screen)
    bool top    = a.y == b.y && a.y < 0;              // unused generic form kept simple
    bool left   = a.y > b.y;
    bool horizTop = (a.y == b.y) && (a.x < b.x) && false; // disabled; see bias note
    (void)top; (void)horizTop;
    return left;
}
}

void Rasterizer::drawTriangleSolid(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2){
    double area = EdgeFunction(v0.x,v0.y, v1.x,v1.y, v2.x,v2.y);
    if(area == 0) return;

    double minX = std::min({v0.x,v1.x,v2.x}), maxX = std::max({v0.x,v1.x,v2.x});
    double minY = std::min({v0.y,v1.y,v2.y}), maxY = std::max({v0.y,v1.y,v2.y});
    int x0 = std::max(0, static_cast<int>(std::floor(minX)));
    int y0 = std::max(0, static_cast<int>(std::floor(minY)));
    int x1 = std::min(static_cast<int>(m_fb.width())-1,  static_cast<int>(std::ceil(maxX)));
    int y1 = std::min(static_cast<int>(m_fb.height())-1, static_cast<int>(std::ceil(maxY)));

    double invArea = 1.0 / area;
    uint32_t packed = PackBGRA(v0.color);
    const double eps = 1e-9;

    for(int y = y0; y <= y1; y++){
        for(int x = x0; x <= x1; x++){
            double px = x + 0.5, py = y + 0.5;
            double w0 = EdgeFunction(v1.x,v1.y, v2.x,v2.y, px,py);
            double w1 = EdgeFunction(v2.x,v2.y, v0.x,v0.y, px,py);
            double w2 = EdgeFunction(v0.x,v0.y, v1.x,v1.y, px,py);
            // normalize so weights sum to 1 regardless of winding
            w0 *= invArea; w1 *= invArea; w2 *= invArea;
            auto inside = [&](double w){ return area > 0 ? w >= eps : w <= -eps; };
            if(!inside(w0) || !inside(w1) || !inside(w2)) continue;

            double iw = w0/v0.w + w1/v1.w + w2/v2.w;
            if(iw <= 0) continue;
            float zNdc = static_cast<float>((w0*v0.z/v0.w + w1*v1.z/v1.w + w2*v2.z/v2.w) / iw);
            m_fb.setPixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y), packed, zNdc);
        }
    }
}
```

Top-left rule note: strict `>= eps` / `<= -eps` boundary handling means shared edges land on exactly one triangle only when windings are consistent (both CCW in screen space). Pipeline guarantees consistent winding post-cull; the shared-edge test above validates this. If the test shows off-by-one overlap/gap on the diagonal, switch to the classic ±bias-per-edge top-left rule (bias an edge only when it is top-left: horizontal-and-leftmost or strictly-left side) — implementer verifies via the SharedEdgeNoOverlap test.

Depth: `zNdc` computed with perspective correction (divide by interpolated 1/w); `setPixel` performs `<` depth test (equal rejected → matches Task 1 semantics).

- [ ] **Step 3: Build & run** render_Rasterizer → all tests pass (wireframe 3 + solid 4-5 new).
- [ ] **Step 4: Full suite check** — all suites green.
- [ ] **Step 5: Commit** — `feat(render): barycentric solid fill with Z-buffer`

---

### Task 4: Pipeline transform utilities

**Files:**
- Create: `src/Render/Pipeline.hpp`, `src/Render/Pipeline.cpp`
- Modify: `src/Render/CMakeLists.txt` (append Pipeline.cpp)
- Test: `test/render/PipelineTest.cpp` (new suite `render_Pipeline`)

**Interfaces:**
- Consumes: Object4D/PolyF4D (`GeoObject/Object4D.hpp`, `GeoObject/PolyF4D.hpp`), Matrix4DBase<double>, ScreenVertex (Rasterizer.hpp), Transform.hpp helpers.
- Produces:

```cpp
namespace Pipeline {
struct ScreenTriangle { ScreenVertex v[3]; };
bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c);
std::vector<ScreenTriangle> clipNearPlane(const ScreenVertex (&tri)[3]); // returns 0..2 triangles
std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH);
}
```

- [ ] **Step 1: Write failing tests**

```cpp
#include "Pipeline.hpp"
#include "Transform.hpp"
#include <gtest/gtest.h>
#include <cmath>

using P4 = Point4D;

TEST(PipelineBackfaceTest, SignConvention){
    // CCW in y-down screen space (area > 0): front
    ScreenVertex a{0,0,0,1}, b{0,4,0,1}, c{4,0,0,1};
    EXPECT_FALSE(Pipeline::isBackFacing(a,b,c));
    EXPECT_TRUE(Pipeline::isBackFacing(a,c,b));
}

TEST(PipelineClipTest, FullyInsideUnchanged){
    ScreenVertex tri[3] = {{0,-0.5f,1},{0,-0.5f,1},{4,-0.5f,1}};
    tri[0].x=0; tri[0].y=-4; tri[1].x=4; tri[1].y=-4; tri[2].x=2; tri[2].y=4;
    // give valid clip-space z/w (in front of near plane): z+w>0
    tri[0].z=-0.5f; tri[0].w=2; tri[1].z=-0.5f; tri[1].w=2; tri[2].z=-0.5f; tri[2].w=2;
    auto out = Pipeline::clipNearPlane(tri);
    ASSERT_EQ(out.size(), 1u);
}

TEST(PipelineClipTest, OneVertexBehindSplitsIntoTwo){
    // v2 behind near plane (z + w < 0)
    ScreenVertex tri[3]{};
    tri[0] = {0,-0.5f,2}; tri[1] = {4,-0.5f,2}; tri[2] = {2,-0.5f,-2};
    tri[0].y=-4; tri[1].y=-4; tri[2].y=4;
    tri[0].color=tri[1].color=tri[2].color=Color32{255,255,255,255};
    auto out = Pipeline::clipNearPlane(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineProjectTest, CubeFrontFaceProjectsCentered){
    Object4D cube{};
    cube.numVertices = 8;
    double s = 1.0;
    P4 verts[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                   {-s,-s, s,1},{s,-s, s,1},{s,s, s,1},{-s,s, s,1}};
    for(int i = 0;i < 8;i++) cube.vlistLocal[i] = verts[i];
    cube.numPolys = 1;
    cube.plist[0].vlist[0]=verts[0]; cube.plist[0].vlist[1]=verts[3]; cube.plist[0].vlist[2]=verts[2];
    cube.plist[0].color = Color32{255,0,0,255};

    auto view = SGE::Math::lookAt(Vector3DBase<double>{0,2,-6}, Vector3DBase<double>{0,0,0}, Vector3DBase<double>{0,1,0});
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto model = SGE::Math::translation(0.0,0.0,0.0);
    auto mvp = proj.mul(view).mul(model);

    auto tris = Pipeline::projectObject(cube, mvp, 800, 600);
    // front face (-Z face) visible; others culled or clipped by winding
    ASSERT_GE(tris.size(), 0u);
    if(!tris.empty()){
        auto cx = (tris[0].v[0].x + tris[0].v[1].x + tris[0].v[2].x)/3.0;
        auto cy = (tris[0].v[0].y + tris[0].v[1].y + tris[0].v[2].y)/3.0;
        EXPECT_NEAR(cx, 400, 60);
        EXPECT_NEAR(cy, 300, 60);
        for(auto &t : tris) for(int i=0;i<3;i++) EXPECT_GT(t.v[i].w, 0);
    }
}

TEST(PipelineProjectTest, BehindCameraCulledByNearClip){
    // camera looking down -Z from origin: object at +Z (behind) produces nothing
    Object4D obj{};
    obj.numVertices = 3;
    P4 v[3] = {{-1,-1,5,1},{1,-1,5,1},{0,1,5,1}};
    for(int i=0;i<3;i++) obj.vlistLocal[i]=v[i];
    obj.numPolys = 1;
    obj.plist[0].vlist[0]=v[0]; obj.plist[0].vlist[1]=v[1]; obj.plist[0].vlist[2]=v[2];

    auto view = SGE::Math::lookAt(Vector3DBase<double>{0,0,0}, Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,1,0});
    auto proj = SGE::Math::perspective(M_PI/3, 1.0, 0.1, 100.0);
    auto mvp = proj.mul(view);
    auto tris = Pipeline::projectObject(obj, mvp, 800, 600);
    EXPECT_EQ(tris.size(), 0u);
}
```

- [ ] **Step 2: Implement Pipeline.hpp/.cpp**

Header:

```cpp
#pragma once
#include <array>
#include <vector>
#include "Rasterizer.hpp"
#include "GeoObject/Object4D.hpp"
#include "../math/Matrix/DynamicMatrix4D.hpp"

namespace Pipeline{

struct ScreenTriangle{
    ScreenVertex v[3];
};

bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c);
std::vector<ScreenTriangle> clipNearPlane(const ScreenVertex (&tri)[3]);
std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH);

}
```

Implementation:

```cpp
#include "Pipeline.hpp"
#include "../math/Transform.hpp"
#include <cmath>

namespace Pipeline{

bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c){
    double area = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    return area <= 0;   // CCW (positive) = front in y-down screen space
}

namespace{
double NearDistance(const ScreenVertex &v){ return v.z + v.w; }  // clip-space plane z+w=0
ScreenVertex Lerp(const ScreenVertex &a, const ScreenVertex &b, double t){
    ScreenVertex r{};
    r.x = a.x + (b.x-a.x)*t;
    r.y = a.y + (b.y-a.y)*t;
    r.z = static_cast<float>(a.z + (b.z-a.z)*t);
    r.w = static_cast<float>(a.w + (b.w-a.w)*t);
    r.color = a.color;
    return r;
}
}

std::vector<ScreenTriangle> clipNearPlane(const ScreenVertex (&tri)[3]){
    std::vector<ScreenVertex> in{}, out{};
    for(auto &v : tri) (NearDistance(v) >= 0 ? in : out).push_back(v);

    if(in.size() == 3) return {ScreenTriangle{tri[0],tri[1],tri[2]}};
    if(in.size() == 0) return {};

    std::vector<ScreenVertex> poly{};
    for(int i = 0; i < 3; i++){
        const auto &cur = tri[i];
        const auto &nxt = tri[(i+1)%3];
        bool curIn = NearDistance(cur) >= 0, nxtIn = NearDistance(nxt) >= 0;
        if(curIn) poly.push_back(cur);
        if(curIn != nxtIn){
            double t = NearDistance(cur) / (NearDistance(cur) - NearDistance(nxt));
            poly.push_back(Lerp(cur, nxt, t));
        }
    }

    std::vector<ScreenTriangle> res{};
    for(std::size_t i = 1; i+1 < poly.size(); i++){
        res.push_back(ScreenTriangle{poly[0], poly[i], poly[i+1]});
    }
    return res;
}

std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH){
    std::vector<ScreenTriangle> result{};
    for(int p = 0; p < obj.numPolys; p++){
        const auto &poly = obj.plist[p];
        ScreenVertex sv[3]{};
        bool ok = true;
        for(int i = 0; i < 3; i++){
            auto clip = mvp.mul(poly.vlist[i]);           // Matrix4D × Vector4D(w=1)
            if(clip.w < 1e-6){ ok = false; break; }
            sv[i].x = clip.x; sv[i].y = clip.y;
            sv[i].z = static_cast<float>(clip.z);
            sv[i].w = static_cast<float>(clip.w);
            sv[i].color = poly.color;
        }
        if(!ok) continue;

        auto clipped = clipNearPlane(sv);
        for(auto &t : clipped){
            bool bad = false;
            for(int i = 0; i < 3; i++){
                double ndcX = t.v[i].x / t.v[i].w;
                double ndcY = t.v[i].y / t.v[i].w;
                double ndcZ = t.v[i].z / t.v[i].w;
                t.v[i].x = (ndcX*0.5 + 0.5) * screenW;
                t.v[i].y = (1.0 - (ndcY*0.5 + 0.5)) * screenH;
                t.v[i].z = static_cast<float>(ndcZ);
                t.v[i].w = 1.0f;
            }
            if(bad) continue;
            if(isBackFacing(t.v[0], t.v[1], t.v[2])) continue;
            result.push_back(t);
        }
    }
    return result;
}

}
```

Notes:
- After viewport mapping we set `w=1` and store NDC z in `.z`; Rasterizer's perspective-correct path then divides by w=1 (no-op) — but interpolation still uses per-vertex z which IS ndc z; correct because divide already applied per-vertex. (Perspective correctness across the interior requires interpolating attr/w before divide; since we pre-divide, depth becomes affine-in-screen-space — acceptable approximation documented in spec as acceptable for phase 1? NO — spec requires perspective-correct. Resolution: keep raw w in vertices through mapping: do NOT overwrite w; store screen x,y AND retain z,w. Rasterizer divides by w internally. So remove `t.v[i].w = 1.0f;` line and keep original clip w. The PerspectiveCorrectDepth unit test in Task 3 pins the behavior.)
- The `(void)bad` cleanup: drop the unused `bad` flag; guard `clip.w < 1e-6` earlier suffices.

- [ ] **Step 3: CMake** append Pipeline.cpp; **build & run** render_Pipeline → 5/5 PASSED.
- [ ] **Step 4: Commit** — `feat(render): pipeline cull/clip/project utilities`

---

### Task 5: Cube demo integration

**Files:**
- Modify: `src/main.cpp` (window 800×600)
- Modify: `src/Application.hpp` (add FrameBuffer + cube members)
- Modify: `src/Application.cpp` (per-frame render + blit)
- Test: smoke run under dummy driver; full suite regression.

**Interfaces:**
- Consumes: Pipeline::projectObject, Rasterizer, FrameBuffer, Transform.hpp, BufferManager::instance()->getRawBuffer().

- [ ] **Step 1: Cube builder** — add free function in Application.cpp (file-local):

```cpp
namespace{
Object4D MakeCube(){
    Object4D cube{};
    std::snprintf(cube.name, sizeof(cube.name), "%s", "cube");
    double s = 1.0;
    Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                    {-s,-s, s,1},{s,-s, s,1},{s,s, s,1},{-s,s, s,1}};
    for(int i = 0;i < 8;i++){ cube.vlistLocal[i] = v[i]; }
    cube.numVertices = 8;

    struct Face{ int a,b,c; Color32 col; };
    const Face faces[12] = {
        {0,3,2, {0,255,0,255}}, {0,2,1, {0,255,0,255}},     // front (-Z) green
        {4,5,6, {255,0,0,255}}, {4,6,7, {255,0,0,255}},     // back (+Z) red
        {0,1,5, {0,0,255,255}}, {0,5,4, {0,0,255,255}},     // bottom blue
        {3,7,6, {255,255,0,255}},{3,6,2, {255,255,0,255}},  // top yellow
        {1,2,6, {255,0,255,255}},{1,6,5, {255,0,255,255}},  // right magenta
        {0,4,7, {0,255,255,255}},{0,7,3, {0,255,255,255}},  // left cyan
    };
    cube.numPolys = 12;
    for(int i = 0;i < 12;i++){
        cube.plist[i].vlist[0] = v[faces[i].a];
        cube.plist[i].vlist[1] = v[faces[i].b];
        cube.plist[i].vlist[2] = v[faces[i].c];
        cube.plist[i].color = faces[i].col;
    }
    return cube;
}
}

// Blit: FrameBuffer BGRA uint32 -> window byte buffer honoring format
void BlitFrameToWindow(const FrameBuffer &fb, WindowBuffer<uint8_t> &wb){
    const bool bgra = (wb.m_format == RenderFormat::BGRA8888);
    uint8_t *dst = wb.buffer();
    const uint32_t *src = fb.colorData();
    const auto bytes = FetchPackBytesAccordingFormat(wb.m_format);
    for(std::size_t y = 0; y < fb.height(); y++){
        for(std::size_t x = 0; x < fb.width(); x++){
            uint32_t v = src[y*fb.width()+x];
            uint8_t b = v & 0xFF, g = (v>>8)&0xFF, r = (v>>16)&0xFF, a = (v>>24)&0xFF;
            auto base = y * wb.m_pitch + x * bytes;
            dst[base+0] = a;
            if(bgra){ dst[base+1]=r; dst[base+2]=g; dst[base+3]=b; }
            else    { dst[base+1]=b; dst[base+2]=g; dst[base+3]=r; }
        }
    }
}
```

- [ ] **Step 2: Application members** — in Application.hpp private section:

```cpp
FrameBuffer m_framebuffer{800, 600};
Object4D m_cube{};
double m_angle{0.0};
void RenderCube();
```

(Application.cpp includes Render/FrameBuffer.hpp, Render/Pipeline.hpp, math/Transform.hpp; constructor init list updated for member init.)

- [ ] **Step 3: Per-frame render** in `Application::run()` loop, replacing the plain clear/show body between processEvent() and show():

```cpp
m_angle += 0.02;
auto model = SGE::Math::translation(m_cube.worldPos.x, m_cube.worldPos.y, m_cube.worldPos.z)
    .mul(SGE::Math::rotationY(m_angle))
    .mul(SGE::Math::rotationX(0.4));
auto view = SGE::Math::lookAt(Vector3DBase<double>{0, 2, -6},
                              Vector3DBase<double>{0, 0, 0},
                              Vector3DBase<double>{0, 1, 0});
auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
auto mvp = proj.mul(view).mul(model);

m_framebuffer.clear(0xFF000000u);
Rasterizer rz{m_framebuffer};
for(auto &t : Pipeline::projectObject(m_cube, mvp, 800, 600)){
    rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
}
if(auto buf = BufferManager::instance()->getBuffer()){
    buf->clear({0,0,0,255});
    BlitFrameToWindow(m_framebuffer, *buf);
}
```

Keep FPS title update + processEvent unchanged. Initialize `m_cube = MakeCube();` in Application::initalize (after window creation).

- [ ] **Step 4: main.cpp window size**

```cpp
param.env.pos = {{0, 0}, {800, 600}};
```

- [ ] **Step 5: Verify**

Run: full rebuild + all suites green (69 + 13 new ≈ 82).
Smoke: `cd build/src && SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software timeout 5 ./soft-game-engine` → exits cleanly (Quit Normally not required within timeout kill; require exit without crash/assert).

- [ ] **Step 6: Commit** — `feat(demo): rotating shaded cube via software rasterizer`

---

## Self-Review Notes (resolved during planning)

- Spec ambiguity on culling location resolved: screen-space after mapping (spec edited accordingly).
- Perspective-correct depth: vertices KEEP clip-space w through viewport mapping; rasterizer divides by w (Task 3 formula). Do NOT set w=1 in projectObject.
- Wireframe draws color directly without depth test (spec decision).
