// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#ifndef OCC_C_GUARD_H
#define OCC_C_GUARD_H

// =====================================================================
// occ-c-bridge C++ companion: RAII 句柄守卫
//
// 所有权契约（详见 docs/ownership-and-handle-lifecycle-plan.md）：
//   * 每个 ocb_* API 返回的句柄都是一份独立所有权登记（id 永不复用）；
//   * 临时句柄用 Guard 包裹 -> 作用域结束自动 release，杜绝漏释放；
//   * 所有权转移（存容器/存成员）必须显式 releaseOwnership()，
//     转移后 Guard 析构不再释放；
//   * addr() 允许把 Guard 直接当作桥 API 的 out 参数（登记即接管）。
//
// 仅依赖 occ_c_bridge.h / occ_c_types.h，header-only。
// =====================================================================

#include "occ_c_bridge.h"
#include "occ_c_types.h"

namespace occ_guard {

template <void (OCB_CALL *ReleaseFn)(uint64_t)>
struct HandleGuard {
    uint64_t h = OCB_INVALID;

    HandleGuard() = default;
    explicit HandleGuard(uint64_t handle) : h(handle) {}

    // 一柄一份所有权：禁拷贝，允许移动（移动即所有权转移）
    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator=(const HandleGuard&) = delete;
    HandleGuard(HandleGuard&& o) : h(o.h) { o.h = OCB_INVALID; }
    HandleGuard& operator=(HandleGuard&& o) {
        if (this != &o) {
            reset();
            h = o.h;
            o.h = OCB_INVALID;
        }
        return *this;
    }

    ~HandleGuard() { reset(); }

    uint64_t get() const { return h; }
    uint64_t* addr() { return &h; }              // 直接作为桥 API 的 out 参数

    // 所有权转出（存入容器/成员后由其负责释放），Guard 析构将不再释放
    uint64_t releaseOwnership() {
        uint64_t t = h;
        h = OCB_INVALID;
        return t;
    }

    // 接管他人转出的句柄（原持有者已 releaseOwnership）
    void adopt(uint64_t v) {
        reset();
        h = v;
    }

    void reset() {
        if (h != OCB_INVALID) {
            ReleaseFn(h);
            h = OCB_INVALID;
        }
    }
};

} // namespace occ_guard

// 按句柄类型的便捷别名（release 函数均为 void OCB_CALL (uint64_t) 签名）
using OcbLabelGuard      = occ_guard::HandleGuard<&ocb_label_release>;
using OcbShapeGuard      = occ_guard::HandleGuard<&ocb_shape_release>;
using OcbAppGuard        = occ_guard::HandleGuard<&ocb_app_release>;
using OcbDocGuard        = occ_guard::HandleGuard<&ocb_doc_release>;
using OcbShapeToolGuard  = occ_guard::HandleGuard<&ocb_shapetool_release>;
using OcbColorToolGuard  = occ_guard::HandleGuard<&ocb_colortool_release>;
using OcbStepReaderGuard = occ_guard::HandleGuard<&ocb_stepreader_release>;
using OcbStepWriterGuard = occ_guard::HandleGuard<&ocb_stepwriter_release>;
using OcbAisObjGuard     = occ_guard::HandleGuard<&ocb_ais_release>;
using OcbAisCtxGuard     = occ_guard::HandleGuard<&ocb_ais_ctx_release>;

#endif // OCC_C_GUARD_H
