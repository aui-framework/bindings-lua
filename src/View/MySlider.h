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


#include <AUI/View/ASlider.h>
#include <uiengine/UIEngine.h>

class MySlider: public ASlider {
public:
    MySlider();

    float value() const {
        return *ASlider::value();
    }

    void onValueChangingCallback(clg::function callback) {
        connect(valueChanging, [callback = std::move(callback)](float v) { callback(v); });
    }
    void onValueChangedCallback(clg::function callback) {
        connect(valueChanged, [callback = std::move(callback)](float v) { callback(v); });
    }
};
