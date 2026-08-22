# 光照系统设计：像素级 Phong（Lambert + Blinn-Phong）

日期：2026-08-22
状态：已确认（方案 A）
前置：纹理映射特性已完成（ScreenVertex.u/v、透视校正插值框架、drawTriangleTextured）

## 目标

为软渲染器加入像素级光照：环境光 + Lambert 漫反射 + Blinn-Phong 高光；支持方向光与点光；法线经逆转置矩阵变换后透视校正插值；接入旋转立方体 Demo 呈现立体明暗。

## 非目标（YAGNI）

- 聚光灯、多级光照衰减模型（二次项）、阴影映射
- Gouraud 顶点着色路径（Object4D 的 ShadeMode 枚举本特性不落地）
- 材质系统抽象（shininess/specularStrength 先放 LightingRig）
- 4D 通用矩阵求逆（法线矩阵用 3×3 解析伴随法绕开）

## 方案选择记录

| 方案 | 说明 | 结论 |
| ---- | ---- | ---- |
| A：Light 组件 + shade() 自由函数 + 扩展现有光栅化器 | 单份像素循环骨架，默认参数保持兼容 | **采用** |
| B：独立 drawTriangleLit 第三份骨架 | 三份重复失控 | 弃 |

## §1 Light 组件与着色数学

新文件 `src/Render/Light.hpp/cpp`（render 库源文件追加）：

```cpp
struct DirectionalLight{
    Vector3DBase<double> direction;   // 指向光源(世界系)，使用前归一化
    ColorFlt color;                   // 光色 0..1 浮点 RGB（既有别名 Color<float>）
};

struct PointLight{
    Vector3DBase<double> position;
    ColorFlt color;
    double range{10.0};               // 线性衰减半径
};

struct LightingRig{
    float ambient{0.15f};
    float shininess{32.0f};
    float specularStrength{0.5f};
    std::vector<DirectionalLight> directional{};
    std::vector<PointLight> point{};
};

uint32_t shade(const LightingRig &rig,
               const Color32 &albedo,                  // 纹理采样结果
               const Vector3DBase<double> &N,          // 世界法线(已归一化)
               const Vector3DBase<double> &P,          // 像素世界坐标
               const Vector3DBase<double> &viewPos);   // 相机世界位置
```

### 着色公式（逐像素）

- 方向光：`L = normalize(direction)`；点光：`L = normalize(position - P)`，
  衰减 `atten = clamp(1 - |position-P| / range, 0, 1)`
- `diffuse = max(dot(N,L), 0) × atten × lightColor`
- 半程向量 `H = normalize(L + V)`，`V = normalize(viewPos - P)`；
  `specular = pow(max(dot(N,H), 0), shininess) × specularStrength × atten × lightColor`
- 结果 = `albedo ⊙ (ambient + Σdiffuse) + Σspecular`（高光加光色，不调制 albedo）
- 无异常风格；空 rig 返回纯 `albedo × ambient`；输出按 a<<24|r<<16|g<<8|b 打包并 clamp 到 [0,255]

## §2 法线矩阵与数据流

### 法线矩阵（Transform.hpp 追加）

```cpp
Matrix3DBase<double> normalMatrix(const Matrix4DBase<double> &model);
```

取 model 左上 3×3 记 `M`，返回 `M⁻ᵀ`。伴随余子式解析求解（约 30 行）；`|det| < 1e-12` 时返回单位阵兜底。不依赖数学库缺失的通用求逆。

### ScreenVertex 扩展

```cpp
    double nx = 0, ny = 0, nz = 0;    // 世界法线(光栅化端插值后归一化)
    double wx = 0, wy = 0, wz = 0;    // 像素世界坐标
```

### PolyF4D 扩展

新增局部法线数组（与 vlist 平行，立方体每面三顶点同值）：

```cpp
    Vector3DBase<double> nlist[kTriangleVerticesNumber]{};
```

### projectObject 双矩阵签名

```cpp
std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &model,
                                          const Matrix4DBase<double> &viewProj,
                                          const Matrix3DBase<double> &normalMat,
                                          std::size_t screenW, std::size_t screenH);
```

顶点处理链：`world = model·local` → `clip = viewProj·world` → 现有屏幕映射不变；同时存世界坐标入 `wx/wy/wz`、`normalize(normalMat · n_local)` 入 `nx/ny/nz`。

### 裁剪属性完整性（吸取 Task 4 fix R1 教训，设计期显式约定）

`LerpClip` 必须同步补 `nx,ny,nz` 与 `wx,wy,wz` 的线性插值（透视校正在光栅化端统一处理），并配裁剪插值回归测试。

### 兼容性约定

PipelineTest 既有测试改用新签名（`model=identity, viewProj=proj·view`），断言语义不变。

## §3 光栅化集成、Demo 与测试策略

### ShadingContext 聚合（单指针判空即无光照）

```cpp
struct ShadingContext{
    const LightingRig *rig{};                 // nullptr = 无光照，行为与现状完全一致
    Vector3DBase<double> viewPos{};
};

void drawTriangleTextured(const ScreenVertex &a, const ScreenVertex &b,
                          const ScreenVertex &c, const Texture &tex,
                          const ShadingContext *shading = nullptr,
                          TextureFilter filter = TextureFilter::Bilinear,
                          TextureWrap wrap = TextureWrap::Repeat);
```

像素循环内：透视校正插值 `nx,ny,nz`（归一化）与 `wx,wy,wz` → `tex.sample()` 得 albedo → `shade(rig, albedo, N, P, viewPos)` → setPixel。`shading==nullptr` 时走现有路径，flat/贴图既有调用零改动。

### Demo 接入

- MakeCube 每面加局部法线（三顶点同值）
- RenderCube 拆分矩阵：`model = T·Ry·Rx`，`viewProj = proj·view`，`normalMat = normalMatrix(model)`；`viewPos = {0,2,-6}`
- 布光：环境光 0.15 + 方向光 `direction=(-0.5, 0.8, -1) 归一化` 白光 1.0 + 点光 `position=(2.5,2.5,-4)` 暖白 (1.0,0.95,0.85) range=12——front 面最亮、top 中亮、right 由点光补光，旋转时高光随角度扫过

### 测试策略

| 测试 | 验证点 |
| ---- | ---- |
| Light.DirectionalLambert | 已知夹角数值：正对=满强、45°≈cos45°、背光=0 |
| Light.PointAttenuation | range 内线性衰减数值精确、超出 range=0 |
| Light.SpecularHalfVector | 正反射位高光=strength，偏离按 pow 衰减 |
| Light.NoLightReturnsAmbient | 空 rig → albedo×ambient |
| Transform.NormalMatrixRotation | 纯旋转 M：M⁻ᵀ==M（正交性质） |
| Transform.NormalMatrixScale | 非均匀缩放 s=(2,1,1)：x 法线分量压缩后归一化正确 |
| Pipeline.NormalPassthrough | projectObject 输出顶点携带正确世界坐标+法线 |
| Pipeline.ClipInterpolatesNormal | 跨近平面裁剪插值顶点的法线/世界坐标按 t 精确 |
| Raster.LitVsUnlitGradient | 同一三角形开/关光照亮度差可判别 |
| 回归 | 全部既有套件通过 |

### 验收标准

1. 全部测试绿（既有 + 新增）；
2. dummy driver 冒烟无 crash；
3. 单帧成像：贴图立方体三面亮度分明、点光侧出现可辨识高光斑。
