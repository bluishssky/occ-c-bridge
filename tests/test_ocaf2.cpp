// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>
#include <cstring>

namespace {
constexpr double kTol = 1e-6;

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}

struct Fixture {
    OcbApp app = OCB_INVALID;
    OcbDoc doc = OCB_INVALID;
    OcbShapeTool tool = OCB_INVALID;

    Fixture() {
        REQUIRE(ocb_app_new(&app) == OCB_OK);
        REQUIRE(ocb_app_new_document(app, "BinXCAF", &doc) == OCB_OK);
        REQUIRE(ocb_doc_set_undo_limit(doc, 10) == OCB_OK);
        REQUIRE(ocb_shapetool_get(doc, &tool) == OCB_OK);
    }
    ~Fixture() {
        ocb_shapetool_release(tool);
        ocb_doc_release(doc);
        ocb_app_release(app);
    }
};
} // namespace

TEST_CASE("ocaf2: label entry, tag, father") {
    Fixture fx;
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1, 1, 1, &box) == OCB_OK);

    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);

    char entry[64] = {};
    REQUIRE(ocb_label_entry(label, entry, sizeof(entry)) == OCB_OK);
    CHECK(entry[0] == '0');
    CHECK(entry[1] == ':');

    int tag = 0;
    REQUIRE(ocb_label_tag(label, &tag) == OCB_OK);
    CHECK(tag >= 1);

    OcbLabel father = OCB_INVALID;
    REQUIRE(ocb_label_father(label, &father) == OCB_OK);

    // father 与 label 不是同一个标签（label 是 free shape，father 是 origin）
    bool equal = true;
    REQUIRE(ocb_label_is_equal(label, father, &equal) == OCB_OK);
    CHECK(!equal);

    ocb_label_release(father);
    ocb_label_release(label);
    ocb_shape_release(box);
}

TEST_CASE("ocaf2: label get shape matches added shape") {
    Fixture fx;
    OcbShape box = makeBox(3.0, 4.0, 5.0);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);

    OcbShape back = OCB_INVALID;
    REQUIRE(ocb_label_get_shape(label, &back) == OCB_OK);
    double v = 0.0;
    REQUIRE(ocb_shape_volume(back, &v) == OCB_OK);
    CHECK(std::fabs(v - 60.0) < 1e-4);

    ocb_shape_release(back);
    ocb_label_release(label);
    ocb_shape_release(box);
}

TEST_CASE("ocaf2: set/get real attribute") {
    Fixture fx;
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);

    REQUIRE(ocb_label_set_real(main, 42.5) == OCB_OK);
    double value = 0.0;
    REQUIRE(ocb_label_get_real(main, &value) == OCB_OK);
    CHECK(std::fabs(value - 42.5) < kTol);

    ocb_label_release(main);
}

TEST_CASE("ocaf2: assembly add component and reference navigation") {
    Fixture fx;
    OcbShape box = makeBox(5, 5, 5);

    // 组件（普通 shape）
    OcbLabel comp = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &comp) == OCB_OK);

    // 装配（空 new shape）
    OcbLabel assembly = OCB_INVALID;
    REQUIRE(ocb_doc_new_command(fx.doc) == OCB_OK);
    REQUIRE(ocb_shapetool_new_shape(fx.tool, &assembly) == OCB_OK);

    OcbTrsf identity = ocb_trsf_identity();
    OcbLabel ref = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_component(fx.tool, assembly, comp, identity, &ref) == OCB_OK);
    REQUIRE(ocb_doc_commit_command(fx.doc) == OCB_OK);

    bool isAssembly = false;
    REQUIRE(ocb_shapetool_is_assembly(fx.tool, assembly, &isAssembly) == OCB_OK);
    CHECK(isAssembly);

    bool isRef = false;
    const OcbStatus refStatus = ocb_shapetool_is_reference(fx.tool, ref, &isRef);
    REQUIRE(refStatus == OCB_OK);
    CHECK(isRef);

    OcbLabel referred = OCB_INVALID;
    REQUIRE(ocb_shapetool_get_referred_shape(fx.tool, ref, &referred) == OCB_OK);
    bool same = false;
    REQUIRE(ocb_label_is_equal(referred, comp, &same) == OCB_OK);
    CHECK(same);

    int compCount = 0;
    REQUIRE(ocb_shapetool_get_component_count(fx.tool, assembly, &compCount) == OCB_OK);
    CHECK(compCount == 1);

    OcbLabel comp2 = OCB_INVALID;
    REQUIRE(ocb_shapetool_get_component(fx.tool, assembly, 0, &comp2) == OCB_OK);
    bool same2 = false;
    REQUIRE(ocb_label_is_equal(ref, comp2, &same2) == OCB_OK);
    CHECK(same2);

    int userCount = 0;
    REQUIRE(ocb_shapetool_get_user_count(fx.tool, comp, &userCount) == OCB_OK);
    CHECK(userCount == 1);

    ocb_label_release(comp2);
    ocb_label_release(referred);
    ocb_label_release(ref);
    ocb_label_release(assembly);
    ocb_label_release(comp);
    ocb_shape_release(box);
}

