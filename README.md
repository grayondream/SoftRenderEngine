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

## 示例场景库（41 项，ImGui 下拉分组或 ]/[ 循环切换；`SGE_START_SCENE=N` 指定起始项）

对齐 GraphicsAPILearn 的 OpenGL samples 渲染效果，按组组织：

| 组 | 场景 | 能力要点 |
|----|------|---------|
| Showcase (7) | 几何陈列 / Mipmap滤波 / 透明排序 / 聚光PCF / 环境反射折射 / PBR球阵 / 光锥光追 | 引擎七大综合演示 |
| Base (5) | Triangle / TexturedRect / TexturedCube(container2) / CameraWalk / SimpleTexture | 顶点色、纹理采样、相机漫游 |
| Light (9) | Ambient / Diffuse / Specular / Material×4 / LightMap / 方向光 / 点光衰减 / 聚光软边 / 多光源 | Blinn-Phong 全家桶 |
| LightAdv (10) | NormalMap(brickwall) / ParallaxMap(steep) / Bloom / HDR三分屏 / Gamma / SSAO / Defer + Shadow/PCF/点光阴影 | 切线空间贴图与后处理链 |
| Advanced (10) | DepthTest / CullFace / Blend窗排序 / Explode / NormalLine / Instancing / Saturn / Skybox(loft.hdr) / RenderToTexture / SSAA | 几何模拟与离屏渲染 |
| PBR (2) | Cook-Torrance 球阵 / rusted_iron 五件套贴图球 / IBL漫反射(loft) | GGX/Smith/Schlick + 环境光 |
| Model (1) | nanosuit.obj 加载旋转展示 | 自研 OBJ 加载器 |

已知简化：MSAA 以 2x 盒滤波超采样等效实现；IBL 为 equirect 直接采样近似（未做 cubemap 辐照度卷积/预滤波/BRDF LUT）；模型 MTL 材质暂以统一色调代替。

## 质量门禁

```bash
for t in build/test/math_* build/test/render_*; do ./$t; done   # 21 套件全绿
cd build && ctest -R golden_image                                # 黄金图像基线比对
```

黄金图像基线说明见 `assets/golden/README.md`。性能基准：`./build/tools/bench_raster`。
