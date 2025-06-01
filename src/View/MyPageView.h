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


#include <AUI/View/APageView.h>
#include <uiengine/UIEngine.h>

/**
 * @brief Как TabView, только без панели вкладок
 * @lua{PageView}
 */
class MyPageView: public APageView {
public:
    MyPageView(clg::table_array array);


    /**
     * @brief Выставить вкладку по индексу.
     * @param index Индекс вкладки (с 0)
     */
    void setPageId(unsigned index);
};


