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
// Created by ilyazavalov on 07/22/2023.
//

#include "MyDrawableView.h"
#include <AUI/Image/AAnimatedDrawable.h>
#include "SignalHelpers.h"

MyDrawableView::MyDrawableView(std::string_view url) : ADrawableView(AString(url)) {
}

void MyDrawableView::animationFinished(const clg::function& callback) {
    if (auto animated = _cast<AAnimatedDrawable>(getDrawable())) {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_animationFinished"] = callback;
        animated->connect(animated->animationFinished, [this]() {
            AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_animationFinished"].invokeNullsafe(aui::ptr::shared_from_this(this));
        });
    }
}


