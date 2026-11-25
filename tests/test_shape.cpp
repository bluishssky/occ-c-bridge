// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

namespace {
constexpr double kTol = 1e-6;

OcbPnt origin() {
    return ocb_point_xyz(0.0, 0.0, 0.0);
}
} // namespace

TEST_CASE("make_box_ok") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 10.0, 20.0, 30.0, &box) == OCB_OK);
    CHECK(box != OCB_INVALID);
    ocb_shape_release(box);
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("make_box_rejects_non_positive_dimensions") {
    OcbShape box = OCB_INVALID;
    CHECK(ocb_shape_make_box(origin(), 0.0, 1.0, 1.0, &box) == OCB_ERR_INVALID_ARG);
    CHECK(box == OCB_INVALID);
    CHECK(ocb_shape_make_box(origin(), -1.0, 1.0, 1.0, &box) == OCB_ERR_INVALID_ARG);
    CHECK(box == OCB_INVALID);
}

TEST_CASE("make_box_rejects_null_out") {
    CHECK(ocb_shape_make_box(origin(), 1.0, 1.0, 1.0, nullptr) == OCB_ERR_INVALID_ARG);
}

TEST_CASE("bounding_box_of_box") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(1.0, 2.0, 3.0), 10.0, 20.0, 30.0, &box) == OCB_OK);

    OcbBox bounds{};
    REQUIRE(ocb_shape_bounding_box(box, &bounds) == OCB_OK);
    CHECK(bounds.valid);
    CHECK(std::fabs(bounds.xmin - 1.0) < kTol);
    CHECK(std::fabs(bounds.ymin - 2.0) < kTol);
    CHECK(std::fabs(bounds.zmin - 3.0) < kTol);
    CHECK(std::fabs(bounds.xmax - 11.0) < kTol);
    CHECK(std::fabs(bounds.ymax - 22.0) < kTol);
    CHECK(std::fabs(bounds.zmax - 33.0) < kTol);

    ocb_shape_release(box);
}

TEST_CASE("bounding_box_rejects_null_out") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 1.0, 1.0, 1.0, &box) == OCB_OK);
    CHECK(ocb_shape_bounding_box(box, nullptr) == OCB_ERR_INVALID_ARG);
    ocb_shape_release(box);
}

TEST_CASE("volume_of_box") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 10.0, 20.0, 30.0, &box) == OCB_OK);

    double volume = 0.0;
    REQUIRE(ocb_shape_volume(box, &volume) == OCB_OK);
    CHECK(std::fabs(volume - 6000.0) < 1e-6);

    ocb_shape_release(box);
}

TEST_CASE("volume_rejects_null_out") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 1.0, 1.0, 1.0, &box) == OCB_OK);
    CHECK(ocb_shape_volume(box, nullptr) == OCB_ERR_INVALID_ARG);
    ocb_shape_release(box);
}

TEST_CASE("invalid_handle_returns_error") {
    OcbBox bounds{};
    CHECK(ocb_shape_bounding_box(OCB_INVALID, &bounds) == OCB_ERR_INVALID_HANDLE);

    double volume = 0.0;
    CHECK(ocb_shape_volume(OCB_INVALID, &volume) == OCB_ERR_INVALID_HANDLE);
}

TEST_CASE("release_invalidates_handle") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 1.0, 1.0, 1.0, &box) == OCB_OK);
    ocb_shape_release(box);

    OcbBox bounds{};
    CHECK(ocb_shape_bounding_box(box, &bounds) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("multiple_boxes_have_independent_lifetimes") {
    OcbShape a = OCB_INVALID;
    OcbShape b = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(origin(), 1.0, 1.0, 1.0, &a) == OCB_OK);
    REQUIRE(ocb_shape_make_box(origin(), 2.0, 2.0, 2.0, &b) == OCB_OK);
    CHECK(a != b);
    CHECK(ocb_live_count() == 2);

    ocb_shape_release(a);
    CHECK(ocb_live_count() == 1);

    double volume = 0.0;
    CHECK(ocb_shape_volume(b, &volume) == OCB_OK);
    CHECK(std::fabs(volume - 8.0) < 1e-6);

    ocb_shape_release(b);
    CHECK(ocb_live_count() == 0);
}


