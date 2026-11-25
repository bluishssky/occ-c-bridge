// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cmath>
#include <Aspect_TypeOfMarker.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Graphic3d_Camera.hxx>
#include <Graphic3d_TransModeFlags.hxx>
#include <V3d_View.hxx>

namespace {

#ifdef _WIN32
HWND makeHiddenWindow() {
    static bool registered = false;
    static const wchar_t* cls = L"OcbV13TestHiddenWindow";
    if (!registered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = cls;
        if (RegisterClassExW(&wc) != 0) registered = true;
    }
    return CreateWindowExW(0, cls, L"ocb-v13-test", WS_OVERLAPPED, 10, 10, 640, 480,
                           nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
}
#endif

OcbColor red() {
    return OcbColor{1.0f, 0.0f, 0.0f};
}

OcbShape makeBox(double dx, double dy, double dz) {
    OcbShape s = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), dx, dy, dz, &s) == OCB_OK);
    return s;
}

} // namespace

#ifdef _WIN32

TEST_CASE("v13: view reset, shading model and immediate update") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);

    CHECK(ocb_view_reset(view) == OCB_OK);
    for (int model = OCB_SHADING_COLOR; model <= OCB_SHADING_VERTEX; ++model) {
        CHECK(ocb_view_set_shading_model(view, model) == OCB_OK);
    }
    CHECK(ocb_view_set_shading_model(view, 99) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_view_set_immediate_update(view, false) == OCB_OK);
    CHECK(ocb_view_set_immediate_update(view, true) == OCB_OK);

    CHECK(ocb_view_reset(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_view_set_shading_model(OCB_INVALID, OCB_SHADING_FLAT) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_view_set_immediate_update(OCB_INVALID, true) == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: viewer computed mode, default lights and view size") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);

    CHECK(ocb_viewer_set_computed_mode(viewer, true, true) == OCB_OK);
    CHECK(ocb_viewer_set_default_lights(viewer) == OCB_OK);
    CHECK(ocb_viewer_set_default_view_size(viewer, 2000.0) == OCB_OK);

    CHECK(ocb_viewer_set_computed_mode(OCB_INVALID, true, true) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_viewer_set_default_lights(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_viewer_set_default_view_size(OCB_INVALID, 2000.0) == OCB_ERR_INVALID_HANDLE);

    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: camera get/copy/lerp/set round trip") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);

    V3d_View* nativeView = static_cast<V3d_View*>(ocb_view_native_ptr(view));
    REQUIRE(nativeView != nullptr);

    const Handle(Graphic3d_Camera)& nativeCamera = nativeView->Camera();
    REQUIRE(!nativeCamera.IsNull());
    nativeCamera->SetEyeAndCenter(gp_Pnt(0.0, 0.0, -10.0), gp_Pnt(0.0, 0.0, 0.0));
    nativeCamera->SetUp(gp_Dir(0.0, 1.0, 0.0));
    nativeCamera->SetScale(100.0);

    OcbCamera cameraA = OCB_INVALID;
    REQUIRE(ocb_view_get_camera(view, &cameraA) == OCB_OK);
    OcbCamera cameraACopy = OCB_INVALID;
    REQUIRE(ocb_camera_copy(cameraA, &cameraACopy) == OCB_OK);

    nativeCamera->SetEyeAndCenter(gp_Pnt(2.0, 4.0, -10.0), gp_Pnt(2.0, 4.0, 0.0));
    nativeCamera->SetUp(gp_Dir(0.0, 1.0, 0.0));
    nativeCamera->SetScale(200.0);

    OcbCamera cameraB = OCB_INVALID;
    REQUIRE(ocb_view_get_camera(view, &cameraB) == OCB_OK);
    OcbCamera halfway = OCB_INVALID;
    REQUIRE(ocb_camera_lerp(cameraACopy, cameraB, 0.5, &halfway) == OCB_OK);
    REQUIRE(ocb_view_set_camera(view, halfway) == OCB_OK);

    OcbCamera readBack = OCB_INVALID;
    REQUIRE(ocb_view_get_camera(view, &readBack) == OCB_OK);
    REQUIRE(ocb_view_set_camera(view, readBack) == OCB_OK);

    const Handle(Graphic3d_Camera)& result = nativeView->Camera();
    CHECK(std::abs(result->Eye().X() - 1.0) < 1e-9);
    CHECK(std::abs(result->Eye().Y() - 2.0) < 1e-9);
    CHECK(std::abs(result->Eye().Z() + 10.0) < 1e-9);
    CHECK(std::abs(result->Center().X() - 1.0) < 1e-9);
    CHECK(std::abs(result->Center().Y() - 2.0) < 1e-9);
    CHECK(std::abs(result->Center().Z()) < 1e-9);
    CHECK(std::abs(result->Up().Y() - 1.0) < 1e-9);
    CHECK(std::abs(result->Scale() - 150.0) < 1e-9);

    ocb_camera_release(readBack);
    ocb_camera_release(halfway);
    ocb_camera_release(cameraB);
    ocb_camera_release(cameraACopy);
    ocb_camera_release(cameraA);
    // Releasing an already released camera must be a harmless no-op.
    ocb_camera_release(cameraA);

    CHECK(ocb_view_get_camera(OCB_INVALID, &cameraA) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_view_get_camera(view, nullptr) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_camera_copy(OCB_INVALID, &cameraA) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_camera_lerp(OCB_INVALID, OCB_INVALID, 0.5, &cameraA) == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: AIS highlight style per type") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);
    OcbAisContext ctx = OCB_INVALID;
    REQUIRE(ocb_viewer_context(viewer, &ctx) == OCB_OK);

    for (int type = OCB_HIGHLIGHT_LOCAL_SELECTED; type <= OCB_HIGHLIGHT_DYNAMIC; ++type) {
        CHECK(ocb_ais_ctx_set_highlight_style(ctx, type, 0, red(), 1, 0.25f) == OCB_OK);
    }
    CHECK(ocb_ais_ctx_set_highlight_style(
              ctx, OCB_HIGHLIGHT_SELECTED, 1, red(), 0, 0.0f) == OCB_OK);
    CHECK(ocb_ais_ctx_set_highlight_style(ctx, 99, 0, red(), 1, 0.0f)
          == OCB_ERR_INVALID_ARG);
    CHECK(ocb_ais_ctx_set_highlight_style(ctx, 0, 7, red(), 1, 0.0f)
          == OCB_ERR_INVALID_ARG);
    CHECK(ocb_ais_ctx_set_highlight_style(OCB_INVALID, 0, 0, red(), 1, 0.0f)
          == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: AIS selection style ex") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);
    OcbAisContext ctx = OCB_INVALID;
    REQUIRE(ocb_viewer_context(viewer, &ctx) == OCB_OK);

    CHECK(ocb_ais_ctx_set_selection_style_ex(
              ctx, red(), 1, 0.2f, OCB_ZLAYER_DEFAULT) == OCB_OK);
    CHECK(ocb_ais_ctx_set_selection_style_ex(
              OCB_INVALID, red(), 1, 0.2f, OCB_ZLAYER_DEFAULT) == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: textlabel new and set text") {
    OcbAisObj label = OCB_INVALID;
    REQUIRE(ocb_ais_textlabel_new("O", &label) == OCB_OK);
    CHECK(label != OCB_INVALID);
    CHECK(ocb_ais_textlabel_set_text(label, "Hello") == OCB_OK);

    OcbAisObj ignored = OCB_INVALID;
    CHECK(ocb_ais_textlabel_new(nullptr, &ignored) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_ais_textlabel_new("x", nullptr) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_ais_textlabel_set_text(label, nullptr) == OCB_ERR_INVALID_ARG);

    ocb_ais_release(label);
    // Released handle must be rejected.
    CHECK(ocb_ais_textlabel_set_text(label, "x") == OCB_ERR_INVALID_HANDLE);
}

TEST_CASE("v13: textlabel color and transform persistence") {
    OcbAisObj label = OCB_INVALID;
    REQUIRE(ocb_ais_textlabel_new("O", &label) == OCB_OK);
    CHECK(ocb_ais_textlabel_set_color(label, red()) == OCB_OK);
    CHECK(ocb_ais_textlabel_set_transform_persistence(
              label, Graphic3d_TMF_TriedronPers, Aspect_TOTP_RIGHT_LOWER, 85, 85)
          == OCB_OK);

    OcbShape box = makeBox(1.0, 2.0, 3.0);
    OcbAisObj shapeObj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &shapeObj) == OCB_OK);
    CHECK(ocb_ais_textlabel_set_color(shapeObj, red()) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_textlabel_set_transform_persistence(
              shapeObj, Graphic3d_TMF_TriedronPers, Aspect_TOTP_RIGHT_LOWER, 0, 0)
          == OCB_ERR_INVALID_HANDLE);

    ocb_ais_release(shapeObj);
    ocb_shape_release(box);
    ocb_ais_release(label);
}

TEST_CASE("v13: AIS connected interactive") {
    OcbShape box = makeBox(10.0, 20.0, 30.0);
    OcbAisObj connected = OCB_INVALID;
    REQUIRE(ocb_ais_new_connected(box, &connected) == OCB_OK);
    CHECK(connected != OCB_INVALID);

    CHECK(ocb_ais_new_connected(OCB_INVALID, &connected) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_new_connected(box, nullptr) == OCB_ERR_INVALID_ARG);

    ocb_ais_release(connected);
    ocb_shape_release(box);
}

TEST_CASE("v13: viewer grid echo marker and rectangular values") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);

    CHECK(ocb_viewer_set_grid_echo_marker(viewer, Aspect_TOM_O_PLUS, red(), 1.5)
          == OCB_OK);
    CHECK(ocb_viewer_set_rect_grid_values(viewer, 0.0, 0.0, 10.0, 10.0, 0.0)
          == OCB_OK);
    CHECK(ocb_viewer_set_grid_echo_marker(OCB_INVALID, Aspect_TOM_O_PLUS, red(), 1.0)
          == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_viewer_set_rect_grid_values(OCB_INVALID, 0, 0, 1, 1, 0)
          == OCB_ERR_INVALID_HANDLE);

    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: activate grid and convert to grid") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);

    REQUIRE(ocb_viewer_set_rect_grid_values(viewer, 0.0, 0.0, 5.0, 5.0, 0.0)
            == OCB_OK);
    CHECK(ocb_viewer_activate_grid(viewer, OCB_GRID_RECTANGULAR, OCB_GRID_LINES)
          == OCB_OK);
    CHECK(ocb_viewer_activate_grid(viewer, OCB_GRID_RECTANGULAR, OCB_GRID_POINTS)
          == OCB_OK);
    CHECK(ocb_viewer_activate_grid(viewer, 9, OCB_GRID_LINES) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_viewer_activate_grid(viewer, OCB_GRID_RECTANGULAR, 9)
          == OCB_ERR_INVALID_ARG);
    CHECK(ocb_viewer_activate_grid(OCB_INVALID, OCB_GRID_RECTANGULAR, OCB_GRID_LINES)
          == OCB_ERR_INVALID_HANDLE);

    double gx = 0.0, gy = 0.0, gz = 0.0;
    REQUIRE(ocb_view_convert_to_grid(view, 100, 100, &gx, &gy, &gz) == OCB_OK);
    CHECK(std::isfinite(gx));
    CHECK(std::isfinite(gy));
    CHECK(std::isfinite(gz));
    CHECK(ocb_view_convert_to_grid(view, 100, 100, nullptr, &gy, &gz)
          == OCB_ERR_INVALID_ARG);
    CHECK(ocb_view_convert_to_grid(OCB_INVALID, 100, 100, &gx, &gy, &gz)
          == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13: show grid echo requires a valid view") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);

    CHECK(ocb_viewer_show_grid_echo(viewer, view, 0.0, 0.0, 0.0) == OCB_OK);
    CHECK(ocb_viewer_show_grid_echo(viewer, OCB_INVALID, 0.0, 0.0, 0.0)
          == OCB_ERR_INVALID_ARG);
    CHECK(ocb_viewer_show_grid_echo(OCB_INVALID, view, 0.0, 0.0, 0.0)
          == OCB_ERR_INVALID_HANDLE);

    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

TEST_CASE("v13.1: make wire from N edges") {
    OcbShape e1 = OCB_INVALID, e2 = OCB_INVALID, wire = OCB_INVALID;
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(0, 0, 0), ocb_point_xyz(1, 0, 0), &e1) == OCB_OK);
    REQUIRE(ocb_make_edge_pnt_pnt(ocb_point_xyz(1, 0, 0), ocb_point_xyz(1, 1, 0), &e2) == OCB_OK);

    OcbShape edges[2] = { e1, e2 };
    REQUIRE(ocb_make_wire_from_shapes(edges, 2, &wire) == OCB_OK);
    REQUIRE(wire != OCB_INVALID);
    int wireType = -1;
    REQUIRE(ocb_shape_type(wire, &wireType) == OCB_OK);
    CHECK(wireType == 5); // TopAbs_WIRE

    CHECK(ocb_make_wire_from_shapes(edges, 0, &wire) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_make_wire_from_shapes(edges, 2, nullptr) == OCB_ERR_INVALID_ARG);
    OcbShape bad[2] = { e1, OCB_INVALID };
    CHECK(ocb_make_wire_from_shapes(bad, 2, &wire) == OCB_ERR_INVALID_HANDLE);

    ocb_shape_release(wire);
    ocb_shape_release(e2);
    ocb_shape_release(e1);
}

TEST_CASE("v15: trihedron and viewcube create and configure") {
    OcbAisObj t = OCB_INVALID;
    REQUIRE(ocb_ais_trihedron_new(&t) == OCB_OK);
    REQUIRE(t != OCB_INVALID);
    CHECK(ocb_ais_trihedron_set_datum_display_mode(t, 0) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_draw_arrows(t, true) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_datum_part_color(t, 0, OcbColor{1,0,0}) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_label(t, 0, "X") == OCB_OK);
    CHECK(ocb_ais_trihedron_set_text_color(t, OcbColor{1,0,0}) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_size(t, 300) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_infinite_state(t, true) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_line_style(t, 1, 3.0, 0) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_zoom_persistence(t) == OCB_OK);
    CHECK(ocb_ais_trihedron_set_size(OCB_INVALID, 1) == OCB_ERR_INVALID_HANDLE);
    ocb_ais_release(t);

    OcbAisObj v = OCB_INVALID;
    REQUIRE(ocb_ais_viewcube_new(&v) == OCB_OK);
    REQUIRE(v != OCB_INVALID);
    CHECK(ocb_ais_viewcube_set_colors(v, OcbColor{1,1,1}, OcbColor{0,0,1}, OcbColor{1,0,0}) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_size(v, 55) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_axes_labels(v, "X", "Y", "Z") == OCB_OK);
    CHECK(ocb_ais_viewcube_set_axes_radius(v, 2) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_transparency(v, 0.1) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_decor(v, true, true, 22, 40, 8) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_font(v, "SimHei", 22) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_box_side_label(v, 3, "Front") == OCB_OK);
    CHECK(ocb_ais_viewcube_set_triedron_pers(v, 3, 85, 105) == OCB_OK);
    CHECK(ocb_ais_viewcube_set_size(OCB_INVALID, 1) == OCB_ERR_INVALID_HANDLE);
    ocb_ais_release(v);
}

TEST_CASE("v142: label child is parent-relative and new_child creates") {
    OcbApp app = OCB_INVALID;
    OcbDoc doc = OCB_INVALID;
    REQUIRE(ocb_app_new(&app) == OCB_OK);
    REQUIRE(ocb_app_new_document(app, "BinXCAF", &doc) == OCB_OK);
    OcbLabel main = OCB_INVALID;
    REQUIRE(ocb_doc_main_label(doc, &main) == OCB_OK);

    OcbLabel c10 = OCB_INVALID;
    REQUIRE(ocb_label_new_child(main, 10, &c10) == OCB_OK);
    REQUIRE(c10 != OCB_INVALID);

    OcbLabel c10b = OCB_INVALID;
    REQUIRE(ocb_label_child(main, 10, &c10b) == OCB_OK);
    bool same = false;
    REQUIRE(ocb_label_is_equal(c10, c10b, &same) == OCB_OK);
    CHECK(same);

    // Lookup must not create.
    OcbLabel c99 = OCB_INVALID;
    CHECK(ocb_label_child(main, 99, &c99) == OCB_ERR_NOT_IMPLEMENTED);

    // Parent-relative: tag 1 exists under c10 but not under main.
    OcbLabel n1 = OCB_INVALID;
    REQUIRE(ocb_label_new_child(c10, 1, &n1) == OCB_OK);
    OcbLabel n1b = OCB_INVALID;
    REQUIRE(ocb_label_child(c10, 1, &n1b) == OCB_OK);
    REQUIRE(ocb_label_is_equal(n1, n1b, &same) == OCB_OK);
    CHECK(same);

    OcbLabel wrong = OCB_INVALID;
    CHECK(ocb_label_child(main, 1, &wrong) == OCB_ERR_NOT_IMPLEMENTED);

    ocb_label_release(n1b);
    ocb_label_release(n1);
    ocb_label_release(c10b);
    ocb_label_release(c10);
    ocb_label_release(main);
    ocb_doc_release(doc);
    ocb_app_release(app);
}

TEST_CASE("v14: topo count faces and compound first") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 2, 3, 4, &box) == OCB_OK);
    int faces = -1;
    REQUIRE(ocb_topo_count_faces(box, &faces) == OCB_OK);
    CHECK(faces == 6);

    OcbShape box2 = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(10, 0, 0), 2, 3, 4, &box2) == OCB_OK);
    OcbShape comp = OCB_INVALID;
    REQUIRE(ocb_topo_make_compound(&comp) == OCB_OK);
    REQUIRE(ocb_topo_add(comp, box) == OCB_OK);
    REQUIRE(ocb_topo_add(comp, box2) == OCB_OK);
    int compFaces = -1;
    REQUIRE(ocb_topo_count_faces(comp, &compFaces) == OCB_OK);
    CHECK(compFaces == 12);

    OcbShape first = OCB_INVALID;
    REQUIRE(ocb_shape_compound_first(comp, &first) == OCB_OK);
    CHECK(first != OCB_INVALID);
    CHECK(ocb_shape_compound_first(comp, nullptr) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_shape_compound_first(OCB_INVALID, &first) == OCB_ERR_INVALID_HANDLE);

    ocb_shape_release(first);
    ocb_shape_release(comp);
    ocb_shape_release(box2);
    ocb_shape_release(box);
}

