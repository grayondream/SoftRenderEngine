# OpenGL Samples 对齐工程设计（2026-08-23）

## 目标
将 `/home/ares/workspace/GraphicsAPILearn` 的 OpenGL samples 渲染效果对齐移植到 SoftGameEngine。
素材（纹理/模型/hdr/skybox）从该仓库拷贝至本工程 `assets/`。

## 用户决策
1. **范围**：~39 个有视觉意义的全部对齐（跳过 UniformBuffer/AdvancedGLSL/TemplateTest 等纯 GL API 学习项）。
2. **架构**：SampleFactory——每个 sample 一个独立文件，统一 IScene 接口。
3. **旧场景**：现有 7 大 showcase 场景迁入统一架构。

## 一、引擎能力新增

| 子系统 | 设计 |
|--------|------|
| ImageLoader | third_party 拷贝 stb_image；`ImageLoader::load(path)` → RGBA8；`loadHdr(path)` → float RGB；Texture 由内存构造（已有） |
| 后处理链 | HDR FrameBufferF（float RGB）；`ScreenQuad::draw(dst, fn)` 全屏像素回调；Pass：ToneMap(Reinhard/ACES)/Gamma/Bloom(阈值提取→HV高斯×N→additive)/SSAO(法线+深度缓冲，64点半球+噪声+模糊)/Defer(G-Buffer albedo+normal+pos 三FB，光照pass) |
| 切线空间 | Primitives/OBJ 计算逐三角形 tangent；ShadingContext 增加 TBN；NormalMap 用切线空间采样；ParallaxMap steep 视差(8层) |
| Cubemap | CubeMap 类(6×Texture)；天空盒=全屏 pass 按逆VP射线方向采样；IBL：hdr→cube、辐照度卷积(余弦加权)、预滤波 mip(GGX 重要性近似)、BRDF LUT(split-sum 解析积分) |
| MSAA | Rasterizer 4x 子采样模式：每像素 4 覆盖判定+加权 resolve |
| MTL | ObjLoader 解析 .mtl Kd/Ks/Ns/map_Kd/map_Ks/map_bump；per-material 分组绘制 |

## 二、SampleFactory 架构

```cpp
class IScene {  // src/Samples/IScene.hpp
public:
    virtual ~IScene() = default;
    virtual void setup(Application&) {}
    virtual void render(Application&) = 0;
    virtual void drawUi(Application&) {}
    virtual const char* name() const = 0;
};
```
- `SceneRegistry`：分组名→有序列表；Application 下拉/树选择；切换时重建场景状态。
- Application 保留共享资源：framebuffer/camera/rig/m_angle/obj缓存；`renderScene()` 委托当前 IScene。
- 目录：`src/Samples/{Base,Light,LightAdv,Advanced,PBR,Model,Showcase}/`

## 三、场景清单（46 项）

**Showcase(7)**：几何陈列 / Mipmap滤波 / 透明排序 / 聚光PCF阴影 / 环境反射折射 / PBR球阵 / 光锥光追（现有逻辑原样迁入）

**Base(5)**：Triangle 彩色渐变三角 / Rect 纹理矩形 / Cube 贴图立方体 / Camera 相机漫游 / SimpleTexture 基础采样

**Light(9)**：Ambient / Diffuse / Specular / Material(四材质盒) / LightMap(漫反射+高光贴图 container2) / 方向光 / 点光线性衰减 / 聚光软边 / 多光源四灯阵

**LightAdv(10)**：Phong vs BlinnPhong 高光对比 / 2D阴影映射 / 点光cube阴影(未接场景能力补场景) / 法线贴图(brickwall_normal) / 视差映射(bricks2_disp) / Bloom / HDR+Tonemap / Gamma / SSAO / Defer

**Advanced(10)**：DepthTest可视化 / Blend半透明窗(grass/window.png排序) / CullFace剔除演示 / Skybox(6面或newport_loft) / FrameBuffer离屏渲染玩具 / Explode顶点爆炸(CPU沿法线偏移+时间) / NormalLine法线线段 / MultiInstance立方体阵(1000次变换复用mesh) / Saturn土星(planet模型+环) / MSAA 4x对比

**PBR(4)**：PBRBase球阵(rusted_iron参数) / PBRTexture五件套容器 / IBL漫反射辐照度(loft.hdr) / IBL高光预滤波+LUT

**Model(1)**：LoadModel(nanosuit/backpack obj+mtl)

## 四、素材拷贝

- `assets/textures/` ← res/img（container2全套/bricks2全套/brickwall/metal/marble/wood/grass/window/dog/rusted_iron/Skybox/newport_loft.hdr）
- `assets/models/nanosuit|backpack` ← res/Model（obj+mtl+同目录贴图）

## 五、实施批次（每批：构建零警告+21套件+golden+冒烟+推送）

- **A 基建**：stb拷贝/ImageLoader/素材拷贝/IScene+Registry/7 showcase 迁入回归
- **B 基础与光照**：Base 5 + Light 9（现有能力组合）
- **C 表面与几何**：tangent链/NormalMap/ParallaxMap/DepthTest/Blend/CullFace/Explode/NormalLine/MultiInstance/Saturn
- **D 后处理**：HDR FB/ScreenQuad/ToneMap/Gamma/Bloom/Skybox
- **E 高级渲染**：SSAO/Defer/FrameBuffer离屏/MSAA
- **F IBL与PBR贴图**：hdr loader/cube卷积/预滤波/LUT/PBR 4场景
- **G 模型与收尾**：MTL解析/LoadModel/README/总回归

## 验收标准
- 46 场景全部可切换渲染，无崩溃；无头冒烟逐场景过一遍
- 21 套件全绿 + golden 门禁保持
- 每批提交推送 develop
