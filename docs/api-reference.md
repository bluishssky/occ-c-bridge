<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# occ_c_bridge API 参考（自动生成）

> 来源：`include/occ_c_bridge.h`（320 个导出 API）。
> 每条为「完整签名 + 声明前紧邻的注释」；注释缺失者汇总在文末。

## 分组索引

- **ais**：76 个
- **shapetool**：27 个
- **view**：26 个
- **shape**：15 个
- **label**：15 个
- **trsf**：12 个
- **doc**：12 个
- **viewer**：12 个
- **make**：11 个
- **topo**：9 个
- **drawer**：8 个
- **stepwriter**：8 个
- **explorer**：7 个
- **bool**：6 个
- **gprops**：6 个
- **stepreader**：6 个
- **point**：5 个
- **box**：5 个
- **colortool**：5 个
- **app**：4 个
- **fillet**：4 个
- **chamfer**：4 个
- **tri**：4 个
- **curve**：3 个
- **classifier**：3 个
- **light**：3 个
- **edge**：3 个
- **last**：2 个
- **vec**：2 个
- **quaternion**：2 个
- **interface**：2 个
- **step**：2 个
- **geom**：2 个
- **line**：2 个
- **version**：1 个
- **live**：1 个
- **dir**：1 个
- **vertex**：1 个
- **mesh**：1 个
- **face**：1 个
- **xcaf**：1 个

## ais（76）

### `ocb_ais_attributes`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_attributes(OcbAisObj obj, OcbDrawer* out);
```

### `ocb_ais_ctx_activate`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_activate(OcbAisContext ctx, int mode);
```

### `ocb_ais_ctx_add_or_remove_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_add_or_remove_selected(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_clear_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_clear_selected(OcbAisContext ctx, bool update);
```

### `ocb_ais_ctx_deactivate`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_deactivate(OcbAisContext ctx);
```

### `ocb_ais_ctx_deactivate_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_deactivate_mode(OcbAisContext ctx, int mode);
```

### `ocb_ais_ctx_detected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_detected(OcbAisContext ctx, OcbAisObj* out);
```

### `ocb_ais_ctx_detected_native`

迁移期适配：直接取检测到的原生对象指针（不建句柄，适合每帧拾取）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_detected_native(OcbAisContext ctx, void** out);
```

### `ocb_ais_ctx_display`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_display(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_display_ex`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_display_ex(OcbAisContext ctx, OcbAisObj obj, int mode, int priority, bool update);
```

### `ocb_ais_ctx_displayed_at`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_at(OcbAisContext ctx, int index, OcbAisObj* out);
```

### `ocb_ais_ctx_displayed_at_native`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_at_native(OcbAisContext ctx, int index, void** out);
```

### `ocb_ais_ctx_displayed_count`

已显示对象枚举：B2 迁移 DisplayedObjects 用（顺序与 OCCT 一致）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_count(OcbAisContext ctx, int* out);
```

### `ocb_ais_ctx_erase`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_erase(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_has_detected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_has_detected(OcbAisContext ctx, bool* out);
```

### `ocb_ais_ctx_init_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_init_selected(OcbAisContext ctx);
```

### `ocb_ais_ctx_is_displayed`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_is_displayed(OcbAisContext ctx, OcbAisObj obj, bool* out);
```

### `ocb_ais_ctx_more_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_more_selected(OcbAisContext ctx, bool* out);
```

### `ocb_ais_ctx_move_to`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_move_to(OcbAisContext ctx, OcbView view, int x, int y, bool update);
```

### `ocb_ais_ctx_native_ptr`

```c
OCB_API void* OCB_CALL ocb_ais_ctx_native_ptr(OcbAisContext ctx);
```

### `ocb_ais_ctx_next_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_next_selected(OcbAisContext ctx);
```

### `ocb_ais_ctx_picked_point`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_picked_point(OcbAisContext ctx, int index, OcbPnt* out);
```

### `ocb_ais_ctx_redisplay`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_redisplay(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_release`

```c
OCB_API void OCB_CALL ocb_ais_ctx_release(OcbAisContext ctx);
```

### `ocb_ais_ctx_remove`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_remove(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_remove_all`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_remove_all(OcbAisContext ctx, bool update);
```

### `ocb_ais_ctx_select_detected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_select_detected(OcbAisContext ctx, int mode);
```

### `ocb_ais_ctx_select_rectangle`

框选（矩形，屏幕像素）；status 返回 AIS_StatusOfPick 数值

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_select_rectangle(OcbAisContext ctx, OcbView view, int x1, int y1, int x2, int y2, int scheme, int* outStatus);
```

### `ocb_ais_ctx_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected(OcbAisContext ctx, OcbAisObj* out);
```

### `ocb_ais_ctx_selected_native`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_native(OcbAisContext ctx, void** out);
```

### `ocb_ais_ctx_selected_owner`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner(OcbAisContext ctx, OcbAisObj* out);
```

### `ocb_ais_ctx_selected_owner_native`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_native(OcbAisContext ctx, void** out);
```

### `ocb_ais_ctx_selected_owner_shape`

------------------------------------------------------------------ 拾取（拓扑实体级） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_shape(OcbAisContext ctx, OcbShape* out);
```

### `ocb_ais_ctx_set_automatic_hilight`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_automatic_hilight(OcbAisContext ctx, bool enabled);
```

### `ocb_ais_ctx_set_color`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_color(OcbAisContext ctx, OcbAisObj obj, OcbColor color, bool update);
```

### `ocb_ais_ctx_set_default_deflection`

默认 drawer 的三角化偏差（type: 0=相对 1=绝对，对应 Aspect_TypeOfDeflection）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_default_deflection(OcbAisContext ctx, int type, double maximalChordalDeviation, double deviationAngleRad);
```

### `ocb_ais_ctx_set_display_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_display_mode(OcbAisContext ctx, int mode, bool update);
```

### `ocb_ais_ctx_set_display_priority`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_display_priority(OcbAisContext ctx, OcbAisObj obj, int priority);
```

### `ocb_ais_ctx_set_highlight_color`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_highlight_color(OcbAisContext ctx, OcbColor color);
```

### `ocb_ais_ctx_set_location`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_location(OcbAisContext ctx, OcbAisObj obj, OcbTrsf trsf);
```

### `ocb_ais_ctx_set_material`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_material(OcbAisContext ctx, OcbAisObj obj, int material, bool update);
```

### `ocb_ais_ctx_set_pixel_tolerance`

像素容差 / 显示模式 / 自动高亮

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_pixel_tolerance(OcbAisContext ctx, float tolerance);
```

### `ocb_ais_ctx_set_selected`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selected(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_set_selection_style`

选择/高亮样式

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selection_style(OcbAisContext ctx, OcbColor color, float transparency, int zlayer);
```

### `ocb_ais_ctx_set_transparency`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_transparency(OcbAisContext ctx, OcbAisObj obj, double value, bool update);
```

### `ocb_ais_ctx_unset_transparency`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_unset_transparency(OcbAisContext ctx, OcbAisObj obj, bool update);
```

### `ocb_ais_ctx_update_viewer`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_update_viewer(OcbAisContext ctx);
```

### `ocb_ais_ctx_wrap_native`

包装外部已有的 native AIS_InteractiveContext（生命周期仍由外部管理， 句柄释放不会销毁原对象）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_wrap_native(void* nativeCtx, OcbAisContext* out);
```

### `ocb_ais_downcast_point`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_downcast_point(OcbAisObj obj, OcbAisObj* out);
```

### `ocb_ais_downcast_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_downcast_shape(OcbAisObj obj, OcbAisObj* out);
```

### `ocb_ais_get_color`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_get_color(OcbAisObj obj, OcbColor* out);
```

