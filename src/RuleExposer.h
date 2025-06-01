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

#include "clg.hpp"
#include "fake_return.h"
#include <uiengine/UIEngine.h>


template<typename Rule>
struct RuleExposer {
    friend class ExposeHelper;
public:


    template<typename... Args>
    RuleExposer& ctor() {
        auto callback = [&uiEngine = mUiEngine](Args... args) -> std::shared_ptr<ass::prop::IPropertyBase> {
            return std::make_shared<ass::prop::Property<Rule>>(Rule{std::move(args)...});
        };
        if (mExtraConstructors == nullptr) {
            mExtraConstructors = &clg::state_interface(clg::state()).register_function_overloaded(mName, std::move(callback));
        } else {
            mExtraConstructors->push_back(clg::state_interface(clg::state()).wrap_lambda_to_cfunction_for_overloading(std::move(callback), mName));
        }

        return *this;
    }

private:
    UIEngine& mUiEngine;
    std::string mName;

    std::vector<lua_CFunction>* mExtraConstructors = nullptr;
    clg::lua_cfunctions mExtraMethods;

    RuleExposer(UIEngine& uiEngine, std::string name) : mUiEngine(uiEngine), mName(std::move(name)) {}
};