# Math Transform Pipeline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add MVP transform pipeline (matrix multiplication + transform matrix builders) to enable 3D rendering.

**Architecture:** Matrix4DBase gets `mul()` member functions for true matrix multiplication and matrix-vector multiplication. A new `Transform.hpp` provides factory functions for translation, rotation, scale, lookAt, perspective, and orthographic matrices.

**Tech Stack:** C++17, existing Matrix4DBase/Vector4DBase classes, `std::cos`/`std::sin`

## Global Constraints

- Build system: `cmake --build build -j$(nproc)`
- Compiler: clang++ (set in CMakeLists.txt)
- Warning flags: `-Wall -Wextra -Werror` (via target_compile_options)
- Test framework: Google Test (FetchContent, release-1.12.1)
- Run tests: `cd build && for t in test/math_*; do ./$t; done`
- All existing 56 tests must continue to pass

---

## Task 1: Add mul() to DynamicMatrix4D

**Files:**
- Modify: `src/math/Matrix/DynamicMatrix4D.hpp`
- Test: `test/math/MatrixMathOp.cpp` (extend existing)

**Interfaces:**
- Consumes: existing Matrix4DBase<T>, Vector4DBase<T>
- Produces: `Matrix4DBase<T>::mul(Matrix4DBase<U>)` and `Matrix4DBase<T>::mul(Vector4DBase<U>)`

- [ ] **Step 1: Read DynamicMatrix4D.hpp to find insertion point**

Read `src/math/Matrix/DynamicMatrix4D.hpp` to understand the class structure. The `mul()` methods should be added as public member functions after the existing `fill()` method (around line 206).

- [ ] **Step 2: Add Matrix4D × Matrix4D mul()**

In `src/math/Matrix/DynamicMatrix4D.hpp`, add after the `fill()` method (line 206):

```cpp
template<class U>
Matrix4DBase<ValueType> mul(const Matrix4DBase<U> &mat) const{
    assert(this->d1 == mat.d2);
    Matrix4DBase<ValueType> ret(this->d4, this->d3, this->d2, mat.d1);
    for(auto c = 0; c < this->d4; c++){
        for(auto k = 0; k < this->d3; k++){
            for(auto i = 0; i < this->d2; i++){
                for(auto j = 0; j < mat.d1; j++){
                    ValueType sum{};
                    for(auto s = 0; s < this->d1; s++){
                        sum += (*this)[c][k][i][s] * mat[c][k][s][j];
                    }
                    ret[c][k][i][j] = sum;
                }
            }
        }
    }
    return ret;
}
```

- [ ] **Step 3: Add Matrix4D × Vector4D mul()**

In `src/math/Matrix/DynamicMatrix4D.hpp`, add after the Matrix4D × Matrix4D mul():

```cpp
template<class U>
Vector4DBase<ValueType> mul(const Vector4DBase<U> &vec) const{
    return Vector4DBase<ValueType>(
        (*this)[0][0][0][0]*vec.x + (*this)[0][0][0][1]*vec.y + (*this)[0][0][0][2]*vec.z + (*this)[0][0][0][3]*vec.w,
        (*this)[0][0][1][0]*vec.x + (*this)[0][0][1][1]*vec.y + (*this)[0][0][1][2]*vec.z + (*this)[0][0][1][3]*vec.w,
        (*this)[0][0][2][0]*vec.x + (*this)[0][0][2][1]*vec.y + (*this)[0][0][2][2]*vec.z + (*this)[0][0][2][3]*vec.w,
        (*this)[0][0][3][0]*vec.x + (*this)[0][0][3][1]*vec.y + (*this)[0][0][3][2]*vec.z + (*this)[0][0][3][3]*vec.w
    );
}
```

- [ ] **Step 4: Build and run existing tests**

Run: `cmake --build build -j$(nproc) 2>&1 | grep error`
Run: `cd build && for t in test/math_*; do r=$(timeout 120 ./$t 2>&1 | grep -E "PASSED|FAILED" | tr '\n' ' '); echo "$t: $r"; done`
Expected: All 56 tests pass, no errors

- [ ] **Step 5: Commit**

```bash
git add src/math/Matrix/DynamicMatrix4D.hpp
git commit -m "feat: add mul() member functions to DynamicMatrix4D"
```

---

## Task 2: Add mul() to StaticMatrix4D

**Files:**
- Modify: `src/math/Matrix/StaticMatrix4D.hpp`
- Test: `test/math/MatrixMathOp.cpp` (extend existing)

**Interfaces:**
- Consumes: existing StaticMatrix4DBase<T, d4, d3, d2, d1>, Vector4DBase<T>
- Produces: `StaticMatrix4DBase<T,...>::mul(StaticMatrix4DBase<U,...>)` and `StaticMatrix4DBase<T,...>::mul(Vector4DBase<U>)`

