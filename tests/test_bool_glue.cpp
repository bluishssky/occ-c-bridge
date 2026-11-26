// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// test_bool_glue.cpp —— v1.5.1 glue-aware boolean 回归
//
// 场景：切削工具与目标「上下表面完全共面」的贯穿切割（模拟 PCB 板层 -
// 贯穿腔体）。非 glue 模式下 BOP 可能在共面处产生细碎面（sliver，渲染成
// 飞线）；GlueShift 走共享面快速路径，避免 sliver。
//
// known-good 口径（2026-11-26，OCCT 7.7.0 官方 vc14 二进制实测）：
//   · target = 板 box(20,10,4) @ (0,0,0)；tool = 腔 box(6,6,4) @ (7,2,0)
//     —— x∈[7,13]∈[0,20]、y∈[2,8]∈[0,10]、z 完全同程（0..4）；
//   · 体积（各 glue 档位一致）= 800 - 144 = 656（绝对容差 1e-6）；
//   · face 数见「回归门禁」用例（实测快照，先打印后写死）；
//   · GlueFull 因工具顶面只是目标顶面的局部子面（非完全重合），
//     按契约允许返回 OCC_EXCEPTION，不作体积断言。
// ======================================================================

#include "doctest.h"
#include "occ_c_bridge.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

constexpr double kAbsTol = 1e-6;

OcbShape makeBoxAt(double x, double y, double z, double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(x, y, z), dx, dy, dz, &s) == OCB_OK);
    return s;
}

double volumeOf(OcbShape s) {
    double v = 0.0;
    REQUIRE(ocb_shape_volume(s, &v) == OCB_OK);
    return v;
}

int faceCountOf(OcbShape s) {
    int n = -1;
    REQUIRE(ocb_topo_count_faces(s, &n) == OCB_OK);
    return n;
}

// 板层 - 贯穿腔体共面切割：同一 target/tool 下按 glue 档位执行。
// 返回的 shape（若 status==OCB_OK）由调用方 release。
struct GlueCutResult {
    OcbStatus status = OCB_ERR_OCC_EXCEPTION;
    OcbShape shape = OCB_INVALID;
    double volume = 0.0;
    int faces = -1;
};

GlueCutResult cutWithGlue(int glue) {
    GlueCutResult r;
    OcbShape target = makeBoxAt(0, 0, 0, 20, 10, 4);
    OcbShape tool = makeBoxAt(7, 2, 0, 6, 6, 4);
    r.status = ocb_bool_cut_glue(target, tool, 0.0, glue, true, &r.shape);
    if (r.status == OCB_OK) {
        r.volume = volumeOf(r.shape);
        r.faces = faceCountOf(r.shape);
    }
    ocb_shape_release(tool);
    ocb_shape_release(target);
    return r;
}

} // namespace

TEST_CASE("bool glue: coplanar through-cavity cut, volume identical across glue modes") {
    GlueCutResult off = cutWithGlue(0);
    GlueCutResult shift = cutWithGlue(1);

    REQUIRE(off.status == OCB_OK);
    REQUIRE(shift.status == OCB_OK);

    // glue 只影响共面处理路径，不改变结果体积。
    CHECK(std::fabs(off.volume - 656.0) < kAbsTol);
    CHECK(std::fabs(shift.volume - 656.0) < kAbsTol);
    CHECK(std::fabs(off.volume - shift.volume) <= 1e-9 * std::max(1.0, off.volume));

    ocb_shape_release(off.shape);
    ocb_shape_release(shift.shape);
}

TEST_CASE("bool glue: GlueShift face count is the regression gate (no sliver growth)")
{
    GlueCutResult off = cutWithGlue(0);
    GlueCutResult shift = cutWithGlue(1);

    REQUIRE(off.status == OCB_OK);
    REQUIRE(shift.status == OCB_OK);

    // 门禁 1：glue 的意义就是不产生额外碎面，Shift 面数不得多于 Off。
    CHECK_MESSAGE(shift.faces <= off.faces,
                  "shift.faces=" << shift.faces << " off.faces=" << off.faces);
    // 门禁 2：实测快照（2026-11-26 OCCT 7.7.0：本例几何简单，Off/Shift 均为 10 面、
    // 未产生 sliver；快照用于发现 OCCT 行为漂移）。
    CHECK_MESSAGE(off.faces == 10, "off.faces=" << off.faces);
    CHECK_MESSAGE(shift.faces == 10, "shift.faces=" << shift.faces);

    ocb_shape_release(off.shape);
    ocb_shape_release(shift.shape);
}