TEST_CASE("v14: stepwriter material mode") {
    OcbStepWriter w = OCB_INVALID;
    REQUIRE(ocb_stepwriter_create(&w) == OCB_OK);
    CHECK(ocb_stepwriter_set_material_mode(w, true) == OCB_OK);
    CHECK(ocb_stepwriter_set_material_mode(w, false) == OCB_OK);
    CHECK(ocb_stepwriter_set_material_mode(OCB_INVALID, true) == OCB_ERR_INVALID_HANDLE);
    ocb_stepwriter_release(w);
}

TEST_CASE("v13.1: ais context erase_all") {
    HWND hwnd = makeHiddenWindow();
    REQUIRE(hwnd != nullptr);

    OcbViewer viewer = OCB_INVALID;
    REQUIRE(ocb_viewer_create(hwnd, &viewer) == OCB_OK);
    OcbView view = OCB_INVALID;
    REQUIRE(ocb_view_create(viewer, hwnd, &view) == OCB_OK);
    OcbAisContext ctx = OCB_INVALID;
    REQUIRE(ocb_viewer_context(viewer, &ctx) == OCB_OK);

    CHECK(ocb_ais_ctx_erase_all(ctx, false) == OCB_OK);
    CHECK(ocb_ais_ctx_erase_all(OCB_INVALID, false) == OCB_ERR_INVALID_HANDLE);

    bool valid = false;
    CHECK(ocb_ais_ctx_is_valid(ctx, &valid) == OCB_OK);
    CHECK(valid);
    CHECK(ocb_ais_ctx_is_valid(OCB_INVALID, &valid) == OCB_OK);
    CHECK(!valid);
    CHECK(ocb_ais_ctx_is_valid(ctx, nullptr) == OCB_ERR_INVALID_ARG);

    CHECK(ocb_ais_ctx_clear_detected(ctx, false) == OCB_OK);
    CHECK(ocb_ais_ctx_clear_detected(OCB_INVALID, false) == OCB_ERR_INVALID_HANDLE);

    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 1, 1, 1, &box) == OCB_OK);
    OcbAisObj obj = OCB_INVALID;
    REQUIRE(ocb_ais_new_shape(box, &obj) == OCB_OK);
    CHECK(ocb_ais_ctx_update(ctx, obj, false) == OCB_OK);
    CHECK(ocb_ais_ctx_update(ctx, OCB_INVALID, false) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_ais_ctx_update(OCB_INVALID, obj, false) == OCB_ERR_INVALID_HANDLE);
    ocb_ais_release(obj);
    ocb_shape_release(box);

    ocb_ais_ctx_release(ctx);
    ocb_view_release(view);
    ocb_viewer_release(viewer);
    DestroyWindow(hwnd);
}

#endif