- [ ] **Step 1: Read StaticMatrix4D.hpp to find insertion point**

Read `src/math/Matrix/StaticMatrix4D.hpp` to understand the class structure. The `mul()` methods should be added as public member functions after the existing `fill()` method.

- [ ] **Step 2: Add StaticMatrix4D × StaticMatrix4D mul()**

In `src/math/Matrix/StaticMatrix4D.hpp`, add after the `fill()` method:

```cpp
template<class U, StaticMatrixSizeType od4, StaticMatrixSizeType od3, StaticMatrixSizeType od2, StaticMatrixSizeType od1>
StaticMatrix4DBase<ValueType, d4, d3, d2, od1> mul(
    const StaticMatrix4DBase<U, od4, od3, od2, od1> &mat) const{
    static_assert(d1 == od2, "Matrix dimensions must match for multiplication");
    StaticMatrix4DBase<ValueType, d4, d3, d2, od1> ret{};
    for(auto c = 0; c < d4; c++){
        for(auto k = 0; k < d3; k++){
            for(auto i = 0; i < d2; i++){
                for(auto j = 0; j < od1; j++){
                    ValueType sum{};
                    for(auto s = 0; s < d1; s++){
                        sum += (*this)[c][k][i][s] * mat[c][k][s][j];
                    }
                    ret[c][k][i][j] = sum;
                }
            }
        }
    }
    return ret;
}
```

- [ ] **Step 3: Add StaticMatrix4D × Vector4D mul()**

In `src/math/Matrix/StaticMatrix4D.hpp`, add after the matrix × matrix mul():

```cpp
template<class U>
Vector4DBase<ValueType> mul(const Vector4DBase<U> &vec) const{
    static_assert(d2 == 4 && d1 == 4, "Matrix must be 4x4 for vector multiplication");
    return Vector4DBase<ValueType>(
        (*this)[0][0][0][0]*vec.x + (*this)[0][0][0][1]*vec.y + (*this)[0][0][0][2]*vec.z + (*this)[0][0][0][3]*vec.w,
        (*this)[0][0][1][0]*vec.x + (*this)[0][0][1][1]*vec.y + (*this)[0][0][1][2]*vec.z + (*this)[0][0][1][3]*vec.w,
        (*this)[0][0][2][0]*vec.x + (*this)[0][0][2][1]*vec.y + (*this)[0][0][2][2]*vec.z + (*this)[0][0][2][3]*vec.w,
        (*this)[0][0][3][0]*vec.x + (*this)[0][0][3][1]*vec.y + (*this)[0][0][3][2]*vec.z + (*this)[0][0][3][3]*vec.w
    );
}
```

- [ ] **Step 4: Build and run existing tests**

Run: `cmake --build build -j$(nproc) 2>&1 | grep error`
Run: `cd build && for t in test/math_*; do r=$(timeout 120 ./$t 2>&1 | grep -E "PASSED|FAILED" | tr '\n' ' '); echo "$t: $r"; done`
Expected: All 56 tests pass, no errors

- [ ] **Step 5: Commit**

```bash
git add src/math/Matrix/StaticMatrix4D.hpp
git commit -m "feat: add mul() member functions to StaticMatrix4D"
```

---

## Task 3: Create Transform.hpp with basic transforms

**Files:**
- Create: `src/math/Transform.hpp`
- Test: `test/math/TransformTest.cpp` (new file)

**Interfaces:**
- Consumes: Matrix4DBase<T>, Vector3DBase<T>, Vector4DBase<T>, `std::cos`, `std::sin`
- Produces: `SGE::Math::translation()`, `SGE::Math::rotationX()`, `SGE::Math::rotationY()`, `SGE::Math::rotationZ()`, `SGE::Math::scale()`

- [ ] **Step 1: Create Transform.hpp**

Create `src/math/Transform.hpp`:

```cpp
#pragma once
#include "Matrix/Matrix.hpp"
#include <cmath>

namespace SGE::Math {

template<class T>
Matrix4DBase<T> translation(T x, T y, T z){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    m[0][0][0][3] = x;
    m[0][0][1][3] = y;
    m[0][0][2][3] = z;
    return m;
}

template<class T>
Matrix4DBase<T> rotationX(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][1][1] = c;
    m[0][0][1][2] = -s;
    m[0][0][2][1] = s;
    m[0][0][2][2] = c;
    return m;
}

template<class T>
Matrix4DBase<T> rotationY(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][0][0] = c;
    m[0][0][0][2] = s;
    m[0][0][2][0] = -s;
    m[0][0][2][2] = c;
    return m;
}

template<class T>
Matrix4DBase<T> rotationZ(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][0][0] = c;
    m[0][0][0][1] = -s;
    m[0][0][1][0] = s;
    m[0][0][1][1] = c;
    return m;
}

template<class T>
Matrix4DBase<T> scale(T sx, T sy, T sz){
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = sx;
    m[0][0][1][1] = sy;
    m[0][0][2][2] = sz;
    m[0][0][3][3] = T{1};
    return m;
}

} // namespace SGE::Math
```