TEST_CASE("bool glue: GlueFull on partially-glued cavity is allowed to fail with OCC exception")
{
    // 工具顶面只是目标顶面的局部子面，不满足 GlueFull 的完全重合前提；
    // 契约：OK（接受结果）或 OCC_EXCEPTION（BOP 拒绝）均可，但不崩溃、不返回其它错误。
    GlueCutResult full = cutWithGlue(2);
    CHECK((full.status == OCB_OK || full.status == OCB_ERR_OCC_EXCEPTION));
    if (full.status == OCB_OK) {
        CHECK(std::fabs(full.volume - 656.0) < kAbsTol);
        ocb_shape_release(full.shape);
    }
}

TEST_CASE("bool glue: invalid glue value rejected")
{
    OcbShape target = makeBoxAt(0, 0, 0, 20, 10, 4);
    OcbShape tool = makeBoxAt(7, 2, 0, 6, 6, 4);
    OcbShape out = OCB_INVALID;

    CHECK(ocb_bool_cut_glue(target, tool, 0.0, -1, true, &out) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_bool_cut_glue(target, tool, 0.0, 3, true, &out) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_bool_cut_glue(target, tool, 0.0, 99, true, &out) == OCB_ERR_INVALID_ARG);
    CHECK(out == OCB_INVALID);

    CHECK(ocb_bool_fuse_glue(target, tool, 0.0, 5, true, &out) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_bool_common_glue(target, tool, 0.0, -2, true, &out) == OCB_ERR_INVALID_ARG);

    ocb_shape_release(tool);
    ocb_shape_release(target);
}

TEST_CASE("bool glue: null out and invalid handles rejected")
{
    OcbShape target = makeBoxAt(0, 0, 0, 20, 10, 4);
    OcbShape tool = makeBoxAt(7, 2, 0, 6, 6, 4);
    OcbShape out = OCB_INVALID;

    CHECK(ocb_bool_cut_glue(target, tool, 0.0, 1, true, nullptr) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_bool_cut_glue(OCB_INVALID, tool, 0.0, 1, true, &out) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_bool_cut_glue(target, OCB_INVALID, 0.0, 1, true, &out) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_bool_fuse_glue(OCB_INVALID, tool, 0.0, 1, true, &out) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_bool_common_glue(target, OCB_INVALID, 0.0, 1, true, &out) == OCB_ERR_INVALID_HANDLE);

    ocb_shape_release(tool);
    ocb_shape_release(target);
}

TEST_CASE("bool glue: fuse/common glue variants keep analytic volumes")
{
    // 不共面、不重叠的两个盒体：fuse 体积 = 16
    {
        OcbShape a = makeBoxAt(0, 0, 0, 2, 2, 2);
        OcbShape b = makeBoxAt(10, 0, 0, 2, 2, 2);
        OcbShape fused = OCB_INVALID;
        REQUIRE(ocb_bool_fuse_glue(a, b, 0.0, 1, true, &fused) == OCB_OK);
        CHECK(std::fabs(volumeOf(fused) - 16.0) < kAbsTol);
        ocb_shape_release(fused);
        ocb_shape_release(b);
        ocb_shape_release(a);
    }
    // 完全重合的两个盒体：common 体积 = 8（GlueFull 适用：所有共享面完全重合）
    {
        OcbShape a = makeBoxAt(0, 0, 0, 2, 2, 2);
        OcbShape b = makeBoxAt(0, 0, 0, 2, 2, 2);
        OcbShape common = OCB_INVALID;
        REQUIRE(ocb_bool_common_glue(a, b, 0.0, 2, true, &common) == OCB_OK);
        CHECK(std::fabs(volumeOf(common) - 8.0) < kAbsTol);
        ocb_shape_release(common);
        ocb_shape_release(b);
        ocb_shape_release(a);
    }
    // 重叠盒体：common 体积 = 重叠区 x∈[2,4]、y∈[0,4]、z∈[0,4] = 2*4*4 = 32
    {
        OcbShape a = makeBoxAt(0, 0, 0, 4, 4, 4);
        OcbShape b = makeBoxAt(2, 0, 0, 4, 4, 4);
        OcbShape common = OCB_INVALID;
        REQUIRE(ocb_bool_common_glue(a, b, 0.0, 1, true, &common) == OCB_OK);
        CHECK(std::fabs(volumeOf(common) - 32.0) < kAbsTol);
        ocb_shape_release(common);
        ocb_shape_release(b);
        ocb_shape_release(a);
    }
}

TEST_CASE("bool glue: version reports 1.5.1")
{
    CHECK(std::string(ocb_version()) == "1.5.1");
}