### `ocb_ais_has_color`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_has_color(OcbAisObj obj, bool* out);
```

### `ocb_ais_has_transformation`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_has_transformation(OcbAisObj obj, bool* out);
```

### `ocb_ais_local_transformation`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_local_transformation(OcbAisObj obj, OcbTrsf* out);
```

### `ocb_ais_material`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_material(OcbAisObj obj, int* out);
```

### `ocb_ais_native_ptr`

取底层 native 指针（只读访问，所有权仍在桥端句柄表； 主程序把它转成 Viewer3D 内部 AIS 调用所需的 raw 指针）

```c
OCB_API void* OCB_CALL ocb_ais_native_ptr(OcbAisObj obj);
```

### `ocb_ais_new_point`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_new_point(OcbGeom geom, OcbAisObj* out);
```

### `ocb_ais_new_shape`

------------------------------------------------------------------ AIS 对象 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_ais_new_shape(OcbShape shape, OcbAisObj* out);
```

### `ocb_ais_new_xcaf_object`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_new_xcaf_object(OcbLabel label, OcbAisObj* out);
```

### `ocb_ais_pick_probe`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_pick_probe(OcbAisObj obj, int* outNbSensitive0, int* outZLayer, int* outDisplayMode, OcbBox* outBBox);
```

### `ocb_ais_point_get_position`

AIS_Point 位置读写（替代 app 侧 Component() downcast 到 Geom_CartesianPoint 的用法）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_point_get_position(OcbAisObj obj, OcbPnt* out);
```

### `ocb_ais_point_set_position`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_point_set_position(OcbAisObj obj, OcbPnt p);
```

### `ocb_ais_release`

```c
OCB_API void OCB_CALL ocb_ais_release(OcbAisObj obj);
```

### `ocb_ais_selection_probe`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_selection_probe(OcbAisObj obj, int* outHasMode0, int* outMode0Empty, int* outActiveModeCount);
```

### `ocb_ais_set_color`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_color(OcbAisObj obj, OcbColor color);
```

### `ocb_ais_set_display_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_display_mode(OcbAisObj obj, int mode);
```

### `ocb_ais_set_local_transformation`

直接设置 AIS 对象局部变换，不触发交互上下文选择结构重建（较 ocb_ais_ctx_set_location 快得多）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_local_transformation(OcbAisObj obj, OcbTrsf trsf);
```

### `ocb_ais_set_marker`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_marker(OcbAisObj obj, int type);
```

### `ocb_ais_set_material`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_material(OcbAisObj obj, int material);
```

### `ocb_ais_set_transparency`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_transparency(OcbAisObj obj, double value);
```

### `ocb_ais_set_zlayer`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_set_zlayer(OcbAisObj obj, int layer);
```

### `ocb_ais_shape_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_shape_shape(OcbAisObj obj, OcbShape* out);
```

### `ocb_ais_transparency`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_transparency(OcbAisObj obj, double* out);
```

### `ocb_ais_wrap_native`

迁移期适配：把已有原生 AIS_InteractiveObject* 包成 OcbAisObj（addRef，release 时仅减引用）

```c
OCB_API OcbStatus OCB_CALL ocb_ais_wrap_native(void* nativeObj, OcbAisObj* out);
```

### `ocb_ais_xcaf_get_label`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_xcaf_get_label(OcbAisObj obj, OcbLabel* out);
```

### `ocb_ais_zlayer`

```c
OCB_API OcbStatus OCB_CALL ocb_ais_zlayer(OcbAisObj obj, int* out);
```

## shapetool（27）

### `ocb_shapetool_add_component`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_component(OcbShapeTool st, OcbLabel parent, OcbLabel child, OcbTrsf trsf, OcbLabel* out);
```

### `ocb_shapetool_add_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_shape(OcbShapeTool st, OcbShape shape, OcbLabel* out);
```

### `ocb_shapetool_add_shape_ex`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_shape_ex(OcbShapeTool st, OcbShape shape, bool makeAssemblySimpleGraph, bool autoLabel, OcbLabel* out);
```

### `ocb_shapetool_free_shape_count`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_free_shape_count(OcbShapeTool st, int* out);
```

### `ocb_shapetool_get`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get(OcbDoc doc, OcbShapeTool* out);
```

### `ocb_shapetool_get_component`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_component(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);
```

### `ocb_shapetool_get_component_count`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_component_count(OcbShapeTool st, OcbLabel label, int* out);
```

### `ocb_shapetool_get_free_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_free_shape(OcbShapeTool st, int index, OcbLabel* out);
```

### `ocb_shapetool_get_location`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_location(OcbShapeTool st, OcbLabel label, OcbTrsf* out);
```

### `ocb_shapetool_get_referred_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_referred_shape(OcbShapeTool st, OcbLabel label, OcbLabel* out);
```

### `ocb_shapetool_get_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_shape(OcbShapeTool st, OcbLabel label, OcbShape* out);
```

### `ocb_shapetool_get_sub_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_sub_shape(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);
```

### `ocb_shapetool_get_sub_shape_count`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_sub_shape_count(OcbShapeTool st, OcbLabel label, int* out);
```

### `ocb_shapetool_get_user`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_user(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);
```

### `ocb_shapetool_get_user_count`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_user_count(OcbShapeTool st, OcbLabel label, int* out);
```

### `ocb_shapetool_is_assembly`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_assembly(OcbShapeTool st, OcbLabel label, bool* out);
```

### `ocb_shapetool_is_component`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_component(OcbShapeTool st, OcbLabel label, bool* out);
```

### `ocb_shapetool_is_reference`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_reference(OcbShapeTool st, OcbLabel label, bool* out);
```

### `ocb_shapetool_new_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_new_shape(OcbShapeTool st, OcbLabel* out);
```

### `ocb_shapetool_rebuild_assembly_shapes`

重建装配标签子树的 TNaming compound（自底向上）： NewShape+AddComponent 建的装配其 TNaming 是空 compound，GetShape 返回空， 导致 STEPCAFControl_Writer 按标签导出时写出空装配；导出前调用本函数修复。

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_rebuild_assembly_shapes(OcbShapeTool st, OcbLabel label);
```

### `ocb_shapetool_release`

```c
OCB_API void OCB_CALL ocb_shapetool_release(OcbShapeTool st);
```

### `ocb_shapetool_remove_component`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_remove_component(OcbShapeTool st, OcbLabel label);
```

### `ocb_shapetool_remove_shape`

序列读取约定:get_free_shape/get_user/get_component/get_sub_shape 的 index 一律 0-based

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_remove_shape(OcbShapeTool st, OcbLabel label);
```

### `ocb_shapetool_set_assembly_placeholder`

装配标签写"空 compound"占位：STEP 导出时结构完整、几何只在零件层各存一份（避免每层重复几何导致文件膨胀）

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_assembly_placeholder(OcbShapeTool st, OcbLabel label);
```

### `ocb_shapetool_set_location`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_location(OcbShapeTool st, OcbLabel label, OcbTrsf trsf, OcbLabel* out);
```