TEST_CASE("ocaf2: set/get location round trip") {
    Fixture fx;
    OcbShape box = makeBox(5, 5, 5);
    OcbLabel comp = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &comp) == OCB_OK);

    OcbLabel assembly = OCB_INVALID;
    REQUIRE(ocb_doc_new_command(fx.doc) == OCB_OK);
    REQUIRE(ocb_shapetool_new_shape(fx.tool, &assembly) == OCB_OK);

    OcbTrsf move = ocb_trsf_translation(ocb_vec_xyz(10, 20, 30));
    OcbLabel ref = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_component(fx.tool, assembly, comp, move, &ref) == OCB_OK);
    REQUIRE(ocb_doc_commit_command(fx.doc) == OCB_OK);

    OcbTrsf got{};
    REQUIRE(ocb_shapetool_get_location(fx.tool, ref, &got) == OCB_OK);
    CHECK(std::fabs(got.v[3] - 10.0) < 1e-6);
    CHECK(std::fabs(got.v[7] - 20.0) < 1e-6);
    CHECK(std::fabs(got.v[11] - 30.0) < 1e-6);

    ocb_label_release(ref);
    ocb_label_release(assembly);
    ocb_label_release(comp);
    ocb_shape_release(box);
}

TEST_CASE("ocaf2: colortool set color") {
    Fixture fx;
    OcbShape box = makeBox(1, 1, 1);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);

    OcbColorTool colorTool = OCB_INVALID;
    REQUIRE(ocb_colortool_get(fx.doc, &colorTool) == OCB_OK);

    const OcbColor red{1.0f, 0.0f, 0.0f};
    CHECK(ocb_colortool_set_color(colorTool, label, red, 0) == OCB_OK); // XCAFDoc_ColorGen

    bool hasColor = false;
    OcbColor got{0.0f, 0.0f, 0.0f};
    REQUIRE(ocb_colortool_get_color(colorTool, label, 0, &hasColor, &got) == OCB_OK);
    CHECK(hasColor);
    CHECK(std::fabs(got.r - 1.0f) < 0.01f);
    CHECK(std::fabs(got.g - 0.0f) < 0.01f);

    // 子形状 API:box 没有子形状,计数应为 0
    int subCount = -1;
    REQUIRE(ocb_shapetool_get_sub_shape_count(fx.tool, label, &subCount) == OCB_OK);
    CHECK(subCount == 0);
    CHECK(ocb_shapetool_get_sub_shape(fx.tool, label, 0, &label) == OCB_ERR_INVALID_ARG);

    // 子形状着色
    CHECK(ocb_colortool_set_shape_color(colorTool, box, red, 1 /*ColorSurf*/) == OCB_OK);
    CHECK(ocb_colortool_set_shape_color(OCB_INVALID, box, red, 1) == OCB_ERR_INVALID_HANDLE);

    // 双旗标 AddShape:简单形状不建 assembly 图
    OcbLabel exLabel = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape_ex(fx.tool, box, false, true, &exLabel) == OCB_OK);
    CHECK(exLabel != OCB_INVALID);

    ocb_label_release(exLabel);
    ocb_label_release(label);

    ocb_colortool_release(colorTool);
    ocb_shape_release(box);
}

TEST_CASE("ocaf2: label is_valid") {
    Fixture fx;
    OcbShape box = makeBox(1, 1, 1);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &label) == OCB_OK);

    bool v = false;
    REQUIRE(ocb_label_is_valid(label, &v) == OCB_OK);
    CHECK(v);

    ocb_label_release(label);          // 释放登记项后句柄失效
    REQUIRE(ocb_label_is_valid(label, &v) == OCB_OK);
    CHECK(!v);

    REQUIRE(ocb_label_is_valid(OCB_INVALID, &v) == OCB_OK);
    CHECK(!v);

    ocb_shape_release(box);
}

TEST_CASE("ocaf2: doc command state") {
    Fixture fx;
    bool open = true;
    REQUIRE(ocb_doc_has_open_command(fx.doc, &open) == OCB_OK);
    CHECK(!open);

    REQUIRE(ocb_doc_new_command(fx.doc) == OCB_OK);
    REQUIRE(ocb_doc_has_open_command(fx.doc, &open) == OCB_OK);
    CHECK(open);

    REQUIRE(ocb_doc_abort_command(fx.doc) == OCB_OK);
    REQUIRE(ocb_doc_has_open_command(fx.doc, &open) == OCB_OK);
    CHECK(!open);
}
