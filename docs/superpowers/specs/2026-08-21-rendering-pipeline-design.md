# Rendering Pipeline Phase 1: Wireframe + Solid + Z-Buffer

## Overview

Implement the core rasterization pipeline so the engine can display its first real 3D object: a rotating, solid-shaded cube in an SDL window.

## Goals

1. FrameBuffer: color + depth pixel storage with depth testing
2. Rasterizer: barycentric solid triangle fill with perspective-correct depth interpolation; Bresenham wireframe lines
3. Pipeline utilities: MVP transform → backface culling → near-plane clipping → perspective divide → viewport mapping
4. Demo: rotating colored cube rendered with the full pipeline

## Non-Goals (later phases)

- Texturing, Gouraud/Phong lighting, alpha blending
- Far/side plane clipping (near-plane only)
- OBJ loading, camera input controls
- SIMD/multithreaded rasterization

## Architecture

```
Object4D → MVP transform → backface cull → near-plane clip → perspective divide → NDC→pixel
        → Rasterizer (solid: barycentric + Z-buffer / wireframe: Bresenham)
        → FrameBuffer → WindowBuffer(BGRA) → SDL present
```

## Components

### 1. FrameBuffer (`src/Render/FrameBuffer.hpp/.cpp`)

Owns color and depth for one frame. WindowBuffer stays SDL-display-only.

```cpp
class FrameBuffer {
public:
    FrameBuffer(std::size_t width, std::size_t height);
    void setPixel(std::size_t x, std::size_t y, uint32_t bgraColor, float depth);
    // Depth test: write only when depth < stored value.
    void clear(uint32_t bgraColor);   // fill color; reset all depth to FLT_MAX
    void clearDepth();
    uint32_t* colorData();
    float* depthData();
    std::size_t width() const;
    std::size_t height() const;
private:
    std::size_t m_width, m_height;
    std::vector<uint32_t> m_color;
    std::vector<float> m_depth;
};
```

- Out-of-bounds setPixel is ignored (no assert — rasterizer clamps by construction, but guard anyway).
- Initial state after construction: cleared to black, depth = FLT_MAX.

### 2. ScreenVertex / ScreenTriangle (`src/Render/Pipeline.hpp`)

```cpp
struct ScreenVertex {
    double x = 0, y = 0;      // pixel coordinates (origin top-left, y down)
    float z = 0;              // NDC depth in [-1, 1] (smaller = closer after divide)
    float w = 0;              // clip w, kept for perspective-correct interpolation
    Color color;              // render Color (uint32 BGRA payload)
};
struct ScreenTriangle { ScreenVertex v[3]; };
```

Note: `Color` here refers to the render-layer template Color from `src/Render/GeoObject/Color.hpp` (window layer was renamed to `WindowColor`, no conflict).

### 3. Pipeline utilities (`src/Render/Pipeline.hpp/.cpp`)

Free functions in `namespace Pipeline`:

```cpp
// Full path: Object4D polygons → screen-space triangles.
// Steps: mul(mvp) per vertex → backface cull → near-plane clip (z > -w kept,
// Sutherland–Hodgman on the single z+w=0 plane) → perspective divide →
// NDC→pixels: px = (ndc.x*0.5+0.5)*W, py = (1-(ndc.y*0.5+0.5))*H.
std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH);

// Lower-level helpers exposed for tests:
bool isBackFacing(const Vector4DBase<double> &a, const Vector4DBase<double> &b,
                  const Vector4DBase<double> &c);          // cross-product sign test
std::vector<ScreenVertex> clipNearPlane(const std::array<ScreenVertex,3> &tri);
```

Conventions:
- Right-handed world/view space, OpenGL-style projection (already built in Transform.hpp): camera looks down -Z, near maps to NDC -1, far to +1.
- Backface culling: compute signed area of the screen-space triangle; skip if winding is clockwise (counter-clockwise = front). Decided on post-projection coordinates — simple and consistent.
- Clipping happens BEFORE perspective divide (in clip space, using x,y,z,w), which is why clipNearPlane operates on vertices that carry both NDC-ish z and raw w.

Implementation detail: projectObject transforms each polygon's 3 vertices through mvp producing clip-space vertices (x,y,z,w). Culling uses the sign of `(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)` in clip space (w uniform across the triangle for our use, so clip-space winding == screen winding up to y-flip; we cull consistently with the y-flip applied later).

