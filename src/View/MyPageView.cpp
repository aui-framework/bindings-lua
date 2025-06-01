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

#include "MyPageView.h"

MyPageView::MyPageView(clg::table_array array) {
    for (const auto& var : array) {
        addPage(var.as<_<AView>>());
    }
}

void MyPageView::setPageId(unsigned int index) {
    assert(index > 0);
    APageView::setPageId(index - 1);
}
