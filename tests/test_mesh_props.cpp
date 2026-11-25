// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-6;

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}
} // namespace

TEST_CASE("mesh: triangulate box, nodes on bbox") {
    OcbShape box = makeBox(10.0, 10.0, 10.0);
    REQUIRE(ocb_mesh_generate(box, 0.5) == OCB_OK);

    // 取第一个面
    OcbExplorer faces = OCB_INVALID;
    REQUIRE(ocb_explorer_new_faces(box, &faces) == OCB_OK);
    bool more = false;
    REQUIRE(ocb_explorer_more(faces, &more) == OCB_OK);
    REQUIRE(more);
    OcbFace face = OCB_INVALID;
    REQUIRE(ocb_explorer_current_face(faces, &face) == OCB_OK);
    ocb_explorer_release(faces);

    OcbTri tri = OCB_INVALID;
    OcbTrsf loc{};
    REQUIRE(ocb_face_triangulation(face, &tri, &loc) == OCB_OK);

    int n = 0;
    REQUIRE(ocb_tri_nb_triangles(tri, &n) == OCB_OK);
    CHECK(n > 0);

    OcbPnt node = ocb_point_xyz(0, 0, 0);
    const OcbStatus nStatus = ocb_tri_node(tri, 1, &node);
    REQUIRE(nStatus == OCB_OK);
    // 节点经 loc 变换后应落在包围盒内（loc 是面的定位）
    CHECK(node.x >= -1e-3);
    CHECK(node.x <= 10.0 + 1e-3);

    ocb_tri_release(tri);
    ocb_shape_release(face);
    ocb_shape_release(box);
}

TEST_CASE("props: volume center of mass of a box") {
    OcbShape box = makeBox(10.0, 20.0, 30.0);

    OcbGProps props = OCB_INVALID;
    const OcbStatus vStatus = ocb_gprops_volume(box, &props);
    REQUIRE(vStatus == OCB_OK);

    double mass = 0.0;
    const OcbStatus mStatus = ocb_gprops_mass(props, &mass);
    REQUIRE(mStatus == OCB_OK);
    CHECK(std::fabs(mass - 6000.0) < 1e-4);

    OcbPnt com = ocb_point_xyz(0, 0, 0);
    const OcbStatus cStatus = ocb_gprops_center_of_mass(props, &com);
    REQUIRE(cStatus == OCB_OK);
    CHECK(std::fabs(com.x - 5.0) < kTol);
    CHECK(std::fabs(com.y - 10.0) < kTol);
    CHECK(std::fabs(com.z - 15.0) < kTol);

    ocb_gprops_release(props);
    ocb_shape_release(box);
}

TEST_CASE("props: surface area of a box") {
    OcbShape box = makeBox(10.0, 20.0, 30.0);

    OcbGProps props = OCB_INVALID;
    REQUIRE(ocb_gprops_surface(box, &props) == OCB_OK);

    double area = 0.0;
    const OcbStatus sStatus = ocb_gprops_mass(props, &area);
    REQUIRE(sStatus == OCB_OK);
    const double expected = 2.0 * (10 * 20 + 20 * 30 + 10 * 30);
    CHECK(std::fabs(area - expected) < 1e-4);

    ocb_gprops_release(props);
    ocb_shape_release(box);
}

TEST_CASE("props: linear properties of an edge") {
    OcbShape edge = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 0), ocb_point_xyz(3, 4, 0), &edge) == OCB_OK);

    OcbGProps props = OCB_INVALID;
    const OcbStatus lStatus = ocb_gprops_linear(edge, &props);
    REQUIRE(lStatus == OCB_OK);

    double length = 0.0;
    const OcbStatus lenStatus = ocb_gprops_mass(props, &length);
    REQUIRE(lenStatus == OCB_OK);
    CHECK(std::fabs(length - 5.0) < 1e-6);

    ocb_gprops_release(props);
    ocb_shape_release(edge);
}

TEST_CASE("props: invalid gprops handles return error, no crash") {
    double mass = 0.0;
    OcbPnt com = ocb_point_xyz(0, 0, 0);

    // invalid (zero) handle
    CHECK(ocb_gprops_mass(OCB_INVALID, &mass) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_gprops_center_of_mass(OCB_INVALID, &com) == OCB_ERR_INVALID_HANDLE);

    OcbShape box = makeBox(1.0, 1.0, 1.0);
    OcbGProps props = OCB_INVALID;
    REQUIRE(ocb_gprops_volume(box, &props) == OCB_OK);

    // null out pointer
    CHECK(ocb_gprops_mass(props, nullptr) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_gprops_center_of_mass(props, nullptr) == OCB_ERR_INVALID_HANDLE);

    // released handle
    ocb_gprops_release(props);
    CHECK(ocb_gprops_mass(props, &mass) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_gprops_center_of_mass(props, &com) == OCB_ERR_INVALID_HANDLE);

    // valid handle of a different type (shape where gprops expected)
    CHECK(ocb_gprops_mass(box, &mass) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_gprops_center_of_mass(box, &com) == OCB_ERR_INVALID_HANDLE);

    ocb_shape_release(box);
}