### 4. Rasterizer (`src/Render/Rasterizer.hpp/.cpp`)

```cpp
class Rasterizer {
public:
    explicit Rasterizer(FrameBuffer &fb);
    void drawLine(const ScreenVertex &a, const ScreenVertex &b);      // Bresenham
    void drawTriangleWireframe(const ScreenVertex &v0,
                               const ScreenVertex &v1,
                               const ScreenVertex &v2);               // 3 edges
    void drawTriangleSolid(const ScreenVertex &v0,
                           const ScreenVertex &v1,
                           const ScreenVertex &v2);                   // barycentric
private:
    FrameBuffer &m_fb;
};
```

Solid fill algorithm:
1. Bounding box of the triangle, clamped to framebuffer bounds.
2. Edge functions E0/E1/E2 for the three edges.
3. If area (sum of edge values at any point, or E(v0,v1,v2)) is 0 → skip (degenerate).
4. For each pixel center (x+0.5, y+0.5): inside iff all three edge values have the same sign as area (handles either winding).
5. Top-left rule: pixel exactly on an edge counts only if that edge is a top-left edge — implemented via bias of -epsilon for non-top-left edges when area > 0 (+ epsilon mirrored when area < 0). Prevents double-draw on shared edges.
6. Perspective-correct depth: interpolate `1/w` affinely with barycentric weights (α,β,γ from edge values / area), then `z_view = α·z0/w0 + β·z1/w1 + γ·z2/w2` normalized by `α/w0+β/w1+γ/w2`... simplified: store per-pixel `depth = interp(z/w) / interp(1/w)`; compare against buffer, write color + depth on pass.
7. Color: flat — vertex colors are expected identical per triangle this phase; take v0.color.

Bresenham line: integer error accumulation over the dominant axis; endpoints included; per-pixel uses endpoint z with nearest-endpoint rule (wireframe has no depth correctness requirement this phase; still passes depth test so hidden lines don't overwrite closer solids... decision: wireframe draws WITHOUT depth write but WITH depth test? No — simplest correct-looking demo: wireframe respects existing depth via test, does not modify depth).

Decision recorded: `drawLine` performs depth TEST against current buffer but never writes depth.

### 5. Demo (`src/main.cpp` modification)

Rotating cube, 800×600 window:
- Cube geometry: 8 vertices (±1), 12 triangles, each face assigned a distinct base color.
- Per frame: model = rotationY(t)*rotationX(0.4), view = lookAt({0,2,-6}→{0,0,0},up{0,1,0}), proj = perspective(60°, 800/600, 0.1, 100).
- Build Object4D once; per frame run Pipeline::projectObject → Rasterizer solid fill → blit FrameBuffer color into existing WindowBuffer path → SDL present.
- Frame timing: reuse existing Application loop structure; t advances per event-loop tick.

### 6. Tests

`test/render/FrameBuffer.cpp` (new suite `render_FrameBuffer`):
1. Construction: size, initial color black, depth FLT_MAX.
2. setPixel basic write/read roundtrip.
3. setPixel out-of-bounds ignored (no crash, no write).
4. Depth test: nearer wins; farther rejected; equal rejected.
5. clear resets color and depth.

`test/render/Rasterizer.cpp` (new suite `render_Rasterizer`):
1. Degenerate (zero-area) triangle writes nothing.
2. Axis-aligned small triangle: exact expected pixel count and bounding pixels.
3. Two triangles sharing an edge: no pixel drawn twice (count total written pixels == union size) — verifies top-left rule.
4. Depth interpolation: steep triangle where naive linear z would pick wrong closest pixel; assert nearest pixel color wins.
5. drawLine horizontal / vertical / diagonal: correct endpoint pixels and count.
6. Wireframe of unit square: perimeter pixels only, interior untouched.

Test style follows existing suites (gtest, EXPECT_*; build via globbed CMake — new .cpp files under test/render/ are picked up automatically after reconfigure).

## Error Handling

- All out-of-range pixel writes clamped/ignored defensively.
- Zero-area triangles skipped silently.
- Clip produces 0, 1, or 2 triangles (near-plane quad split); 0-triangle results simply drop.

## Testing Strategy

Unit tests above verify each stage in isolation (FrameBuffer alone, Rasterizer alone, Pipeline helpers with hand-computed vertices). The cube demo is verified by smoke-run (dummy video driver exits cleanly) plus manual visual check.
