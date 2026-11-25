// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kRad = 0.70710678118654752440;
constexpr double kPi = 3.14159265358979323846;
} // namespace

TEST_CASE("math: trsf from quat translation rotates and shifts") {
    OcbQuat z90{0.0, 0.0, kRad, kRad}; // 绕 Z 轴 90°: (x,y,z,w)
    OcbTrsf t = ocb_trsf_identity();
    REQUIRE(ocb_trsf_from_quat_translation(z90, 10.0, 0.0, 0.0, &t) == OCB_OK);

    OcbPnt p = ocb_trsf_apply(t, ocb_point_xyz(1, 0, 0));
    CHECK(std::fabs(p.x - 10.0) < 1e-9);
    CHECK(std::fabs(p.y - 1.0) < 1e-9);
    CHECK(std::fabs(p.z) < 1e-9);
}

TEST_CASE("math: get quat round trips") {
    OcbQuat q{0.0, 0.0, kRad, kRad};
    OcbTrsf t = ocb_trsf_identity();
    REQUIRE(ocb_trsf_from_quat_translation(q, 0.0, 0.0, 0.0, &t) == OCB_OK);

    OcbQuat back{0, 0, 0, 1};
    REQUIRE(ocb_trsf_get_quat(t, &back) == OCB_OK);

    double d = 0.0;
    REQUIRE(ocb_quaternion_dot(q, back, &d) == OCB_OK);
    const double dmag = std::fabs(d);
    CHECK(std::fabs(dmag - 1.0) < 1e-6);
}

TEST_CASE("math: euler zyx round trip") {
    OcbQuat q{};
    REQUIRE(ocb_quaternion_from_euler_zyx(kPi / 2.0, 0.0, 0.0, &q) == OCB_OK);

    OcbTrsf t = ocb_trsf_identity();
    REQUIRE(ocb_trsf_from_quat_translation(q, 0.0, 0.0, 0.0, &t) == OCB_OK);

    double rz = 0.0;
    double ry = 0.0;
    double rx = 0.0;
    REQUIRE(ocb_trsf_get_euler_zyx(t, &rz, &ry, &rx) == OCB_OK);
    CHECK(std::fabs(rz - kPi / 2.0) < 1e-6);
    CHECK(std::fabs(ry) < 1e-6);
    CHECK(std::fabs(rx) < 1e-6);

    OcbQuat q2{};
    REQUIRE(ocb_quaternion_from_euler_zyx(rz, ry, rx, &q2) == OCB_OK);
    double d = 0.0;
    REQUIRE(ocb_quaternion_dot(q, q2, &d) == OCB_OK);
    CHECK(std::fabs(std::fabs(d) - 1.0) < 1e-6);
}

TEST_CASE("math: quaternion dot of orthogonal rotations") {
    OcbQuat z90{0.0, 0.0, kRad, kRad};  // 绕 Z 轴 90°
    OcbQuat x90{kRad, 0.0, 0.0, kRad};  // 绕 X 轴 90°
    double d = 0.0;
    REQUIRE(ocb_quaternion_dot(z90, x90, &d) == OCB_OK);
    CHECK(std::fabs(d - 0.5) < 1e-6);
}

TEST_CASE("math: interface static params accept calls") {
    CHECK(ocb_interface_set_int("read.step.product.mode", 1) == OCB_OK);
    CHECK(ocb_interface_set_real("read.maxprecision.val", 0.1) == OCB_OK);
}
