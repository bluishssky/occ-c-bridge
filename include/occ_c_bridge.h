// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#ifndef OCC_C_BRIDGE_H
#define OCC_C_BRIDGE_H

#include "occ_c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * occ_c_bridge v1.0 - 使用约定（先读这一段）
 *
 * 1) 句柄与释放
 *    - 所有 Ocb* 都是 uint64_t 句柄，OCB_INVALID(0) 表示无效。
 *    - 句柄按值传递，拷贝不增加引用；禁止重复 release。
 *    - 创建/返回句柄的一方负责 ocb_*_release()；*_wrap_native() 会新增条目，
 *      同样必须 release。
 *
 * 2) 返回码
 *    - 函数返回 OcbStatus：OCB_OK(0) 成功；失败原因用 ocb_last_status() /
 *      ocb_last_error() 获取（内部为线程本地缓冲，多线程下请立即读取）。
 *    - 返回句柄的函数不报错，用 OCB_INVALID 表示无效。
 *
 * 3) 典型调用顺序（按逆序释放）
 *    - 几何：ocb_shape_make_* -> 查询/计算 -> ocb_shape_release
 *    - OCAF：ocb_app_new -> ocb_app_new_document -> ocb_shapetool_get ->
 *            ocb_shapetool_add_shape -> ... -> release(shapetool/label/doc/app)
 *    - 视图：ocb_viewer_create(hwnd) -> ocb_view_create(viewer,hwnd) ->
 *            ocb_viewer_context(viewer,&ctx) -> ocb_ais_ctx_display -> ...
 *    - STEP：ocb_stepreader_create -> set_color/name_mode -> read_file ->
 *            transfer(doc) -> ocb_stepreader_release
 *
 * 4) 线程与线程安全
 *    - 句柄表非线程安全：多线程调用需在调用方串行化。
 *    - 视图/上下文/AIS 对象只能在创建它的 UI 线程使用（OCCT 渲染约束）。
 *
 * 5) 依赖
 *    - 头文件自包含，不引任何 OpenCASCADE 头；运行时需要 occ_c_bridge.dll
 *      与 OCCT/TBB/FreeType 运行库（发行包已放在 bin/）。
 *
 * 6) 版本
 *    - ocb_version() 返回 "1.0.0"；遵循语义化版本，同主版本内 ABI 兼容。
 * ================================================================== */


/* ------------------------------------------------------------------
 * 生命周期 / 诊断
 * ------------------------------------------------------------------ */

OCB_API const char* OCB_CALL ocb_version(void);
OCB_API OcbStatus   OCB_CALL ocb_last_status(void);
OCB_API const char* OCB_CALL ocb_last_error(void);
OCB_API uint64_t    OCB_CALL ocb_live_count(void);

/* ------------------------------------------------------------------
 * 基础几何
 * ------------------------------------------------------------------ */

OCB_API OcbPnt  OCB_CALL ocb_point_xyz(double x, double y, double z);
OCB_API double  OCB_CALL ocb_point_distance(OcbPnt a, OcbPnt b);
OCB_API OcbDir  OCB_CALL ocb_dir_xyz(double dx, double dy, double dz);

OCB_API OcbTrsf OCB_CALL ocb_trsf_identity(void);
OCB_API OcbTrsf OCB_CALL ocb_trsf_translation(OcbVec v);
OCB_API OcbPnt  OCB_CALL ocb_trsf_apply(OcbTrsf t, OcbPnt p);

/* ------------------------------------------------------------------
 * 形状（P0 只实现最小闭环：Box / 包围盒 / 体积 / 释放）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_shape_make_box(
    OcbPnt corner, double dx, double dy, double dz, OcbShape* out);

OCB_API OcbStatus OCB_CALL ocb_shape_bounding_box(
    OcbShape shape, OcbBox* out);
/* --- 盒运算（Bnd_Box 最小替代，供迁移期替换 BRepBndLib/Bnd_Box 判断） --- */
OCB_API void      OCB_CALL ocb_box_from_shape(OcbShape shape, bool useTriangulation, OcbBox* out);
OCB_API bool      OCB_CALL ocb_box_is_out(const OcbBox* a, const OcbBox* b);
OCB_API bool      OCB_CALL ocb_box_is_out_gap(const OcbBox* a, const OcbBox* b, double gap);
OCB_API void      OCB_CALL ocb_box_add(OcbBox* dst, const OcbBox* src);
OCB_API void      OCB_CALL ocb_box_transformed(const OcbBox* box, OcbTrsf trsf, OcbBox* out);
/* --- 曲线离散：把 curve 在 [first,last] 上按弦高误差离散为点列 --- */
OCB_API OcbStatus OCB_CALL ocb_curve_polyline(OcbCurve curve, double firstParam, double lastParam,
    double deflection, OcbPnt* buffer, int capacity, int* outCount);
/* --- 射线-实体求交：origin + dir 与 shape 求交，返回最近交点与面序号 --- */
OCB_API OcbStatus OCB_CALL ocb_shape_ray_intersect(OcbShape shape, OcbPnt origin, OcbDir dir,
    double* outParam, OcbPnt* outPnt, int* outFaceIndex);

/* useTriangulation 与 BRepBndLib::Add 的同名参数一致 */
OCB_API OcbStatus OCB_CALL ocb_shape_bounding_box_ex(
    OcbShape shape, bool useTriangulation, OcbBox* out);

/* 复制 shape 并清空其 TopLoc_Location(得到不带定位的本地形态) */
OCB_API OcbStatus OCB_CALL ocb_shape_location_cleared(
    OcbShape shape, OcbShape* out);

/* Read the shape's own TopLoc_Location as a transform (the value
   TopoDS_Shape::Location().Transformation() would return). */
OCB_API OcbStatus OCB_CALL ocb_shape_get_location(
    OcbShape shape, OcbTrsf* out);

OCB_API OcbStatus OCB_CALL ocb_shape_volume(
    OcbShape shape, double* out);

OCB_API void OCB_CALL ocb_shape_release(OcbShape shape);

