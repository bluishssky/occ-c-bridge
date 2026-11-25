// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>
#include <vector>
#include <string>

// 纯计算/几何查询类的补测：不依赖窗口、不依赖上下文，可在无显示环境稳定运行。
// 目的：把「完全没被单测引用」的 API 里、无 GL 依赖的部分补齐（见 docs/api-test-coverage.md）。

TEST_CASE("api: shape 查询与包装") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(OcbPnt{ 0, 0, 0 }, 2.0, 3.0, 4.0, &box) == OCB_OK);

    bool isNull = true;
    CHECK(ocb_shape_is_null(box, &isNull) == OCB_OK);
    CHECK(isNull == false);
    CHECK(ocb_shape_is_null(OCB_INVALID, &isNull) == OCB_ERR_INVALID_HANDLE);

    OcbBox bb{};
    CHECK(ocb_shape_bounding_box_ex(box, false, &bb) == OCB_OK);
    CHECK(bb.valid);
    CHECK(bb.xmax == doctest::Approx(2.0));

    // 原生包装 + 清 location
    void* native = ocb_shape_native_ptr(box);
    REQUIRE(native != nullptr);
    OcbShape wrapped = OCB_INVALID;
    CHECK(ocb_shape_wrap_native(native, &wrapped) == OCB_OK);
    CHECK(wrapped != OCB_INVALID);
    OcbShape cleared = OCB_INVALID;
    CHECK(ocb_shape_location_cleared(wrapped, &cleared) == OCB_OK);
    CHECK(cleared != OCB_INVALID);

    ocb_shape_release(cleared);
    ocb_shape_release(wrapped);
    ocb_shape_release(box);
}

TEST_CASE("api: 拓扑遍历（边/顶点）") {
    OcbShape e1 = OCB_INVALID, e2 = OCB_INVALID, e3 = OCB_INVALID, wire = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(OcbPnt{ 0, 0, 0 }, OcbPnt{ 10, 0, 0 }, &e1) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(OcbPnt{ 10, 0, 0 }, OcbPnt{ 10, 10, 0 }, &e2) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(OcbPnt{ 10, 10, 0 }, OcbPnt{ 0, 10, 0 }, &e3) == OCB_OK);
    REQUIRE(ocb_make_wire_from_edges(e1, e2, e3, &wire) == OCB_OK);

    OcbEdge edge = OCB_INVALID;
    CHECK(ocb_topo_edge_from_shape(e1, &edge) == OCB_OK);
    CHECK(edge != OCB_INVALID);

    double first = -1, last = -1;
    CHECK(ocb_edge_first_param(edge, &first) == OCB_OK);
    CHECK(ocb_edge_last_param(edge, &last) == OCB_OK);
    CHECK(last > first);

    OcbShape v0 = OCB_INVALID;
    REQUIRE(ocb_topo_get_vertex(e1, 0, &v0) == OCB_OK);
    CHECK(v0 != OCB_INVALID);
    // 顶点形状 → 再取顶点（应成功）
    OcbShape vertex = OCB_INVALID;
    CHECK(ocb_topo_vertex_from_shape(v0, &vertex) == OCB_OK);
    if (vertex != OCB_INVALID) ocb_shape_release(vertex);
    ocb_shape_release(v0);

    ocb_shape_release((OcbShape)edge);

    ocb_shape_release(wire); ocb_shape_release(e1); ocb_shape_release(e2); ocb_shape_release(e3);
}

TEST_CASE("api: 变换与构造（make_transform / make_edge_from_curve / dir）") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(OcbPnt{ 0, 0, 0 }, 1.0, 1.0, 1.0, &box) == OCB_OK);

    OcbTrsf t = ocb_trsf_translation(OcbVec{ 5.0, 0.0, 0.0 });
    OcbShape moved = OCB_INVALID;
    CHECK(ocb_make_transform(box, t, &moved) == OCB_OK);
    CHECK(moved != OCB_INVALID);
    OcbBox bb{};
    CHECK(ocb_shape_bounding_box(moved, &bb) == OCB_OK);
    CHECK(bb.valid);
    CHECK(bb.xmin == doctest::Approx(5.0));

    OcbDir d = ocb_dir_xyz(0.0, 1.0, 0.0);
    CHECK(d.dy == doctest::Approx(1.0));

    // 曲线 → 边：从已有边的曲线句柄构造
    OcbShape e = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(OcbPnt{ 0, 0, 0 }, OcbPnt{ 1, 1, 1 }, &e) == OCB_OK);
    OcbCurve curve = OCB_INVALID;
    double f = 0, l = 0;
    CHECK(ocb_edge_curve(e, &f, &l, &curve) == OCB_OK);
    if (curve != OCB_INVALID) {
        OcbShape e2 = OCB_INVALID;
        CHECK(ocb_make_edge_from_curve(curve, &e2) == OCB_OK);
        if (e2 != OCB_INVALID) ocb_shape_release(e2);
        ocb_curve_release(curve);
    }

    if (moved != OCB_INVALID) ocb_shape_release(moved);
    ocb_shape_release(e);
    ocb_shape_release(box);
}

