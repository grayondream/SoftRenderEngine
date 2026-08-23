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

## 七大经典场景（ImGui 或 ]/[ 切换）

| # | 场景 | 展示能力 |
|---|------|---------|
| 0 | 基础几何与光照 | 球/圆环/Utah 茶壶参数化生成器、Blinn-Phong、tile 多线程 |
| 1 | 纹理映射与滤波 | Mipmap 链 + 三线性滤波 vs 最近邻，多距离对比 |
| 2 | 深度测试与透明混合 | Z-Buffer、画家算法排序、src-over 半透明 |
| 3 | 阴影映射 + PCF | 聚光透视深度 Pass、PCF 软边阴影 |
| 4 | 环境反射与折射 | 解析环境探针、菲涅尔、Snell 折射 |
| 5 | PBR 材质球阵 | Cook-Torrance (GGX/Smith/Schlick) + 解析 IBL 近似 |
| 6 | 光锥光线追踪 | CPU 光追：聚光体积锥、玻璃折射、伪焦散、20 球体 |

## 质量门禁

```bash
for t in build/test/math_* build/test/render_*; do ./$t; done   # 21 套件全绿
cd build && ctest -R golden_image                                # 黄金图像基线比对
```

黄金图像基线说明见 `assets/golden/README.md`。性能基准：`./build/tools/bench_raster`。
