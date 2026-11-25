// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>
#include <cstdio>

namespace {
constexpr double kTol = 1e-3;
constexpr double kPi = 3.14159265358979323846;

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}

OcbShape firstEdgeOf(OcbShape box) {
    OcbExplorer exp = OCB_INVALID;
    REQUIRE(ocb_explorer_new_edges(box, &exp) == OCB_OK);
    bool more = false;
    REQUIRE(ocb_explorer_more(exp, &more) == OCB_OK);
    REQUIRE(more);
    OcbShape edge = OCB_INVALID;
    REQUIRE(ocb_explorer_current_edge(exp, &edge) == OCB_OK);
    ocb_explorer_release(exp);
    return edge;
}
} // namespace

TEST_CASE("feature: chamfer one edge removes 0.5*L*d*d") {
    OcbShape box = makeBox(10, 10, 10);
    OcbShape edge = firstEdgeOf(box);

    OcbChamfer ch = OCB_INVALID;
    REQUIRE(ocb_chamfer_new(box, &ch) == OCB_OK);
    REQUIRE(ocb_chamfer_add(ch, edge, 1.0) == OCB_OK);

    OcbShape result = OCB_INVALID;
    REQUIRE(ocb_chamfer_build(ch, &result) == OCB_OK);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(result, &v) == OCB_OK);
    CHECK(std::fabs(v - 995.0) < kTol);

    ocb_shape_release(result);
    ocb_chamfer_release(ch);
    ocb_shape_release(edge);
    ocb_shape_release(box);
}

TEST_CASE("feature: fillet one edge removes (1-pi/4)*L*r*r") {
    OcbShape box = makeBox(10, 10, 10);
    OcbShape edge = firstEdgeOf(box);

    OcbFillet fl = OCB_INVALID;
    REQUIRE(ocb_fillet_new(box, &fl) == OCB_OK);
    REQUIRE(ocb_fillet_add(fl, edge, 1.0) == OCB_OK);

    OcbShape result = OCB_INVALID;
    REQUIRE(ocb_fillet_build(fl, &result) == OCB_OK);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(result, &v) == OCB_OK);
    const double expected = 1000.0 - (1.0 - kPi / 4.0) * 10.0;
    CHECK(std::fabs(v - expected) < kTol);

    ocb_shape_release(result);
    ocb_fillet_release(fl);
    ocb_shape_release(edge);
    ocb_shape_release(box);
}

TEST_CASE("feature: pipe sweeps circle along line") {
    // 三段 120 度圆弧闭合构成 XY 平面圆盘（z=0，法向沿 Z，垂直于 spine）
    const double u = 0.86602540378;
    OcbShape e1 = OCB_INVALID;
    OcbShape e2 = OCB_INVALID;
    OcbShape e3 = OCB_INVALID;
    REQUIRE(ocb_make_arc_of_circle(
                ocb_point_xyz(1, 0, 0),
                ocb_point_xyz(0.5, u, 0),
                ocb_point_xyz(-0.5, u, 0), &e1) == OCB_OK);
    REQUIRE(ocb_make_arc_of_circle(
                ocb_point_xyz(-0.5, u, 0),
                ocb_point_xyz(-1, 0, 0),
                ocb_point_xyz(-0.5, -u, 0), &e2) == OCB_OK);
    REQUIRE(ocb_make_arc_of_circle(
                ocb_point_xyz(-0.5, -u, 0),
                ocb_point_xyz(0.5, -u, 0),
                ocb_point_xyz(1, 0, 0), &e3) == OCB_OK);

    OcbShape wire = OCB_INVALID;
    REQUIRE(ocb_make_wire_from_edges(e1, e2, e3, &wire) == OCB_OK);

    OcbShape profile = OCB_INVALID;
    REQUIRE(ocb_make_face_from_wire(wire, &profile) == OCB_OK);

    // spine:3 段首尾相连的直线 wire(总长 10,方向沿 Z)
    OcbShape s1 = OCB_INVALID;
    OcbShape s2 = OCB_INVALID;
    OcbShape s3 = OCB_INVALID;
    OcbShape spine = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 0), ocb_point_xyz(0, 0, 4), &s1) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 4), ocb_point_xyz(0, 0, 8), &s2) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 8), ocb_point_xyz(0, 0, 10), &s3) == OCB_OK);
    REQUIRE(ocb_make_wire_from_edges(s1, s2, s3, &spine) == OCB_OK);

    OcbShape swept = OCB_INVALID;
    const OcbStatus pipeStatus = ocb_make_pipe(spine, profile, &swept);
    if (pipeStatus != OCB_OK) {
        std::printf("pipe failed: status=%d err=%s\n", (int)pipeStatus, ocb_last_error());
    }
    REQUIRE(pipeStatus == OCB_OK);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(swept, &v) == OCB_OK);
    CHECK(std::fabs(v - kPi * 10.0) < 0.01);

    ocb_shape_release(swept);
    ocb_shape_release(spine);
    ocb_shape_release(s3);
    ocb_shape_release(s2);
    ocb_shape_release(s1);
    ocb_shape_release(profile);
    ocb_shape_release(wire);
    ocb_shape_release(e3);
    ocb_shape_release(e2);
    ocb_shape_release(e1);
}

TEST_CASE("feature: fillet/chamfer/pipe invalid handles rejected") {
    OcbFillet fl = OCB_INVALID;
    OcbChamfer ch = OCB_INVALID;
    CHECK(ocb_fillet_new(OCB_INVALID, &fl) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_fillet_add(OCB_INVALID, OCB_INVALID, 1.0) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_fillet_build(OCB_INVALID, &fl) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_chamfer_new(OCB_INVALID, &ch) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_chamfer_add(OCB_INVALID, OCB_INVALID, 1.0) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_chamfer_build(OCB_INVALID, &ch) == OCB_ERR_INVALID_HANDLE);

    OcbShape out = OCB_INVALID;
    CHECK(ocb_make_pipe(OCB_INVALID, OCB_INVALID, &out) == OCB_ERR_INVALID_HANDLE);
}
