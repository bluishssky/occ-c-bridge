// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
// 最小消费者：只包含 occ_c_bridge.h（不引任何 OpenCASCADE 头文件）。
// 验证点：
//   1) 头文件自包含（能单独编译）
//   2) 基本几何/拓扑/文档 API 可用
//   3) 句柄释放不崩（进程正常退出）
#include "occ_c_bridge.h"

#include <cmath>
#include <cstdio>

static int g_fail = 0;

static void check(bool ok, const char* what)
{
    std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

int main()
{
    // ---- 版本 ----
    const char* ver = ocb_version();
    check(ver != nullptr && ver[0] != '\0', "ocb_version");

    // ---- 盒体：体积/包围盒/拓扑 ----
    OcbShape box = OCB_INVALID;
    check(ocb_shape_make_box(OcbPnt{0, 0, 0}, 10, 20, 30, &box) == OCB_OK, "make_box");

    double vol = 0.0;
    check(ocb_shape_volume(box, &vol) == OCB_OK && std::fabs(vol - 6000.0) < 1e-6, "volume == 6000");

    OcbBox bb{};
    check(ocb_shape_bounding_box(box, &bb) == OCB_OK && bb.valid &&
          std::fabs(bb.xmax - 10) < 1e-5 && std::fabs(bb.zmax - 30) < 1e-5, "bbox");

    int solids = 0;
    check(ocb_topo_count_solids(box, &solids) == OCB_OK && solids == 1, "count_solids == 1");

    // ---- 变换：平移后包围盒移动 ----
    OcbShape moved = OCB_INVALID;
    const OcbTrsf t = ocb_trsf_translation(OcbVec{5, 0, 0});
    check(ocb_shape_moved(box, t, &moved) == OCB_OK, "shape_moved");
    OcbBox mb{};
    if (ocb_shape_bounding_box(moved, &mb) == OCB_OK && mb.valid)
        check(std::fabs(mb.xmin - 5) < 1e-5, "moved bbox xmin == 5");
    else
        check(false, "moved bbox");

    // ---- 距离 ----
    OcbShape box2 = OCB_INVALID;
    ocb_shape_make_box(OcbPnt{20, 0, 0}, 10, 20, 30, &box2);
    double dist = -1.0;
    check(ocb_shape_distance(box, box2, &dist) == OCB_OK && std::fabs(dist - 10.0) < 1e-6,
          "distance == 10");

    // ---- 面遍历（explorer）----
    OcbExplorer ex = OCB_INVALID;
    int faces = 0;
    if (ocb_explorer_new_faces(box, &ex) == OCB_OK && ex != OCB_INVALID) {
        bool more = false;
        for (ocb_explorer_more(ex, &more); more; ocb_explorer_next(ex), ocb_explorer_more(ex, &more))
            ++faces;
        ocb_explorer_release(ex);
    }
    check(faces == 6, "6 faces");

    // ---- 文档/标签（OCAF 最小链路）----
    OcbApp app = OCB_INVALID;
    OcbDoc doc = OCB_INVALID;
    if (ocb_app_new(&app) == OCB_OK && ocb_app_new_document(app, "BinXCAF", &doc) == OCB_OK) {
        OcbShapeTool st = OCB_INVALID;
        OcbLabel label = OCB_INVALID;
        bool ok = (ocb_shapetool_get(doc, &st) == OCB_OK) &&
                  (ocb_shapetool_add_shape(st, box, &label) == OCB_OK) &&
                  (label != OCB_INVALID);
        check(ok, "ocaf add shape");
        if (st != OCB_INVALID) ocb_shapetool_release(st);
        if (label != OCB_INVALID) ocb_label_release(label);
        ocb_doc_release(doc);
    }
    else {
        check(false, "ocaf app/doc");
    }
    if (app != OCB_INVALID) ocb_app_release(app);

    // ---- 释放 ----
    if (box2 != OCB_INVALID) ocb_shape_release(box2);
    if (moved != OCB_INVALID) ocb_shape_release(moved);
    if (box != OCB_INVALID) ocb_shape_release(box);

    std::printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
