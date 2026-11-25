// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
// gp_parity: prove the bridge transform API is numerically identical to native gp_Trsf.
// S3 (gp_* migration) acceptance relies on this: if the bridge agrees with gp_Trsf to
// 1e-12 on translation / rotation / composition / inversion / point mapping, then a
// mechanical call swap cannot change geometry.
#include "doctest.h"
#include "occ_c_bridge.h"

#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

gp_Trsf makeNativeChain()
{
    gp_Trsf t1;
    t1.SetTranslation(gp_Vec(1.5, -2.25, 3.75));
    gp_Trsf r1;
    r1.SetRotation(gp_Ax1(gp_Pnt(1.0, 2.0, 3.0), gp_Dir(0.0, 0.0, 1.0)), 0.7);
    gp_Trsf t2;
    t2.SetTranslation(gp_Vec(-4.0, 0.5, 2.0));
    return t1 * r1 * t2;
}

// Compare the 3x4 elements (row, col) used by OcbTrsf (row-major v[12]).
void requireSameTrsf(const gp_Trsf& native, OcbTrsf bridge, double tol)
{
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col) {
            const double expect = native.Value(row, col);
            const double got = bridge.v[(row - 1) * 4 + (col - 1)];
            INFO("row " << row << " col " << col << " expect " << expect << " got " << got);
            CHECK(std::abs(expect - got) <= tol);
        }
    }
}

OcbTrsf toBridge(const gp_Trsf& t)
{
    OcbTrsf out{};
    REQUIRE(ocb_trsf_from_native(&t, &out) == OCB_OK);
    return out;
}

OcbPnt toOcbP(const gp_Pnt& p) { return OcbPnt{p.X(), p.Y(), p.Z()}; }

} // namespace

TEST_CASE("parity: translation matches gp_Trsf") {
    gp_Trsf n;
    n.SetTranslation(gp_Vec(3.25, -0.5, 7.125));
    const OcbTrsf b = ocb_trsf_translation(OcbVec{3.25, -0.5, 7.125});
    requireSameTrsf(n, b, 1e-12);
}

TEST_CASE("parity: rotation matches gp_Trsf") {
    const gp_Pnt origin(1.0, 2.0, 3.0);
    const gp_Dir axis(0.0, 0.0, 1.0);
    const double angle = -1.234;
    gp_Trsf n;
    n.SetRotation(gp_Ax1(origin, axis), angle);
    const OcbTrsf b = ocb_trsf_rotation(toOcbP(origin), OcbDir{0.0, 0.0, 1.0}, angle);
    requireSameTrsf(n, b, 1e-12);
}

TEST_CASE("parity: composition order matches operator*") {
    const gp_Trsf n = makeNativeChain();

    gp_Trsf t1;
    t1.SetTranslation(gp_Vec(1.5, -2.25, 3.75));
    gp_Trsf r1;
    r1.SetRotation(gp_Ax1(gp_Pnt(1.0, 2.0, 3.0), gp_Dir(0.0, 0.0, 1.0)), 0.7);
    gp_Trsf t2;
    t2.SetTranslation(gp_Vec(-4.0, 0.5, 2.0));

    const OcbTrsf chain = ocb_trsf_multiply(
        ocb_trsf_multiply(toBridge(t1), toBridge(r1)), toBridge(t2));
    requireSameTrsf(n, chain, 1e-12);
}

TEST_CASE("parity: inverted matches gp_Trsf") {
    const gp_Trsf n = makeNativeChain();
    const gp_Trsf inv = n.Inverted();
    const OcbTrsf b = ocb_trsf_inverted(toBridge(n));
    requireSameTrsf(inv, b, 1e-12);
}

TEST_CASE("parity: point mapping matches Transforms") {
    const gp_Trsf n = makeNativeChain();
    const OcbTrsf b = toBridge(n);
    const gp_Pnt pts[4] = {
        gp_Pnt(0.0, 0.0, 0.0),
        gp_Pnt(1.0, 2.0, 3.0),
        gp_Pnt(-5.5, 0.25, 12.0),
        gp_Pnt(100.0, -200.0, 0.5)
    };
    for (const gp_Pnt& p : pts) {
        gp_Pnt expect = p.Transformed(n);
        const OcbPnt got = ocb_trsf_apply(b, toOcbP(p));
        INFO("p " << p.X() << "," << p.Y() << "," << p.Z());
        CHECK(std::abs(expect.X() - got.x) <= 1e-12);
        CHECK(std::abs(expect.Y() - got.y) <= 1e-12);
        CHECK(std::abs(expect.Z() - got.z) <= 1e-12);
    }
}

TEST_CASE("parity: left-multiply vs right-multiply stay distinguishable") {
    // S2/S4 bug class: writing acc*T where T*acc was meant. The bridge must reproduce
    // gp_Trsf exactly in both orders, so a wrong order shows up as a geometry change.
    gp_Trsf acc;
    acc.SetRotation(gp_Ax1(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(1.0, 1.0, 0.0)), 0.9);
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(10.0, 0.0, 0.0));

    const gp_Trsf nativeLeft = tr * acc;
    const gp_Trsf nativeRight = acc * tr;
    const OcbTrsf bridgeLeft = ocb_trsf_multiply(toBridge(tr), toBridge(acc));
    const OcbTrsf bridgeRight = ocb_trsf_multiply(toBridge(acc), toBridge(tr));

    requireSameTrsf(nativeLeft, bridgeLeft, 1e-12);
    requireSameTrsf(nativeRight, bridgeRight, 1e-12);

    // And the two orders must actually differ, otherwise this test proves nothing.
    CHECK(std::abs(nativeLeft.Value(1, 4) - nativeRight.Value(1, 4)) > 0.5);
}

TEST_CASE("parity: round trip native -> bridge -> native") {
    const gp_Trsf n = makeNativeChain();
    const OcbTrsf b = toBridge(n);
    // Rebuild gp_Trsf from the bridge matrix the same way Viewer3D::trsfToGp does.
    gp_Trsf back;
    back.SetValues(b.v[0], b.v[1], b.v[2], b.v[3],
                   b.v[4], b.v[5], b.v[6], b.v[7],
                   b.v[8], b.v[9], b.v[10], b.v[11]);
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col) {
            CHECK(std::abs(n.Value(row, col) - back.Value(row, col)) <= 1e-12);
        }
    }
}

TEST_CASE("shape location: get_location matches native Location().Transformation()") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(OcbPnt{0.0, 0.0, 0.0}, 10.0, 20.0, 30.0, &box) == OCB_OK);
    REQUIRE(box != OCB_INVALID);

    const gp_Trsf native = makeNativeChain();
    OcbShape moved = OCB_INVALID;
    REQUIRE(ocb_shape_moved(box, toBridge(native), &moved) == OCB_OK);
    REQUIRE(moved != OCB_INVALID);

    OcbTrsf loc{};
    REQUIRE(ocb_shape_get_location(moved, &loc) == OCB_OK);
    requireSameTrsf(native, loc, 1e-12);

    OcbShape cleared = OCB_INVALID;
    REQUIRE(ocb_shape_location_cleared(moved, &cleared) == OCB_OK);
    OcbTrsf clearedLoc{};
    REQUIRE(ocb_shape_get_location(cleared, &clearedLoc) == OCB_OK);
    const OcbTrsf id = ocb_trsf_identity();
    for (int k = 0; k < 12; ++k) CHECK(std::abs(clearedLoc.v[k] - id.v[k]) <= 1e-12);

    ocb_shape_release(box);
    ocb_shape_release(moved);
    ocb_shape_release(cleared);
}