### `ocb_shapetool_set_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_shape(OcbShapeTool st, OcbLabel label, OcbShape shape);
```

### `ocb_shapetool_update`

```c
OCB_API OcbStatus OCB_CALL ocb_shapetool_update(OcbShapeTool st);
```

## view（26）

### `ocb_view_convert_with_proj`

```c
OCB_API OcbStatus OCB_CALL ocb_view_convert_with_proj(OcbView view, int x, int y, double* xp, double* yp, double* zp, double* vx, double* vy, double* vz);
```

### `ocb_view_create`

视图宿主：创建 V3d_View 并绑定 Win32 窗口句柄（G1，Viewer3D 切桥的硬阻断项）

```c
OCB_API OcbStatus OCB_CALL ocb_view_create(OcbViewer viewer, void* hwnd, OcbView* out);
```

### `ocb_view_dump`

截图写文件（扩展名决定格式）；width/height<=0 用当前视口尺寸

```c
OCB_API OcbStatus OCB_CALL ocb_view_dump(OcbView view, const char* path, int width, int height);
```

### `ocb_view_fit_all`

------------------------------------------------------------------ 视图 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_view_fit_all(OcbView view);
```

### `ocb_view_fit_all_margin`

```c
OCB_API OcbStatus OCB_CALL ocb_view_fit_all_margin(OcbView view, double margin);
```

### `ocb_view_invalidate`

```c
OCB_API OcbStatus OCB_CALL ocb_view_invalidate(OcbView view);
```

### `ocb_view_must_be_resized`

窗口尺寸变化后同步（Qt paintEvent/resizeEvent 调用）

```c
OCB_API OcbStatus OCB_CALL ocb_view_must_be_resized(OcbView view);
```

### `ocb_view_native_ptr`

```c
OCB_API void* OCB_CALL ocb_view_native_ptr(OcbView view);
```

### `ocb_view_pan`

```c
OCB_API OcbStatus OCB_CALL ocb_view_pan(OcbView view, double dx, double dy);
```

### `ocb_view_project_point`

```c
OCB_API OcbStatus OCB_CALL ocb_view_project_point(OcbView view, double x, double y, double z, int* outSx, int* outSy);
```

### `ocb_view_redraw`

```c
OCB_API OcbStatus OCB_CALL ocb_view_redraw(OcbView view);
```

### `ocb_view_redraw_immediate`

```c
OCB_API OcbStatus OCB_CALL ocb_view_redraw_immediate(OcbView view);
```

### `ocb_view_release`

释放视图句柄（由 ocb_view_create 创建）

```c
OCB_API void OCB_CALL ocb_view_release(OcbView view);
```

### `ocb_view_rotation`

```c
OCB_API OcbStatus OCB_CALL ocb_view_rotation(OcbView view, int x, int y);
```

### `ocb_view_set_bg_gradient`

```c
OCB_API OcbStatus OCB_CALL ocb_view_set_bg_gradient(OcbView view, OcbColor top, OcbColor bottom, int method);
```

### `ocb_view_set_proj`

投影朝向：OCB_PROJ_*

```c
OCB_API OcbStatus OCB_CALL ocb_view_set_proj(OcbView view, int projType);
```

### `ocb_view_set_proj_type`

直接按 V3d_TypeOfOrientation 数值设置投影（消费方传枚举值）

```c
OCB_API OcbStatus OCB_CALL ocb_view_set_proj_type(OcbView view, int v3dOrientation);
```

### `ocb_view_set_rendering_params`

渲染参数（透明方式/渲染方式/阴影/抗锯齿/光追深度/色调映射/MSAA）

```c
OCB_API OcbStatus OCB_CALL ocb_view_set_rendering_params(OcbView view, int transparencyMethod, int method, bool shadow, bool antialiasing, int raytracingDepth, int toneMapping, int msaa);
```

### `ocb_view_start_rotation`

交互：旋转 / 平移 / 滚轮缩放

```c
OCB_API OcbStatus OCB_CALL ocb_view_start_rotation(OcbView view, int x, int y);
```

### `ocb_view_start_zoom_at_point`

```c
OCB_API OcbStatus OCB_CALL ocb_view_start_zoom_at_point(OcbView view, int x, int y);
```

### `ocb_view_to_rgb32`

离屏抓图到 RGB32 缓冲区（每像素 B,G,R,X 顺序，适配 Qt Format_RGB32）； bufferSize 需 >= 4*width*height；实际尺寸经 outWidth/outHeight 返回

```c
OCB_API OcbStatus OCB_CALL ocb_view_to_rgb32(OcbView view, int width, int height, unsigned char* buffer, int bufferSize, int* outWidth, int* outHeight);
```

### `ocb_view_trihedron`

坐标三轴显示/隐藏

```c
OCB_API OcbStatus OCB_CALL ocb_view_trihedron(OcbView view, bool show);
```

### `ocb_view_trihedron_ex`

坐标三轴（可指定颜色/尺寸）

```c
OCB_API OcbStatus OCB_CALL ocb_view_trihedron_ex(OcbView view, bool show, OcbColor color, double scale);
```

### `ocb_view_wrap_native`

迁移期适配：把已有原生 V3d_View* 包成 OcbView（或取回原生指针）， 便于逐调用点切换，不必一次性替换成员类型

```c
OCB_API OcbStatus OCB_CALL ocb_view_wrap_native(void* nativeView, OcbView* out);
```

### `ocb_view_zfit_all`

```c
OCB_API OcbStatus OCB_CALL ocb_view_zfit_all(OcbView view);
```

### `ocb_view_zoom_at_point`

```c
OCB_API OcbStatus OCB_CALL ocb_view_zoom_at_point(OcbView view, int x, int y, double dx, double dy);
```

## shape（15）

### `ocb_shape_bounding_box`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_bounding_box( OcbShape shape, OcbBox* out);
```

### `ocb_shape_bounding_box_ex`

useTriangulation 与 BRepBndLib::Add 的同名参数一致

```c
OCB_API OcbStatus OCB_CALL ocb_shape_bounding_box_ex( OcbShape shape, bool useTriangulation, OcbBox* out);
```

### `ocb_shape_copy`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_copy(OcbShape shape, OcbShape* out);
```

### `ocb_shape_distance`

------------------------------------------------------------------ 几何分析（干涉检测） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_shape_distance(OcbShape a, OcbShape b, double* out);
```

### `ocb_shape_is_equal`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_is_equal(OcbShape a, OcbShape b, bool* out);
```

### `ocb_shape_is_null`

------------------------------------------------------------------ 拓扑与形状工具 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_shape_is_null(OcbShape shape, bool* out);
```

### `ocb_shape_location_cleared`

复制 shape 并清空其 TopLoc_Location(得到不带定位的本地形态)

```c
OCB_API OcbStatus OCB_CALL ocb_shape_location_cleared( OcbShape shape, OcbShape* out);
```

### `ocb_shape_make_box`

------------------------------------------------------------------ 形状（P0 只实现最小闭环：Box / 包围盒 / 体积 / 释放） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_shape_make_box( OcbPnt corner, double dx, double dy, double dz, OcbShape* out);
```

### `ocb_shape_moved`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_moved(OcbShape shape, OcbTrsf t, OcbShape* out);
```

### `ocb_shape_native_ptr`

```c
OCB_API void* OCB_CALL ocb_shape_native_ptr(OcbShape shape);
```

### `ocb_shape_ray_intersect`

--- 射线-实体求交：origin + dir 与 shape 求交，返回最近交点与面序号 ---

```c
OCB_API OcbStatus OCB_CALL ocb_shape_ray_intersect(OcbShape shape, OcbPnt origin, OcbDir dir, double* outParam, OcbPnt* outPnt, int* outFaceIndex);
```

### `ocb_shape_release`

```c
OCB_API void OCB_CALL ocb_shape_release(OcbShape shape);
```

### `ocb_shape_type`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_type(OcbShape shape, int* out);
```

### `ocb_shape_volume`

```c
OCB_API OcbStatus OCB_CALL ocb_shape_volume( OcbShape shape, double* out);
```

### `ocb_shape_wrap_native`

包装外部存在的 native TopoDS_Shape(浅拷贝接管值语义,原对象生命周期不受影响)

