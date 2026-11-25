// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-6;
} // namespace

TEST_CASE("topology: box has 12 edges") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10, 20, 30, &box) == OCB_OK);
    int edges = 0;
    REQUIRE(ocb_topo_count_edges(box, &edges) == OCB_OK);
    CHECK(edges == 12);
    ocb_shape_release(box);
}

TEST_CASE("topology: compound adds children") {
    OcbShape compound = OCB_INVALID;
    REQUIRE(ocb_topo_make_compound(&compound) == OCB_OK);

    OcbShape a = OCB_INVALID;
    OcbShape b = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1, 1, 1, &a) == OCB_OK);
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(5, 0, 0), 1, 1, 1, &b) == OCB_OK);

    REQUIRE(ocb_topo_add(compound, a) == OCB_OK);
    REQUIRE(ocb_topo_add(compound, b) == OCB_OK);

    int edges = 0;
    REQUIRE(ocb_topo_count_edges(compound, &edges) == OCB_OK);
    CHECK(edges == 24);

    OcbShape child0 = OCB_INVALID;
    REQUIRE(ocb_topo_get_child(compound, 0, &child0) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(child0, &v) == OCB_OK);
    CHECK(std::fabs(v - 1.0) < 1e-4);

    ocb_shape_release(child0);
    ocb_shape_release(b);
    ocb_shape_release(a);
    ocb_shape_release(compound);
}

TEST_CASE("topology: count and get solids of a compound") {
    OcbShape compound = OCB_INVALID;
    REQUIRE(ocb_topo_make_compound(&compound) == OCB_OK);

    OcbShape a = OCB_INVALID;
    OcbShape b = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 2, 2, 2, &a) == OCB_OK);
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(5, 0, 0), 3, 2, 2, &b) == OCB_OK);
    REQUIRE(ocb_topo_add(compound, a) == OCB_OK);
    REQUIRE(ocb_topo_add(compound, b) == OCB_OK);

    int solids = 0;
    REQUIRE(ocb_topo_count_solids(compound, &solids) == OCB_OK);
    CHECK(solids == 2);

    OcbShape s0 = OCB_INVALID;
    OcbShape s1 = OCB_INVALID;
    REQUIRE(ocb_topo_get_solid(compound, 0, &s0) == OCB_OK);
    REQUIRE(ocb_topo_get_solid(compound, 1, &s1) == OCB_OK);
    double v0 = 0.0, v1 = 0.0;
    REQUIRE(ocb_shape_volume(s0, &v0) == OCB_OK);
    REQUIRE(ocb_shape_volume(s1, &v1) == OCB_OK);
    CHECK(std::fabs(v0 - 8.0) < 1e-4);
    CHECK(std::fabs(v1 - 12.0) < 1e-4);

    OcbShape sBad = OCB_INVALID;
    CHECK(ocb_topo_get_solid(compound, 2, &sBad) != OCB_OK);

    // 单实体 box：1 个 solid
    int one = 0;
    REQUIRE(ocb_topo_count_solids(a, &one) == OCB_OK);
    CHECK(one == 1);

    ocb_shape_release(s0);
    ocb_shape_release(s1);
    ocb_shape_release(a);
    ocb_shape_release(b);
    ocb_shape_release(compound);
}

TEST_CASE("topology: shape type distinguishes solid from edge") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1, 1, 1, &box) == OCB_OK);

    int boxType = -1;
    REQUIRE(ocb_shape_type(box, &boxType) == OCB_OK);
    CHECK(boxType == 2); // TopAbs_SOLID

    OcbShape edge = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 0), ocb_point_xyz(1, 0, 0), &edge) == OCB_OK);
    int edgeType = -1;
    REQUIRE(ocb_shape_type(edge, &edgeType) == OCB_OK);
    CHECK(edgeType == 6); // TopAbs_EDGE

    ocb_shape_release(edge);
    ocb_shape_release(box);
}

TEST_CASE("topology: moved shape changes bbox") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10, 10, 10, &box) == OCB_OK);

    const OcbTrsf move = ocb_trsf_translation(ocb_vec_xyz(100, 0, 0));
    OcbShape moved = OCB_INVALID;
    REQUIRE(ocb_shape_moved(box, move, &moved) == OCB_OK);

    OcbBox bounds{};
    REQUIRE(ocb_shape_bounding_box(moved, &bounds) == OCB_OK);
    CHECK(std::fabs(bounds.xmin - 100.0) < 1e-6);
    CHECK(std::fabs(bounds.xmax - 110.0) < 1e-6);

    ocb_shape_release(moved);
    ocb_shape_release(box);
}

TEST_CASE("topology: copy is a new shape with same geometry") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10, 10, 10, &box) == OCB_OK);

    OcbShape copy = OCB_INVALID;
    REQUIRE(ocb_shape_copy(box, &copy) == OCB_OK);
    CHECK(copy != box); // 不同句柄

    bool equal = false;
    REQUIRE(ocb_shape_is_equal(box, copy, &equal) == OCB_OK);
    CHECK_FALSE(equal);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(copy, &v) == OCB_OK);
    CHECK(std::fabs(v - 1000.0) < 1e-4);

    ocb_shape_release(copy);
    ocb_shape_release(box);
}

