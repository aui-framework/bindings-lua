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

#pragma once

#include "uiengine/UIEngine.h"
#include <AUI/View/ADrawableView.h>
#include "LuaSelfAccessor.h"

class MyDrawableView : public ADrawableView, private LuaSelfAccessor {
public:
    explicit MyDrawableView(std::string_view ref);
    void animationFinished(const clg::function& callback);
};
