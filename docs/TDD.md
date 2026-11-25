<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# TDD 开发规范

`occ-c-bridge` 是 C ABI 中间层，接口一旦发布就要保持稳定。所有功能按 **TDD（测试驱动开发）** 推进，测试是"不 break"的契约。

测试框架使用 [doctest](https://github.com/doctest/doctest) 2.5.0（单头、MIT），已 vendor 到 `tests/third_party/doctest.h`，不依赖构建时网络。

## 1. 流程：红 → 绿 → 重构

```
1. Red     先写一个失败的测试，描述期望行为
2. Green   写最小实现让测试通过（不追求完美）
3. Refactor 在测试保护下重构实现/接口
4. 提交    测试 + 实现一起提交
```

对每个新增 API，顺序固定为：

```
测试（编译失败或断言失败）
  → 声明（头文件）
  → 最小实现
  → 测试通过
  → 补边界用例
  → 重构
```

## 2. 每个 API 的测试矩阵

| 维度 | 必测内容 |
|---|---|
| 正常路径 | 合法输入 → 返回值、数值、句柄 |
| 边界 | 0、负数、空字符串、极大/极小值 |
| 空指针 | `out == nullptr`、`path == nullptr` |
| 无效句柄 | `OCB_INVALID`、已释放句柄、错误类型句柄 |
| 错误状态 | `ocb_last_status()`、`ocb_last_error()` 正确 |
| 生命周期 | 创建/释放配对，`ocb_live_count()` 归零 |
| 异常路径 | 触发 OCC 异常时返回 `OCB_ERR_OCC_EXCEPTION`，不崩溃 |
| 契约 | 数值与 OCC 原调用一致（容差按几何类型定） |

## 3. 测试组织

```
tests/
├── third_party/
│   ├── doctest.h              doctest 2.5.0（单头）
│   └── LICENSE.doctest.txt
├── test_main.cpp              DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
├── test_geometry.cpp          点、向量、变换
├── test_shape.cpp             形状创建/属性/释放
└── test_handle.cpp            句柄生命周期、错误状态、泄漏
```

命名：`TEST_CASE("对象_行为_条件")`，例如：

```cpp
TEST_CASE("make_box_rejects_non_positive_dimensions")
TEST_CASE("invalid_handle_returns_error")
TEST_CASE("create_and_release_many_shapes_does_not_leak")
```

每个测试必须**自己清理**（释放创建的句柄），避免污染共享的 `HandleTable`。

doctest 的常规断言：

| 宏 | 用途 |
|---|---|
| `CHECK(expr)` | 失败后继续执行 |
| `REQUIRE(expr)` | 失败后立即结束当前用例 |
| `CHECK_THROWS_AS(expr, type)` | 异常路径（中间层内部使用） |
| `SUBCASE("name")` | 同一前置条件下的多分支 |

## 4. 示例：新增 `ocb_shape_translate`

### 第一步：写失败测试（Red）

```cpp
TEST_CASE("translate_moves_bounding_box") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0,0,0), 10, 10, 10, &box) == OCB_OK);

    OcbTrsf t = ocb_trsf_translation(OcbVec{5, 0, 0});
    OcbShape moved = OCB_INVALID;
    REQUIRE(ocb_shape_translate(box, t, &moved) == OCB_OK);

    OcbBox b{};
    REQUIRE(ocb_shape_bounding_box(moved, &b) == OCB_OK);
    CHECK(std::fabs(b.xmin - 5.0) < 1e-6);
    CHECK(std::fabs(b.xmax - 15.0) < 1e-6);

    ocb_shape_release(moved);
    ocb_shape_release(box);
}
```

此时编译失败（函数不存在）——这就是 Red。

### 第二步：声明 + 最小实现（Green）

```cpp
// occ_c_bridge.h
OCB_API OcbStatus OCB_CALL ocb_shape_translate(OcbShape s, OcbTrsf t, OcbShape* out);

// occ_c_bridge.cpp
OcbStatus OCB_CALL ocb_shape_translate(OcbShape s, OcbTrsf t, OcbShape* out) {
    if (!out) return OCB_ERR_INVALID_ARG;
    TopoDS_Shape* shape = HandleTable::instance().get<TopoDS_Shape>(s);
    if (!shape) return OCB_ERR_INVALID_HANDLE;
    try {
        TopoDS_Shape moved = shape->Moved(TopLoc_Location(toTrsf(t)));
        *out = HandleTable::instance().add(std::move(moved));
        return OCB_OK;
    } OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}
```

### 第三步：补边界（仍 Red → Green）

```cpp
TEST_CASE("translate_rejects_invalid_handle")
TEST_CASE("translate_rejects_null_out")
TEST_CASE("translate_does_not_modify_source")
```

## 5. 运行

```powershell
cmake -S . -B build -DOCB_OCCT_ROOT=C:/OpenCASCADE-7.7.0-vc14-64/opencascade-7.7.0
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

或直接运行：

```powershell
build\Release\occ_c_bridge_tests.exe
```

输出示例：

```
[doctest] doctest version is "2.5.0"
===============================================================================
[doctest] test cases:  21 |  21 passed | 0 failed | 0 skipped
[doctest] assertions: 267 | 267 passed | 0 failed |
[doctest] Status: SUCCESS!
```

## 6. 提交规范

- 一个 commit 对应一个功能/一批相关用例；
- 提交信息写清"测了什么"，例如：
  - `feat: ocb_shape_translate (+6 tests)`
  - `fix: invalid handle returns OCB_ERR_INVALID_HANDLE (+2 tests)`
- 不允许"只提交实现、测试后补"——P0 是例外，从 P1 开始严格执行。

## 7. 后续可增强

| 项 | 说明 |
|---|---|
| 契约测试 | 测试程序直接链接 OCC，比较 `ocb_*` 与原生调用结果 |
| 覆盖率 | OpenCppCoverage / VS Code Coverage，目标 ≥ 80% |
| 模糊测试 | 对路径/句柄输入做 fuzz（P2 的 IO API） |
| ASan | MSVC `/fsanitize=address`，检测越界/泄漏 |
| CI | Gitea Actions 或内网 runner，构建 + ctest |

## 8. 当前覆盖

| 文件 | 用例数 | 覆盖 |
|---|---|---|
| `test_geometry.cpp` | 6 | 点、距离、单位变换、平移 |
| `test_shape.cpp` | 10 | box 创建/边界/空指针、bbox、体积、无效句柄、多句柄 |
| `test_handle.cpp` | 5 | 释放 0、双重释放、错误状态、状态清除、200 个对象泄漏 |
| **合计** | **21** | **267 个断言** |