/* ------------------------------------------------------------------
 * OCAF 文档 / 标签（P1c-1 最小集）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_app_new(OcbApp* out);
OCB_API OcbStatus OCB_CALL ocb_app_new_document(OcbApp app, const char* format, OcbDoc* out);
OCB_API OcbStatus OCB_CALL ocb_app_close(OcbApp app, OcbDoc doc);
OCB_API void      OCB_CALL ocb_app_release(OcbApp app);

OCB_API OcbStatus OCB_CALL ocb_doc_new_command(OcbDoc doc);
OCB_API OcbStatus OCB_CALL ocb_doc_commit_command(OcbDoc doc);
OCB_API OcbStatus OCB_CALL ocb_doc_undo(OcbDoc doc, bool* ok);
OCB_API OcbStatus OCB_CALL ocb_doc_redo(OcbDoc doc, bool* ok);
OCB_API OcbStatus OCB_CALL ocb_doc_main_label(OcbDoc doc, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_doc_save(OcbApp app, OcbDoc doc, const char* path);
OCB_API OcbStatus OCB_CALL ocb_doc_open(OcbApp app, const char* path, OcbDoc* out);
OCB_API OcbStatus OCB_CALL ocb_doc_set_undo_limit(OcbDoc doc, int limit);
OCB_API OcbStatus OCB_CALL ocb_doc_get_undo_limit(OcbDoc doc, int* out);
OCB_API void      OCB_CALL ocb_doc_release(OcbDoc doc);

OCB_API OcbStatus OCB_CALL ocb_shapetool_get(OcbDoc doc, OcbShapeTool* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_shape(OcbShapeTool st, OcbShape shape, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_shape_ex(OcbShapeTool st, OcbShape shape, bool makeAssemblySimpleGraph, bool autoLabel, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_shape(OcbShapeTool st, OcbLabel label, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_shape(OcbShapeTool st, OcbLabel label, OcbShape shape);
OCB_API OcbStatus OCB_CALL ocb_shapetool_update(OcbShapeTool st);
OCB_API OcbStatus OCB_CALL ocb_shapetool_free_shape_count(OcbShapeTool st, int* out);
OCB_API void      OCB_CALL ocb_shapetool_release(OcbShapeTool st);

OCB_API OcbStatus OCB_CALL ocb_label_set_name(OcbLabel label, const char* name);
OCB_API OcbStatus OCB_CALL ocb_label_get_name(OcbLabel label, char* buf, int bufSize);
OCB_API void      OCB_CALL ocb_label_release(OcbLabel label);

/* ------------------------------------------------------------------
 * 基础几何（补充）
 * ------------------------------------------------------------------ */

OCB_API OcbVec  OCB_CALL ocb_vec_xyz(double x, double y, double z);
OCB_API double  OCB_CALL ocb_vec_modulus(OcbVec v);
OCB_API OcbPnt  OCB_CALL ocb_point_transformed(OcbPnt p, OcbTrsf t);
OCB_API OcbTrsf OCB_CALL ocb_trsf_rotation(OcbPnt origin, OcbDir axis, double angle_rad);
OCB_API OcbTrsf OCB_CALL ocb_trsf_set_values(
    double v11, double v12, double v13, double v14,
    double v21, double v22, double v23, double v24,
    double v31, double v32, double v33, double v34);
OCB_API OcbVec  OCB_CALL ocb_trsf_translation_part(OcbTrsf t);
OCB_API OcbTrsf OCB_CALL ocb_trsf_multiply(OcbTrsf a, OcbTrsf b);
/* 迁移期适配：原生 gp_Trsf* → OcbTrsf（值拷贝，不持有原生对象） */
OCB_API OcbStatus OCB_CALL ocb_trsf_from_native(const void* nativeTrsf, OcbTrsf* out);
OCB_API OcbTrsf OCB_CALL ocb_trsf_inverted(OcbTrsf t);

/* ------------------------------------------------------------------
 * 体素（圆柱/圆锥/球）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_make_cylinder(OcbAx2 axis, double r, double h, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_cone(OcbAx2 axis, double r1, double r2, double h, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_sphere(OcbPnt center, double r, OcbShape* out);

/* ------------------------------------------------------------------
 * 布尔操作
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_bool_fuse(OcbShape a, OcbShape b, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_bool_cut(OcbShape a, OcbShape b, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_bool_common(OcbShape a, OcbShape b, OcbShape* out);

OCB_API OcbStatus OCB_CALL ocb_bool_fuse_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_bool_cut_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_bool_common_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out);
/* same as common_ex, plus OBB control (thin/co-planar shallow embedding needs true) */
OCB_API OcbStatus OCB_CALL ocb_bool_common_ex2(OcbShape a, OcbShape b, double fuzzy, bool parallel, bool useObb, OcbShape* out);

/* ------------------------------------------------------------------
 * 几何分析（干涉检测）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_shape_distance(OcbShape a, OcbShape b, double* out);

/* state 取值: 0=TopAbs_IN, 1=TopAbs_OUT, 2=TopAbs_ON, 3=TopAbs_UNKNOWN */
OCB_API OcbStatus OCB_CALL ocb_classifier_new(OcbShape shape, OcbCls* out);
OCB_API OcbStatus OCB_CALL ocb_classifier_perform(OcbCls cls, OcbPnt pnt, double tol, int* state);
OCB_API void      OCB_CALL ocb_classifier_release(OcbCls cls);

/* ------------------------------------------------------------------
 * 特征操作（倒圆 / 倒角 / 扫掠）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_fillet_new(OcbShape base, OcbFillet* out);
OCB_API OcbStatus OCB_CALL ocb_fillet_add(OcbFillet fillet, OcbShape edge, double radius);
OCB_API OcbStatus OCB_CALL ocb_fillet_build(OcbFillet fillet, OcbShape* out);
OCB_API void      OCB_CALL ocb_fillet_release(OcbFillet fillet);

OCB_API OcbStatus OCB_CALL ocb_chamfer_new(OcbShape base, OcbChamfer* out);
OCB_API OcbStatus OCB_CALL ocb_chamfer_add(OcbChamfer chamfer, OcbShape edge, double dist);
OCB_API OcbStatus OCB_CALL ocb_chamfer_build(OcbChamfer chamfer, OcbShape* out);
OCB_API void      OCB_CALL ocb_chamfer_release(OcbChamfer chamfer);

OCB_API OcbStatus OCB_CALL ocb_make_pipe(OcbShape spine_wire, OcbShape profile_face, OcbShape* out);

/* ------------------------------------------------------------------
 * 四元数与变换
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_quaternion_from_euler_zyx(double rz, double ry, double rx, OcbQuat* out);
OCB_API OcbStatus OCB_CALL ocb_quaternion_dot(OcbQuat a, OcbQuat b, double* out);
OCB_API OcbStatus OCB_CALL ocb_trsf_from_quat_translation(OcbQuat q, double tx, double ty, double tz, OcbTrsf* out);
OCB_API OcbStatus OCB_CALL ocb_trsf_get_quat(OcbTrsf t, OcbQuat* out);
OCB_API OcbStatus OCB_CALL ocb_trsf_get_euler_zyx(OcbTrsf t, double* rz, double* ry, double* rx);

/* ------------------------------------------------------------------
 * STEP 全局参数（Interface_Static）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_interface_set_int(const char* key, int value);
OCB_API OcbStatus OCB_CALL ocb_interface_set_real(const char* key, double value);
/* 等价 Interface_Static::SetCVal(key, value)；key 不存在/只读返回 OCB_ERR_INVALID_ARG */
OCB_API OcbStatus OCB_CALL ocb_interface_set_string(const char* key, const char* value);

