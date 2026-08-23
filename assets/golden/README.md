# 黄金图像基线说明

`lit_cube.md5` 是 `tools/golden_render.cpp` 确定性单帧渲染输出的 P6 PPM 的 md5。

## 再生成

```bash
cmake -B build -S . && cmake --build build --target golden_render
./build/tools/golden_render /tmp/g.ppm && md5sum /tmp/g.ppm | awk '{print $1}' > assets/golden/lit_cube.md5
```

仅当**有意变更**渲染输出（新特性、算法调整）时再生成并随变更一同提交，提交说明需注明视觉影响。

## 漂移条件（基线绑定的环境）

基线对以下因素敏感，跨这些维度比较前必须先在本地重建基线：

- 编译器与优化级别（浮点重排/FMA 差异；Debug clang 与 g++ -O3+ASan 已验证一致）
- libm 版本（sin/cos/pow 的 ULP 差异）
- CPU 浮点架构差异（x86-64 SSE/AVX）

任何像素的 1-bit 差异都会导致比对失败——这是设计意图（回归哨兵），而非缺陷。
