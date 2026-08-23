# 防御补强批次设计（批次3）

日期：2026-08-23
状态：已确认（用户授权连续执行模式）
前置：清偿历史审查备案的全部 Minor/脚枪缺陷

## 六项修复

| # | 位置 | 缺陷 | 方案 |
| - | ---- | ---- | ---- |
| D-1 | `Light.cpp` ch lambda | NaN 经 `s<0/s>255` 双比较均 false → uint32 cast UB | `if(!(s >= 0)) s = 0; else if(s > 255) s = 255;`（NaN 归 0）|
| D-2 | `Rasterizer.hpp` ScreenVertex / `ObjLoader.cpp` | 默认 Color32 α=0 → solid 路径静默隐形脚枪 | ScreenVertex::color 默认 {255,255,255,255}；ObjLoader 显式设置 object 颜色白不透明 |
| D-3 | `Camera.cpp` right() | pitch≈±90° 时 forward×up 叉积长度趋 0 → normalize 退化 | 改直接公式 normalize({-cos(yaw),0,sin(yaw)})——数学上与原式等价（cp 因子归一化消去）且对任意 pitch 稳定；yaw=0 → (-1,0,0) 保持约定 |
| D-4 | `FrameBuffer.hpp` setPixel/blendPixel | NaN/inf depth 通过比较守卫污染深度缓冲 | 两函数深度守卫前加 `if(!std::isfinite(depth)) return;` |
| D-5 | `ObjLoader.cpp` | vt/vn 数组无容量上限（恶意文件内存膨胀）；退化共线面零法线路径无测试 | vt/vn 各加上限 kMaxObjUV=4096/kMaxObjNormal=4096 超限 fail()；补退化面测试 |
| D-6 | `Texture.cpp` sample | u*w 超 int 范围 cast UB | 入口 `!isfinite(u/v)` 返回兜底黑；u/v clamp 至 ±INT_MAX/max(w,h) 安全域（wrap/clamp 正常域语义不变）|

## 测试策略（每项判别）

1. D-1: DirectionalLight.direction 含 NaN → shade 输出 == 仅 ambient 确定值
2. D-2: 默认 ScreenVertex drawTriangleSolid 后像素 != 背景色
3. D-3: pitch=π/2−ε 与 pitch=1.5 下 |right|==1 且方向符合 yaw 公式
4. D-4: blendPixel(NaN)/setPixel(inf) 后 colorData/depthData 不变
5. D-5: 4097 行 vt 文件 fail；三点共线 f 面 load 成功且法线为零向量
6. D-6: sample(1e300, 1e300) 不崩且返回合法像素（UBSan 下洁净）

回归：16 套件全绿 + SGE_MAX_FRAMES 冒烟 exit=0。
