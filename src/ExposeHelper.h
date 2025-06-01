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

#include <uiengine/UIEngine.h>
#include <AUI/View/AButton.h>
#include <tuple>
#include "ViewExposer.h"
#include "RuleExposer.h"
#include "clg.hpp"



struct ExposeHelper {
public:
    ExposeHelper(UIEngine& uiEngine);

    static lua_State* state();


    template<typename View>
    ViewExposer<View> view(std::string name) {
        return { mUiEngine, std::move(name) };
    }

    template<typename Rule, bool asEnum = std::is_enum_v<Rule>>
    std::conditional_t<asEnum, void, RuleExposer<Rule>> rule(std::string name = clg::class_name<Rule>()) {
        if constexpr (asEnum) {
            clg::state_interface(clg::state()).register_enum<Rule>(name.c_str(), [](Rule r) -> std::shared_ptr<ass::prop::IPropertyBase> {
                return std::make_shared<ass::prop::Property<Rule>>(r);
            });
        } else {
            return {mUiEngine, std::move(name)};
        }
    }

    template<typename LayoutOrContainer>
    void container(std::string_view name) {
        static constexpr auto isLayout = std::is_base_of_v<ALayout, LayoutOrContainer>;

        clg::state_interface(clg::state()).register_function(std::string(name), [&uiEngine = mUiEngine](clg::vararg args) {
            _<AViewContainer> viewContainer;

            if constexpr (isLayout) {
                viewContainer = _new<LuaExposedView<AViewContainer>>(uiEngine);
                viewContainer->setLayout(std::make_unique<LayoutOrContainer>());
            } else {
                viewContainer = _new<LuaExposedView<LayoutOrContainer>>(uiEngine);
            }

            {
                // for data holder initializating
                clg::push_to_lua(clg::state(), viewContainer);
                clg::pop_from_lua<decltype(viewContainer)>(clg::state());
            }

            viewContainer->addAssName("ViewContainer");

            if (args.size() == 1) {
                // table-style?
                if (!args[0].isNull()) {
                    if (auto t = args[0].is<clg::table_array>()) {

                        bool result = false;

                        if (t->empty()) {
                            return viewContainer; // пустая таблица - тоже допустимо
                        }

                        for (const auto& arg: *t) {
                            if (auto view = arg.template as<_<AView>>()) { // обратная совместимость
                                result = true;
                                viewContainer->addView(view);
                                UIEngine::addChild(viewContainer, view);
                            }
                        }
                        if (result) {
                            return viewContainer;
                        }
                    }
                }
            }

            for (const auto& arg: args) {
                if (!arg.isNull()) {
                    auto v = arg.as_converter_result<_<AView>>();
                    if (v.is_error()) {
                        throw AException("Failed to create container: only view or nil expected in argument list");
                    }
                    if (*v == nullptr) {
                        // luashers can pass to ++ any kind of type (e.g., (not boolean_flag) or View(...))
                        // so we may have nullptr here
                        continue;
                    }
                    viewContainer->addView(*v);
                    UIEngine::addChild(viewContainer, *v);
                }
            }
            return viewContainer;
        });
    }

private:
    UIEngine& mUiEngine;
};
