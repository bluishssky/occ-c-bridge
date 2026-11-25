// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-9;
constexpr double kVolTol = 1e-4;

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}
} // namespace

TEST_CASE("geom: vec modulus") {
    CHECK(std::fabs(ocb_vec_modulus(ocb_vec_xyz(3, 4, 0)) - 5.0) < kTol);
    CHECK(std::fabs(ocb_vec_modulus(ocb_vec_xyz(1, 1, 1)) - std::sqrt(3.0)) < kTol);
    CHECK(ocb_vec_modulus(ocb_vec_xyz(0, 0, 0)) == 0.0);
}

TEST_CASE("geom: point transformed by translation") {
    const OcbTrsf t = ocb_trsf_translation(ocb_vec_xyz(5, -3, 2));
    const OcbPnt p = ocb_point_transformed(ocb_point_xyz(1, 1, 1), t);
    CHECK(std::fabs(p.x - 6.0) < kTol);
    CHECK(std::fabs(p.y + 2.0) < kTol);
    CHECK(std::fabs(p.z - 3.0) < kTol);
}

TEST_CASE("geom: rotation by 90 degrees around Z") {
    const OcbTrsf r = ocb_trsf_rotation(ocb_point_xyz(0, 0, 0),
                                        OcbDir{0, 0, 1}, 3.14159265358979 / 2.0);
    const OcbPnt p = ocb_point_transformed(ocb_point_xyz(1, 0, 0), r);
    CHECK(std::fabs(p.x - 0.0) < 1e-6);
    CHECK(std::fabs(p.y - 1.0) < 1e-6);
    CHECK(std::fabs(p.z - 0.0) < 1e-6);
}

TEST_CASE("geom: trsf set values round trip") {
    const OcbTrsf t = ocb_trsf_set_values(
        1, 0, 0, 10, 0, 1, 0, 20, 0, 0, 1, 30);
    const OcbVec part = ocb_trsf_translation_part(t);
    CHECK(std::fabs(part.x - 10.0) < kTol);
    CHECK(std::fabs(part.y - 20.0) < kTol);
    CHECK(std::fabs(part.z - 30.0) < kTol);
}

TEST_CASE("geom: trsf multiply equals composition") {
    const OcbTrsf a = ocb_trsf_translation(ocb_vec_xyz(1, 0, 0));
    const OcbTrsf b = ocb_trsf_translation(ocb_vec_xyz(0, 2, 0));
    const OcbTrsf ab = ocb_trsf_multiply(a, b);
    const OcbPnt p = ocb_point_transformed(ocb_point_xyz(0, 0, 0), ab);
    CHECK(std::fabs(p.x - 1.0) < kTol);
    CHECK(std::fabs(p.y - 2.0) < kTol);
}

TEST_CASE("geom: inverted transforms back") {
    const OcbTrsf t = ocb_trsf_translation(ocb_vec_xyz(7, 8, 9));
    const OcbTrsf inv = ocb_trsf_inverted(t);
    const OcbPnt p = ocb_point_transformed(ocb_point_xyz(7, 8, 9), inv);
    CHECK(std::fabs(p.x) < kTol);
    CHECK(std::fabs(p.y) < kTol);
    CHECK(std::fabs(p.z) < kTol);
}
