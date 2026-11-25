// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-6;
constexpr double kPI = 3.14159265358979323846;

OcbAx2 zAxis(double x, double y, double z) {
    OcbAx2 ax;
    ax.origin = OcbPnt{x, y, z};
    ax.normal = OcbDir{0, 0, 1};
    ax.xdir = OcbDir{1, 0, 0};
    return ax;
}

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}
} // namespace

TEST_CASE("primitive: cylinder volume") {
    OcbShape cyl = OCB_INVALID;
    REQUIRE(ocb_make_cylinder(zAxis(0, 0, 0), 5.0, 10.0, &cyl) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(cyl, &v) == OCB_OK);
    CHECK(std::fabs(v - kPI * 25.0 * 10.0) < 1.0);
    ocb_shape_release(cyl);
}

TEST_CASE("primitive: cone volume") {
    OcbShape cone = OCB_INVALID;
    REQUIRE(ocb_make_cone(zAxis(0, 0, 0), 0.0, 5.0, 10.0, &cone) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(cone, &v) == OCB_OK);
    CHECK(std::fabs(v - kPI * 25.0 * 10.0 / 3.0) < 1.0);
    ocb_shape_release(cone);
}

TEST_CASE("primitive: sphere volume") {
    OcbShape sphere = OCB_INVALID;
    REQUIRE(ocb_make_sphere(ocb_point_xyz(0, 0, 0), 5.0, &sphere) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(sphere, &v) == OCB_OK);
    CHECK(std::fabs(v - 4.0 / 3.0 * kPI * 125.0) < 1.0);
    ocb_shape_release(sphere);
}

TEST_CASE("primitive: rejects invalid dimensions") {
    OcbShape s = OCB_INVALID;
    CHECK(ocb_make_cylinder(zAxis(0, 0, 0), 0.0, 1.0, &s) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_make_cone(zAxis(0, 0, 0), 1.0, 2.0, -1.0, &s) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_make_sphere(ocb_point_xyz(0, 0, 0), -5.0, &s) == OCB_ERR_INVALID_ARG);
}

TEST_CASE("boolean: fuse volume = sum minus overlap") {
    OcbShape a = makeBox(10, 10, 10);
    OcbShape b = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(5, 0, 0), 10, 10, 10, &b) == OCB_OK);

    OcbShape fused = OCB_INVALID;
    REQUIRE(ocb_bool_fuse(a, b, &fused) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(fused, &v) == OCB_OK);
    CHECK(std::fabs(v - 1500.0) < 1.0);

    ocb_shape_release(fused);
    ocb_shape_release(b);
    ocb_shape_release(a);
}

TEST_CASE("boolean: cut and common") {
    OcbShape a = makeBox(10, 10, 10);
    OcbShape b = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(5, 0, 0), 10, 10, 10, &b) == OCB_OK);

    OcbShape cut = OCB_INVALID;
    REQUIRE(ocb_bool_cut(a, b, &cut) == OCB_OK);
    double vc = 0.0;
    REQUIRE(ocb_shape_volume(cut, &vc) == OCB_OK);
    CHECK(std::fabs(vc - 500.0) < 1.0);

    OcbShape common = OCB_INVALID;
    REQUIRE(ocb_bool_common(a, b, &common) == OCB_OK);
    double vm = 0.0;
    REQUIRE(ocb_shape_volume(common, &vm) == OCB_OK);
    CHECK(std::fabs(vm - 500.0) < 1.0);

    ocb_shape_release(common);
    ocb_shape_release(cut);
    ocb_shape_release(b);
    ocb_shape_release(a);
}
