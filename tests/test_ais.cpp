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

TEST_CASE("ais: create shape and point objects") {
    OcbShape box = makeBox(1, 1, 1);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);
    CHECK(obj != OCB_INVALID);

    OcbGeom point = OCB_INVALID;
    REQUIRE(ocb_geom_new_cartesian_point(1, 2, 3, &point) == OCB_OK);
    OcbAisObj pobj = OCB_INVALID;
    REQUIRE(ocb_ais_new_point(point, &pobj) == OCB_OK);
    CHECK(pobj != OCB_INVALID);

    ocb_ais_release(pobj);
    ocb_geom_release(point);
    ocb_ais_release(obj);
    ocb_shape_release(box);
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("ais: color set and get") {
    OcbShape box = makeBox(1, 1, 1);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);

    const OcbColor red{1.0f, 0.0f, 0.0f};
    REQUIRE(ocb_ais_set_color(obj, red) == OCB_OK);

    bool has = false;
    REQUIRE(ocb_ais_has_color(obj, &has) == OCB_OK);
    CHECK(has);

    OcbColor got{0, 0, 0};
    REQUIRE(ocb_ais_get_color(obj, &got) == OCB_OK);
    CHECK(std::fabs(got.r - 1.0f) < 0.01f);
    CHECK(std::fabs(got.g - 0.0f) < 0.01f);
    CHECK(std::fabs(got.b - 0.0f) < 0.01f);

    ocb_ais_release(obj);
    ocb_shape_release(box);
}

TEST_CASE("ais: transparency set and get") {
    OcbShape box = makeBox(1, 1, 1);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);

    REQUIRE(ocb_ais_set_transparency(obj, 0.5) == OCB_OK);
    double t = 0.0;
    REQUIRE(ocb_ais_transparency(obj, &t) == OCB_OK);
    CHECK(std::fabs(t - 0.5) < 0.01);

    ocb_ais_release(obj);
    ocb_shape_release(box);
}

TEST_CASE("ais: downcast and shape extraction") {
    OcbShape box = makeBox(2, 3, 4);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);

    OcbAisObj down = OCB_INVALID;
    REQUIRE(ocb_ais_downcast_shape(obj, &down) == OCB_OK);
    CHECK(down != OCB_INVALID);

    OcbShape back = OCB_INVALID;
    REQUIRE(ocb_ais_shape_shape(down, &back) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(back, &v) == OCB_OK);
    CHECK(std::fabs(v - 24.0) < 1e-4);

    ocb_shape_release(back);
    ocb_ais_release(down);
    ocb_ais_release(obj);
    ocb_shape_release(box);
}

TEST_CASE("ais: drawer own shading aspect and polygon offsets") {
    OcbShape box = makeBox(2, 3, 4);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);

    OcbDrawer drawer = OCB_INVALID;
    REQUIRE(ocb_ais_attributes(obj, &drawer) == OCB_OK);
    CHECK(drawer != OCB_INVALID);

    REQUIRE(ocb_drawer_setup_own_shading_aspect(drawer) == OCB_OK);
    REQUIRE(ocb_drawer_set_polygon_offsets(drawer, 1 /*Aspect_POM_Fill*/, 1.0f, 2.0f) == OCB_OK);

    CHECK(ocb_drawer_setup_own_shading_aspect(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_drawer_set_polygon_offsets(OCB_INVALID, 1, 1.0f, 1.0f) == OCB_ERR_INVALID_HANDLE);

    ocb_drawer_release(drawer);
    ocb_ais_release(obj);
    ocb_shape_release(box);
}

TEST_CASE("ais: invalid handles rejected") {
    OcbColor c{0, 0, 0};
    bool b = false;
    double d = 0.0;
    int i = 0;
    OcbAisObj out = OCB_INVALID;
    OcbLabel label = OCB_INVALID;
    OcbDrawer drawer = OCB_INVALID;
    OcbTrsf t{};

    CHECK(ocb_ais_set_color(OCB_INVALID, c) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_set_transparency(OCB_INVALID, 0.5) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_has_color(OCB_INVALID, &b) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_get_color(OCB_INVALID, &c) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_transparency(OCB_INVALID, &d) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_material(OCB_INVALID, &i) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_zlayer(OCB_INVALID, &i) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_attributes(OCB_INVALID, &drawer) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_downcast_shape(OCB_INVALID, &out) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_shape_shape(OCB_INVALID, &out) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_has_transformation(OCB_INVALID, &b) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_local_transformation(OCB_INVALID, &t) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_xcaf_get_label(OCB_INVALID, &label) == OCB_ERR_INVALID_HANDLE);

    OcbGeom geom = OCB_INVALID;
    CHECK(ocb_ais_new_point(OCB_INVALID, &out) == OCB_ERR_INVALID_HANDLE);
    (void)geom;
}
