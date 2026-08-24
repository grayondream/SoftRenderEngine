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

## 示例场景库（39 项，严格按 GraphicsAPILearn `AppType` 枚举顺序排列，渲染参数从参考源码逐项提取对齐）

ImGui 下拉分组或 ]/[ 循环切换；`SGE_START_SCENE=N` 指定起始项；F12 截图导出 PPM。

| # | 场景 | 参考要点 | # | 场景 | 参考要点 |
|---|------|---------|---|------|---------|
| 0 | Triangle | NDC 红蓝绿三角 | 20 | Simple Geometry Houses | 四角彩色小房子+白顶(GS 模拟) |
| 1 | Rect | 四角红蓝绿白 | 21 | Explode | 渐变球面法线呼吸爆炸 |
| 2 | Simple Texture | dog.jpg 正立直出 | 22 | Normal Lines | 红线框球+黄法线刺 |
| 3 | Textured Cubes | 10 狗图立方体阵 fov45 自转 | 23 | Instanced Sphere Grid | 红梯度线框球海 gap4 |
| 4 | Camera Walkthrough | 同上可漫游 fov60 | 24 | Saturn Ring System | 岩石环 radius20 公转 |
| 5 | Ambient Light | 铜球 0.2·白光+灯泡标记 | 25 | Anti-aliasing (2x SSAA) | dog cube+灰度开关 |
| 6 | Diffuse Light | 光位 (1,1,1.5) amb0.3 | 26 | Blinn-Phong vs Phong | wood 地板 pow32/8 切换 |
| 7 | Specular Light | 轨道光 r=5 amb.1 spec.5 | 27 | Gamma Correction | wood 地板 5 彩光横排 |
| 8 | Material Ramp | 5 球强度 v=(i+1)/5 cam z=6 | 28 | Spot Shadow + PCF | PCF 半径 0-4 可调 |
| 9 | Light Map | container2 双贴图 cube@(1,0,0) | 29 | Point Cube Shadow | 房间盒+z 振荡点光 |
| 10 | Directional Casters | 125 格阵 dir(-.2,-1,-.3) | 30 | Normal Mapping | brickwall 光(0,0,1) |
| 11 | Point Casters | 轨道点光衰减 | 31 | Parallax Mapping | bricks2 steep h=0.1 |
| 12 | Spot Casters | cos12.5°/17.5° 软边 | 32 | HDR Corridor | 反法线长廊+1-exp 曝光 |
| 13 | Multiple Sources | dir+4 点光+聚光组合 | 33 | Bloom | 10 木箱+4 彩灯泛光 |
| 14 | Load Model | nanosuit.obj unlit 直出 | 34 | Deferred Shading | G-buffer+彩虹灯阵 |
| 15 | Depth Test Grid | 64 大理石箱深度灰度化 | 35 | SSAO Contact Shadows | 房间盒+模型接触暗化 |
| 16 | Alpha Blend Windows | 参考 5 窗位排序 | 36 | PBR Base | 25 球红 ramp metal/rough 可调 |
| 17 | Backface Culling | 45° 单箱剔除开关 | 37 | PBR Textured | rusted_iron 五件套 |
| 18 | FrameBuffer Post FX | 64 容器箱反相/灰度/锐化 | 38 | IBL Irradiance | loft.hdr 环境+背景 |
| 19 | Skybox 6-face | 六面 jpg cubemap 射线采样 | | | |

跳过的纯 GL API 学习项：TemplateTest(stencil)/AdvancedGLSL/UniformBuffer/IBL_Irradiance_Conversion/IBL_Specular。
已知简化：MSAA 以 2x 盒滤波超采样等效；IBL 为 equirect 直接采样近似；MTL 材质暂以统一色调代替。

## 质量门禁

```bash
for t in build/test/math_* build/test/render_*; do ./$t; done   # 21 套件全绿
cd build && ctest -R golden_image                                # 黄金图像基线比对
```

黄金图像基线说明见 `assets/golden/README.md`。性能基准：`./build/tools/bench_raster`。
