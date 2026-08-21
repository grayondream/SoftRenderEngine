# Math Library Enhancement: MVP Transform Pipeline

## Overview

Add Model-View-Projection (MVP) transform pipeline to the math library, enabling 3D rendering.

## Goals

1. Matrix4D × Matrix4D multiplication (matrix composition)
2. Matrix4D × Vector4D multiplication (vertex transformation)
3. Complete set of transform matrix builders (translation, rotation, scale, lookAt, perspective, orthographic)

## Design Decisions

- **mul() as member function**: Consistent with existing Matrix1D/2D `mul()` style
- **operator* stays element-wise**: No breaking changes to existing tests
- **Transform.hpp as separate file**: Clean separation from generic Matrix4D class, matches Trigonometric.hpp pattern
- **Use `std::cos`/`std::sin`**: Consistent with existing Trigonometric.hpp usage

## Part 1: Matrix4DBase mul()

### Files Modified
- `src/math/Matrix/DynamicMatrix4D.hpp`
- `src/math/Matrix/StaticMatrix4D.hpp`

### API

```cpp
// Matrix4D × Matrix4D → Matrix4D (true matrix multiplication)
template<class U>
Matrix4DBase<ValueType> mul(const Matrix4DBase<U> &mat) const;

// Matrix4D × Vector4D → Vector4D
template<class U>
Vector4DBase<ValueType> mul(const Vector4DBase<U> &vec) const;
```

### Implementation Notes

- Matrix4D storage is 1×1×4×4 (d4=1, d3=1, d2=4, d1=4), so all matrix operations use `[0][0][i][j]`
- Assert `this->d1 == mat.d2` for matrix multiplication
- Vector multiplication is a special case: 4×4 matrix × 4×1 vector = 4×1 vector
- The vector multiplication directly computes the 4 dot products (row × vector)

## Part 2: Transform.hpp

### File Created
- `src/math/Transform.hpp`

### Namespace
```cpp
namespace SGE::Math { ... }
```

### Transform Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `translation` | `(T x, T y, T z) → Matrix4DBase<T>` | Translation matrix |
| `rotationX` | `(T angle) → Matrix4DBase<T>` | Rotation around X axis |
| `rotationY` | `(T angle) → Matrix4DBase<T>` | Rotation around Y axis |
| `rotationZ` | `(T angle) → Matrix4DBase<T>` | Rotation around Z axis |
| `scale` | `(T sx, T sy, T sz) → Matrix4DBase<T>` | Scale matrix |
| `lookAt` | `(eye, center, up) → Matrix4DBase<T>` | View matrix (right-handed) |
| `perspective` | `(fovY, aspect, near, far) → Matrix4DBase<T>` | Perspective projection |
| `orthographic` | `(l, r, b, t, near, far) → Matrix4DBase<T>` | Orthographic projection |

### Implementation Notes

- All functions return `Matrix4DBase<T>` with dimensions (1,1,4,4)
- `lookAt` builds an orthonormal camera basis (forward, right, up) then constructs the view matrix
- `perspective` uses the standard OpenGL-style perspective matrix with `cot(fovY/2)`
- Angles for rotation functions are in radians
- All matrices are right-handed coordinate system

## Testing

### New Test File
- `test/math/TransformTest.cpp`

### Test Cases

1. **Matrix mul()**: Verify Matrix4D × Matrix4D matches expected result
2. **Matrix-Vector mul()**: Verify Matrix4D × Vector4D matches expected result
3. **Translation**: Verify translation matrix applied to point
4. **Rotation**: Verify rotation around each axis (identity at 0°, back at 180°)
5. **Scale**: Verify scale matrix applied to point
6. **LookAt**: Verify view matrix transforms eye to origin
7. **Perspective**: Verify perspective matrix maps NDC correctly
8. **Orthographic**: Verify orthographic matrix maps bounds correctly
9. **Composition**: Verify MVP pipeline produces correct screen coordinates

## Usage Example

```cpp
#include "Transform.hpp"

// Model transform
auto model = SGE::Math::translation(0.0, 0.0, -5.0)
    .mul(SEGE::Math::rotationY(angle));

// View transform
auto view = SGE::Math::lookAt(
    {0.0, 0.0, 0.0},   // eye
    {0.0, 0.0, -1.0},  // center
    {0.0, 1.0, 0.0}); // up

// Projection
auto proj = SGE::Math::perspective(
    M_PI / 4.0,        // 45° FOV
    800.0 / 600.0,     // aspect ratio
    0.1, 100.0);       // near/far

// MVP composition
auto mvp = proj.mul(view).mul(model);

// Transform vertex
auto clipPos = mvp.mul(worldPos);
```
