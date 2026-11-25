// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

TEST_CASE("point_xyz_keeps_values") {
    const OcbPnt p = ocb_point_xyz(1.5, -2.5, 3.25);
    CHECK(p.x == 1.5);
    CHECK(p.y == -2.5);
    CHECK(p.z == 3.25);
}

TEST_CASE("point_distance_basic") {
    const OcbPnt a = ocb_point_xyz(0.0, 0.0, 0.0);
    const OcbPnt b = ocb_point_xyz(3.0, 4.0, 0.0);
    CHECK(std::fabs(ocb_point_distance(a, b) - 5.0) < 1e-9);
    CHECK(ocb_point_distance(a, a) == 0.0);
}

TEST_CASE("point_distance_negative_coordinates") {
    const OcbPnt a = ocb_point_xyz(-1.0, -2.0, -3.0);
    const OcbPnt b = ocb_point_xyz(1.0, 2.0, 3.0);
    CHECK(std::fabs(ocb_point_distance(a, b) - std::sqrt(56.0)) < 1e-9);
}

TEST_CASE("trsf_identity_layout") {
    const OcbTrsf t = ocb_trsf_identity();
    CHECK(t.v[0] == 1.0);
    CHECK(t.v[5] == 1.0);
    CHECK(t.v[10] == 1.0);
    CHECK(t.v[3] == 0.0);
    CHECK(t.v[7] == 0.0);
    CHECK(t.v[11] == 0.0);
}

TEST_CASE("trsf_translation_applies") {
    const OcbTrsf t = ocb_trsf_translation(OcbVec{1.0, 2.0, 3.0});
    const OcbPnt p = ocb_trsf_apply(t, ocb_point_xyz(10.0, 20.0, 30.0));
    CHECK(std::fabs(p.x - 11.0) < 1e-12);
    CHECK(std::fabs(p.y - 22.0) < 1e-12);
    CHECK(std::fabs(p.z - 33.0) < 1e-12);
}

TEST_CASE("trsf_identity_does_not_move_point") {
    const OcbPnt p = ocb_point_xyz(5.0, -6.0, 7.0);
    const OcbPnt q = ocb_trsf_apply(ocb_trsf_identity(), p);
    CHECK(std::fabs(p.x - q.x) < 1e-12);
    CHECK(std::fabs(p.y - q.y) < 1e-12);
    CHECK(std::fabs(p.z - q.z) < 1e-12);
}