```c
OCB_API OcbStatus OCB_CALL ocb_shape_wrap_native(void* nativeShape, OcbShape* out);
```

## label（15）

### `ocb_label_child`

```c
OCB_API OcbStatus OCB_CALL ocb_label_child(OcbLabel parent, int tag, OcbLabel* out);
```

### `ocb_label_entry`

```c
OCB_API OcbStatus OCB_CALL ocb_label_entry(OcbLabel label, char* buf, int bufSize);
```

### `ocb_label_father`

```c
OCB_API OcbStatus OCB_CALL ocb_label_father(OcbLabel label, OcbLabel* out);
```

### `ocb_label_get_name`

```c
OCB_API OcbStatus OCB_CALL ocb_label_get_name(OcbLabel label, char* buf, int bufSize);
```

### `ocb_label_get_real`

```c
OCB_API OcbStatus OCB_CALL ocb_label_get_real(OcbLabel label, double* out);
```

### `ocb_label_get_shape`

------------------------------------------------------------------ OCAF 扩充 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_label_get_shape(OcbLabel label, OcbShape* out);
```

### `ocb_label_is_equal`

```c
OCB_API OcbStatus OCB_CALL ocb_label_is_equal(OcbLabel a, OcbLabel b, bool* out);
```

### `ocb_label_is_null`

```c
OCB_API OcbStatus OCB_CALL ocb_label_is_null(OcbLabel label, bool* out);
```

### `ocb_label_is_valid`

主动有效性查询:句柄登记存在且标签非空(对齐 IsWindow 的诊断便利性)

```c
OCB_API OcbStatus OCB_CALL ocb_label_is_valid(OcbLabel label, bool* out);
```

### `ocb_label_release`

```c
OCB_API void OCB_CALL ocb_label_release(OcbLabel label);
```

### `ocb_label_set_name`

```c
OCB_API OcbStatus OCB_CALL ocb_label_set_name(OcbLabel label, const char* name);
```

### `ocb_label_set_real`

```c
OCB_API OcbStatus OCB_CALL ocb_label_set_real(OcbLabel label, double value);
```

### `ocb_label_tag`

```c
OCB_API OcbStatus OCB_CALL ocb_label_tag(OcbLabel label, int* out);
```

### `ocb_label_walk_fathers_touch`

沿 Father 链向上重写 TNaming_NamedShape 小改(唤醒 UpdateAssemblies)

```c
OCB_API OcbStatus OCB_CALL ocb_label_walk_fathers_touch(OcbLabel label);
```

### `ocb_label_wrap_native`

迁移期适配：原生 TDF_Label* -> OcbLabel（值拷贝，不持有原生对象）

```c
OCB_API OcbStatus OCB_CALL ocb_label_wrap_native(const void* nativeLabel, OcbLabel* out);
```

## trsf（12）

### `ocb_trsf_apply`

```c
OCB_API OcbPnt OCB_CALL ocb_trsf_apply(OcbTrsf t, OcbPnt p);
```

### `ocb_trsf_from_native`

迁移期适配：原生 gp_Trsf* → OcbTrsf（值拷贝，不持有原生对象）

```c
OCB_API OcbStatus OCB_CALL ocb_trsf_from_native(const void* nativeTrsf, OcbTrsf* out);
```

### `ocb_trsf_from_quat_translation`

```c
OCB_API OcbStatus OCB_CALL ocb_trsf_from_quat_translation(OcbQuat q, double tx, double ty, double tz, OcbTrsf* out);
```

### `ocb_trsf_get_euler_zyx`

```c
OCB_API OcbStatus OCB_CALL ocb_trsf_get_euler_zyx(OcbTrsf t, double* rz, double* ry, double* rx);
```

### `ocb_trsf_get_quat`

```c
OCB_API OcbStatus OCB_CALL ocb_trsf_get_quat(OcbTrsf t, OcbQuat* out);
```

### `ocb_trsf_identity`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_identity(void);
```

### `ocb_trsf_inverted`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_inverted(OcbTrsf t);
```

### `ocb_trsf_multiply`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_multiply(OcbTrsf a, OcbTrsf b);
```

### `ocb_trsf_rotation`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_rotation(OcbPnt origin, OcbDir axis, double angle_rad);
```

### `ocb_trsf_set_values`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_set_values( double v11, double v12, double v13, double v14, double v21, double v22, double v23, double v24, double v31, double v32, double v33, double v34);
```

### `ocb_trsf_translation`

```c
OCB_API OcbTrsf OCB_CALL ocb_trsf_translation(OcbVec v);
```

### `ocb_trsf_translation_part`

```c
OCB_API OcbVec OCB_CALL ocb_trsf_translation_part(OcbTrsf t);
```

## doc（12）

### `ocb_doc_abort_command`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_abort_command(OcbDoc doc);
```

### `ocb_doc_commit_command`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_commit_command(OcbDoc doc);
```

### `ocb_doc_get_undo_limit`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_get_undo_limit(OcbDoc doc, int* out);
```

### `ocb_doc_has_open_command`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_has_open_command(OcbDoc doc, bool* out);
```

### `ocb_doc_main_label`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_main_label(OcbDoc doc, OcbLabel* out);
```

### `ocb_doc_new_command`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_new_command(OcbDoc doc);
```

### `ocb_doc_open`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_open(OcbApp app, const char* path, OcbDoc* out);
```

### `ocb_doc_redo`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_redo(OcbDoc doc, bool* ok);
```

### `ocb_doc_release`

```c
OCB_API void OCB_CALL ocb_doc_release(OcbDoc doc);
```

### `ocb_doc_save`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_save(OcbApp app, OcbDoc doc, const char* path);
```

### `ocb_doc_set_undo_limit`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_set_undo_limit(OcbDoc doc, int limit);
```

### `ocb_doc_undo`

```c
OCB_API OcbStatus OCB_CALL ocb_doc_undo(OcbDoc doc, bool* ok);
```

## viewer（12）

### `ocb_viewer_active_view`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_active_view(OcbViewer viewer, OcbView* out);
```

### `ocb_viewer_add_light`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_add_light(OcbViewer viewer, OcbLight light);
```

### `ocb_viewer_context`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_context(OcbViewer viewer, OcbAisContext* out);
```

### `ocb_viewer_create`

------------------------------------------------------------------ AIS 上下文（交互） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_create(void* hwnd, OcbViewer* out);
```

### `ocb_viewer_deactivate_grid`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_deactivate_grid(OcbViewer viewer);
```

### `ocb_viewer_lights_off`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_lights_off(OcbViewer viewer);
```

### `ocb_viewer_lights_on`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_lights_on(OcbViewer viewer);
```

### `ocb_viewer_native_ptr`

```c
OCB_API void* OCB_CALL ocb_viewer_native_ptr(OcbViewer viewer);
```

### `ocb_viewer_release`

```c
OCB_API void OCB_CALL ocb_viewer_release(OcbViewer viewer);
```

### `ocb_viewer_set_z_up_plane`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_set_z_up_plane(OcbViewer viewer);
```

### `ocb_viewer_setup_rect_grid`

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_setup_rect_grid(OcbViewer viewer, double step, double graphicSize, OcbColor echoColor, bool activate);
```

### `ocb_viewer_wrap_native`

迁移期适配：原生 V3d_Viewer* -> OcbViewer（addRef）

```c
OCB_API OcbStatus OCB_CALL ocb_viewer_wrap_native(void* nativeViewer, OcbViewer* out);
```

## make（11）

### `ocb_make_arc_of_circle`

```c
OCB_API OcbStatus OCB_CALL ocb_make_arc_of_circle(OcbPnt p1, OcbPnt p2, OcbPnt p3, OcbShape* out);
```

### `ocb_make_cone`

```c
OCB_API OcbStatus OCB_CALL ocb_make_cone(OcbAx2 axis, double r1, double r2, double h, OcbShape* out);
```

### `ocb_make_cylinder`

------------------------------------------------------------------ 体素（圆柱/圆锥/球） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_make_cylinder(OcbAx2 axis, double r, double h, OcbShape* out);
```

