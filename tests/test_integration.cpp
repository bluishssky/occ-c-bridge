// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>
#include <vector>

namespace {
constexpr double kTol = 1e-6;

struct BoxSpec {
    double x, y, z;
    double dx, dy, dz;
};
} // namespace

TEST_CASE("integration: multiple boxes keep independent bounds and volumes") {
    const BoxSpec specs[] = {
        {0.0, 0.0, 0.0, 10.0, 10.0, 10.0},
        {20.0, 0.0, 0.0, 5.0, 5.0, 5.0},
        {0.0, 30.0, 0.0, 2.0, 3.0, 4.0},
    };
    constexpr int kCount = static_cast<int>(sizeof(specs) / sizeof(specs[0]));

    std::vector<OcbShape> shapes;
    for (int i = 0; i < kCount; ++i) {
        OcbShape id = OCB_INVALID;
        REQUIRE(ocb_shape_make_box(
                    ocb_point_xyz(specs[i].x, specs[i].y, specs[i].z),
                    specs[i].dx, specs[i].dy, specs[i].dz, &id) == OCB_OK);
        shapes.push_back(id);
    }
    CHECK(ocb_live_count() == static_cast<uint64_t>(kCount));

    for (int i = 0; i < kCount; ++i) {
        OcbBox bounds{};
        REQUIRE(ocb_shape_bounding_box(shapes[i], &bounds) == OCB_OK);
        CHECK(std::fabs(bounds.xmin - specs[i].x) < kTol);
        CHECK(std::fabs(bounds.ymin - specs[i].y) < kTol);
        CHECK(std::fabs(bounds.zmin - specs[i].z) < kTol);
        CHECK(std::fabs(bounds.xmax - (specs[i].x + specs[i].dx)) < kTol);
        CHECK(std::fabs(bounds.ymax - (specs[i].y + specs[i].dy)) < kTol);
        CHECK(std::fabs(bounds.zmax - (specs[i].z + specs[i].dz)) < kTol);

        double volume = 0.0;
        REQUIRE(ocb_shape_volume(shapes[i], &volume) == OCB_OK);
        CHECK(std::fabs(volume - specs[i].dx * specs[i].dy * specs[i].dz) < kTol);
    }

    for (OcbShape id : shapes) {
        ocb_shape_release(id);
    }
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("integration: batch create/query/release stays stable") {
    constexpr int kBatch = 100;

    for (int round = 0; round < 5; ++round) {
        std::vector<OcbShape> shapes;
        shapes.reserve(kBatch);

        for (int i = 0; i < kBatch; ++i) {
            OcbShape id = OCB_INVALID;
            REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1.0, 1.0, 1.0, &id) == OCB_OK);
            shapes.push_back(id);
        }
        CHECK(ocb_live_count() == static_cast<uint64_t>(kBatch));

        double volume = 0.0;
        REQUIRE(ocb_shape_volume(shapes.front(), &volume) == OCB_OK);
        CHECK(std::fabs(volume - 1.0) < kTol);

        for (OcbShape id : shapes) {
            ocb_shape_release(id);
        }
        CHECK(ocb_live_count() == 0);
    }
}

TEST_CASE("integration: invalid handle does not corrupt other shapes") {
    OcbShape good = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 2.0, 2.0, 2.0, &good) == OCB_OK);

    OcbBox bounds{};
    CHECK(ocb_shape_bounding_box(OCB_INVALID, &bounds) == OCB_ERR_INVALID_HANDLE);

    double volume = 0.0;
    REQUIRE(ocb_shape_volume(good, &volume) == OCB_OK);
    CHECK(std::fabs(volume - 8.0) < kTol);

    ocb_shape_release(good);
    CHECK(ocb_live_count() == 0);
}
