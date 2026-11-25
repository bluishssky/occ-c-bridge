// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "doctest.h"
#include "occ_c_bridge.h"

#include <cmath>

TEST_CASE("light: ambient/directional light handles") {
    OcbLight amb = OCB_INVALID;
    REQUIRE(ocb_light_new_ambient(1.0f, 1.0f, 1.0f, 0.3f, &amb) == OCB_OK);
    CHECK(amb != OCB_INVALID);

    OcbLight head = OCB_INVALID;
    REQUIRE(ocb_light_new_directional(1.0f, 1.0f, 1.0f, 2.0f, true, 0.0, 0.0, -1.0, &head) == OCB_OK);
    CHECK(head != OCB_INVALID);

    ocb_light_release(amb);
    ocb_light_release(head);
}

TEST_CASE("light: viewer ops reject invalid handles") {
    OcbLight l = OCB_INVALID;
    CHECK(ocb_light_new_ambient(1.0f, 1.0f, 1.0f, 0.3f, &l) == OCB_OK);
    CHECK(ocb_viewer_add_light(OCB_INVALID, l) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_viewer_lights_on(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);
    CHECK(ocb_viewer_lights_off(OCB_INVALID) == OCB_ERR_INVALID_HANDLE);
    ocb_light_release(l);
}
