// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

#ifdef _WIN32
HWND makeHiddenWindow()
{
    static bool registered = false;
    static const wchar_t* cls = L"OcbV12TestHiddenWindow";
    if (!registered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = cls;
        if (RegisterClassExW(&wc) != 0) registered = true;
    }
    return CreateWindowExW(0, cls, L"ocb-v12-test", WS_OVERLAPPED, 10, 10, 640, 480,
                           nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
}
#endif

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
        REQUIRE(ocb_shapetool_get(doc, &tool) == OCB_OK);
    }
    ~Fixture() {
        ocb_shapetool_release(tool);
        ocb_doc_release(doc);
        ocb_app_release(app);
    }
};

bool containsKind(int* kinds, int count, int wanted) {
    for (int i = 0; i < count; ++i) {
        if (kinds[i] == wanted) return true;
    }
    return false;
}

} // namespace

TEST_CASE("v12p0: label child count/at/has_child over a 3-level tree") {
    Fixture fx;

    // ocb_doc_main_label 返回 XCAF 文档标签；其子标签 1/2 由 ShapeTool/ColorTool 创建。
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);

    OcbColorTool colorTool = OCB_INVALID;
    REQUIRE(ocb_colortool_get(fx.doc, &colorTool) == OCB_OK);

    int count = -1;
    REQUIRE(ocb_label_child_count(main, &count) == OCB_OK);
    CHECK(count == 2);

    bool hasChild = false;
    REQUIRE(ocb_label_has_child(main, &hasChild) == OCB_OK);
    CHECK(hasChild);

    OcbLabel shapes = OCB_INVALID;
    OcbLabel colors = OCB_INVALID;
    REQUIRE(ocb_label_child_at(main, 0, &shapes) == OCB_OK);
    REQUIRE(ocb_label_child_at(main, 1, &colors) == OCB_OK);

    int tag = -1;
    REQUIRE(ocb_label_tag(shapes, &tag) == OCB_OK);
    CHECK(tag == 1); // ShapesLabel
    REQUIRE(ocb_label_tag(colors, &tag) == OCB_OK);
    CHECK(tag == 2); // ColorsLabel

    // 第三层（孙）：ShapesLabel 下新增一个 shape 标签。
    bool shapesHasChild = true;
    REQUIRE(ocb_label_has_child(shapes, &shapesHasChild) == OCB_OK);
    CHECK(!shapesHasChild);

    OcbShape box = makeBox(1.0, 2.0, 3.0);
    OcbLabel shapeLabel = OCB_INVALID;
    REQUIRE(ocb_shapetool_add_shape(fx.tool, box, &shapeLabel) == OCB_OK);

    REQUIRE(ocb_label_child_count(shapes, &count) == OCB_OK);
    CHECK(count == 1);
    REQUIRE(ocb_label_has_child(shapes, &shapesHasChild) == OCB_OK);
    CHECK(shapesHasChild);

    OcbLabel grandChild = OCB_INVALID;
    REQUIRE(ocb_label_child_at(shapes, 0, &grandChild) == OCB_OK);
    bool same = false;
    REQUIRE(ocb_label_is_equal(grandChild, shapeLabel, &same) == OCB_OK);
    CHECK(same);

    bool colorsHasChild = true;
    REQUIRE(ocb_label_has_child(colors, &colorsHasChild) == OCB_OK);
    CHECK(!colorsHasChild);

    // 越界 / 负索引 / 空 out。
    OcbLabel tmp = OCB_INVALID;
    CHECK(ocb_label_child_at(main, 2, &tmp) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_label_child_at(main, -1, &tmp) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_label_child_at(main, 0, nullptr) == OCB_ERR_INVALID_ARG);

    // 无效句柄。
    CHECK(ocb_label_child_count(OCB_INVALID, &count) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_label_child_at(OCB_INVALID, 0, &tmp) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_label_has_child(OCB_INVALID, &hasChild) == OCB_ERR_INVALID_HANDLE);

    ocb_label_release(grandChild);
    ocb_label_release(shapeLabel);
    ocb_shape_release(box);
    ocb_label_release(shapes);
    ocb_label_release(colors);
    ocb_colortool_release(colorTool);
    ocb_label_release(main);
}

TEST_CASE("v12p0: label integer attribute round trip") {
    Fixture fx;
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);

    int value = 0;
    CHECK(ocb_label_get_int(main, &value) == OCB_ERR_NOT_IMPLEMENTED);

    REQUIRE(ocb_label_set_int(main, 1234) == OCB_OK);
    REQUIRE(ocb_label_get_int(main, &value) == OCB_OK);
    CHECK(value == 1234);

    REQUIRE(ocb_label_set_int(main, -4321) == OCB_OK);
    REQUIRE(ocb_label_get_int(main, &value) == OCB_OK);
    CHECK(value == -4321);

    CHECK(ocb_label_get_int(main, nullptr) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_label_set_int(OCB_INVALID, 1) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_label_get_int(OCB_INVALID, &value) == OCB_ERR_INVALID_HANDLE);

    ocb_label_release(main);
}

