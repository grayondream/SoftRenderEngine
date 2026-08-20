# 进度记录

## 任务背景与目标

- 项目：SoftGameEngine（C++20 软渲染游戏引擎，SDL2 窗口 + 自研 math 库）
- 目标：审查当前实现并修复问题，恢复可构建、可测试的状态
- 分支：develop；现状：项目无法 configure（render 库无源文件）

## 记录

| 时间 | 日志 | 任务类型 | 内容 | 备注 |
| ---- | ---- | -------- | ---- | ---- |
| 2026-08-20 | 完成代码审查 | 审查 | 输出约 36 项问题清单，实测复现：构建阻断(render 无源)、定点数 FP32Mag=18、Pow 负指数段错误、矩阵复制别名、StaticMatrix2D operator== 恒 false、eye 不置零、Color union 错乱、Vector 无默认构造、Object4D 无法构造等 | 已写入审查结果，待用户确认修复范围 |
| 2026-08-20 | 修复构建阻断 | 修复 | CMake 编译器设置移到 project() 前；新建 src/Render/GeoObject/GeoObject.cpp 使 render 库有源文件；configure+build 成功 | clang++ 生效；首次配置 201.6s |
| 2026-08-20 | 搭建测试基础设施 | 修复 | gtest 改用 FetchContent(googletest release-1.12.1)，BUILD_GMOCK OFF；测试链接 gtest/pthread/math/render；新增 ENABLE_SANITIZER 选项(默认 OFF) | 网络可用；clang ASAN 运行时缺失 |
| 2026-08-20 | 修复数学库 bug | 修复 | MathConst FP32Mag/FP64Mag 异或→移位；FixPoint include guard+FPGetShift+&&；Pow 负指数取倒数；PrimitiveLine intersect 斜率/极值/优先级；Vector2D distance+默认构造+operator= 返回；Matrix 复制赋值指针别名(1D/3D/4D)；StaticMatrix operator== 重写；eye() 置零+补 return(含 StaticMatrix4D assert)；Matrix1D mul(StaticMatrix2D) 重写 | 新增回归测试 test/math/BugFix.cpp 18 项全部通过 |
| 2026-08-20 | 修复渲染库 bug | 修复 | Color.hpp union 错乱修复；Object4D 可默认构造；UUID 链接 uuid 库、to_string 输出十六进制、move 用右值引用；WindowBuffer reserve→resize、clear 越界修复 | 新增 test/render/GeoObject.cpp 5 项通过 |
| 2026-08-20 | 修复窗口层 | 修复 | src 非 main 源文件抽成 core 库(测试可链接)；Window 纹理格式用 Format2SDLFormat、draw 按 pitch 逐行拷贝、析构顺序 renderer→texture→window、Rule of Five(禁拷贝启移动)；GenerateColor alpha=255；Application switch 补 break；main 用 return 替代 exit(-1)；Environment 加 SDL_Quit | 新增 test/render/WindowBuffer.cpp 4 项通过；主程序 dummy 驱动冒烟测试正常退出 |
| 2026-08-20 | 收尾清理 | 修复 | 补 DynamicMatrix4DIndex.hpp include guard；修复 StaticMatrixTest.CreateAndPrint 测试越界断言 | 全部 8 个测试套件 49 项测试通过 |
| 2026-08-20 | 二次审查 | 审查 | 用子代理逐函数核对数学正确性，发现 3D/4D inverse() 悬垂引用桩、thetha NaN、normalize 零向量除零、Trigonometric 无角度归约、invert 非方阵越界、矩阵二元运算维度断言缺失、1D det 语义错、嵌套 init_list 越界等 15 项 | 大多未被既有测试覆盖 |
| 2026-08-20 | 二次修复 | 修复 | 删除 3D/4D inverse() 悬垂桩；thetha 加 clamp+零向量保护；normalize 零向量保护；Sin/Cos/Tan 统一 double+角度归约；invert 运行时校验方阵；2D/3D/4D 静态/动态矩阵二元运算补维度断言；1D det 改元素值；eye() 空矩阵保护；嵌套 init_list 长度断言；dot 加 const；QuantNumber qo→q0 并补 return；BGRA8888 clear 字节序；WindowEventType 拼写；RenderList4D::reset 实现；fmodf64→std::fmod | 新增 6 项回归测试；全部 8 套件 56 项测试通过 |