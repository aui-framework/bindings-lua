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

#include <AUI/View/AViewContainer.h>
#include <AUI/View/AScrollArea.h>
#include <uiengine/UIEngine.h>
#include "MyScrollbar.h"

/**
 * @brief Скроллареа
 * @ingroup lua_views
 * @lua{ScrollArea, CustomScrollArea}
 */
class MyScrollArea: public AScrollArea {
public:
    explicit MyScrollArea(_<AView> wrappedView);
    MyScrollArea(_<AView> wrappedView, _<MyScrollbar> verticalScrollbar, _<MyScrollbar> horizontalScrollbar);

    void setScrollRatioX(aui::ranged_number<float, 0, 100> ratio);
    void setScrollRatioY(aui::ranged_number<float, 0, 100> ratio);
    void setContent(_<AView> view);

    void scrollTo2(const _<AView>& view, bool nearestBorder) {
        scrollTo(view, nearestBorder);
    }
    bool onGesture(const glm::ivec2& origin, const AGestureEvent& event) override;
    void onScroll(const AScrollEvent& event) override;

    void setAllowUserScroll(bool v) {
        mAllowUserScroll = v;
    }

private:
    bool mAllowUserScroll = true;

    static _<MyScrollbar> createScrollbar(_<MyScrollbar> providedScrollbar, ALayoutDirection direction);
    static AScrollArea::Builder createBuilder(_<AView> wrappedView, _<MyScrollbar> verticalScrollbar, _<MyScrollbar> horizontalScrollbar);
};


