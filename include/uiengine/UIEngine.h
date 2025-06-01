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
#include "uiengine/Converters.h"
#include <AUI/View/AViewContainer.h>

class UIEngine {
public:
    UIEngine(AViewContainer& surface);

    UIEngine(const UIEngine&) = delete;

    _<AView> loadForm(std::string_view file);


    [[nodiscard]]
    AViewContainer& surface() const noexcept {
        return mSurface;
    }

    static const char* anyToString(const clg::ref& r); // для lldb.py
    static const char* anyToString(lua_State* L, int n = -1); // для lldb.py

    /**
     * @brief Преобразовать любое значение в строковое json-совместимое представление (для отладки).
     * @param any Любое луа значение.
     * @details
     * Квадратные скобки [a, b] обозначают, что у сущности a есть метатаблица b.
     * @code{lua}
     * vec = { x = 1, y = 2 }
     * setmetatable(vec, { __index = { x = 0, y = 0 } })
     * clgDump(vec) -- прим. вывод [{"x":1,"y":2}, {"__index":{"x":0,"y":0}}]
     * @endcode
     */
    static std::string clgDump(clg::ref any);

    static clg::table_view luaChildrenTable(const _<AViewContainer>& cont);

    static void addChild(const _<AViewContainer>& cont, const _<AView>& view);

    static void removeChild(const _<AViewContainer>& cont, const _<AView>& view);

    static void removeAllChildren(const _<AViewContainer>& cont);

    _<AView> wrapViewWithLuaWrapper(const _<AView>& v);

private:
    AViewContainer& mSurface;
};