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
// Created by Alex2772 on 10/10/2022.
//

#include "MyTextArea.h"
#include <AUI/Platform/AWindow.h>

MyTextArea::MyTextArea(std::string_view s) {
    ATextArea::setText(s);
    connect(textChanging, [this]() {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onTextChanging"].invokeNullsafe(aui::ptr::shared_from_this(this));
    });
    connect(textChanged, [this]() {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onTextChanged"].invokeNullsafe(aui::ptr::shared_from_this(this));
    });
}

void MyTextArea::onCharEntered(AChar c) {
    ATextArea::onCharEntered(c);
    if (c == '\n' || c == '\r') {
        AUI_NULLSAFE(asLuaSelf(this))->luaDataHolder()["cpp_onEnterPressed"].invokeNullsafe(aui::ptr::shared_from_this(this));
    }
}
