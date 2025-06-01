// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#pragma once

#include <clg.hpp>

namespace {
    struct fake{};
}

namespace clg {

    template<>
    struct converter<fake> {
        static int to_lua(lua_State* l, fake v) { // чтобы наебать return систему clg
            return 1;
        }
    };
}