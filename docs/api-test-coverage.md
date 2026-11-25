<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# 桥 API 测试覆盖审计（2026-11-18 夜班自动生成）

- 导出 API 总数：**320**
- 单测中被引用：**227**
- 未被引用：**93**（覆盖率 71%）

> 说明：被引用 ≠ 有断言。本表用于找「完全没碰过」的 API；GL/窗口类（view/v3d 相关）在无显示环境不便测，标注为豁免。

## ais（48 个未覆盖）

ais_ctx_add_or_remove_selected, ais_ctx_clear_selected, ais_ctx_deactivate, ais_ctx_deactivate_mode, ais_ctx_detected, ais_ctx_detected_native, ais_ctx_display_ex, ais_ctx_displayed_at_native, ais_ctx_erase, ais_ctx_has_detected, ais_ctx_init_selected, ais_ctx_more_selected, ais_ctx_move_to, ais_ctx_native_ptr, ais_ctx_next_selected, ais_ctx_picked_point, ais_ctx_redisplay, ais_ctx_remove, ais_ctx_remove_all, ais_ctx_select_detected, ais_ctx_select_rectangle, ais_ctx_selected, ais_ctx_selected_native, ais_ctx_selected_owner, ais_ctx_selected_owner_native, ais_ctx_set_automatic_hilight, ais_ctx_set_color, ais_ctx_set_default_deflection, ais_ctx_set_display_mode, ais_ctx_set_display_priority, ais_ctx_set_highlight_color, ais_ctx_set_location, ais_ctx_set_material, ais_ctx_set_pixel_tolerance, ais_ctx_set_selected, ais_ctx_set_selection_style, ais_ctx_set_transparency, ais_ctx_unset_transparency, ais_ctx_update_viewer, ais_downcast_point, ais_new_xcaf_object, ais_pick_probe, ais_selection_probe, ais_set_display_mode, ais_set_local_transformation, ais_set_marker, ais_set_material, ais_set_zlayer

## view（14 个未覆盖）

view_fit_all_margin, view_invalidate, view_native_ptr, view_pan, view_redraw_immediate, view_rotation, view_set_bg_gradient, view_set_proj_type, view_set_rendering_params, view_start_rotation, view_start_zoom_at_point, view_trihedron_ex, view_wrap_native, view_zoom_at_point

## shapetool（10 个未覆盖）

shapetool_get_free_shape, shapetool_get_user, shapetool_is_component, shapetool_rebuild_assembly_shapes, shapetool_remove_component, shapetool_remove_shape, shapetool_set_assembly_placeholder, shapetool_set_location, shapetool_set_shape, shapetool_update

## stepwriter（7 个未覆盖）

stepwriter_release, stepwriter_set_color_mode, stepwriter_set_layer_mode, stepwriter_set_name_mode, stepwriter_transfer, stepwriter_transfer_label, stepwriter_write

## viewer（6 个未覆盖）

viewer_active_view, viewer_deactivate_grid, viewer_native_ptr, viewer_set_z_up_plane, viewer_setup_rect_grid, viewer_wrap_native

## label（2 个未覆盖）

label_walk_fathers_touch, label_wrap_native

## doc（1 个未覆盖）

doc_get_undo_limit

## stepreader（1 个未覆盖）

stepreader_set_color_mode

## tri（1 个未覆盖）

tri_get_triangle

## trsf（1 个未覆盖）

trsf_from_native

## vertex（1 个未覆盖）

vertex_pnt

## xcaf（1 个未覆盖）

xcaf_location_set
