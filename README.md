# SoftGameEngine

C++20 软件光栅化游戏引擎：纯 CPU 渲染（无 GPU API），SDL2 仅负责窗口/输入/上屏，Dear ImGui 仅用于调试与参数切换。

## 构建

```bash
scripts/buildandrun.sh          # 配置(如需)+构建+运行主 Demo
# 或手动
cmake -B build -S . && cmake --build build -j$(nproc)
```

可选：`-DENABLE_SANITIZER=ON`（ASan+UBSan，建议配合 g++ 工具链）。

## 运行示例

```bash
./build/src/soft-game-engine     # 交互窗口
SGE_MAX_FRAMES=30 ./build/src/soft-game-engine        # 无头确定性冒烟
SGE_TEST_BARS=1   ./build/src/soft-game-engine        # 通道诊断色带
```

操作：WASD/RF 平移，方向键转向；`[`/`]` 切换场景；ImGui 面板切换场景与参数。

## 示例场景库（39 项，严格按 GraphicsAPILearn `AppType` 枚举顺序排列）

ImGui 下拉分组或 ]/[ 循环切换；`SGE_START_SCENE=N` 指定起始项。

| # | GL AppType | 场景 | # | GL AppType | 场景 |
|---|-----------|------|---|-----------|------|
| 0 | Triangle | 彩色渐变三角 | 20 | SimpleGeometry | 参数化几何陈列(球/环/茶壶/锥/柱) |
| 1 | Rect | 纹理矩形 | 21 | Explode | CPU 顶点爆炸 |
| 2 | SimpleTexture | 基础纹理 | 22 | NormalLine | 法线线段可视化 |
| 3 | Cube | container2 贴图立方体 | 23 | MultiInstance | 100 立方体实例化 |
| 4 | Camera | 相机漫游 | 24 | MultiInstance_Saturn | 土星环系统 |
| 5 | Ambient | 纯环境光 | 25 | Msaa(SSAA) | 2x 超采样抗锯齿 |
| 6 | Diffuse | Lambert 漫反射 | 26 | BlinnPhong | Phong vs Blinn 高光对比 |
| 7 | Specular | 镜面高光 | 27 | Gamma | gamma 校正阶梯 |
| 8 | Material | 四材质系统 | 28 | Shadow_Map | 聚光阴影 + PCF 可调 |
| 9 | LightMap | 漫反射贴图光照 | 29 | Shadow_PointLight | 点光六面 cube 阴影 |
| 10 | Source_Direction | 方向光 | 30 | NormalMap | brickwall 法线贴图 |
| 11 | Source_Point | 点光衰减 | 31 | ParallaxMap | bricks2 steep 视差 |
| 12 | Source_Spot | 聚光软边 | 32 | Hdr | Clamp/Reinhard/ACES 三分屏 |
| 13 | Source_Mult | 四色多光源 | 33 | Bloom | 亮度提取+高斯泛光 |
| 14 | LoadModel | nanosuit.obj 展示 | 34 | Defer | 延迟着色 G-buffer |
| 15 | DepthTest | 深度交叠 | 35 | SSAO | 深度对比 AO |
| 16 | Blend | 半透明窗排序 | 36 | PBR_Base | metal×rough 球阵可调 |
| 17 | CullFace | 背面剔除开关 | 37 | PBR_Texture | rusted_iron 五件套 |
| 18 | FrameBuffer | 离屏渲染贴图立方体 | 38 | PBR_IBL_Irradiance | loft.hdr 环境漫反射 |
| 19 | SkyBox | newport_loft.hdr 天空盒 | | | |

跳过的纯 GL API 学习项：TemplateTest(stencil)/AdvancedGLSL/UniformBuffer/IBL_Irradiance_Conversion/IBL_Specular。
已知简化：MSAA 以 2x 盒滤波超采样等效；IBL 为 equirect 直接采样近似；MTL 材质暂以统一色调代替。

## 质量门禁

```bash
for t in build/test/math_* build/test/render_*; do ./$t; done   # 21 套件全绿
cd build && ctest -R golden_image                                # 黄金图像基线比对
```

黄金图像基线说明见 `assets/golden/README.md`。性能基准：`./build/tools/bench_raster`。