TEST_CASE("modeling: edge from points, wire, face, prism") {
    // 底面 10x10 的正方形轮廓
    OcbShape e1 = OCB_INVALID;
    OcbShape e2 = OCB_INVALID;
    OcbShape e3 = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 0), ocb_point_xyz(10, 0, 0), &e1) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(10, 0, 0), ocb_point_xyz(10, 10, 0), &e2) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(10, 10, 0), ocb_point_xyz(0, 0, 0), &e3) == OCB_OK);

    OcbShape wire = OCB_INVALID;
    REQUIRE(ocb_make_wire_from_edges(e1, e2, e3, &wire) == OCB_OK);

    OcbShape face = OCB_INVALID;
    REQUIRE(ocb_make_face_from_wire(wire, &face) == OCB_OK);

    OcbShape prism = OCB_INVALID;
    REQUIRE(ocb_make_prism(face, OcbVec{0, 0, 5}, &prism) == OCB_OK);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(prism, &v) == OCB_OK);
    CHECK(std::fabs(v - 250.0) < 1e-4); // 50mm2 * 5mm / 2 (三角形)

    ocb_shape_release(prism);
    ocb_shape_release(face);
    ocb_shape_release(wire);
    ocb_shape_release(e3);
    ocb_shape_release(e2);
    ocb_shape_release(e1);
}

TEST_CASE("modeling: edge curve parameter endpoints") {
    OcbShape edge = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(1, 2, 3), ocb_point_xyz(4, 6, 3), &edge) == OCB_OK);

    double first = 0.0;
    double last = 0.0;
    OcbCurve curve = OCB_INVALID;
    REQUIRE(ocb_edge_curve(edge, &first, &last, &curve) == OCB_OK);

    OcbPnt start = ocb_point_xyz(0, 0, 0);
    REQUIRE(ocb_curve_value(curve, first, &start) == OCB_OK);
    CHECK(std::fabs(start.x - 1.0) < 1e-6);
    CHECK(std::fabs(start.y - 2.0) < 1e-6);

    ocb_curve_release(curve);
    ocb_shape_release(edge);
}

TEST_CASE("explorer: count faces and edges of a box") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10, 10, 10, &box) == OCB_OK);

    OcbExplorer faces = OCB_INVALID;
    REQUIRE(ocb_explorer_new_faces(box, &faces) == OCB_OK);
    int faceCount = 0;
    bool more = true;
    while (more) {
        REQUIRE(ocb_explorer_more(faces, &more) == OCB_OK);
        if (!more) break;
        OcbFace f = OCB_INVALID;
        REQUIRE(ocb_explorer_current_face(faces, &f) == OCB_OK);
        ocb_shape_release(f);
        ocb_explorer_next(faces);
        ++faceCount;
    }
    ocb_explorer_release(faces);
    CHECK(faceCount == 6);

    OcbExplorer edges = OCB_INVALID;
    REQUIRE(ocb_explorer_new_edges(box, &edges) == OCB_OK);
    int edgeCount = 0;
    bool moreE = true;
    while (moreE) {
        REQUIRE(ocb_explorer_more(edges, &moreE) == OCB_OK);
        if (!moreE) break;
        OcbEdge e = OCB_INVALID;
        REQUIRE(ocb_explorer_current_edge(edges, &e) == OCB_OK);
        ocb_shape_release(e);
        ocb_explorer_next(edges);
        ++edgeCount;
    }
    ocb_explorer_release(edges);
    CHECK(edgeCount == 24);

    ocb_shape_release(box);
}

TEST_CASE("explorer: invalid explorer handles return error, no crash") {
    bool more = true;
    OcbFace face = OCB_INVALID;
    OcbEdge edge = OCB_INVALID;
    OcbShape wire = OCB_INVALID;

    // invalid (zero) handle
    CHECK(ocb_explorer_more(OCB_INVALID, &more) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_explorer_current_face(OCB_INVALID, &face) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_explorer_current_edge(OCB_INVALID, &edge) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_explorer_current_wire(OCB_INVALID, &wire) == OCB_ERR_INVALID_HANDLE);
    ocb_explorer_next(OCB_INVALID);   // no-op, must not crash

    // null out pointer
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1, 1, 1, &box) == OCB_OK);
    OcbExplorer exp = OCB_INVALID;
    REQUIRE(ocb_explorer_new_faces(box, &exp) == OCB_OK);
    CHECK(ocb_explorer_more(exp, nullptr) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_explorer_current_face(exp, nullptr) == OCB_ERR_INVALID_HANDLE);

    // released handle
    ocb_explorer_release(exp);
    CHECK(ocb_explorer_more(exp, &more) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_explorer_current_edge(exp, &edge) == OCB_ERR_INVALID_HANDLE);
    ocb_explorer_next(exp);   // no-op, must not crash

    ocb_shape_release(box);
}
