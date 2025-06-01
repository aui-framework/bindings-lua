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
#include <clg.hpp>
#include "AUI/ASS/Selector/AAssSelector.h"
#include <AUI/ASS/ASS.h>


class StyleHelper {
public:
    [[nodiscard]]
    bool processDeclaration(const clg::table& table);

    using Property = std::shared_ptr<ass::prop::IPropertyBase>;
    using Rules = std::vector<ass::Rule>;


    Rules& rules() noexcept {
        return mRules;
    }

    static void processDeclarations(ass::PropertyList& rule, const clg::table_array& items);

private:
    void processTableWithoutSelector(const clg::table& t, const AString& className, AOptional<ass::AAssSelector> parent);


    ass::AAssSelector parseSelector(std::string_view str);

    Rules mRules;
};


