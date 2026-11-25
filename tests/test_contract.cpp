// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

// 契约测试：直接调用 OCC 原生 API，和 ocb_* 的结果对比。
// 该文件只在测试中 include OCC，库本身仍然只暴露 C ABI。
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <cmath>

namespace {
constexpr double kTol = 1e-6;

double nativeVolume(const TopoDS_Shape& shape) {
    GProp_GProps props;
    BRepGProp::VolumeProperties(shape, props);
    return props.Mass();
}

void nativeBounds(const TopoDS_Shape& shape, double out[6]) {
    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    box.Get(out[0], out[1], out[2], out[3], out[4], out[5]);
}
} // namespace

TEST_CASE("contract: box volume matches native OCC") {
    const double dx = 10.0;
    const double dy = 20.0;
    const double dz = 30.0;

    const TopoDS_Shape native = BRepPrimAPI_MakeBox(gp_Pnt(1.0, 2.0, 3.0), dx, dy, dz).Shape();
    const double expected = nativeVolume(native);

    OcbShape shape = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(1.0, 2.0, 3.0), dx, dy, dz, &shape) == OCB_OK);

    double actual = 0.0;
    REQUIRE(ocb_shape_volume(shape, &actual) == OCB_OK);
    CHECK(std::fabs(actual - expected) < kTol);

    ocb_shape_release(shape);
}

TEST_CASE("contract: bounding box matches native OCC") {
    const TopoDS_Shape native = BRepPrimAPI_MakeBox(gp_Pnt(-5.0, 7.0, 1.5), 4.0, 6.0, 8.0).Shape();
    double expected[6] = {};
    nativeBounds(native, expected);

    OcbShape shape = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(-5.0, 7.0, 1.5), 4.0, 6.0, 8.0, &shape) == OCB_OK);

    OcbBox actual{};
    REQUIRE(ocb_shape_bounding_box(shape, &actual) == OCB_OK);
    CHECK(actual.valid);
    CHECK(std::fabs(actual.xmin - expected[0]) < kTol);
    CHECK(std::fabs(actual.ymin - expected[1]) < kTol);
    CHECK(std::fabs(actual.zmin - expected[2]) < kTol);
    CHECK(std::fabs(actual.xmax - expected[3]) < kTol);
    CHECK(std::fabs(actual.ymax - expected[4]) < kTol);
    CHECK(std::fabs(actual.zmax - expected[5]) < kTol);

    ocb_shape_release(shape);
}

TEST_CASE("contract: volume is additive for disjoint boxes") {
    const TopoDS_Shape a = BRepPrimAPI_MakeBox(gp_Pnt(0, 0, 0), 2, 3, 4).Shape();
    const TopoDS_Shape b = BRepPrimAPI_MakeBox(gp_Pnt(10, 0, 0), 5, 6, 7).Shape();
    const double expected = nativeVolume(a) + nativeVolume(b);

    OcbShape oa = OCB_INVALID;
    OcbShape ob = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 2, 3, 4, &oa) == OCB_OK);
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(10, 0, 0), 5, 6, 7, &ob) == OCB_OK);

    double va = 0.0;
    double vb = 0.0;
    REQUIRE(ocb_shape_volume(oa, &va) == OCB_OK);
    REQUIRE(ocb_shape_volume(ob, &vb) == OCB_OK);
    CHECK(std::fabs((va + vb) - expected) < kTol);

    ocb_shape_release(oa);
    ocb_shape_release(ob);
}
