// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>
#include <cstdio>

namespace {
constexpr double kTol = 1e-3;
} // namespace

TEST_CASE("io: STEP write and read round trip") {
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10.0, 20.0, 30.0, &box) == OCB_OK);

    const char* path = "io_roundtrip.step";
    REQUIRE(ocb_step_write(box, path) == OCB_OK);

    FILE* f = std::fopen(path, "rb");
    REQUIRE(f != nullptr);
    std::fclose(f);

    OcbShape back = OCB_INVALID;
    REQUIRE(ocb_step_read(path, &back) == OCB_OK);

    double v = 0.0;
    REQUIRE(ocb_shape_volume(back, &v) == OCB_OK);
    CHECK(std::fabs(v - 6000.0) < 1.0);

    ocb_shape_release(back);
    ocb_shape_release(box);
    std::remove(path);
}

TEST_CASE("io: STEP read rejects missing file") {
    OcbShape s = OCB_INVALID;
    CHECK(ocb_step_read("no_such_file_for_ocb_test.step", &s) == OCB_ERR_IO);
}

TEST_CASE("io: STEPCAF transfer into document") {
    OcbApp app = OCB_INVALID;
    OcbDoc doc = OCB_INVALID;
    REQUIRE(ocb_app_new(&app) == OCB_OK);
    REQUIRE(ocb_app_new_document(app, "BinXCAF", &doc) == OCB_OK);
    REQUIRE(ocb_doc_set_undo_limit(doc, 10) == OCB_OK);

    // 用原生链路造一个文档再存成 STEP：先造 shape
    OcbShape box = OCB_INVALID;
    REQUIRE(ocb_shape_make_box(ocb_point_xyz(0, 0, 0), 10.0, 20.0, 30.0, &box) == OCB_OK);

    OcbShapeTool tool = OCB_INVALID;
    REQUIRE(ocb_shapetool_get(doc, &tool) == OCB_OK);
    OcbLabel label = OCB_INVALID;
    REQUIRE(ocb_doc_new_command(doc) == OCB_OK);
    REQUIRE(ocb_shapetool_add_shape(tool, box, &label) == OCB_OK);
    REQUIRE(ocb_doc_commit_command(doc) == OCB_OK);

    // 写 STEP（用简单写即可）
    const char* path = "io_caf.step";
    REQUIRE(ocb_step_write(box, path) == OCB_OK);

    // 用 STEPCAF Reader 读入另一个文档
    OcbStepReader reader = OCB_INVALID;
    REQUIRE(ocb_stepreader_create(&reader) == OCB_OK);
    REQUIRE(ocb_stepreader_set_name_mode(reader, true) == OCB_OK);

    OcbApp app2 = OCB_INVALID;
    OcbDoc doc2 = OCB_INVALID;
    REQUIRE(ocb_app_new(&app2) == OCB_OK);
    REQUIRE(ocb_app_new_document(app2, "BinXCAF", &doc2) == OCB_OK);
    REQUIRE(ocb_stepreader_read_file(reader, path) == OCB_OK);
    REQUIRE(ocb_stepreader_transfer(reader, doc2) == OCB_OK);

    // 验证 doc2 中的 shape 数量
    OcbShapeTool tool2 = OCB_INVALID;
    REQUIRE(ocb_shapetool_get(doc2, &tool2) == OCB_OK);
    int count = 0;
    REQUIRE(ocb_shapetool_free_shape_count(tool2, &count) == OCB_OK);
    CHECK(count >= 1);

    ocb_stepreader_release(reader);
    ocb_shapetool_release(tool2);
    ocb_doc_release(doc2);
    ocb_app_release(app2);
    ocb_label_release(label);
    ocb_shapetool_release(tool);
    ocb_shape_release(box);
    ocb_doc_release(doc);
    ocb_app_release(app);
    std::remove(path);
}
