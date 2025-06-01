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
// Created by Alex2772 on 7/8/2022.
//

#include <AUI/Util/ARaiiHelper.h>
#include "StyleHelper.h"
#include "AUI/ASS/Selector/AAssSelector.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/IO/AByteBufferInputStream.h"

using namespace ass;

template<typename Selector>
static ass::AAssSelector parentify(const AOptional<ass::AAssSelector>& parent, Selector selector) {
    if (parent) {
        return *parent >> std::move(selector);
    }
    return std::move(selector);
}

void StyleHelper::processTableWithoutSelector(const clg::table& t, const AString& className, AOptional<ass::AAssSelector> parent) {
    Rule rule(parentify(parent, ass::c(className)));
    AOptional<Rule> hover, active, focus, disabled, selected;

    for (const auto&[subSelector, v] : t) {
        if (v.isNull()) {
            continue;
        }
        auto processSubSelectorRule = [&](ass::AAssSelector selector, clg::table items) {
            Rule r(ass::AAssSelector::makeCopy(selector));
            for (const auto&[i, v] : items) {
                if (i.empty()) {
                    continue;
                }
                if (v.isNull()) {
                    continue;
                }
                if (i[0] >= '0' && i[0] <= '9') {
                    // regular declaration entry
                    r.addDeclaration(v.as<std::shared_ptr<ass::prop::IPropertyBase>>());
                    continue;
                }
                
                // derivative? e.g. hover = { Button = { ... } }
                //                             i          v
                processTableWithoutSelector(v.as<clg::table>(), i, ass::AAssSelector::makeCopy(selector));
            }
            return r;
        };
        if (subSelector == "hover") {
            hover = processSubSelectorRule(parentify(parent, ass::c::hover(className)), v.as<clg::table>());
            continue;
        }
        if (subSelector == "active") {
            active = processSubSelectorRule(parentify(parent, ass::c::active(className)), v.as<clg::table>());
            continue;
        }
        if (subSelector == "focus") {
            focus = processSubSelectorRule(parentify(parent, ass::c::focus(className)), v.as<clg::table>());
            continue;
        }
        if (subSelector == "disabled") {
            disabled = processSubSelectorRule(parentify(parent, ass::c::disabled(className)), v.as<clg::table>());
            continue;
        }
        if (subSelector == "checked" || subSelector == "selected") {
            selected = processSubSelectorRule(parentify(parent, ass::Selected(ass::c(className))), v.as<clg::table>());
            continue;
        }

        if (!subSelector.empty()) {
            if (auto first = subSelector.front(); !(first >= '0' && first <= '9')) {
                // sass-style nested selector:
                // Container = { -- parent
                //   Icon = {    -- subSelector
                //   }
                // }
                if (auto table = v.is<clg::table>()) {
                    processTableWithoutSelector(*table, subSelector, rule.getSelector());
                    continue;
                } else {
                    throw AException("setStyle: unexpected value at {} {} (table expected to make sass-style nested selector) : {}"_format(className, subSelector, v.debug_str()));
                }
            }
        }

        if (auto propertyBase = v.is<std::shared_ptr<ass::prop::IPropertyBase>>()) {
            rule.addDeclaration(std::move(*propertyBase));
        } else {
            throw AException("setStyle: unexpected value at {} {} : {}"_format(className, subSelector, v.debug_str()));
        }
    }

    mRules.push_back(std::move(rule));
    if (hover) mRules.push_back(std::move(*hover));
    if (focus) mRules.push_back(std::move(*focus));
    if (active) mRules.push_back(std::move(*active));
    if (disabled) mRules.push_back(std::move(*disabled));
    if (selected) mRules.push_back(std::move(*selected));
}

bool StyleHelper::processDeclaration(const clg::table& table) {
    const auto L = clg::state();
    for (const auto&[className, v] : table) {
        clg::stack_integrity_check stackIntegrityCheck(L);
        if (v.isNull()) {
            continue;
        }
        v.push_value_to_stack();
        auto& tmp = v;
        ARaiiHelper h = [&] {
            lua_pop(L, 1);
        };
        if (!lua_istable(L, -1)) {
            return false;
        }

        auto t = clg::get_from_lua<clg::table>(L, -1);
        processTableWithoutSelector(t, className, std::nullopt);
    }
    return true;
}

void StyleHelper::processDeclarations(PropertyList& rule, const clg::table_array& items) {
    for (const auto& v : items) {
        if (v.isNull()) {
            continue;
        }
        rule.addDeclaration(v.as<std::shared_ptr<ass::prop::IPropertyBase>>());
    }
}
