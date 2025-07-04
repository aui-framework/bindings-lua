// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

//
// Created by ilyazavalov on 8/27/24.
//

#pragma once

#include "clg.hpp"

class LuaSelfAccessor {
public:
    template<typename T>
    clg::lua_self* asLuaSelf(T* self) {
        if (mAsLuaSelf) {
            return mAsLuaSelf;
        }

        return mAsLuaSelf = dynamic_cast<clg::lua_self*>(self);
    }

private:
    clg::lua_self* mAsLuaSelf = nullptr;
};