TEST_CASE("api: 曲线离散 curve_polyline") {
    OcbShape e = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(OcbPnt{ 0, 0, 0 }, OcbPnt{ 10, 0, 0 }, &e) == OCB_OK);
    OcbCurve curve = OCB_INVALID;
    double f = 0, l = 0;
    REQUIRE(ocb_edge_curve(e, &f, &l, &curve) == OCB_OK);

    std::vector<OcbPnt> buf(256);
    int n = 0;
    CHECK(ocb_curve_polyline(curve, f, l, 0.01, buf.data(), (int)buf.size(), &n) == OCB_OK);
    CHECK(n >= 2);
    CHECK(buf[0].x == doctest::Approx(0.0).epsilon(1e-6));
    CHECK(buf[n - 1].x == doctest::Approx(10.0).epsilon(1e-6));

    ocb_curve_release(curve);
    ocb_shape_release(e);
}

TEST_CASE("api: 样式对象（line/point aspect、drawer）") {
    OcbLineAspect la = OCB_INVALID;
    CHECK(ocb_line_aspect_new(OcbColor{ 1.0f, 0.0f, 0.0f }, 1.5, &la) == OCB_OK);
    CHECK(la != OCB_INVALID);
    ocb_line_aspect_release(la);

    OcbPointAspect pa = OCB_INVALID;
    CHECK(ocb_point_aspect_new(3, OcbColor{ 0.0f, 1.0f, 0.0f }, 2.0, &pa) == OCB_OK);
    CHECK(pa != OCB_INVALID);
    ocb_point_aspect_release(pa);

    // drawer 需要 AIS 对象提供
    OcbShape box = OCB_INVALID;
    OcbPnt corner{ 0, 0, 0 };
    REQUIRE(ocb_shape_make_box(corner, 1.0, 1.0, 1.0, &box) == OCB_OK);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);

    OcbDrawer dr = OCB_INVALID;
    CHECK(ocb_ais_attributes(obj, &dr) == OCB_OK);
    if (dr != OCB_INVALID) {
        OcbLineAspect l2 = OCB_INVALID;
        ocb_line_aspect_new(OcbColor{ 0.0f, 0.0f, 1.0f }, 1.0, &l2);
        CHECK(ocb_drawer_set_line_aspect(dr, l2) == OCB_OK);
        OcbPointAspect p2 = OCB_INVALID;
        ocb_point_aspect_new(3, OcbColor{ 1.0f, 1.0f, 0.0f }, 1.0, &p2);
        CHECK(ocb_drawer_set_point_aspect(dr, p2) == OCB_OK);
        CHECK(ocb_drawer_set_face_boundary_draw(dr, true) == OCB_OK);
        CHECK(ocb_drawer_set_free_boundary_draw(dr, true) == OCB_OK);
        CHECK(ocb_drawer_set_face_boundary_aspect(dr, l2) == OCB_OK);
        ocb_point_aspect_release(p2);
        ocb_line_aspect_release(l2);
        ocb_drawer_release(dr);
    }

    ocb_ais_release(obj);
    ocb_shape_release(box);
}

TEST_CASE("api: version 与无效句柄防御") {
    const char* v = ocb_version();
    CHECK(v != nullptr);
    CHECK(std::string(v).size() > 0);

    bool n = false;
    CHECK(ocb_label_is_null(OCB_INVALID, &n) == OCB_ERR_INVALID_HANDLE);
    OcbLabel child = OCB_INVALID;
    CHECK(ocb_label_child(OCB_INVALID, 1, &child) == OCB_ERR_INVALID_HANDLE);
    OcbShape s = OCB_INVALID;
    CHECK(ocb_topo_edge_from_shape(OCB_INVALID, (OcbEdge*)&s) == OCB_ERR_INVALID_HANDLE);
}