### `ocb_make_edge_from_curve`

```c
OCB_API OcbStatus OCB_CALL ocb_make_edge_from_curve(OcbCurve curve, OcbShape* out);
```

### `ocb_make_edge_pnt_pnt`

```c
OCB_API OcbStatus OCB_CALL ocb_make_edge_pnt_pnt(OcbPnt p1, OcbPnt p2, OcbShape* out);
```

### `ocb_make_face_from_wire`

```c
OCB_API OcbStatus OCB_CALL ocb_make_face_from_wire(OcbShape wire, OcbShape* out);
```

### `ocb_make_pipe`

```c
OCB_API OcbStatus OCB_CALL ocb_make_pipe(OcbShape spine_wire, OcbShape profile_face, OcbShape* out);
```

### `ocb_make_prism`

```c
OCB_API OcbStatus OCB_CALL ocb_make_prism(OcbShape face, OcbVec vec, OcbShape* out);
```

### `ocb_make_sphere`

```c
OCB_API OcbStatus OCB_CALL ocb_make_sphere(OcbPnt center, double r, OcbShape* out);
```

### `ocb_make_transform`

```c
OCB_API OcbStatus OCB_CALL ocb_make_transform(OcbShape shape, OcbTrsf t, OcbShape* out);
```

### `ocb_make_wire_from_edges`

```c
OCB_API OcbStatus OCB_CALL ocb_make_wire_from_edges(OcbEdge e1, OcbEdge e2, OcbEdge e3, OcbShape* out);
```

## topo（9）

### `ocb_topo_add`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_add(OcbShape compound, OcbShape part);
```

### `ocb_topo_count_edges`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_count_edges(OcbShape shape, int* out);
```

### `ocb_topo_count_solids`

统计 shape 中的 SOLID 数（compound/solid 通用；非实体形状返回 0）

```c
OCB_API OcbStatus OCB_CALL ocb_topo_count_solids(OcbShape shape, int* out);
```

### `ocb_topo_edge_from_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_edge_from_shape(OcbShape shape, OcbEdge* out);
```

### `ocb_topo_get_child`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_get_child(OcbShape shape, int index, OcbShape* out);
```

### `ocb_topo_get_solid`

取第 index 个 SOLID（0-based）；越界返回 OCB_ERR_INVALID_ARG

```c
OCB_API OcbStatus OCB_CALL ocb_topo_get_solid(OcbShape shape, int index, OcbShape* out);
```

### `ocb_topo_get_vertex`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_get_vertex(OcbShape shape, int index, OcbShape* out);
```

### `ocb_topo_make_compound`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_make_compound(OcbShape* out);
```

### `ocb_topo_vertex_from_shape`

```c
OCB_API OcbStatus OCB_CALL ocb_topo_vertex_from_shape(OcbShape shape, OcbShape* out);
```

## drawer（8）

### `ocb_drawer_release`

```c
OCB_API void OCB_CALL ocb_drawer_release(OcbDrawer drawer);
```

### `ocb_drawer_set_face_boundary_aspect`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_face_boundary_aspect(OcbDrawer drawer, OcbLineAspect aspect);
```

### `ocb_drawer_set_face_boundary_draw`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_face_boundary_draw(OcbDrawer drawer, bool enable);
```

### `ocb_drawer_set_free_boundary_draw`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_free_boundary_draw(OcbDrawer drawer, bool enable);
```

### `ocb_drawer_set_line_aspect`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_line_aspect(OcbDrawer drawer, OcbLineAspect aspect);
```

### `ocb_drawer_set_point_aspect`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_point_aspect(OcbDrawer drawer, OcbPointAspect aspect);
```

### `ocb_drawer_set_polygon_offsets`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_set_polygon_offsets(OcbDrawer drawer, int mode, float factor, float units);
```

### `ocb_drawer_setup_own_shading_aspect`

```c
OCB_API OcbStatus OCB_CALL ocb_drawer_setup_own_shading_aspect(OcbDrawer drawer);
```

## stepwriter（8）

### `ocb_stepwriter_create`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_create(OcbStepWriter* out);
```

### `ocb_stepwriter_release`

```c
OCB_API void OCB_CALL ocb_stepwriter_release(OcbStepWriter w);
```

### `ocb_stepwriter_set_color_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_color_mode(OcbStepWriter w, bool mode);
```

### `ocb_stepwriter_set_layer_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_layer_mode(OcbStepWriter w, bool mode);
```

### `ocb_stepwriter_set_name_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_name_mode(OcbStepWriter w, bool mode);
```

### `ocb_stepwriter_transfer`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_transfer(OcbStepWriter w, OcbDoc doc);
```

### `ocb_stepwriter_transfer_label`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_transfer_label(OcbStepWriter w, OcbLabel label);
```

### `ocb_stepwriter_write`

```c
OCB_API OcbStatus OCB_CALL ocb_stepwriter_write(OcbStepWriter w, const char* path);
```

## explorer（7）

### `ocb_explorer_current_edge`

```c
OCB_API OcbStatus OCB_CALL ocb_explorer_current_edge(OcbExplorer exp, OcbEdge* out);
```

### `ocb_explorer_current_face`

```c
OCB_API OcbStatus OCB_CALL ocb_explorer_current_face(OcbExplorer exp, OcbFace* out);
```

### `ocb_explorer_more`

```c
OCB_API OcbStatus OCB_CALL ocb_explorer_more(OcbExplorer exp, bool* out);
```

### `ocb_explorer_new_edges`

```c
OCB_API OcbStatus OCB_CALL ocb_explorer_new_edges(OcbShape shape, OcbExplorer* out);
```

### `ocb_explorer_new_faces`

```c
OCB_API OcbStatus OCB_CALL ocb_explorer_new_faces(OcbShape shape, OcbExplorer* out);
```

### `ocb_explorer_next`

```c
OCB_API void OCB_CALL ocb_explorer_next(OcbExplorer exp);
```

### `ocb_explorer_release`

```c
OCB_API void OCB_CALL ocb_explorer_release(OcbExplorer exp);
```

## bool（6）

### `ocb_bool_common`

```c
OCB_API OcbStatus OCB_CALL ocb_bool_common(OcbShape a, OcbShape b, OcbShape* out);
```

### `ocb_bool_common_ex`

```c
OCB_API OcbStatus OCB_CALL ocb_bool_common_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
```

### `ocb_bool_cut`

```c
OCB_API OcbStatus OCB_CALL ocb_bool_cut(OcbShape a, OcbShape b, OcbShape* out);
```

### `ocb_bool_cut_ex`

```c
OCB_API OcbStatus OCB_CALL ocb_bool_cut_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
```

### `ocb_bool_fuse`

------------------------------------------------------------------ 布尔操作 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_bool_fuse(OcbShape a, OcbShape b, OcbShape* out);
```

### `ocb_bool_fuse_ex`

```c
OCB_API OcbStatus OCB_CALL ocb_bool_fuse_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
```

## gprops（6）

### `ocb_gprops_center_of_mass`

```c
OCB_API OcbStatus OCB_CALL ocb_gprops_center_of_mass(OcbGProps props, OcbPnt* out);
```

### `ocb_gprops_linear`

```c
OCB_API OcbStatus OCB_CALL ocb_gprops_linear(OcbShape shape, OcbGProps* out);
```

