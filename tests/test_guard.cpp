// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
// T1: ocb_c_guard.h RAII 句柄守卫的所有权语义测试
#include "occ_c_guard.h"

#include "occ_c_bridge.h"
#include "occ_c_types.h"

#include <doctest.h>

namespace {

OcbShape makeBoxShape() {
    OcbShape s = OCB_INVALID;
    OcbPnt o{};
    o.x = 0; o.y = 0; o.z = 0;
    REQUIRE(ocb_shape_make_box(o, 2.0, 2.0, 2.0, &s) == OCB_OK);
    REQUIRE(s != OCB_INVALID);
    return s;
}

} // namespace

TEST_CASE("guard: destructor releases the handle (live_count settles)") {
    const uint64_t base = ocb_live_count();
    {
        OcbShapeGuard g(makeBoxShape());
        REQUIRE(g.get() != OCB_INVALID);
        CHECK(ocb_live_count() == base + 1);
    }
    CHECK(ocb_live_count() == base);
}

TEST_CASE("guard: releaseOwnership transfers, dtor does not double-release") {
    const uint64_t base = ocb_live_count();
    OcbShape transferred = OCB_INVALID;
    {
        OcbShapeGuard g(makeBoxShape());
        transferred = g.releaseOwnership();
        CHECK(g.get() == OCB_INVALID);      // 所有权已转移
    }                                        // 析构：h==INVALID，不释放
    CHECK(ocb_live_count() == base + 1);     // 仍由调用方持有
    ocb_shape_release(transferred);
    CHECK(ocb_live_count() == base);
}

TEST_CASE("guard: get() usable in bridge calls") {
    const uint64_t base = ocb_live_count();
    OcbShapeGuard g(makeBoxShape());
    double v = 0.0;
    REQUIRE(ocb_shape_volume(g.get(), &v) == OCB_OK);
    CHECK(v == doctest::Approx(8.0));
    g.reset();
    CHECK(ocb_live_count() == base);
}

TEST_CASE("guard: out-param adoption (create straight into guard)") {
    const uint64_t base = ocb_live_count();
    OcbShapeGuard g;
    OcbPnt o{};
    REQUIRE(ocb_shape_make_box(o, 1.0, 1.0, 1.0, g.addr()) == OCB_OK);
    CHECK(g.get() != OCB_INVALID);
    CHECK(ocb_live_count() == base + 1);
    // 析构自动释放
}

TEST_CASE("guard: all typed guards smoke (9 handle kinds settle)") {
    const uint64_t base = ocb_live_count();
    {
        OcbAppGuard app;            REQUIRE(ocb_app_new(app.addr()) == OCB_OK);
        OcbDocGuard doc;            REQUIRE(ocb_app_new_document(app.get(), "BinXCAF", doc.addr()) == OCB_OK);
        OcbShapeToolGuard st;       REQUIRE(ocb_shapetool_get(doc.get(), st.addr()) == OCB_OK);
        OcbColorToolGuard ct;       REQUIRE(ocb_colortool_get(doc.get(), ct.addr()) == OCB_OK);
        OcbShapeGuard box;          REQUIRE(ocb_shape_make_box(OcbPnt{}, 1.0, 1.0, 1.0, box.addr()) == OCB_OK);
        OcbLabelGuard lab;          REQUIRE(ocb_shapetool_add_shape(st.get(), box.get(), lab.addr()) == OCB_OK);
        OcbStepReaderGuard rd;      REQUIRE(ocb_stepreader_create(rd.addr()) == OCB_OK);
        OcbStepWriterGuard wr;      REQUIRE(ocb_stepwriter_create(wr.addr()) == OCB_OK);
        OcbAisObjGuard ais;         REQUIRE(ocb_ais_new_shape(box.get(), ais.addr()) == OCB_OK);
        CHECK(ocb_live_count() == base + 9);
    }
    CHECK(ocb_live_count() == base);
}
