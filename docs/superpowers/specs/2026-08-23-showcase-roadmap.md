# 七大经典示例场景路线图

日期：2026-08-23
状态：已确认（用户规格书）

## 场景矩阵与依赖特性

| # | 场景 | 新增特性 |
| - | ---- | -------- |
| S1 | 基础几何陈列（球/立方体/圆环/茶壶+双光源） | 参数化几何生成器库、Utah 茶壶贝塞尔曲面 |
| S2 | 多距离纹理滤波对比 | Mipmap 链生成 + 三线性采样 |
| S3 | 透明玻璃球/金属环/透明贴图平面 | SpotLight、物体级透明排序（画家算法）|
| S4 | 聚光侧投障碍物阴影（PCF 软边） | 复用 ShadowMapping+pcfRadius |
| S5 | 镜面球+水晶球+程序化天空环境 | 解析环境采样（反射/折射/菲涅尔）|
| S6 | PBR 金属/粗糙度球阵 | Cook-Torrance BRDF + 解析 IBL 近似 |
| S7 | 光锥内 15-20 材质各异球体 | RayTracer 扩展：聚光介质光锥、折射色散、伪焦散 |

## 本批 S1 范围

- `kObject4vListLen` 64→1024、`kPolyListLen` 128→1024（支撑网格生成器与茶壶细分）
- `Render/Geometry.hpp`：makeSphere(r,segU,segV) / makeTorus(R,r,su,sv) / makeCylinder(r,h,seg) / makeCone(r,h,seg) → Object4D（平滑法线+UV）
- `makeTeapot()`：内嵌标准 Utah 茶壶 32 贝塞尔 patch/306 控制点，d=4 细分 + 量化焊接去重
- 测试：各生成器顶点/三角形计数、法线单位性、茶壶规模与焊接有效性
- Application 场景 0 重排为四件套陈列（key directional + warm point）

后续批次按矩阵顺序推进，ImGui 列表随批更新。