### `ocb_gprops_mass`

```c
OCB_API OcbStatus OCB_CALL ocb_gprops_mass(OcbGProps props, double* out);
```

### `ocb_gprops_release`

```c
OCB_API void OCB_CALL ocb_gprops_release(OcbGProps props);
```

### `ocb_gprops_surface`

```c
OCB_API OcbStatus OCB_CALL ocb_gprops_surface(OcbShape shape, OcbGProps* out);
```

### `ocb_gprops_volume`

------------------------------------------------------------------ 属性（质量/体积/质心） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_gprops_volume(OcbShape shape, OcbGProps* out);
```

## stepreader（6）

### `ocb_stepreader_create`

```c
OCB_API OcbStatus OCB_CALL ocb_stepreader_create(OcbStepReader* out);
```

### `ocb_stepreader_read_file`

```c
OCB_API OcbStatus OCB_CALL ocb_stepreader_read_file(OcbStepReader r, const char* path);
```

### `ocb_stepreader_release`

```c
OCB_API void OCB_CALL ocb_stepreader_release(OcbStepReader r);
```

### `ocb_stepreader_set_color_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_color_mode(OcbStepReader r, bool mode);
```

### `ocb_stepreader_set_name_mode`

```c
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_name_mode(OcbStepReader r, bool mode);
```

### `ocb_stepreader_transfer`

```c
OCB_API OcbStatus OCB_CALL ocb_stepreader_transfer(OcbStepReader r, OcbDoc doc);
```

## point（5）

### `ocb_point_aspect_new`

```c
OCB_API OcbStatus OCB_CALL ocb_point_aspect_new(int markerType, OcbColor color, double size, OcbPointAspect* out);
```

### `ocb_point_aspect_release`

```c
OCB_API void OCB_CALL ocb_point_aspect_release(OcbPointAspect aspect);
```

### `ocb_point_distance`

```c
OCB_API double OCB_CALL ocb_point_distance(OcbPnt a, OcbPnt b);
```

### `ocb_point_transformed`

```c
OCB_API OcbPnt OCB_CALL ocb_point_transformed(OcbPnt p, OcbTrsf t);
```

### `ocb_point_xyz`

------------------------------------------------------------------ 基础几何 ------------------------------------------------------------------

```c
OCB_API OcbPnt OCB_CALL ocb_point_xyz(double x, double y, double z);
```

## box（5）

### `ocb_box_add`

```c
OCB_API void OCB_CALL ocb_box_add(OcbBox* dst, const OcbBox* src);
```

### `ocb_box_from_shape`

--- 盒运算（Bnd_Box 最小替代，供迁移期替换 BRepBndLib/Bnd_Box 判断） ---

```c
OCB_API void OCB_CALL ocb_box_from_shape(OcbShape shape, bool useTriangulation, OcbBox* out);
```

### `ocb_box_is_out`

```c
OCB_API bool OCB_CALL ocb_box_is_out(const OcbBox* a, const OcbBox* b);
```

### `ocb_box_is_out_gap`

```c
OCB_API bool OCB_CALL ocb_box_is_out_gap(const OcbBox* a, const OcbBox* b, double gap);
```

### `ocb_box_transformed`

```c
OCB_API void OCB_CALL ocb_box_transformed(const OcbBox* box, OcbTrsf trsf, OcbBox* out);
```

## colortool（5）

### `ocb_colortool_get`

```c
OCB_API OcbStatus OCB_CALL ocb_colortool_get(OcbDoc doc, OcbColorTool* out);
```

### `ocb_colortool_get_color`

```c
OCB_API OcbStatus OCB_CALL ocb_colortool_get_color(OcbColorTool tool, OcbLabel label, int colorType, bool* okOut, OcbColor* out);
```

### `ocb_colortool_release`

```c
OCB_API void OCB_CALL ocb_colortool_release(OcbColorTool tool);
```

### `ocb_colortool_set_color`

```c
OCB_API OcbStatus OCB_CALL ocb_colortool_set_color(OcbColorTool tool, OcbLabel label, OcbColor color, int colorType);
```

### `ocb_colortool_set_shape_color`

```c
OCB_API OcbStatus OCB_CALL ocb_colortool_set_shape_color(OcbColorTool tool, OcbShape shape, OcbColor color, int colorType);
```

## app（4）

### `ocb_app_close`

```c
OCB_API OcbStatus OCB_CALL ocb_app_close(OcbApp app, OcbDoc doc);
```

### `ocb_app_new`

------------------------------------------------------------------ OCAF 文档 / 标签（P1c-1 最小集） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_app_new(OcbApp* out);
```

### `ocb_app_new_document`

```c
OCB_API OcbStatus OCB_CALL ocb_app_new_document(OcbApp app, const char* format, OcbDoc* out);
```

### `ocb_app_release`

```c
OCB_API void OCB_CALL ocb_app_release(OcbApp app);
```

## fillet（4）

### `ocb_fillet_add`

```c
OCB_API OcbStatus OCB_CALL ocb_fillet_add(OcbFillet fillet, OcbShape edge, double radius);
```

### `ocb_fillet_build`

```c
OCB_API OcbStatus OCB_CALL ocb_fillet_build(OcbFillet fillet, OcbShape* out);
```

### `ocb_fillet_new`

------------------------------------------------------------------ 特征操作（倒圆 / 倒角 / 扫掠） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_fillet_new(OcbShape base, OcbFillet* out);
```

### `ocb_fillet_release`

```c
OCB_API void OCB_CALL ocb_fillet_release(OcbFillet fillet);
```

## chamfer（4）

### `ocb_chamfer_add`

```c
OCB_API OcbStatus OCB_CALL ocb_chamfer_add(OcbChamfer chamfer, OcbShape edge, double dist);
```

### `ocb_chamfer_build`

```c
OCB_API OcbStatus OCB_CALL ocb_chamfer_build(OcbChamfer chamfer, OcbShape* out);
```

### `ocb_chamfer_new`

```c
OCB_API OcbStatus OCB_CALL ocb_chamfer_new(OcbShape base, OcbChamfer* out);
```

### `ocb_chamfer_release`

```c
OCB_API void OCB_CALL ocb_chamfer_release(OcbChamfer chamfer);
```

## tri（4）

### `ocb_tri_get_triangle`

```c
OCB_API OcbStatus OCB_CALL ocb_tri_get_triangle(OcbTri tri, int index, int* n1, int* n2, int* n3);
```

### `ocb_tri_nb_triangles`

```c
OCB_API OcbStatus OCB_CALL ocb_tri_nb_triangles(OcbTri tri, int* out);
```

### `ocb_tri_node`

```c
OCB_API OcbStatus OCB_CALL ocb_tri_node(OcbTri tri, int index, OcbPnt* pnt);
```

### `ocb_tri_release`

```c
OCB_API void OCB_CALL ocb_tri_release(OcbTri tri);
```

## curve（3）

### `ocb_curve_polyline`

--- 曲线离散：把 curve 在 [first,last] 上按弦高误差离散为点列 ---

```c
OCB_API OcbStatus OCB_CALL ocb_curve_polyline(OcbCurve curve, double firstParam, double lastParam, double deflection, OcbPnt* buffer, int capacity, int* outCount);
```

### `ocb_curve_release`

```c
OCB_API void OCB_CALL ocb_curve_release(OcbCurve curve);
```

### `ocb_curve_value`

```c
OCB_API OcbStatus OCB_CALL ocb_curve_value(OcbCurve curve, double param, OcbPnt* out);
```

## classifier（3）

### `ocb_classifier_new`

state 取值: 0=TopAbs_IN, 1=TopAbs_OUT, 2=TopAbs_ON, 3=TopAbs_UNKNOWN

```c
OCB_API OcbStatus OCB_CALL ocb_classifier_new(OcbShape shape, OcbCls* out);
```

### `ocb_classifier_perform`

```c
OCB_API OcbStatus OCB_CALL ocb_classifier_perform(OcbCls cls, OcbPnt pnt, double tol, int* state);
```

### `ocb_classifier_release`

```c
OCB_API void OCB_CALL ocb_classifier_release(OcbCls cls);
```

## light（3）

### `ocb_light_new_ambient`

------------------------------------------------------------------ 灯光 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_light_new_ambient(float r, float g, float b, float intensity, OcbLight* out);
```

