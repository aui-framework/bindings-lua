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
// Created by Alex2772 on 10/25/2022.
//

#include "MyScrollArea.h"
#include "MyScrollbar.h"
#include <AUI/View/AScrollArea.h>
#include <AUI/Util/UIBuildingHelpers.h>

MyScrollArea::MyScrollArea(_<AView> wrappedView) :
        MyScrollArea(std::move(wrappedView), _new<MyScrollbar>(ALayoutDirection::VERTICAL),
                     _new<MyScrollbar>(ALayoutDirection::HORIZONTAL)) {
}

MyScrollArea::MyScrollArea(_<AView> wrappedView, _<MyScrollbar> verticalScrollbar, _<MyScrollbar> horizontalScrollbar) :
        AScrollArea(createBuilder(std::move(wrappedView),
                                  createScrollbar(std::move(verticalScrollbar), ALayoutDirection::VERTICAL),
                                  createScrollbar(std::move(horizontalScrollbar), ALayoutDirection::HORIZONTAL))) {
    for (const auto& s : {AScrollArea::verticalScrollbar(), AScrollArea::horizontalScrollbar()}) {
        if (!s) {
            continue;
        }
        addView(s);
    }
}

void MyScrollArea::setScrollRatioX(aui::ranged_number<float, 0, 100> ratio) {
    auto scrollbar = horizontalScrollbar();
    AUI_NULLSAFE(_cast<MyScrollbar>(scrollbar))->setScrollRatio(ratio);
}

void MyScrollArea::setScrollRatioY(aui::ranged_number<float, 0, 100> ratio) {
    auto scrollbar = verticalScrollbar();
    AUI_NULLSAFE(_cast<MyScrollbar>(scrollbar))->setScrollRatio(ratio);
}

AScrollArea::Builder MyScrollArea::createBuilder(_<AView> wrappedView, _<MyScrollbar> verticalScrollbar,
                                                 _<MyScrollbar> horizontalScrollbar) {
    return AScrollArea::Builder().withContents(declarative::Vertical{std::move(wrappedView)})
            .withExternalVerticalScrollbar(std::move(verticalScrollbar))
            .withExternalHorizontalScrollbar(std::move(horizontalScrollbar));
}

_<MyScrollbar> MyScrollArea::createScrollbar(_<MyScrollbar> providedScrollbar, ALayoutDirection direction) {
    if (!providedScrollbar) {
        auto result = _new<MyScrollbar>(direction);
        result->setVisibility(Visibility::GONE);
        return result;
    }

    return providedScrollbar;
}

void MyScrollArea::setContent(_<AView> view) {
    AScrollArea::setContents(std::move(view));
    markMinContentSizeInvalid();
}

void MyScrollArea::onScroll(const AScrollEvent& event) {
    if (!mAllowUserScroll) {
        return;
    }
    AScrollArea::onScroll(event);
}

bool MyScrollArea::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    if (!mAllowUserScroll) {
        return false;
    }
    return AScrollArea::onGesture(origin, event);
}
