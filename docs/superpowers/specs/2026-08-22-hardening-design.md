# 防御性加固批次设计

日期：2026-08-22
状态：已确认（用户授权连续执行模式）
前置：多特性审查中累积的备案缺陷集中清理

## 目标

清偿四个特性审查中备案的防御性缺陷，每项配判别测试：

| # | 位置 | 缺陷 | 修复 |
| - | ---- | ---- | ---- |
| H-1 | `src/Render/Light.cpp` accum 点光循环 | `range<=0` 时 `atten=Clamp01(1-d/range)` 出现除零/负除 → NaN 可逃逸 `atten<=0` 守卫流入 uint32 cast UB | 循环首行 `if(pl.range <= 0) continue;` |
| H-2 | `src/Render/Texture.cpp` 内存构造 | `Texture(w,h,nullptr)` 且 w·h>0 时 m_pixels 空 → sample 越界读 UB | 构造函数加 `assert(bgra != nullptr || (w == 0 && h == 0));`（需 `<cassert>`）|
| H-3 | `src/Render/Camera.cpp` update | dt 无界：首帧大步长 / alt-tab 恢复瞬移穿模 | 入口 `dt = std::clamp(dt, 0.0, 0.1);`（上限 100ms）|
| H-4 | `.gitignore` | CTest 未跟踪产物 Testing/ 目录噪声 | 追加 `Testing/` |

## 测试策略

| 测试 | 验证点 |
| ---- | ---- |
| Light.ZeroRangePointLightIgnored | range=0 点光不产生贡献也不崩溃（修复前 NaN→uint32 为未定义值，修复后输出确定）|
| Camera.UpdateClampsLargeDt | dt=100 时位移等价 dt=0.1（上界生效）；dt=-5 等价 0（下界）|
| Texture.NullptrZeroSizeOk / NullptrNonZeroAsserts | 前者正常构造；后者 EXPECT_DEATH（assert 生效域）|

回归：全量套件照常通过；布光 Demo 冒烟正常。

## 验收标准

全部新测试绿 + 全量回归绿 + 冒烟无 crash。