- [ ] **Step 2: Create TransformTest.cpp**

Create `test/math/TransformTest.cpp`:

```cpp
#include "Transform.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(TransformTest, Translation){
    auto m = SGE::Math::translation(1.0, 2.0, 3.0);
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_DOUBLE_EQ(result.x, 1.0);
    EXPECT_DOUBLE_EQ(result.y, 2.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
    EXPECT_DOUBLE_EQ(result.w, 1.0);
}

TEST(TransformTest, RotationX){
    auto m = SGE::Math::rotationX(M_PI / 2.0);
    Vector4DBase<double> v{0.0, 1.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, 1.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, RotationY){
    auto m = SGE::Math::rotationY(M_PI / 2.0);
    Vector4DBase<double> v{1.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, -1.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, RotationZ){
    auto m = SGE::Math::rotationZ(M_PI / 2.0);
    Vector4DBase<double> v{1.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 1.0, 1e-10);
    EXPECT_NEAR(result.z, 0.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, Scale){
    auto m = SGE::Math::scale(2.0, 3.0, 4.0);
    Vector4DBase<double> v{1.0, 1.0, 1.0, 1.0};
    auto result = m.mul(v);
    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 4.0);
    EXPECT_DOUBLE_EQ(result.w, 1.0);
}

TEST(TransformTest, MatrixMul){
    auto t = SGE::Math::translation(1.0, 0.0, 0.0);
    auto s = SGE::Math::scale(2.0, 2.0, 2.0);
    auto ts = t.mul(s);
    Vector4DBase<double> v{1.0, 1.0, 1.0, 1.0};
    auto result = ts.mul(v);
    EXPECT_NEAR(result.x, 3.0, 1e-10);
    EXPECT_NEAR(result.y, 2.0, 1e-10);
    EXPECT_NEAR(result.z, 2.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

- [ ] **Step 3: Add TransformTest to CMakeLists.txt**

In `test/CMakeLists.txt`, add:

```cmake
add_executable(math_Transform TransformTest.cpp)
target_link_libraries(math_Transform gtest gtest_main pthread math)
```

- [ ] **Step 4: Build and run tests**

Run: `cmake --build build -j$(nproc) 2>&1 | grep error`
Run: `cd build && ./test/math_Transform`
Expected: All 6 transform tests pass

- [ ] **Step 5: Commit**

```bash
git add src/math/Transform.hpp test/math/TransformTest.cpp test/CMakeLists.txt
git commit -m "feat: add Transform.hpp with translation, rotation, scale"
```

---

## Task 4: Add lookAt and projection transforms

**Files:**
- Modify: `src/math/Transform.hpp`
- Modify: `test/math/TransformTest.cpp`

**Interfaces:**
- Consumes: Matrix4DBase<T>, Vector3DBase<T>, `std::tan`, `std::sqrt`
- Produces: `SGE::Math::lookAt()`, `SGE::Math::perspective()`, `SGE::Math::orthographic()`

- [ ] **Step 1: Add lookAt to Transform.hpp**

In `src/math/Transform.hpp`, add before the closing `} // namespace SGE::Math`:

```cpp
template<class T>
Matrix4DBase<T> lookAt(
    const Vector3DBase<T> &eye,
    const Vector3DBase<T> &center,
    const Vector3DBase<T> &up){
    // Build camera basis
    Vector3DBase<T> f = center - eye;
    f.normalize();
    Vector3DBase<T> r = f.cross(up);
    r.normalize();
    Vector3DBase<T> u = r.cross(f);

    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = r.x;
    m[0][0][0][1] = r.y;
    m[0][0][0][2] = r.z;
    m[0][0][1][0] = u.x;
    m[0][0][1][1] = u.y;
    m[0][0][1][2] = u.z;
    m[0][0][2][0] = -f.x;
    m[0][0][2][1] = -f.y;
    m[0][0][2][2] = -f.z;
    m[0][0][3][3] = T{1};

    // Translation component
    m[0][0][0][3] = -(r.dot(eye));
    m[0][0][1][3] = -(u.dot(eye));
    m[0][0][2][3] = f.dot(eye);

    return m;
}
```

