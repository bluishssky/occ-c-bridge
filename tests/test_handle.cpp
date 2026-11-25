// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

TEST_CASE("release_zero_is_safe") {
    const uint64_t before = ocb_live_count();
    ocb_shape_release(OCB_INVALID);
    CHECK(ocb_live_count() == before);
}

TEST_CASE("double_release_is_safe") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1.0, 1.0, 1.0, &box) == OCB_OK);
    ocb_shape_release(box);
    ocb_shape_release(box); // 第二次释放不应崩溃
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("invalid_handle_sets_last_error") {
    OcbBox bounds{};
    CHECK(ocb_shape_bounding_box(OCB_INVALID, &bounds) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_last_status() == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_last_error()[0] != '\0');
}

TEST_CASE("success_clears_previous_error") {
    OcbBox bounds{};
    (void)ocb_shape_bounding_box(OCB_INVALID, &bounds);
    CHECK(ocb_last_status() == OCB_ERR_INVALID_HANDLE);

    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1.0, 1.0, 1.0, &box) == OCB_OK);
    CHECK(ocb_last_status() == OCB_OK);
    CHECK(ocb_last_error()[0] == '\0');

    ocb_shape_release(box);
}

TEST_CASE("create_and_release_many_shapes_does_not_leak") {
    constexpr int kCount = 200;
    OcbShape shapes[kCount];

    for (int i = 0; i < kCount; ++i) {
        shapes[i] = OCB_INVALID;
        REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1.0, 1.0, 1.0, &shapes[i]) == OCB_OK);
    }
    CHECK(ocb_live_count() == kCount);

    for (int i = 0; i < kCount; ++i) {
        ocb_shape_release(shapes[i]);
    }
    CHECK(ocb_live_count() == 0);
}


