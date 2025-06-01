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
// Created by Alex2772 on 12/1/2022.
//

#include "MyTabView.h"

MyTabView::MyTabView(clg::table_array array) {
    for (const auto& var : array) {
        auto entry = var.as<clg::table_array>();
        auto name = entry[0].as<std::string>();
        auto contents = entry[1].as<_<AView>>();

        addTab(std::move(contents), name);
    }
}

void MyTabView::setTabId(unsigned int index) {
    assert(index > 0);
    ATabView::setTabId(index - 1);
}
