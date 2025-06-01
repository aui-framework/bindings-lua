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

#include <AUI/Util/UIBuildingHelpers.h>
#include "MyText.h"

void MyText::setText(const clg::table_array& items) {
    clearContent();
    setItems(AVector<std::variant<AString, _<AView>>>::fromRange(aui::range(items), [](const clg::ref& r) -> std::variant<AString, _<AView>> {
        if (r.isNull()) {
            throw AException("null Text entry");
        }

        if (auto s = r.is<std::string_view>()) {
            return AString(*s);
        }

        if (auto v = r.is<_<AView>>()) {
            return *v;
        }

        throw AException("invalid text entry: {}"_format(r.debug_str()));
    }));
}