### `ocb_light_new_directional`

```c
OCB_API OcbStatus OCB_CALL ocb_light_new_directional( float r, float g, float b, float intensity, bool headlight, double dx, double dy, double dz, OcbLight* out);
```

### `ocb_light_release`

```c
OCB_API void OCB_CALL ocb_light_release(OcbLight light);
```

## edge（3）

### `ocb_edge_curve`

```c
OCB_API OcbStatus OCB_CALL ocb_edge_curve(OcbEdge edge, double* first, double* last, OcbCurve* out);
```

### `ocb_edge_first_param`

```c
OCB_API OcbStatus OCB_CALL ocb_edge_first_param(OcbEdge edge, double* out);
```

### `ocb_edge_last_param`

```c
OCB_API OcbStatus OCB_CALL ocb_edge_last_param(OcbEdge edge, double* out);
```

## last（2）

### `ocb_last_error`

```c
OCB_API const char* OCB_CALL ocb_last_error(void);
```

### `ocb_last_status`

```c
OCB_API OcbStatus OCB_CALL ocb_last_status(void);
```

## vec（2）

### `ocb_vec_modulus`

```c
OCB_API double OCB_CALL ocb_vec_modulus(OcbVec v);
```

### `ocb_vec_xyz`

------------------------------------------------------------------ 基础几何（补充） ------------------------------------------------------------------

```c
OCB_API OcbVec OCB_CALL ocb_vec_xyz(double x, double y, double z);
```

## quaternion（2）

### `ocb_quaternion_dot`

```c
OCB_API OcbStatus OCB_CALL ocb_quaternion_dot(OcbQuat a, OcbQuat b, double* out);
```

### `ocb_quaternion_from_euler_zyx`

------------------------------------------------------------------ 四元数与变换 ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_quaternion_from_euler_zyx(double rz, double ry, double rx, OcbQuat* out);
```

## interface（2）

### `ocb_interface_set_int`

------------------------------------------------------------------ STEP 全局参数（Interface_Static） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_interface_set_int(const char* key, int value);
```

### `ocb_interface_set_real`

```c
OCB_API OcbStatus OCB_CALL ocb_interface_set_real(const char* key, double value);
```

## step（2）

### `ocb_step_read`

------------------------------------------------------------------ STEP 文件 IO ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_step_read(const char* path, OcbShape* out);
```

### `ocb_step_write`

```c
OCB_API OcbStatus OCB_CALL ocb_step_write(OcbShape shape, const char* path);
```

## geom（2）

### `ocb_geom_new_cartesian_point`

------------------------------------------------------------------ AIS 对象（补充） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_geom_new_cartesian_point(double x, double y, double z, OcbGeom* out);
```

### `ocb_geom_release`

```c
OCB_API void OCB_CALL ocb_geom_release(OcbGeom geom);
```

## line（2）

### `ocb_line_aspect_new`

```c
OCB_API OcbStatus OCB_CALL ocb_line_aspect_new(OcbColor color, double width, OcbLineAspect* out);
```

### `ocb_line_aspect_release`

```c
OCB_API void OCB_CALL ocb_line_aspect_release(OcbLineAspect la);
```

## version（1）

### `ocb_version`

------------------------------------------------------------------ 生命周期 / 诊断 ------------------------------------------------------------------

```c
OCB_API const char* OCB_CALL ocb_version(void);
```

## live（1）

### `ocb_live_count`

```c
OCB_API uint64_t OCB_CALL ocb_live_count(void);
```

## dir（1）

### `ocb_dir_xyz`

```c
OCB_API OcbDir OCB_CALL ocb_dir_xyz(double dx, double dy, double dz);
```

## vertex（1）

### `ocb_vertex_pnt`

```c
OCB_API OcbStatus OCB_CALL ocb_vertex_pnt(OcbShape vertex, OcbPnt* out);
```

## mesh（1）

### `ocb_mesh_generate`

------------------------------------------------------------------ 网格（三角化） ------------------------------------------------------------------

```c
OCB_API OcbStatus OCB_CALL ocb_mesh_generate(OcbShape shape, double deflection);
```

## face（1）

### `ocb_face_triangulation`

```c
OCB_API OcbStatus OCB_CALL ocb_face_triangulation(OcbFace face, OcbTri* out, OcbTrsf* loc);
```

## xcaf（1）

### `ocb_xcaf_location_set`

```c
OCB_API OcbStatus OCB_CALL ocb_xcaf_location_set(OcbLabel label, OcbTrsf trsf);
```

## 缺注释清单（258 / 320）

