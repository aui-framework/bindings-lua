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


#include <AUI/View/ATextArea.h>
#include <uiengine/UIEngine.h>
#include "LuaSelfAccessor.h"

/**
 * @brief Текстовое поле.
 * @ingroup lua_views
 * @lua{Input}
 */
class MyTextArea: public ATextArea, private LuaSelfAccessor {
public:
    MyTextArea(std::string_view s);

    /**
     * @brief Выставить каллбек на изменение текста.
     * @param callback Каллбек
     */
    void onTextChangingCallback(const clg::function& callback) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onTextChanging"] = callback;
    }

    /**
     * @brief Выставить каллбек на потеру фокуса после изменения текста.
     * @param callback Каллбек
     */
    void onTextChangedCallback(const clg::function& callback) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onTextChanged"] = callback;
    }

    /**
     * @brief Выставить каллбек на нажатия клавиши ввода.
     * @param callback Каллбек
     */
    void onEnterPressedCallback(const clg::function& callback) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onEnterPressed"] = callback;
    }

    void onCharEntered(AChar c) override;

private:
};


