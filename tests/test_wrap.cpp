// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

TEST_CASE("wrap: native context wrapper rejects null") {
    OcbAisContext ctx = OCB_INVALID;
    CHECK(ocb_ais_ctx_wrap_native(nullptr, &ctx) == OCB_ERR_INVALID_ARG);
    CHECK(ocb_ais_ctx_wrap_native(nullptr, nullptr) == OCB_ERR_INVALID_ARG);
}

TEST_CASE("wrap: released wrapped context handle is invalid") {
    OcbAisContext ctx = OCB_INVALID;
    CHECK(ocb_ais_ctx_wrap_native(nullptr, &ctx) == OCB_ERR_INVALID_ARG);
    bool displayed = true;
    CHECK(ocb_ais_ctx_is_displayed(OCB_INVALID, OCB_INVALID, &displayed) == OCB_ERR_INVALID_HANDLE);
}