/* ------------------------------------------------------------------
 * 灯光
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_light_new_ambient(float r, float g, float b, float intensity, OcbLight* out);
OCB_API OcbStatus OCB_CALL ocb_light_new_directional(
    float r, float g, float b, float intensity, bool headlight,
    double dx, double dy, double dz, OcbLight* out);
OCB_API void      OCB_CALL ocb_light_release(OcbLight light);
OCB_API OcbStatus OCB_CALL ocb_point_aspect_new(int markerType, OcbColor color, double size, OcbPointAspect* out);
OCB_API void      OCB_CALL ocb_point_aspect_release(OcbPointAspect aspect);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_point_aspect(OcbDrawer drawer, OcbPointAspect aspect);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_line_aspect(OcbDrawer drawer, OcbLineAspect aspect);
OCB_API OcbStatus OCB_CALL ocb_viewer_add_light(OcbViewer viewer, OcbLight light);
OCB_API OcbStatus OCB_CALL ocb_viewer_lights_on(OcbViewer viewer);
OCB_API OcbStatus OCB_CALL ocb_viewer_lights_off(OcbViewer viewer);

/* ------------------------------------------------------------------
 * 拾取（拓扑实体级）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_shape(OcbAisContext ctx, OcbShape* out);

/* 包装外部已有的 native AIS_InteractiveContext（生命周期仍由外部管理，
 * 句柄释放不会销毁原对象） */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_wrap_native(void* nativeCtx, OcbAisContext* out);

/* 包装外部存在的 native TopoDS_Shape(浅拷贝接管值语义,原对象生命周期不受影响) */
OCB_API OcbStatus OCB_CALL ocb_shape_wrap_native(void* nativeShape, OcbShape* out);

/* 取底层 native 指针（只读访问，所有权仍在桥端句柄表；
 * 主程序把它转成 Viewer3D 内部 AIS 调用所需的 raw 指针） */
OCB_API void*      OCB_CALL ocb_ais_native_ptr(OcbAisObj obj);
OCB_API void*      OCB_CALL ocb_ais_ctx_native_ptr(OcbAisContext ctx);
OCB_API void*      OCB_CALL ocb_shape_native_ptr(OcbShape shape);