- `ocb_last_status`
- `ocb_last_error`
- `ocb_live_count`
- `ocb_point_distance`
- `ocb_dir_xyz`
- `ocb_trsf_identity`
- `ocb_trsf_translation`
- `ocb_trsf_apply`
- `ocb_shape_bounding_box`
- `ocb_box_is_out`
- `ocb_box_is_out_gap`
- `ocb_box_add`
- `ocb_box_transformed`
- `ocb_shape_volume`
- `ocb_shape_release`
- `ocb_app_new_document`
- `ocb_app_close`
- `ocb_app_release`
- `ocb_doc_new_command`
- `ocb_doc_commit_command`
- `ocb_doc_undo`
- `ocb_doc_redo`
- `ocb_doc_main_label`
- `ocb_doc_save`
- `ocb_doc_open`
- `ocb_doc_set_undo_limit`
- `ocb_doc_get_undo_limit`
- `ocb_doc_release`
- `ocb_shapetool_get`
- `ocb_shapetool_add_shape`
- `ocb_shapetool_add_shape_ex`
- `ocb_shapetool_get_shape`
- `ocb_shapetool_set_shape`
- `ocb_shapetool_update`
- `ocb_shapetool_free_shape_count`
- `ocb_shapetool_release`
- `ocb_label_set_name`
- `ocb_label_get_name`
- `ocb_label_release`
- `ocb_vec_modulus`
- `ocb_point_transformed`
- `ocb_trsf_rotation`
- `ocb_trsf_set_values`
- `ocb_trsf_translation_part`
- `ocb_trsf_multiply`
- `ocb_trsf_inverted`
- `ocb_make_cone`
- `ocb_make_sphere`
- `ocb_bool_cut`
- `ocb_bool_common`
- `ocb_bool_fuse_ex`
- `ocb_bool_cut_ex`
- `ocb_bool_common_ex`
- `ocb_classifier_perform`
- `ocb_classifier_release`
- `ocb_fillet_add`
- `ocb_fillet_build`
- `ocb_fillet_release`
- `ocb_chamfer_new`
- `ocb_chamfer_add`
- `ocb_chamfer_build`
- `ocb_chamfer_release`
- `ocb_make_pipe`
- `ocb_quaternion_dot`
- `ocb_trsf_from_quat_translation`
- `ocb_trsf_get_quat`
- `ocb_trsf_get_euler_zyx`
- `ocb_interface_set_real`
- `ocb_light_new_directional`
- `ocb_light_release`
- `ocb_point_aspect_new`
- `ocb_point_aspect_release`
- `ocb_drawer_set_point_aspect`
- `ocb_drawer_set_line_aspect`
- `ocb_viewer_add_light`
- `ocb_viewer_lights_on`
- `ocb_viewer_lights_off`
- `ocb_ais_ctx_native_ptr`
- `ocb_shape_native_ptr`
- `ocb_shape_is_equal`
- `ocb_shape_type`
- `ocb_shape_moved`
- `ocb_shape_copy`
- `ocb_topo_make_compound`
- `ocb_topo_add`
- `ocb_topo_count_edges`
- `ocb_topo_get_child`
- `ocb_topo_edge_from_shape`
- `ocb_topo_vertex_from_shape`
- `ocb_topo_get_vertex`
- `ocb_vertex_pnt`
- `ocb_edge_first_param`
- `ocb_edge_last_param`
- `ocb_edge_curve`
- `ocb_curve_value`
- `ocb_curve_release`
- `ocb_make_edge_pnt_pnt`
- `ocb_make_edge_from_curve`
- `ocb_make_arc_of_circle`
- `ocb_make_wire_from_edges`
- `ocb_make_face_from_wire`
- `ocb_make_transform`
- `ocb_make_prism`
- `ocb_explorer_new_faces`
- `ocb_explorer_new_edges`
- `ocb_explorer_more`
- `ocb_explorer_next`
- `ocb_explorer_current_face`
- `ocb_explorer_current_edge`
- `ocb_explorer_release`
- `ocb_face_triangulation`
- `ocb_tri_nb_triangles`
- `ocb_tri_get_triangle`
- `ocb_tri_node`
- `ocb_tri_release`
- `ocb_gprops_linear`
- `ocb_gprops_surface`
- `ocb_gprops_mass`
- `ocb_gprops_center_of_mass`
- `ocb_gprops_release`
- `ocb_step_write`
- `ocb_stepreader_create`
- `ocb_stepreader_set_color_mode`
- `ocb_stepreader_set_name_mode`
- `ocb_stepreader_read_file`
- `ocb_stepreader_transfer`
- `ocb_stepreader_release`
- `ocb_stepwriter_create`
- `ocb_stepwriter_set_color_mode`
- `ocb_stepwriter_set_name_mode`
- `ocb_stepwriter_set_layer_mode`
- `ocb_stepwriter_transfer`
- `ocb_stepwriter_transfer_label`
- `ocb_stepwriter_write`
- `ocb_stepwriter_release`
- `ocb_geom_release`
- `ocb_ais_new_point`
- `ocb_ais_new_xcaf_object`
- `ocb_ais_set_color`
- `ocb_ais_set_material`
- `ocb_ais_set_display_mode`
- `ocb_ais_set_transparency`
- `ocb_ais_set_marker`
- `ocb_ais_set_zlayer`
- `ocb_ais_zlayer`
- `ocb_ais_has_color`
- `ocb_ais_get_color`
- `ocb_ais_transparency`
- `ocb_ais_material`
- `ocb_ais_attributes`
- `ocb_ais_downcast_shape`
- `ocb_ais_downcast_point`
- `ocb_ais_point_set_position`
- `ocb_ais_shape_shape`
- `ocb_ais_has_transformation`
- `ocb_ais_local_transformation`
- `ocb_ais_xcaf_get_label`
- `ocb_line_aspect_new`
- `ocb_line_aspect_release`
- `ocb_drawer_set_face_boundary_draw`
- `ocb_drawer_set_free_boundary_draw`
- `ocb_drawer_set_face_boundary_aspect`
- `ocb_drawer_set_polygon_offsets`
- `ocb_drawer_setup_own_shading_aspect`
- `ocb_drawer_release`
- `ocb_viewer_context`
- `ocb_viewer_active_view`
- `ocb_viewer_release`
- `ocb_ais_ctx_display`
- `ocb_ais_ctx_display_ex`
- `ocb_ais_ctx_erase`
- `ocb_ais_ctx_remove`
- `ocb_ais_ctx_update_viewer`
- `ocb_ais_ctx_displayed_at`
- `ocb_ais_ctx_is_displayed`
- `ocb_ais_ctx_clear_selected`
- `ocb_ais_ctx_activate`
- `ocb_ais_ctx_deactivate`
- `ocb_ais_ctx_deactivate_mode`
- `ocb_ais_ctx_set_color`
- `ocb_ais_ctx_set_material`
- `ocb_ais_ctx_set_transparency`
- `ocb_ais_ctx_unset_transparency`
- `ocb_ais_ctx_set_display_priority`
- `ocb_ais_ctx_remove_all`
- `ocb_ais_ctx_move_to`
- `ocb_ais_ctx_has_detected`
- `ocb_ais_ctx_detected`
- `ocb_ais_ctx_selected_native`
- `ocb_ais_ctx_selected_owner_native`
- `ocb_ais_ctx_displayed_at_native`
- `ocb_ais_ctx_selected_owner`
- `ocb_ais_ctx_select_detected`
- `ocb_ais_ctx_set_display_mode`
- `ocb_ais_ctx_set_automatic_hilight`
- `ocb_ais_ctx_set_highlight_color`
- `ocb_ais_ctx_init_selected`
- `ocb_ais_ctx_more_selected`
- `ocb_ais_ctx_next_selected`
- `ocb_ais_ctx_set_selected`
- `ocb_ais_ctx_add_or_remove_selected`
- `ocb_ais_ctx_selected`
- `ocb_ais_ctx_picked_point`
- `ocb_ais_ctx_set_location`
- `ocb_ais_ctx_redisplay`
- `ocb_ais_ctx_release`
- `ocb_view_fit_all_margin`
- `ocb_view_zfit_all`
- `ocb_view_redraw`
- `ocb_view_redraw_immediate`
- `ocb_view_invalidate`
- `ocb_view_set_bg_gradient`
- `ocb_view_convert_with_proj`
- `ocb_view_native_ptr`
- `ocb_viewer_native_ptr`
- `ocb_ais_selection_probe`
- `ocb_ais_pick_probe`
- `ocb_view_project_point`
- `ocb_view_rotation`
- `ocb_view_pan`
- `ocb_view_start_zoom_at_point`
- `ocb_view_zoom_at_point`
- `ocb_viewer_set_z_up_plane`
- `ocb_viewer_deactivate_grid`
- `ocb_viewer_setup_rect_grid`
- `ocb_label_is_equal`
- `ocb_label_is_null`
- `ocb_label_tag`
- `ocb_label_entry`
- `ocb_label_child`
- `ocb_label_father`
- `ocb_label_set_real`
- `ocb_label_get_real`
- `ocb_doc_has_open_command`
- `ocb_doc_abort_command`
- `ocb_shapetool_new_shape`
- `ocb_shapetool_is_reference`
- `ocb_shapetool_get_referred_shape`
- `ocb_shapetool_is_component`
- `ocb_shapetool_is_assembly`
- `ocb_shapetool_get_component_count`
- `ocb_shapetool_get_user_count`
- `ocb_shapetool_get_user`
- `ocb_shapetool_get_component`
- `ocb_shapetool_add_component`
- `ocb_shapetool_remove_component`
- `ocb_shapetool_set_location`
- `ocb_shapetool_get_location`
- `ocb_shapetool_get_free_shape`
- `ocb_shapetool_get_sub_shape_count`
- `ocb_shapetool_get_sub_shape`
- `ocb_colortool_get`
- `ocb_colortool_set_color`
- `ocb_colortool_set_shape_color`
- `ocb_colortool_get_color`
- `ocb_colortool_release`
- `ocb_xcaf_location_set`
- `ocb_ais_release`