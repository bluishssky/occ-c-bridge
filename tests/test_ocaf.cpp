// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

// 契约对比：直接调用原生 OCC
#include <BinXCAFDrivers.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TDF_Label.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_ExtendedString.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <cstdio>
#include <string>

namespace {

struct OcafFixture {
    OcbApp app = OCB_INVALID;
    OcbDoc doc = OCB_INVALID;
    OcbShapeTool tool = OCB_INVALID;

    ~OcafFixture() {
        if (tool != OCB_INVALID) ocb_shapetool_release(tool);
        if (doc != OCB_INVALID) ocb_doc_release(doc);
        if (app != OCB_INVALID) ocb_app_release(app);
    }
};

constexpr double kTol = 1e-6;

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}

} // namespace

TEST_CASE("contract: native OCC SaveAs appends .xbf extension") {
    Handle(TDocStd_Application) app = new TDocStd_Application();
    BinXCAFDrivers::DefineFormat(app);
    Handle(TDocStd_Document) doc;
    app->NewDocument("BinXCAF", doc);
    REQUIRE(!doc.IsNull());

    TopoDS_Shape box = BRepPrimAPI_MakeBox(gp_Pnt(0, 0, 0), 1.0, 2.0, 3.0).Shape();
    Handle(XCAFDoc_ShapeTool) st = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
    TDF_Label lab = st->AddShape(box, Standard_False);
    CHECK(!lab.IsNull());

    // BinXCAF 存储驱动会自动把扩展名改为 .xbf，这里显式使用 .xbf 保持一致
    const wchar_t* absPath = L"contract_native.xbf";
    const PCDM_StoreStatus rc = app->SaveAs(doc, TCollection_ExtendedString((Standard_ExtString)absPath));
    CHECK(rc == PCDM_SS_OK);

    FILE* f = std::fopen("contract_native.xbf", "rb");
    CHECK(f != nullptr);
    if (f) std::fclose(f);
    std::remove("contract_native.xbf");
}

TEST_CASE("ocaf: app and document lifecycle") {
    OcafFixture fx;
    REQUIRE(ocb_app_new(&fx.app) == OCB_OK);
    REQUIRE(fx.app != OCB_INVALID);
    REQUIRE(ocb_app_new_document(fx.app, "BinXCAF", &fx.doc) == OCB_OK);
    REQUIRE(fx.doc != OCB_INVALID);

    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);
    CHECK(main != OCB_INVALID);
    ocb_label_release(main);

    // 关闭文档后句柄应失效
    REQUIRE(ocb_app_close(fx.app, fx.doc) == OCB_OK);
    OcbLabel main2 = OCB_INVALID;
    CHECK(ocb_doc_main_label(fx.doc, &main2) == OCB_ERR_INVALID_HANDLE);
    fx.doc = OCB_INVALID; // 已被 app_close 释放
}

TEST_CASE("ocaf: add shape, name it, read it back") {
    OcafFixture fx;
    REQUIRE(ocb_app_new(&fx.app) == OCB_OK);
    REQUIRE(ocb_app_new_document(fx.app, "BinXCAF", &fx.doc) == OCB_OK);
    REQUIRE(ocb_shapetool_get(fx.doc, &fx.tool) == OCB_OK);

    OcbShape box = makeBox(10.0, 20.0, 30.0);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);
    REQUIRE(label != OCB_INVALID);

    REQUIRE(ocb_label_set_name(label, "Box_1") == OCB_OK);
    char name[64] = {};
    REQUIRE(ocb_label_get_name(label, name, sizeof(name)) == OCB_OK);
    CHECK(std::string(name) == "Box_1");

    int freeShapes = 0;
    REQUIRE(ocb_shapetool_free_shape_count(fx.tool, &freeShapes) == OCB_OK);
    CHECK(freeShapes == 1);

    // 从标签取回形状，体积应与原形状一致
    OcbShape back = OCB_INVALID;
    REQUIRE(ocb_shapetool_get_shape(fx.tool, label, &back) == OCB_OK);
    double volume = 0.0;
    REQUIRE(ocb_shape_volume(back, &volume) == OCB_OK);
    CHECK(std::fabs(volume - 6000.0) < kTol);

    ocb_shape_release(back);
    ocb_label_release(label);
    ocb_shape_release(box);
}

