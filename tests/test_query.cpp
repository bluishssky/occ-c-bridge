// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-6;

OcbShape makeBox(double x, double y, double z, double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(x, y, z), dx, dy, dz, &s) == OCB_OK);
    return s;
}
} // namespace

TEST_CASE("query: distance between disjoint boxes") {
    OcbShape a = makeBox(0, 0, 0, 10, 10, 10);
    OcbShape b = makeBox(20, 0, 0, 10, 10, 10);

    double d = -1.0;
    REQUIRE(ocb_shape_distance(a, b, &d) == OCB_OK);
    CHECK(std::fabs(d - 10.0) < kTol);

    ocb_shape_release(a);
    ocb_shape_release(b);
}

TEST_CASE("query: distance of touching boxes is zero") {
    OcbShape a = makeBox(0, 0, 0, 10, 10, 10);
    OcbShape b = makeBox(10, 0, 0, 10, 10, 10);

    double d = -1.0;
    REQUIRE(ocb_shape_distance(a, b, &d) == OCB_OK);
    CHECK(std::fabs(d) < kTol);

    ocb_shape_release(a);
    ocb_shape_release(b);
}

TEST_CASE("query: solid classifier in/out/on") {
    OcbShape box = makeBox(0, 0, 0, 10, 10, 10);

    OcbCls cls = OCB_INVALID;
    REQUIRE(ocb_classifier_new(box, &cls) == OCB_OK);

    int state = -1;
    REQUIRE(ocb_classifier_perform(cls, ocb_point_xyz(5, 5, 5), 1e-6, &state) == OCB_OK);
    CHECK(state == 0); // TopAbs_IN

    REQUIRE(ocb_classifier_perform(cls, ocb_point_xyz(50, 50, 50), 1e-6, &state) == OCB_OK);
    CHECK(state == 1); // TopAbs_OUT

    REQUIRE(ocb_classifier_perform(cls, ocb_point_xyz(0, 5, 5), 1e-6, &state) == OCB_OK);
    CHECK(state == 2); // TopAbs_ON

    ocb_classifier_release(cls);
    ocb_shape_release(box);
}

TEST_CASE("query: classifier invalid handles rejected") {
    int state = -1;
    OcbCls cls = OCB_INVALID;
    CHECK(ocb_classifier_new(OCB_INVALID, &cls) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_classifier_perform(OCB_INVALID, ocb_point_xyz(0, 0, 0), 1e-6, &state) == OCB_ERR_INVALID_HANDLE);
}

TEST_CASE("query: boolean ex fuse/cut/common with fuzzy value") {
    OcbShape a = makeBox(0, 0, 0, 10, 10, 10);
    OcbShape b = makeBox(5, 0, 0, 10, 10, 10);

    OcbShape fused = OCB_INVALID;
    REQUIRE(ocb_bool_fuse_ex(a, b, 1e-4, false, &fused) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(fused, &v) == OCB_OK);
    CHECK(std::fabs(v - 1500.0) < 1e-3);

    OcbShape cut = OCB_INVALID;
    REQUIRE(ocb_bool_cut_ex(a, b, 1e-4, false, &cut) == OCB_OK);
    double vc = 0.0;
    REQUIRE(ocb_shape_volume(cut, &vc) == OCB_OK);
    CHECK(std::fabs(vc - 500.0) < 1e-3);

    OcbShape common = OCB_INVALID;
    REQUIRE(ocb_bool_common_ex(a, b, 1e-4, false, &common) == OCB_OK);
    double vcm = 0.0;
    REQUIRE(ocb_shape_volume(common, &vcm) == OCB_OK);
    CHECK(std::fabs(vcm - 500.0) < 1e-3);

    ocb_shape_release(common);
    ocb_shape_release(cut);
    ocb_shape_release(fused);
    ocb_shape_release(a);
    ocb_shape_release(b);
}

TEST_CASE("query: selected owner shape rejects invalid context") {
    OcbShape out = OCB_INVALID;
    CHECK(ocb_ais_ctx_selected_owner_shape(OCB_INVALID, &out) == OCB_ERR_INVALID_HANDLE);
}
