# OBJ 网格加载设计（Wavefront 子集解析器）

日期：2026-08-22
状态：已确认（用户授权连续执行模式）
前置：渲染管线全特性就绪（视锥裁剪/光照/相机）

## 目标

提供 Wavefront OBJ 文本网格加载能力，摆脱硬编码几何——`loadObjFromFile(path, Object4D&)` 解析 v/vt/vn/f 行填充引擎对象结构，为真实资产渲染铺路。

## 非目标（YAGNI）

- mtl 材质文件与 usemtl 颜色映射（PolyF4D.color 保持单色默认）
- 曲线/自由曲面、多文件合并、二进制格式
- 动态容量扩容（Object4D 固定数组语义不变）

## 方案要点

### 支持的语法子集

| 行类型 | 处理 |
| ------ | ---- |
| `v x y z` | 追加顶点（w=1）|
| `vt u v` | 追加 UV |
| `vn x y z` | 追加法线 |
| `f ...` | 面定义，3..N 顶点 fan 三角化 |
| `# 注释` | 跳过 |
| 其他行（o/g/s/usemtl/mtllib 等）| 静默跳过 |

### f 行索引格式（全部支持）

`f a b c...` / `f a/b c/d ...` / `f a//c ...` / `f a/b/c ...`
- 索引 1-based；负数 `-n` 表示「从当前已解析顶点数倒数第 n 个」
- 引用越界（0、超上限、负数超界）→ 整个加载失败

### 兜底规则

- 无 `vn` 行或该面未引用 vn：`nlist` 用前三顶点叉积计算平面法线并归一化（退化面积零则置零）
- 无 `vt` 或未引用：`uvlist` 全零
- 成功时 `numVertices/numPolys/name("obj")` 就位；`worldPos` 不动由调用方设置

### 失败条件（返回 false，out 保持可重入清空态）

- 文件无法打开；任一行 token 格式非法；顶点/面引用越界；超出 Object4D 数组容量（kObject4vListLen=64 / kPolyListLen=128）

## 接口

```cpp
// src/Render/ObjLoader.hpp（render 库新源 ObjLoader.cpp）
bool loadObjFromFile(const std::string &path, Object4D &out);
```

## 资产

提交 `assets/cube.obj`：带 v(8)/vt/vn(6)/f(12) 的立方体文本资产（UV 布局与程序化 MakeCube 同构），供真实文件加载测试与后续 demo 使用。

## 测试策略（test/render/ObjLoader.cpp → render_ObjLoader）

测试用例通过运行时向 `/tmp/opencode/obj_test_*.obj` 写入文本再加载（覆盖解析分支），另含真实资产用例：

| 测试 | 验证点 |
| ---- | ---- |
| Obj.VerticesAndTriangleFaces | 最小 v/f：numVertices/numPolys/plist 内容逐项 |
| Obj.QuadFaceFanTriangulated | 四边形面 → 2 个三角形且共享对角 |
| Obj.SlashFormats | a/b、a//c、a/b/c 三种变体 uvlist/nlist 映射正确 |
| Obj.NegativeIndices | -1/-2 相对索引解析 |
| Obj.MissingNormalsFlatComputed | 无 vn → 叉积面法线归一化数值 |
| Obj.MissingUvsZeroed | 无 vt → uvlist 全零 |
| Obj.CapacityExceededFails | 超 64 顶点 → false 且 numPolys==0 |
| Obj.BadTokenFails | "v x"（非数值）→ false |
| Obj.MissingFileFails | 不存在路径 → false |
| Obj.RealCubeAsset | assets/cube.obj：8 顶点/12 面/法线朝向抽查 |
| 回归 | 全部既有套件照常通过 |

## 验收标准

1. 全部测试绿；
2. dummy 冒烟无 crash（主程序不改动，成像与上一版一致）；
3. 渲染链路端到端验证：单测内 loadObj + projectObject 渲染 cube.obj 到 FrameBuffer 断言非黑像素 > 1000。
