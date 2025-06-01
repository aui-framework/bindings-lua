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


#include <AUI/View/ANumberPicker.h>
#include <uiengine/UIEngine.h>
#include "LuaSelfAccessor.h"

/**
 * @brief Текстовое поле для ввода чисел с двумя кнопками для увеличения/уменьшения значения.
 * @ingroup lua_views
 * @lua{NumberPicker}
 */
class MyNumberPicker: public ANumberPicker, private LuaSelfAccessor {
public:
    MyNumberPicker(int v);

    /**
     * @brief Выставить численное значение текстового поля.
     * @param v Значение.
     */
    void setValue(int v) {
        ANumberPicker::setValue(v);
    }

    /**
     * @brief Получить численное представление текстового поля.
     * @return Значение.
     */
    int value() const {
        return ANumberPicker::getValue();
    }

    /**
     * @brief Получить строковое представление текстового поля.
     * @return Значение.
     */
    [[nodiscard]]
    const AString& text() const noexcept {
        return ANumberPicker::text();
    }


    /**
     * @brief Выставить минимальное возможное значение.
     * @param min Минимальное значение. По умолчанию 0.
     */
    void setMin(const int min) {
        ANumberPicker::setMin(min);
    }

    /**
     * @brief Выставить максимальное возможное значение.
     * @param max Максимальное значение. По умолчанию 100.
     */
    void setMax(const int max) {
        ANumberPicker::setMax(max);
    }


    /**
     * @brief Выставить каллбек на изменение значения.
     * @param callback Каллбек
     */
    void onValueChangingCallback(const clg::function& callback) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onValueChangingCallback"] = callback;
    }

    /**
     * @brief Выставить каллбек на потеру фокуса после изменения значения.
     * @param callback Каллбек
     */
    void onValueChangedCallback(const clg::function& callback) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onValueChangedCallback"] = callback;
    }
};