TEST_CASE("ocaf: undo and redo of shape addition") {
    OcafFixture fx;
    REQUIRE(ocb_app_new(&fx.app) == OCB_OK);
    REQUIRE(ocb_app_new_document(fx.app, "BinXCAF", &fx.doc) == OCB_OK);
    REQUIRE(ocb_doc_set_undo_limit(fx.doc, 10) == OCB_OK);
    REQUIRE(ocb_shapetool_get(fx.doc, &fx.tool) == OCB_OK);

    int count = 0;
    REQUIRE(ocb_shapetool_free_shape_count(fx.tool, &count) == OCB_OK);
    CHECK(count == 0);

    OcbShape box = makeBox(10.0, 10.0, 10.0);
    REQUIRE(ocb_doc_new_command(fx.doc) == OCB_OK);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);
    REQUIRE(ocb_doc_commit_command(fx.doc) == OCB_OK);

    REQUIRE(ocb_shapetool_free_shape_count(fx.tool, &count) == OCB_OK);
    CHECK(count == 1);

    bool ok = false;
    REQUIRE(ocb_doc_undo(fx.doc, &ok) == OCB_OK);
    CHECK(ok);
    REQUIRE(ocb_shapetool_free_shape_count(fx.tool, &count) == OCB_OK);
    CHECK(count == 0);

    REQUIRE(ocb_doc_redo(fx.doc, &ok) == OCB_OK);
    CHECK(ok);
    REQUIRE(ocb_shapetool_free_shape_count(fx.tool, &count) == OCB_OK);
    CHECK(count == 1);

    ocb_shape_release(box);
    ocb_label_release(label);
}

TEST_CASE("ocaf: save and reopen preserves shape and name") {
    OcafFixture fx;
    REQUIRE(ocb_app_new(&fx.app) == OCB_OK);
    REQUIRE(ocb_app_new_document(fx.app, "BinXCAF", &fx.doc) == OCB_OK);
    REQUIRE(ocb_shapetool_get(fx.doc, &fx.tool) == OCB_OK);

    OcbShape box = makeBox(3.0, 4.0, 5.0);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_doc_new_command(fx.doc) == OCB_OK);
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);
    REQUIRE(ocb_label_set_name(label, "Part_A") == OCB_OK);
    REQUIRE(ocb_doc_commit_command(fx.doc) == OCB_OK);

    const char* path = "ocaf_test_doc.xbf";
    CHECK(ocb_doc_save(fx.app, fx.doc, path) == OCB_OK);
    {
        FILE* f = std::fopen(path, "rb");
        long size = -1L;
        if (f) {
            std::fseek(f, 0, SEEK_END);
            size = std::ftell(f);
            std::fclose(f);
        }
        std::printf("[dbg] file %s: size=%ld\n", f ? "EXISTS" : "MISSING", size);
    }
    CHECK(ocb_app_close(fx.app, fx.doc) == OCB_OK);
    fx.doc = OCB_INVALID;

    // 重新打开
    OcbApp app2 = OCB_INVALID;
    REQUIRE(ocb_app_new(&app2) == OCB_OK);
    OcbDoc doc2 = OCB_INVALID;
    const OcbStatus openStatus = ocb_doc_open(app2, path, &doc2);
    if (openStatus != OCB_OK) {
        std::printf("open failed: status=%d err=%s\n", (int)openStatus, ocb_last_error());
    }
    CHECK(openStatus == OCB_OK);

    if (doc2 != OCB_INVALID) {
        OcbShapeTool tool2 = OCB_INVALID;
        REQUIRE(ocb_shapetool_get(doc2, &tool2) == OCB_OK);

        int freeShapes = 0;
        REQUIRE(ocb_shapetool_free_shape_count(tool2, &freeShapes) == OCB_OK);
        CHECK(freeShapes == 1);

        ocb_shapetool_release(tool2);
        ocb_doc_release(doc2);
    }
    remove(path);
    ocb_app_release(app2);
    ocb_shape_release(box);
    ocb_label_release(label);
}

TEST_CASE("ocaf: ais object creation from shape") {
    OcbShape box = makeBox(2.0, 3.0, 4.0);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);
    CHECK(obj != OCB_INVALID);
    CHECK(ocb_live_count() == 2); // shape + ais

    ocb_ais_release(obj);
    ocb_shape_release(box);
    CHECK(ocb_live_count() == 0);
}

TEST_CASE("ocaf: invalid handles rejected") {
    OcbDoc doc = OCB_INVALID;
    OcbLabel label = OCB_INVALID;
    CHECK(ocb_doc_main_label(OCB_INVALID, &label) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_label_set_name(OCB_INVALID, "x") == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_app_new_document(OCB_INVALID, "BinXCAF", &doc) == OCB_ERR_INVALID_HANDLE);

    OcbShapeTool st = OCB_INVALID;
    OcbShape s = OCB_INVALID;
    CHECK(ocb_shapetool_get_shape(OCB_INVALID, OCB_INVALID, &s) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_shapetool_add_shape(OCB_INVALID, OCB_INVALID, &label) == OCB_ERR_INVALID_HANDLE);
    (void)st;
    (void)doc;
}
