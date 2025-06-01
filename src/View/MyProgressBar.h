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


#include <AUI/View/AProgressBar.h>
#include <uiengine/UIEngine.h>

/**
 * @brief Прогресс бар.
 * @ingroup lua_views
 */
class MyProgressBar: public AProgressBar {
public:
    MyProgressBar() {
        getViews()[0]->addAssName("ProgressbarInner");
    }

    [[nodiscard]]
    float value() const noexcept {
        return *AProgressBar::value();
    }

    /**
     * @brief Выставить значение прогресс бара
     * @param v Значение от 0.0 до 1.0.
     * @return self
     */
    void setValue(float v) {
        AProgressBar::setValue(v);
    }
};