- [ ] **Step 2: Add perspective to Transform.hpp**

In `src/math/Transform.hpp`, add after `lookAt`:

```cpp
template<class T>
Matrix4DBase<T> perspective(T fovY, T aspect, T nearVal, T farVal){
    T tanHalf = std::tan(fovY / T{2});
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = T{1} / (aspect * tanHalf);
    m[0][0][1][1] = T{1} / tanHalf;
    m[0][0][2][2] = -(farVal + nearVal) / (farVal - nearVal);
    m[0][0][2][3] = -(T{2} * farVal * nearVal) / (farVal - nearVal);
    m[0][0][3][2] = T{-1};
    return m;
}
```

- [ ] **Step 3: Add orthographic to Transform.hpp**

In `src/math/Transform.hpp`, add after `perspective`:

```cpp
template<class T>
Matrix4DBase<T> orthographic(T left, T right, T bottom, T top, T nearVal, T farVal){
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = T{2} / (right - left);
    m[0][0][1][1] = T{2} / (top - bottom);
    m[0][0][2][2] = T{-2} / (farVal - nearVal);
    m[0][0][0][3] = -(right + left) / (right - left);
    m[0][0][1][3] = -(top + bottom) / (top - bottom);
    m[0][0][2][3] = -(farVal + nearVal) / (farVal - nearVal);
    m[0][0][3][3] = T{1};
    return m;
}
```

- [ ] **Step 4: Add tests for lookAt, perspective, orthographic**

In `test/math/TransformTest.cpp`, add before `main`:

```cpp
TEST(TransformTest, LookAt){
    auto m = SGE::Math::lookAt(
        Vector3DBase<double>{0.0, 0.0, 0.0},
        Vector3DBase<double>{0.0, 0.0, -1.0},
        Vector3DBase<double>{0.0, 1.0, 0.0});
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, 0.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, Perspective){
    auto m = SGE::Math::perspective(M_PI / 4.0, 800.0/600.0, 0.1, 100.0);
    Vector4DBase<double> v{0.0, 0.0, -1.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_TRUE(result.z < 0.0);
    EXPECT_TRUE(result.w > 0.0);
}

TEST(TransformTest, Orthographic){
    auto m = SGE::Math::orthographic(-1.0, 1.0, -1.0, 1.0, 0.1, 100.0);
    Vector4DBase<double> v{0.0, 0.0, -1.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, -1.0 + 2.0 * (0.1 - (-1.0)) / (100.0 - 0.1), 1e-6);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, MVPPipeline){
    auto model = SGE::Math::translation(0.0, 0.0, -5.0);
    auto view = SGE::Math::lookAt(
        Vector3DBase<double>{0.0, 0.0, 0.0},
        Vector3DBase<double>{0.0, 0.0, -1.0},
        Vector3DBase<double>{0.0, 1.0, 0.0});
    auto proj = SGE::Math::perspective(M_PI / 4.0, 1.0, 0.1, 100.0);
    auto mvp = proj.mul(view).mul(model);
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = mvp.mul(v);
    // Point at (0,0,-5) should project to center of screen
    EXPECT_NEAR(result.x / result.w, 0.0, 1e-6);
    EXPECT_NEAR(result.y / result.w, 0.0, 1e-6);
    EXPECT_TRUE(result.z / result.w > -1.0 && result.z / result.w < 1.0);
}
```

- [ ] **Step 5: Build and run all tests**

Run: `cmake --build build -j$(nproc) 2>&1 | grep error`
Run: `cd build && for t in test/math_*; do r=$(timeout 120 ./$t 2>&1 | grep -E "PASSED|FAILED" | tr '\n' ' '); echo "$t: $r"; done`
Expected: All tests pass including new TransformTest (10 tests)

- [ ] **Step 6: Commit**

```bash
git add src/math/Transform.hpp test/math/TransformTest.cpp
git commit -m "feat: add lookAt, perspective, orthographic transforms"
```

---

## Task 5: Final verification

- [ ] **Step 1: Full build and test**

Run: `cmake --build build -j$(nproc) 2>&1 | grep error`
Run: `cd build && for t in test/math_* test/render_*; do r=$(timeout 120 ./$t 2>&1 | grep -E "PASSED|FAILED" | tr '\n' ' '); echo "$t: $r"; done`
Expected: All tests pass (original 56 + new TransformTest 10 = 66)

- [ ] **Step 2: Update PROGRESS.md**

Add entry to `PROGRESS.md` documenting the new features.

- [ ] **Step 3: Final commit**

```bash
git add -A
git commit -m "docs: update PROGRESS.md with transform pipeline"
```