/* ------------------------------------------------------------------
 * 拓扑与形状工具
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_shape_is_null(OcbShape shape, bool* out);
OCB_API OcbStatus OCB_CALL ocb_shape_is_equal(OcbShape a, OcbShape b, bool* out);
OCB_API OcbStatus OCB_CALL ocb_shape_type(OcbShape shape, int* out);
OCB_API OcbStatus OCB_CALL ocb_shape_moved(OcbShape shape, OcbTrsf t, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_shape_copy(OcbShape shape, OcbShape* out);

OCB_API OcbStatus OCB_CALL ocb_topo_make_compound(OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_topo_add(OcbShape compound, OcbShape part);
OCB_API OcbStatus OCB_CALL ocb_topo_count_edges(OcbShape shape, int* out);
/* 统计 shape 中的 SOLID 数（compound/solid 通用；非实体形状返回 0） */
OCB_API OcbStatus OCB_CALL ocb_topo_count_solids(OcbShape shape, int* out);
/* unique face count (TopExp::MapShapes + TopAbs_FACE) */
OCB_API OcbStatus OCB_CALL ocb_topo_count_faces(OcbShape shape, int* out);
/* first sub-shape of a compound (TopoDS_Iterator, first child) */
OCB_API OcbStatus OCB_CALL ocb_shape_compound_first(OcbShape shape, OcbShape* out);
/* 取第 index 个 SOLID（0-based）；越界返回 OCB_ERR_INVALID_ARG */
OCB_API OcbStatus OCB_CALL ocb_topo_get_solid(OcbShape shape, int index, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_topo_get_child(OcbShape shape, int index, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_topo_edge_from_shape(OcbShape shape, OcbEdge* out);
OCB_API OcbStatus OCB_CALL ocb_topo_vertex_from_shape(OcbShape shape, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_topo_get_vertex(OcbShape shape, int index, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_vertex_pnt(OcbShape vertex, OcbPnt* out);

OCB_API OcbStatus OCB_CALL ocb_edge_first_param(OcbEdge edge, double* out);
OCB_API OcbStatus OCB_CALL ocb_edge_last_param(OcbEdge edge, double* out);
OCB_API OcbStatus OCB_CALL ocb_edge_curve(OcbEdge edge, double* first, double* last, OcbCurve* out);
OCB_API OcbStatus OCB_CALL ocb_curve_value(OcbCurve curve, double param, OcbPnt* out);
OCB_API void      OCB_CALL ocb_curve_release(OcbCurve curve);

OCB_API OcbStatus OCB_CALL ocb_make_edge_pnt_pnt(OcbPnt p1, OcbPnt p2, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_edge_from_curve(OcbCurve curve, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_arc_of_circle(OcbPnt p1, OcbPnt p2, OcbPnt p3, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_wire_from_edges(OcbEdge e1, OcbEdge e2, OcbEdge e3, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_face_from_wire(OcbShape wire, OcbShape* out);
/* 单边 wire（金丝/圆截面单段轮廓） */
OCB_API OcbStatus OCB_CALL ocb_make_wire_from_edge(OcbShape edge, OcbShape* out);
/* N edges wire (BRepBuilderAPI_MakeWire::Add loop; N >= 1) */
OCB_API OcbStatus OCB_CALL ocb_make_wire_from_shapes(const OcbShape* edges, int n, OcbShape* out);
/* 边上某参数处的点与单位切向（等价 BRepAdaptor_Curve::D1） */
OCB_API OcbStatus OCB_CALL ocb_edge_point_and_tangent(OcbShape edge, double param, OcbPnt* pnt, OcbDir* tangent);
/* 圆边（等价 gp_Circ + BRepBuilderAPI_MakeEdge） */
OCB_API OcbStatus OCB_CALL ocb_make_circle_edge(OcbPnt center, OcbDir normal, double radius, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_transform(OcbShape shape, OcbTrsf t, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_make_prism(OcbShape face, OcbVec vec, OcbShape* out);

OCB_API OcbStatus OCB_CALL ocb_explorer_new_faces(OcbShape shape, OcbExplorer* out);
OCB_API OcbStatus OCB_CALL ocb_explorer_new_edges(OcbShape shape, OcbExplorer* out);
OCB_API OcbStatus OCB_CALL ocb_explorer_new_wires(OcbShape shape, OcbExplorer* out);
OCB_API OcbStatus OCB_CALL ocb_explorer_more(OcbExplorer exp, bool* out);
OCB_API void      OCB_CALL ocb_explorer_next(OcbExplorer exp);
OCB_API OcbStatus OCB_CALL ocb_explorer_current_face(OcbExplorer exp, OcbFace* out);
OCB_API OcbStatus OCB_CALL ocb_explorer_current_edge(OcbExplorer exp, OcbEdge* out);
OCB_API OcbStatus OCB_CALL ocb_explorer_current_wire(OcbExplorer exp, OcbShape* out);
OCB_API void      OCB_CALL ocb_explorer_release(OcbExplorer exp);

/* 面朝向与平面参数（替代 face.Orientation() / BRepAdaptor_Surface）：
   orientation: 0=FORWARD 1=REVERSED 2=INTERNAL 3=EXTERNAL */
OCB_API OcbStatus OCB_CALL ocb_face_orientation(OcbFace face, int* out);
OCB_API OcbStatus OCB_CALL ocb_face_outer_wire(OcbFace face, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_face_is_plane(OcbFace face, bool* out);
OCB_API OcbStatus OCB_CALL ocb_face_plane_normal(OcbFace face, OcbDir* out);

/* ------------------------------------------------------------------
 * 网格（三角化）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_mesh_generate(OcbShape shape, double deflection);
/* 网格生成（完整参数）：deflection/angle(弧度)/minSize/relative/inParallel */
OCB_API OcbStatus OCB_CALL ocb_mesh_generate_ex(OcbShape shape, double deflection, double angle, double minSize, bool relative, bool inParallel);
OCB_API OcbStatus OCB_CALL ocb_face_triangulation(OcbFace face, OcbTri* out, OcbTrsf* loc);
OCB_API OcbStatus OCB_CALL ocb_tri_nb_triangles(OcbTri tri, int* out);
OCB_API OcbStatus OCB_CALL ocb_tri_get_triangle(OcbTri tri, int index, int* n1, int* n2, int* n3);
OCB_API OcbStatus OCB_CALL ocb_tri_node(OcbTri tri, int index, OcbPnt* pnt);
OCB_API void      OCB_CALL ocb_tri_release(OcbTri tri);
/* 任意 EntityOwner -> 形状（拾取回调用；接受 SelectMgr_EntityOwner 裸指针） */
OCB_API OcbStatus OCB_CALL ocb_owner_native_shape(void* owner, OcbShape* out);

/* ------------------------------------------------------------------
 * 属性（质量/体积/质心）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_gprops_volume(OcbShape shape, OcbGProps* out);
OCB_API OcbStatus OCB_CALL ocb_gprops_linear(OcbShape shape, OcbGProps* out);
OCB_API OcbStatus OCB_CALL ocb_gprops_surface(OcbShape shape, OcbGProps* out);
OCB_API OcbStatus OCB_CALL ocb_gprops_mass(OcbGProps props, double* out);
OCB_API OcbStatus OCB_CALL ocb_gprops_center_of_mass(OcbGProps props, OcbPnt* out);
OCB_API void      OCB_CALL ocb_gprops_release(OcbGProps props);

/* ------------------------------------------------------------------
 * STEP 文件 IO
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_step_read(const char* path, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_step_write(OcbShape shape, const char* path);

OCB_API OcbStatus OCB_CALL ocb_stepreader_create(OcbStepReader* out);
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_color_mode(OcbStepReader r, bool mode);
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_name_mode(OcbStepReader r, bool mode);
/* STEP 读取参数（等价 Interface_Static::SetIVal/SetRVal，全局生效） */
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_param_int(const char* name, int value);
OCB_API OcbStatus OCB_CALL ocb_stepreader_set_param_real(const char* name, double value);
OCB_API OcbStatus OCB_CALL ocb_stepreader_read_file(OcbStepReader r, const char* path);
OCB_API OcbStatus OCB_CALL ocb_stepreader_transfer(OcbStepReader r, OcbDoc doc);
OCB_API void      OCB_CALL ocb_stepreader_release(OcbStepReader r);

OCB_API OcbStatus OCB_CALL ocb_stepwriter_create(OcbStepWriter* out);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_color_mode(OcbStepWriter w, bool mode);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_name_mode(OcbStepWriter w, bool mode);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_layer_mode(OcbStepWriter w, bool mode);
/* STEPCAFControl_Writer::SetMaterialMode */
OCB_API OcbStatus OCB_CALL ocb_stepwriter_set_material_mode(OcbStepWriter w, bool mode);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_transfer(OcbStepWriter w, OcbDoc doc);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_transfer_label(OcbStepWriter w, OcbLabel label);
OCB_API OcbStatus OCB_CALL ocb_stepwriter_write(OcbStepWriter w, const char* path);
OCB_API void      OCB_CALL ocb_stepwriter_release(OcbStepWriter w);

/* ------------------------------------------------------------------
 * AIS 对象（补充）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_geom_new_cartesian_point(double x, double y, double z, OcbGeom* out);
OCB_API void      OCB_CALL ocb_geom_release(OcbGeom geom);

OCB_API OcbStatus OCB_CALL ocb_ais_new_point(OcbGeom geom, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_new_xcaf_object(OcbLabel label, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_set_color(OcbAisObj obj, OcbColor color);
OCB_API OcbStatus OCB_CALL ocb_ais_set_material(OcbAisObj obj, int material);
OCB_API OcbStatus OCB_CALL ocb_ais_set_display_mode(OcbAisObj obj, int mode);
OCB_API OcbStatus OCB_CALL ocb_ais_set_transparency(OcbAisObj obj, double value);
OCB_API OcbStatus OCB_CALL ocb_ais_set_marker(OcbAisObj obj, int type);
OCB_API OcbStatus OCB_CALL ocb_ais_set_zlayer(OcbAisObj obj, int layer);
OCB_API OcbStatus OCB_CALL ocb_ais_zlayer(OcbAisObj obj, int* out);
OCB_API OcbStatus OCB_CALL ocb_ais_has_color(OcbAisObj obj, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_get_color(OcbAisObj obj, OcbColor* out);
OCB_API OcbStatus OCB_CALL ocb_ais_transparency(OcbAisObj obj, double* out);
OCB_API OcbStatus OCB_CALL ocb_ais_material(OcbAisObj obj, int* out);
OCB_API OcbStatus OCB_CALL ocb_ais_attributes(OcbAisObj obj, OcbDrawer* out);

OCB_API OcbStatus OCB_CALL ocb_ais_downcast_shape(OcbAisObj obj, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_downcast_point(OcbAisObj obj, OcbAisObj* out);
/* AIS_Point 位置读写（替代 app 侧 Component() downcast 到 Geom_CartesianPoint 的用法） */
OCB_API OcbStatus OCB_CALL ocb_ais_point_get_position(OcbAisObj obj, OcbPnt* out);
OCB_API OcbStatus OCB_CALL ocb_ais_point_set_position(OcbAisObj obj, OcbPnt p);
OCB_API OcbStatus OCB_CALL ocb_ais_shape_shape(OcbAisObj obj, OcbShape* out);
OCB_API OcbStatus OCB_CALL ocb_ais_has_transformation(OcbAisObj obj, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_local_transformation(OcbAisObj obj, OcbTrsf* out);
/* 直接设置 AIS 对象局部变换，不触发交互上下文选择结构重建（较 ocb_ais_ctx_set_location 快得多） */
OCB_API OcbStatus OCB_CALL ocb_ais_set_local_transformation(OcbAisObj obj, OcbTrsf trsf);
OCB_API OcbStatus OCB_CALL ocb_ais_xcaf_get_label(OcbAisObj obj, OcbLabel* out);

OCB_API OcbStatus OCB_CALL ocb_line_aspect_new(OcbColor color, double width, OcbLineAspect* out);
OCB_API void      OCB_CALL ocb_line_aspect_release(OcbLineAspect la);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_face_boundary_draw(OcbDrawer drawer, bool enable);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_free_boundary_draw(OcbDrawer drawer, bool enable);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_face_boundary_aspect(OcbDrawer drawer, OcbLineAspect aspect);
OCB_API OcbStatus OCB_CALL ocb_drawer_set_polygon_offsets(OcbDrawer drawer, int mode, float factor, float units);

OCB_API OcbStatus OCB_CALL ocb_drawer_setup_own_shading_aspect(OcbDrawer drawer);

OCB_API void      OCB_CALL ocb_drawer_release(OcbDrawer drawer);

/* ------------------------------------------------------------------
 * AIS 上下文（交互）
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_viewer_create(void* hwnd, OcbViewer* out);
OCB_API OcbStatus OCB_CALL ocb_viewer_context(OcbViewer viewer, OcbAisContext* out);
OCB_API OcbStatus OCB_CALL ocb_viewer_active_view(OcbViewer viewer, OcbView* out);
OCB_API void      OCB_CALL ocb_viewer_release(OcbViewer viewer);

OCB_API OcbStatus OCB_CALL ocb_ais_ctx_display(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_display_ex(OcbAisContext ctx, OcbAisObj obj, int mode, int priority, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_erase(OcbAisContext ctx, OcbAisObj obj, bool update);
/* AIS_InteractiveContext::EraseAll(update) -- hide all, keep structures */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_erase_all(OcbAisContext ctx, bool update);
/* AIS_InteractiveContext::ClearDetected(update) */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_clear_detected(OcbAisContext ctx, bool update);
/* handle validity (equivalent to Handle(AIS_InteractiveContext)::IsNull() == false) */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_is_valid(OcbAisContext ctx, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_remove(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_update_viewer(OcbAisContext ctx);
/* AIS_InteractiveContext::Update(aisObj, updateViewer) */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_update(OcbAisContext ctx, OcbAisObj obj, bool updateViewer);

/* 迁移期适配：把已有原生 AIS_InteractiveObject* 包成 OcbAisObj（addRef，release 时仅减引用） */
OCB_API OcbStatus OCB_CALL ocb_ais_wrap_native(void* nativeObj, OcbAisObj* out);
/* 已显示对象枚举：B2 迁移 DisplayedObjects 用（顺序与 OCCT 一致） */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_count(OcbAisContext ctx, int* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_at(OcbAisContext ctx, int index, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_is_displayed(OcbAisContext ctx, OcbAisObj obj, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_clear_selected(OcbAisContext ctx, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_activate(OcbAisContext ctx, int mode);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_deactivate(OcbAisContext ctx);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_deactivate_mode(OcbAisContext ctx, int mode);
/* 等价 ctx->Activate(aisObj, mode) / ctx->Deactivate(aisObj, mode)；
   mode 为 AIS_SelectionMode（0=shape,1=vertex,2=edge,3=wire,4=face,...），直接透传。 */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selection_mode_active(
    OcbAisContext ctx, OcbAisObj obj, int mode, bool active);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_color(OcbAisContext ctx, OcbAisObj obj, OcbColor color, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_material(OcbAisContext ctx, OcbAisObj obj, int material, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_transparency(OcbAisContext ctx, OcbAisObj obj, double value, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_unset_transparency(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_display_priority(OcbAisContext ctx, OcbAisObj obj, int priority);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_remove_all(OcbAisContext ctx, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_move_to(OcbAisContext ctx, OcbView view, int x, int y, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_has_detected(OcbAisContext ctx, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_detected(OcbAisContext ctx, OcbAisObj* out);
/* 迁移期适配：直接取检测到的原生对象指针（不建句柄，适合每帧拾取） */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_detected_native(OcbAisContext ctx, void** out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_native(OcbAisContext ctx, void** out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_native(OcbAisContext ctx, void** out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_displayed_at_native(OcbAisContext ctx, int index, void** out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected_owner(OcbAisContext ctx, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_select_detected(OcbAisContext ctx, int mode);
/* 框选（矩形，屏幕像素）；status 返回 AIS_StatusOfPick 数值 */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_select_rectangle(OcbAisContext ctx, OcbView view,
    int x1, int y1, int x2, int y2, int scheme, int* outStatus);
/* 像素容差 / 显示模式 / 自动高亮 */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_pixel_tolerance(OcbAisContext ctx, float tolerance);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_display_mode(OcbAisContext ctx, int mode, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_automatic_hilight(OcbAisContext ctx, bool enabled);
/* 默认 drawer 的三角化偏差（type: 0=相对 1=绝对，对应 Aspect_TypeOfDeflection） */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_default_deflection(OcbAisContext ctx, int type,
    double maximalChordalDeviation, double deviationAngleRad);
/* 选择/高亮样式 */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selection_style(OcbAisContext ctx, OcbColor color,
    float transparency, int zlayer);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_highlight_color(OcbAisContext ctx, OcbColor color);
/* v1.3 高亮/选择样式（type 见 OcbHighlightType；method 0=COLOR 1=BOUNDBOX） */
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_highlight_style(
    OcbAisContext ctx, int type, int method, OcbColor color,
    int displayMode, float transparency);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selection_style_ex(
    OcbAisContext ctx, OcbColor color, int displayMode, float transparency, int zlayer);
/* v1.3 AIS_TextLabel */
OCB_API OcbStatus OCB_CALL ocb_ais_textlabel_new(const char* utf8, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_textlabel_set_text(OcbAisObj obj, const char* utf8);
OCB_API OcbStatus OCB_CALL ocb_ais_textlabel_set_color(OcbAisObj obj, OcbColor color);
OCB_API OcbStatus OCB_CALL ocb_ais_textlabel_set_transform_persistence(
    OcbAisObj obj, int mode, int corner, int x, int y);
/* v1.3 AIS_ConnectedInteractive（内部持有 AIS_Shape 引用） */
OCB_API OcbStatus OCB_CALL ocb_ais_new_connected(OcbShape shape, OcbAisObj* out);

/* --- v1.5.0 AIS_Trihedron（原点/局部三轴；part 取 Prs3d_DP_* 数值） --- */
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_new(OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_datum_display_mode(OcbAisObj obj, int mode); /* 0=WireFrame 1=Shaded */
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_draw_arrows(OcbAisObj obj, bool on);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_datum_part_color(OcbAisObj obj, int part, OcbColor color);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_label(OcbAisObj obj, int part, const char* utf8);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_text_color(OcbAisObj obj, OcbColor color);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_size(OcbAisObj obj, double size);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_infinite_state(OcbAisObj obj, bool on);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_line_style(OcbAisObj obj, int part, double width, int typeOfLine);
OCB_API OcbStatus OCB_CALL ocb_ais_trihedron_set_zoom_persistence(OcbAisObj obj);

/* --- v1.5.0 AIS_ViewCube（导航立方体；faceId 取 V3d_* 朝向数值） --- */
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_new(OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_colors(OcbAisObj obj, OcbColor box, OcbColor inner, OcbColor text);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_size(OcbAisObj obj, double size);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_axes_labels(OcbAisObj obj, const char* x, const char* y, const char* z);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_axes_radius(OcbAisObj obj, double radius);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_transparency(OcbAisObj obj, double transparency);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_decor(OcbAisObj obj, bool drawAxes, bool drawEdges,
    double boxEdgeGap, double axesPadding, double boxEdgeMinSize);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_font(OcbAisObj obj, const char* family, int height);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_box_side_label(OcbAisObj obj, int faceId, const char* utf8);
OCB_API OcbStatus OCB_CALL ocb_ais_viewcube_set_triedron_pers(OcbAisObj obj, int corner, int x, int y);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_init_selected(OcbAisContext ctx);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_more_selected(OcbAisContext ctx, bool* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_next_selected(OcbAisContext ctx);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_selected(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_add_or_remove_selected(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_selected(OcbAisContext ctx, OcbAisObj* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_picked_point(OcbAisContext ctx, int index, OcbPnt* out);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_set_location(OcbAisContext ctx, OcbAisObj obj, OcbTrsf trsf);
OCB_API OcbStatus OCB_CALL ocb_ais_ctx_redisplay(OcbAisContext ctx, OcbAisObj obj, bool update);
OCB_API void      OCB_CALL ocb_ais_ctx_release(OcbAisContext ctx);

/* ------------------------------------------------------------------
 * 视图
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_view_fit_all(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_view_fit_all_margin(OcbView view, double margin);
OCB_API OcbStatus OCB_CALL ocb_view_zfit_all(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_view_redraw(OcbView view);

OCB_API OcbStatus OCB_CALL ocb_view_redraw_immediate(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_view_invalidate(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_view_set_bg_gradient(OcbView view, OcbColor top, OcbColor bottom, int method);
OCB_API OcbStatus OCB_CALL ocb_view_convert_with_proj(OcbView view, int x, int y,
    double* xp, double* yp, double* zp, double* vx, double* vy, double* vz);

/* 视图宿主：创建 V3d_View 并绑定 Win32 窗口句柄（G1，Viewer3D 切桥的硬阻断项） */
OCB_API OcbStatus OCB_CALL ocb_view_create(OcbViewer viewer, void* hwnd, OcbView* out);
/* 迁移期适配：把已有原生 V3d_View* 包成 OcbView（或取回原生指针），
   便于逐调用点切换，不必一次性替换成员类型 */
OCB_API OcbStatus OCB_CALL ocb_view_wrap_native(void* nativeView, OcbView* out);
OCB_API void*      OCB_CALL ocb_view_native_ptr(OcbView view);
OCB_API void*      OCB_CALL ocb_viewer_native_ptr(OcbViewer viewer);
/* 窗口尺寸变化后同步（Qt paintEvent/resizeEvent 调用） */
OCB_API OcbStatus OCB_CALL ocb_view_must_be_resized(OcbView view);
/* 投影朝向：OCB_PROJ_* */
OCB_API OcbStatus OCB_CALL ocb_view_set_proj(OcbView view, int projType);
/* 直接按 V3d_TypeOfOrientation 数值设置投影（消费方传枚举值） */
OCB_API OcbStatus OCB_CALL ocb_view_set_proj_type(OcbView view, int v3dOrientation);
/* 截图写文件（扩展名决定格式）；width/height<=0 用当前视口尺寸 */
OCB_API OcbStatus OCB_CALL ocb_view_dump(OcbView view, const char* path, int width, int height);
/* 离屏抓图到 RGB32 缓冲区（每像素 B,G,R,X 顺序，适配 Qt Format_RGB32）；
   bufferSize 需 >= 4*width*height；实际尺寸经 outWidth/outHeight 返回 */
OCB_API OcbStatus OCB_CALL ocb_view_to_rgb32(OcbView view, int width, int height,
    unsigned char* buffer, int bufferSize, int* outWidth, int* outHeight);
/* 坐标三轴显示/隐藏 */
OCB_API OcbStatus OCB_CALL ocb_view_trihedron(OcbView view, bool show);
/* 释放视图句柄（由 ocb_view_create 创建） */
OCB_API void      OCB_CALL ocb_view_release(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_ais_selection_probe(OcbAisObj obj, int* outHasMode0,
    int* outMode0Empty, int* outActiveModeCount);
OCB_API OcbStatus OCB_CALL ocb_ais_pick_probe(OcbAisObj obj, int* outNbSensitive0,
    int* outZLayer, int* outDisplayMode, OcbBox* outBBox);
OCB_API OcbStatus OCB_CALL ocb_view_project_point(OcbView view, double x, double y, double z,
    int* outSx, int* outSy);

/* 交互：旋转 / 平移 / 滚轮缩放 */
OCB_API OcbStatus OCB_CALL ocb_view_start_rotation(OcbView view, int x, int y);
OCB_API OcbStatus OCB_CALL ocb_view_rotation(OcbView view, int x, int y);
OCB_API OcbStatus OCB_CALL ocb_view_pan(OcbView view, double dx, double dy);
OCB_API OcbStatus OCB_CALL ocb_view_start_zoom_at_point(OcbView view, int x, int y);
OCB_API OcbStatus OCB_CALL ocb_view_zoom_at_point(OcbView view, int x, int y, double dx, double dy);
/* 渲染参数（透明方式/渲染方式/阴影/抗锯齿/光追深度/色调映射/MSAA） */
OCB_API OcbStatus OCB_CALL ocb_view_set_rendering_params(OcbView view,
    int transparencyMethod, int method, bool shadow, bool antialiasing,
    int raytracingDepth, int toneMapping, int msaa);
/* 坐标三轴（可指定颜色/尺寸） */
OCB_API OcbStatus OCB_CALL ocb_view_trihedron_ex(OcbView view, bool show, OcbColor color, double scale);
/* v1.3 视图状态 */
OCB_API OcbStatus OCB_CALL ocb_view_reset(OcbView view);
OCB_API OcbStatus OCB_CALL ocb_view_set_shading_model(OcbView view, int model);
OCB_API OcbStatus OCB_CALL ocb_view_set_immediate_update(OcbView view, bool on);
/* v1.3 相机（OcbCamera 为深拷贝句柄；set 不接管入参所有权） */
OCB_API OcbStatus OCB_CALL ocb_view_get_camera(OcbView view, OcbCamera* out);
OCB_API OcbStatus OCB_CALL ocb_view_set_camera(OcbView view, OcbCamera cam);
OCB_API OcbStatus OCB_CALL ocb_camera_copy(OcbCamera src, OcbCamera* out);
OCB_API OcbStatus OCB_CALL ocb_camera_lerp(OcbCamera a, OcbCamera b, double t, OcbCamera* out);
OCB_API void      OCB_CALL ocb_camera_release(OcbCamera cam);

/* Viewer 级：Z 轴朝上的特权平面 / 矩形网格 */
/* 迁移期适配：原生 V3d_Viewer* -> OcbViewer（addRef） */
OCB_API OcbStatus OCB_CALL ocb_viewer_wrap_native(void* nativeViewer, OcbViewer* out);
OCB_API OcbStatus OCB_CALL ocb_viewer_set_z_up_plane(OcbViewer viewer);
OCB_API OcbStatus OCB_CALL ocb_viewer_deactivate_grid(OcbViewer viewer);
OCB_API OcbStatus OCB_CALL ocb_viewer_setup_rect_grid(OcbViewer viewer,
    double step, double graphicSize, OcbColor echoColor, bool activate);
OCB_API OcbStatus OCB_CALL ocb_viewer_set_computed_mode(
    OcbViewer viewer, bool computed, bool defaultComputed);
OCB_API OcbStatus OCB_CALL ocb_viewer_set_default_lights(OcbViewer viewer);
OCB_API OcbStatus OCB_CALL ocb_viewer_set_default_view_size(OcbViewer viewer, double size);
/* v1.3 网格（gridType 见 OcbGridType；drawMode 见 OcbGridDrawMode） */
OCB_API OcbStatus OCB_CALL ocb_viewer_set_grid_echo_marker(
    OcbViewer viewer, int style, OcbColor color, double scale);
OCB_API OcbStatus OCB_CALL ocb_viewer_set_rect_grid_values(
    OcbViewer viewer, double xOrigin, double yOrigin, double xStep, double yStep,
    double rotation);
OCB_API OcbStatus OCB_CALL ocb_viewer_activate_grid(
    OcbViewer viewer, int gridType, int drawMode);
OCB_API OcbStatus OCB_CALL ocb_view_convert_to_grid(
    OcbView view, int x, int y, double* gx, double* gy, double* gz);
OCB_API OcbStatus OCB_CALL ocb_viewer_show_grid_echo(
    OcbViewer viewer, OcbView view, double x, double y, double z);

/* ocb_view_set_proj 的投影朝向 */
enum OcbProjType {
    OCB_PROJ_ISO = 0,      /* 等轴测（左上-前） */
    OCB_PROJ_TOP = 1,      /* 俯视 +Z */
    OCB_PROJ_FRONT = 2,    /* 前视 -Y */
    OCB_PROJ_RIGHT = 3,    /* 右视 +X */
    OCB_PROJ_AXO_RIGHT = 4 /* 轴测（右-前-上） */
};

/* ------------------------------------------------------------------
 * OCAF 扩充
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_label_get_shape(OcbLabel label, OcbShape* out);
/* label 自身 location（等价 XCAFDoc_ShapeTool::GetLocation） */
OCB_API OcbStatus OCB_CALL ocb_label_get_location(OcbLabel label, OcbTrsf* out);
OCB_API OcbStatus OCB_CALL ocb_label_is_equal(OcbLabel a, OcbLabel b, bool* out);
OCB_API OcbStatus OCB_CALL ocb_label_is_null(OcbLabel label, bool* out);
/* 主动有效性查询:句柄登记存在且标签非空(对齐 IsWindow 的诊断便利性) */
OCB_API OcbStatus OCB_CALL ocb_label_is_valid(OcbLabel label, bool* out);
/* 迁移期适配：原生 TDF_Label* -> OcbLabel（值拷贝，不持有原生对象） */
OCB_API OcbStatus OCB_CALL ocb_label_wrap_native(const void* nativeLabel, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_label_tag(OcbLabel label, int* out);
OCB_API OcbStatus OCB_CALL ocb_label_entry(OcbLabel label, char* buf, int bufSize);
/* 沿 Father 链向上重写 TNaming_NamedShape 小改(唤醒 UpdateAssemblies) */
OCB_API OcbStatus OCB_CALL ocb_label_walk_fathers_touch(OcbLabel label);
OCB_API OcbStatus OCB_CALL ocb_label_child(OcbLabel parent, int tag, OcbLabel* out);
/* Parent-relative child with create=true (TDF_Label::FindChild(tag, child, true)) */
OCB_API OcbStatus OCB_CALL ocb_label_new_child(OcbLabel parent, int tag, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_label_father(OcbLabel label, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_label_set_real(OcbLabel label, double value);
OCB_API OcbStatus OCB_CALL ocb_label_get_real(OcbLabel label, double* out);

/* --- 子标签遍历（等价 TDF_ChildIterator，index 0 基） --- */
OCB_API OcbStatus OCB_CALL ocb_label_child_count(OcbLabel parent, int* out);
OCB_API OcbStatus OCB_CALL ocb_label_child_at(OcbLabel parent, int index, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_label_has_child(OcbLabel label, bool* out);

/* --- 整型属性（等价 TDataStd_Integer） --- */
OCB_API OcbStatus OCB_CALL ocb_label_set_int(OcbLabel label, int value);
OCB_API OcbStatus OCB_CALL ocb_label_get_int(OcbLabel label, int* out);

/* --- 属性枚举（等价 TDF_AttributeIterator）+ 类型分类（OcbAttrKind） --- */
OCB_API OcbStatus OCB_CALL ocb_label_attribute_count(OcbLabel label, int* out);
OCB_API OcbStatus OCB_CALL ocb_label_attribute_kind_at(OcbLabel label, int index, int* kindOut);
OCB_API OcbStatus OCB_CALL ocb_label_forget_attributes(OcbLabel label);   /* ForgetAllAttributes(false) */

OCB_API OcbStatus OCB_CALL ocb_doc_has_open_command(OcbDoc doc, bool* out);
OCB_API OcbStatus OCB_CALL ocb_doc_abort_command(OcbDoc doc);

OCB_API OcbStatus OCB_CALL ocb_shapetool_new_shape(OcbShapeTool st, OcbLabel* out);
/* 序列读取约定:get_free_shape/get_user/get_component/get_sub_shape 的 index 一律 0-based */
OCB_API OcbStatus OCB_CALL ocb_shapetool_remove_shape(OcbShapeTool st, OcbLabel label);
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_reference(OcbShapeTool st, OcbLabel label, bool* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_referred_shape(OcbShapeTool st, OcbLabel label, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_component(OcbShapeTool st, OcbLabel label, bool* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_is_assembly(OcbShapeTool st, OcbLabel label, bool* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_component_count(OcbShapeTool st, OcbLabel label, int* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_user_count(OcbShapeTool st, OcbLabel label, int* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_user(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_component(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_add_component(OcbShapeTool st, OcbLabel parent, OcbLabel child, OcbTrsf trsf, OcbLabel* out);
/* 重建装配标签子树的 TNaming compound（自底向上）：
   NewShape+AddComponent 建的装配其 TNaming 是空 compound，GetShape 返回空，
   导致 STEPCAFControl_Writer 按标签导出时写出空装配；导出前调用本函数修复。 */
OCB_API OcbStatus OCB_CALL ocb_shapetool_rebuild_assembly_shapes(OcbShapeTool st, OcbLabel label);
/* 装配标签写"空 compound"占位：STEP 导出时结构完整、几何只在零件层各存一份（避免每层重复几何导致文件膨胀） */
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_assembly_placeholder(OcbShapeTool st, OcbLabel label);
OCB_API OcbStatus OCB_CALL ocb_shapetool_remove_component(OcbShapeTool st, OcbLabel label);
OCB_API OcbStatus OCB_CALL ocb_shapetool_set_location(OcbShapeTool st, OcbLabel label, OcbTrsf trsf, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_location(OcbShapeTool st, OcbLabel label, OcbTrsf* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_free_shape(OcbShapeTool st, int index, OcbLabel* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_sub_shape_count(OcbShapeTool st, OcbLabel label, int* out);
OCB_API OcbStatus OCB_CALL ocb_shapetool_get_sub_shape(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out);

OCB_API OcbStatus OCB_CALL ocb_colortool_get(OcbDoc doc, OcbColorTool* out);
OCB_API OcbStatus OCB_CALL ocb_colortool_set_color(OcbColorTool tool, OcbLabel label, OcbColor color, int colorType);
OCB_API OcbStatus OCB_CALL ocb_colortool_set_shape_color(OcbColorTool tool, OcbShape shape, OcbColor color, int colorType);
OCB_API OcbStatus OCB_CALL ocb_colortool_get_color(OcbColorTool tool, OcbLabel label, int colorType, bool* okOut, OcbColor* out);
OCB_API void      OCB_CALL ocb_colortool_release(OcbColorTool tool);
OCB_API OcbStatus OCB_CALL ocb_xcaf_location_set(OcbLabel label, OcbTrsf trsf);

/* ------------------------------------------------------------------
 * AIS 对象
 * ------------------------------------------------------------------ */

OCB_API OcbStatus OCB_CALL ocb_ais_new_shape(OcbShape shape, OcbAisObj* out);
OCB_API void      OCB_CALL ocb_ais_release(OcbAisObj obj);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OCC_C_BRIDGE_H */

/* 诊断：对象的选择状态——有选择结构但实体为空(IsEmpty)即"显示但拾取不到" */
/* 诊断 v3：拾取相关的全部属性——敏感实体数 / ZLayer / 显示模式 / 包围盒 */
/* 世界坐标 → 屏幕像素（V3d_View::Project） */
