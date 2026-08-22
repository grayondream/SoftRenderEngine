# 纹理映射设计：透视校正 UV 插值 + 纹理采样

日期：2026-08-22
状态：已确认（方案 A）
前置：渲染管线 Phase 1（FrameBuffer/Bresenham/重心坐标实体填充/Pipeline 剔除裁剪投影/立方体 Demo）已完成

## 目标

为软渲染器加入纹理映射能力：透视校正的 UV 插值、stb_image 图像加载、最近邻/双线性采样、Repeat/Clamp 寻址，并接入旋转立方体 Demo 验收。

## 非目标（YAGNI）

- mipmap、各向异性过滤
- fragment shader / varying 泛化框架（待 UV/深度/法线三类属性稳定后评估）
- scanline/SIMD 光栅化重写
- 法线插值与光照（下一特性）

## 方案选择记录

| 方案 | 说明 | 结论 |
| ---- | ---- | ---- |
| A：扩展 ScreenVertex + drawTriangleTextured | 复制 solid 骨架，仅替换着色端 | **采用**：贴合现有直白风格、TDD 容易、零回归风险 |
| B：varying + fragment 回调泛化 | 光栅器只算权重，着色委托回调 | 弃：抽象提前，违反 YAGNI |
| C：scanline 重写 | 顺带做 UV 但推翻已验证实现 | 弃：风险大收益小 |

## §1 Texture 类与采样器

### 第三方集成

- `third_party/stb/stb_image.h` vendor 进仓库
- `STB_IMAGE_IMPLEMENTATION` 只在 `Texture.cpp` 一个编译单元定义
- CMake：`third_party/` 加入全局 include 路径；`render` 库源文件追加 `Texture.cpp`

### 接口

```cpp
enum class TextureFilter{ Nearest, Bilinear };
enum class TextureWrap{ Repeat, Clamp };

class Texture{
public:
    Texture(std::size_t w, std::size_t h, const uint32_t *bgra);   // 内存构造(测试/demo棋盘格)
    static Texture loadFromFile(const std::string &path);          // stb_image, 强制4通道
    uint32_t sample(double u, double v,
                    TextureFilter f = TextureFilter::Bilinear,
                    TextureWrap wrap = TextureWrap::Repeat) const;
    std::size_t width() const;
    std::size_t height() const;
private:
    std::size_t m_w{}, m_h{};
    std::vector<uint32_t> m_pixels{};   // 与 FrameBuffer 同款 BGRA 打包(a<<24|r<<16|g<<8|b)
};
```

### 关键约定

1. 像素存储与 `FrameBuffer` 完全同格式（`a<<24|r<<16|g<<8|b`），采样结果直接进 `setPixel`，零转换成本。
2. `loadFromFile` 失败返回 `w=h=0` 的空纹理，不抛异常；`sample` 对空纹理返回黑色 `0xFF000000u`。贴合代码库无异常风格。
3. 采样坐标：
   - 双线性：标准半像素偏移 `fx = u*m_w - 0.5`，对 `floor(fx)` 与 `floor(fx)+1` 两列/两行 4 texel 双线性加权；
   - 最近邻：`floor(u*m_w)` 直接取样；
   - Wrap 先于过滤应用：Repeat 对 texel 坐标取模（`((x % w) + w) % w`），Clamp 钳到 `[0, w-1]`。

## §2 UV 插值数学与 drawTriangleTextured

### ScreenVertex 扩展

```cpp
struct ScreenVertex{
    double x = 0;
    double y = 0;
    float z = 0;
    float w = 1;
    float u = 0, v = 0;      // 新增：纹理坐标(0..1)
    Color32 color{};
};
```

### 透视校正插值公式

屏幕像素处重心权重 `λ0,λ1,λ2` 已知（现有 EdgeFunction 归一化结果），则：

```
invW  = Σ λi / wi              （solid 中已实现，复用同一套变量）
u     = (Σ λi·ui/wi) / invW
v     = (Σ λi·vi/wi) / invW
depth = (Σ λi·zi/wi) / invW    （现有逻辑不变）
```

与 Task 3 深度公式同构，均为「属性/w over 1/w」。

### 函数签名

```cpp
void drawTriangleTextured(const ScreenVertex &a, const ScreenVertex &b,
                          const ScreenVertex &c, const Texture &tex,
                          TextureFilter f = TextureFilter::Bilinear,
                          TextureWrap wrap = TextureWrap::Repeat);
```

### 行为约定

1. 边界框、EdgeFunction、top-left 规则、深度测试、`setPixel` 骨架与 `drawTriangleSolid` 完全一致；仅把"写 color"换成"算 UV → tex.sample → 写纹素"。
2. 深度语义不变：以 clip-space `w` 做透视校正，z-buffer 存校正后的深度。
3. 不修改 `drawTriangleSolid`（flat 色路径保持零回归风险）。

## §3 数据流、Demo 接入与测试策略

### 数据流

```
PolyF4D 新增 uvlist[3] (struct UV2D{double u,v})
        │ projectObject: sv[i].u/v = poly.uvlist[i].u/v （透传，无变换）
        ▼
ScreenVertex.u/v ── drawTriangleTextured ──► tex.sample ──► setPixel
```

- `UV2D` 默认 `{0,0}`，既有代码零影响。
- `projectObject` 在填充 `ScreenVertex` 时同步透传 UV。

### Demo 接入

- 立方体每面四角 UV 按顶点索引 `A(0,0) B(1,0) C(1,1) D(0,1)` 映射；对角线 AC 将面拆为两个三角形：`A,B,C` 与 `A,C,D`；
- Demo 纹理：运行时生成的 8×8 双色棋盘格（内存构造），仓库不引入二进制图片资产；
- `RenderCube()` 改用 `drawTriangleTextured`。

### 测试策略

新增 `test/render/Texture.cpp`（二进制名 render_Texture），扩充 `test/render/Rasterizer.cpp`：

| 测试 | 验证点 |
| ---- | ---- |
| Texture.NearestSample | floor 取样边界正确 |
| Texture.BilinearBlend | 半像素偏移后 4 texel 加权混合数值精确 |
| Texture.WrapRepeat / WrapClamp | u=-0.25、u=1.25 等越界行为 |
| Texture.EmptyReturnsBlack | 空纹理 sample 返回 0xFF000000u |
| Texture.LoadFromFile | 加载提交的 `test/assets/test_4x4.png`，逐像素比对 |
| Textured.AffineVsPerspective | 核心用例：梯形远顶点/近顶点，仿射插值会错、透视校正必须命中正确纹素 |
| Textured.QuadCenterContinuity | 两三角形拼 quad，共享对角线上 UV 连续 |
| 回归 | 全部 12 个既有套件照常通过 |

`test/assets/test_4x4.png`：手工构造的最小合法 PNG（4×4 RGBA），随仓库提交供加载器测试。

### 验收标准

1. 全部测试通过（既有 + 新增）；
2. dummy driver 冒烟正常退出；
3. 单帧 PPM 成像检查：贴图立方体近大远小处棋盘格无仿射扭曲（透视校正生效）。
