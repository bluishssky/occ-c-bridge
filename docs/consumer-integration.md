<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# occ_c_bridge 接入指南（最小可用）

> 版本：见 `occ_c_bridge.h` 顶部 `ocb_version()`；本项目遵循语义化版本，`SameMajorVersion` 兼容。
> 适用：任何 C/C++ 工程（MSVC x64）。本文以 CMake 为例，非 CMake 工程见 §4。

## 1. 目录结构（安装后）

```
<prefix>/
  bin/occ_c_bridge.dll
  lib/occ_c_bridge.lib
  include/occ_c_bridge.h
  include/occ_c_types.h
  include/occ_c_guard.h
  lib/cmake/occ_c_bridge/occ_c_bridgeConfig.cmake
  lib/cmake/occ_c_bridge/occ_c_bridgeConfigVersion.cmake
  lib/cmake/occ_c_bridge/occ_c_bridgeTargets*.cmake
```

## 2. CMake 接入

```cmake
find_package(occ_c_bridge 0.1 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE occ_c_bridge::occ_c_bridge)
```

运行时需要 `occ_c_bridge.dll` 与 **OpenCASCADE 运行库**（`TK*.dll`）在 PATH 或可执行文件同目录。
若使用维护者提供的预编译发行包，OCCT/TBB/FreeType 运行库已自带，拷 `bin/` 即可；
许可证与第三方声明见包内 `share/doc/occ_c_bridge/`。

## 3. 最小代码骨架

```cpp
#include "occ_c_bridge.h"
#include <cstdio>

int main()
{
    // ① 应用与文档（OCAF）
    OcbApp app = OCB_INVALID;
    if (ocb_app_new(&app) != OCB_OK) return 1;
    OcbDoc doc = OCB_INVALID;
    ocb_app_new_document(app, "BinXCAF", &doc);

    // ② 形状：建盒/导入 STP
    OcbShape box = OCB_INVALID;
    ocb_shape_make_box(OcbPnt{0, 0, 0}, 10, 20, 30, &box);
    double vol = 0;
    ocb_shape_volume(box, &vol);
    printf("volume=%.1f\n", vol);

    // ③ 视图（需要窗口句柄；Windows 下传 HWND）
    OcbViewer viewer = OCB_INVALID;
    ocb_viewer_create(hwnd, &viewer);
    OcbView view = OCB_INVALID;
    ocb_view_create(viewer, hwnd, &view);
    OcbAisContext ctx = OCB_INVALID;
    ocb_viewer_context(viewer, &ctx);

    OcbAisObj obj = OCB_INVALID;
    ocb_ais_new_shape(box, &obj);
    ocb_ais_ctx_display(ctx, obj, true);
    ocb_view_fit_all(view);

    // ④ 释放（C API，无 RAII；推荐配合 occ_c_guard.h）
    ocb_ais_release(obj);
    ocb_ais_ctx_release(ctx);
    ocb_view_release(view);
    ocb_viewer_release(viewer);
    ocb_shape_release(box);
    ocb_app_close(app, doc);
    ocb_app_release(app);
    return 0;
}
```

## 4. 非 CMake（直接链接）

- 编译：`/I<prefix>\include /utf-8`，链接 `occ_c_bridge.lib`，并链接你使用的 OCCT 库
  （`TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKTopAlgo TKPrim TKBO TKXCAF TKLCAF TKV3d TKService TKOpenGl ...`）
- 部署：`occ_c_bridge.dll` + 对应 OCCT `TK*.dll` 与 TBB/FreeType 运行库。

## 5. 所有权与错误处理（必读）

1. **所有 `OcbXxx` 都是句柄（uint64）**，不是指针；`OCB_INVALID`（0）表示空。
2. **谁创建谁释放**：`ocb_*_new/create` 的产物必须 `ocb_*_release`；
   `ocb_*_wrap_native` 只增加引用，释放时仅减引用，不会销毁原生对象。
3. 传入非法句柄返回 `OCB_ERR_INVALID_HANDLE`，**不会崩溃**（已覆盖单测）。
4. `ocb_last_error()` 取最近一次错误描述；`ocb_last_status()` 取状态码。
5. 视图未就绪（窗口未映射/尺寸为 0）时，投影/拾取类 API 返回 `OCB_ERR_NOT_READY`（错误码 7），
   调用方应跳过而不是重试到崩溃。
6. 句柄泄漏可用 `ocb_live_count()` 自查（单测里用它断言 0 泄漏）。

## 6. 线程约定

桥内部不做线程调度：`AIS_InteractiveContext` / `V3d_View` 相关调用必须在创建它们的线程（通常 UI 线程）执行；
纯几何/OCAF 操作可在后台线程使用独立文档对象，但**同一个 `OcbDoc` 不允许并发访问**。

## 6. 已验证的最小消费者（examples/minimal-consumer）

仓库内提供可直接编译运行的最小示例，**只 include `occ_c_bridge.h`，不引任何 OCCT 头**：

```powershell
# 1) 安装到临时前缀
cmake --install build --config Release --prefix %TEMP%\ocbstage

# 2) 配置并编译消费者
cmake -S examples/minimal-consumer -B build-consumer -A x64 `
      -Docc_c_bridge_DIR=%TEMP%\ocbstage/lib/cmake/occ_c_bridge
cmake --build build-consumer --config Release

# 3) 运行（DLL 已在 PATH 或与 exe 同目录）
build-consumer\Release\ocb_minimal_consumer.exe
```

覆盖项：`ocb_version`、`ocb_shape_make_box`、`ocb_shape_volume`、`ocb_shape_bounding_box`、
`ocb_topo_count_solids`、`ocb_shape_moved`、`ocb_shape_distance`、`ocb_explorer_*`、
`ocb_app_new` / `ocb_app_new_document` / `ocb_shapetool_*` / `ocb_label_*`。
预期输出 `RESULT: PASS`，退出码 0。

> 注意：`Bnd_Box` 带 1e-7 级 gap，比较包围盒请用 1e-5 容差（示例中已按此设置）。
