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
// Created by Alex2772 on 12/2/2022.
//

#include "MyScrollbar.h"

MyScrollbar::MyScrollbar(ALayoutDirection direction) : AScrollbar(direction) {
    mBackwardButton->addAssName("ScrollbarButton");
    mForwardButton->addAssName("ScrollbarButton");
    switch (direction) {
        case ALayoutDirection::VERTICAL:
            mBackwardButton->addAssName("ScrollbarButtonVertical");
            mBackwardButton->addAssName("ScrollbarButtonUp");
            mForwardButton->addAssName("ScrollbarButtonVertical");
            mForwardButton->addAssName("ScrollbarButtonDown");
            mHandle->addAssName("ScrollbarHandle");
            addAssName("ScrollbarVertical");
            break;
        case ALayoutDirection::HORIZONTAL:
            mBackwardButton->addAssName("ScrollbarButtonHorizontal");
            mBackwardButton->addAssName("ScrollbarButtonRight");
            mForwardButton->addAssName("ScrollbarButtonHorizontal");
            mForwardButton->addAssName("ScrollbarButtonLeft");
            mHandle->addAssName("ScrollbarHandleHorizontal");
            addAssName("ScrollbarHorizontal");
            break;
    }

    addAssName("Scrollbar");
}

void MyScrollbar::setScrollRatio(aui::ranged_number<float, 0, 100> ratio) {
    int scroll = ceil(ratio * (static_cast<float> (getMaxScroll()) / 100.f));
    scroll = glm::clamp(scroll, 0, static_cast<int> (getMaxScroll()));
    setScroll(scroll);
}
