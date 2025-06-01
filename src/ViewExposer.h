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
#include <LuaExposedView.h>
#include <cfunction.hpp>
#include <uiengine/UIEngine.h>

template<typename Clazz>
struct ViewExposer {
    friend class ExposeHelper;
private:

    template<auto m>
    struct the_method {
        template<class... Args>
        static auto builder_with_args(clg::state_interface::types<Args...>) {
            return [](_<Clazz> self, Args&& ... args) -> clg::builder_return_type {
                if (!self) throw std::runtime_error("self could not be null");
                auto selfViewPtr = self.get();
                if (!selfViewPtr) {
                    throw std::runtime_error(std::string("type mismatch: expected ") + typeid(Clazz).name() + ", got " + typeid(*self.get()).name());
                }
                (dynamic_cast<Clazz*>(selfViewPtr)->*m)(std::forward<Args>(args)...);

                return clg::builder_return_type{};
            };
        }

        template<class... Args>
        static auto method_with_args(clg::state_interface::types<Args...>) {
            return [](_<Clazz> self, Args&& ... args) {
                if (!self) throw std::runtime_error("self could not be null");
                auto selfViewPtr = self.get();
                if (!selfViewPtr) {
                    throw std::runtime_error(std::string("type mismatch: expected ") + typeid(Clazz).name() + ", got " + typeid(*self.get()).name());
                }
                using return_t = decltype((dynamic_cast<Clazz*>(selfViewPtr)->*m)(std::forward<Args>(args)...));
                if constexpr (std::is_void_v<return_t>) {
                    (dynamic_cast<Clazz*>(selfViewPtr)->*m)(std::forward<Args>(args)...);
                } else {
                    return (dynamic_cast<Clazz*>(selfViewPtr)->*m)(std::forward<Args>(args)...);
                }
            };
        }
    };

    template<typename Callback, typename... Args>
    struct the_builder {
        the_builder(Callback&& callback, clg::state_interface::types<Args...>): mCallback(std::move(callback)) {}

        clg::builder_return_type operator()(Args&& ... args) {
            mCallback(std::forward<Args>(args)...);
            return {};
        }
    private:
        Callback mCallback;
    };


    template<typename Callback, typename... Args>
    static the_builder<Callback, Args...> make_the_builder(Callback&& callback, clg::state_interface::types<Args...> args) {
        return { std::forward<Callback>(callback), args};
    }

public:

    template<auto m>
    ViewExposer& builder(std::string name) {
        using class_info = clg::state_interface::callable_class_info<decltype(m)>;
        using args = typename class_info::args;
        return method(std::move(name), the_method<m>::builder_with_args(args()));
    }

    template<typename Callback>
    ViewExposer& builder(std::string name, Callback&& callback) {
        using function_info = clg::state_interface::callable_class_info<decltype(&Callback::operator())>;
        auto w = clg::state_interface(clg::state()).wrap_lambda_to_cfunction(make_the_builder(std::forward<Callback>(callback), function_info::args()), name);
        mExtraMethods.push_back({std::move(name), w});
        return *this;
    }

    template<auto m>
    ViewExposer& method(std::string name) {
        using class_info = clg::state_interface::callable_class_info<decltype(m)>;
        using args = typename class_info::args;
        return method(std::move(name), the_method<m>::method_with_args(args()));
    }

    template<typename Callback>
    ViewExposer& method(std::string name, Callback&& callback) {
        auto w = clg::state_interface(clg::state()).wrap_lambda_to_cfunction(std::forward<Callback>(callback), name);
        mExtraMethods.push_back({std::move(name), w});
        return *this;
    }

    template<auto f>
    ViewExposer& static_function(std::string name) {
        auto w = clg::cfunction<f>(name);
        mExtraMethods.push_back({std::move(name), w});
        return *this;
    }

    template<typename... Args>
    void ctor() {
        assert(("unnamed class could not have a constructor; use pushMetatable() instead", !mName.empty()));
        if (!mExtraMethods.empty()) {
            lua_State* L = clg::state();
            clg::stack_integrity_check check(L);

            clg::impl::newlib(L, mExtraMethods);
            auto indexForNewMT = clg::pop_from_lua<clg::table>(L);


            clg::state_interface(clg::state()).register_function(mName, [name = mName, &uiEngine = mUiEngine, indexForNewMT = std::move(indexForNewMT)](lua_State* lua, Args... args) {
                auto view = std::make_shared<LuaExposedView<Clazz>>(uiEngine, std::move(args)...);
                view->addAssName(name);

                auto ref = clg::ref::from_cpp(lua, view);

                auto destination = view->luaDataHolder();
                assert(!destination.isNull());
                for (const auto& [k, v] : indexForNewMT) {
                    destination[k] = v;
                }

                ref.push_value_to_stack(lua);

                return fake{};
            });

            return;
        }
        clg::state_interface(clg::state()).register_function(mName, [name = mName, &uiEngine = mUiEngine](Args... args) -> _<AView> {
            auto view = std::make_shared<LuaExposedView<Clazz>>(uiEngine, std::move(args)...);
            view->addAssName(name);

            return view;
        });
    }

private:
    UIEngine& mUiEngine;
    std::string mName;

    clg::lua_cfunctions mExtraMethods;

    ViewExposer(UIEngine& uiEngine, std::string name = "") : mUiEngine(uiEngine), mName(std::move(name)) {}
};