TEST_CASE("v12p0: label attribute count and kind classification") {
    Fixture fx;
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);

    int count = -1;
    REQUIRE(ocb_label_attribute_count(main, &count) == OCB_OK);
    CHECK(count == 0);

    int kind = -1;
    CHECK(ocb_label_attribute_kind_at(main, 0, &kind) == OCB_ERR_INVALID_ARG);

    REQUIRE(ocb_label_set_name(main, "v12p0") == OCB_OK);
    REQUIRE(ocb_label_attribute_count(main, &count) == OCB_OK);
    REQUIRE(count == 1);
    REQUIRE(ocb_label_attribute_kind_at(main, 0, &kind) == OCB_OK);
    CHECK(kind == OCB_ATTR_NAME);

    REQUIRE(ocb_label_set_int(main, 77) == OCB_OK);
    REQUIRE(ocb_label_set_real(main, 2.5) == OCB_OK);
    REQUIRE(ocb_label_attribute_count(main, &count) == OCB_OK);
    REQUIRE(count == 3);

    int kinds[8] = {};
    for (int i = 0; i < count; ++i) {
        REQUIRE(ocb_label_attribute_kind_at(main, i, &kinds[i]) == OCB_OK);
    }
    CHECK(containsKind(kinds, count, OCB_ATTR_NAME));
    CHECK(containsKind(kinds, count, OCB_ATTR_INT));
    CHECK(containsKind(kinds, count, OCB_ATTR_REAL));

    CHECK(ocb_label_attribute_kind_at(main, count, &kind) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_label_attribute_kind_at(main, -1, &kind) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_label_attribute_kind_at(main, 0, nullptr) == OCB_ERR_INVALID_ARG);

    ocb_label_release(main);
}

TEST_CASE("v12p0: forget_attributes clears attributes but keeps label and children") {
    Fixture fx;
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(fx.doc, &main) == OCB_OK);

    REQUIRE(ocb_label_set_name(main, "to-forget") == OCB_OK);
    REQUIRE(ocb_label_set_int(main, 9) == OCB_OK);
    REQUIRE(ocb_label_set_real(main, 1.25) == OCB_OK);
    int count = -1;
    REQUIRE(ocb_label_attribute_count(main, &count) == OCB_OK);
    REQUIRE(count == 3);

    REQUIRE(ocb_label_forget_attributes(main) == OCB_OK);
    REQUIRE(ocb_label_attribute_count(main, &count) == OCB_OK);
    CHECK(count == 0);

    bool valid = false;
    REQUIRE(ocb_label_is_valid(main, &valid) == OCB_OK);
    CHECK(valid);

    // ForgetAllAttributes(false) 不清理子标签：ShapeTool 标签仍在。
    bool hasChild = false;
    REQUIRE(ocb_label_has_child(main, &hasChild) == OCB_OK);
    CHECK(hasChild);

    CHECK(ocb_label_forget_attributes(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);

    ocb_label_release(main);
}

TEST_CASE("v12p0: per-object selection mode activate/deactivate") {
#ifndef _WIN32
    return;
#else
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);
    OcbAisContext ctx = OCB_INVALID;
    REQUIRE(ocb_viewer_context(viewer, &ctx) == OCB_OK);

    OcbShape box = makeBox(10.0, 20.0, 30.0);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);
    REQUIRE(ocb_ais_ctx_display(ctx, obj, false) == OCB_OK);

    // mode 4 = face（AIS_SelectionMode 直接透传）。
    CHECK(ocb_ais_ctx_set_selection_mode_active(ctx, obj, 4, true) == OCB_OK);
    CHECK(ocb_ais_ctx_set_selection_mode_active(ctx, obj, 4, false) == OCB_OK);
    CHECK(ocb_ais_ctx_set_selection_mode_active(ctx, obj, 0, true) == OCB_OK);
    CHECK(ocb_ais_ctx_set_selection_mode_active(ctx, obj, 0, false) == OCB_OK);

    CHECK(ocb_ais_ctx_set_selection_mode_active(OCB_INVALID, obj, 4, true) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_ctx_set_selection_mode_active(ctx, OCB_INVALID, 4, true) == OCB_ERR_INVALID_HANDLE);

    ocb_ais_release(obj);
    ocb_shape_release(box);
    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
#endif
}

TEST_CASE("v12p0: interface static string parameter") {
    // 先创建 STEP writer，确保 STEP 的 Interface_Static 参数表已初始化。
    OcbStepWriter writer = OCB_INVALID;
    REQUIRE(ocb_stepwriter_create(&writer) == OCB_OK);

    CHECK(ocb_interface_set_string("write.step.schema", "AP203") == OCB_OK);

    CHECK(ocb_interface_set_string("no.such.key", "x") == OCB_ERR_INVALID_ARG);
    CHECK(ocb_interface_set_string(nullptr, "x") == OCB_ERR_INVALID_ARG);
    CHECK(ocb_interface_set_string("write.step.schema", nullptr) == OCB_ERR_INVALID_ARG);

    ocb_stepwriter_release(writer);
}
