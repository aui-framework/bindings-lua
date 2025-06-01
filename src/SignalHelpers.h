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
// Created by ilyazavalov on 10/5/23.
//

#pragma once

#include <AUI/Common/ASignal.h>
#include "ExposeHelper.h"

class AView;
namespace clg {
    class function;
}

namespace {
    namespace signal {

        template<auto signalField, typename ViewType = AView>
        auto& getSignal(const _<ViewType>& view) {
            decltype(auto) result = std::invoke(signalField, *view);
            if constexpr (requires{ { result } -> AAnyProperty; }) {
                return result.changed;
            } else {
                return result;
            }
        }

        template<typename... Args>
        struct ArgumentDeducer;

        template<typename... Args>
        struct ArgumentDeducer<ASignal<Args...>> {

            template<typename ViewType>
            static void connect(const _<ViewType>& self, const ASignal<Args...>& signal, clg::function callback) {
                AObject::connect(signal, self, [self = self.get(), callback = std::move(callback)](Args... args) {
                    callback(self->sharedPtr(), std::move(args)...);
                });
            }

            template<typename ViewType>
            static void connectLuaSelf(const _<ViewType>& self, const ASignal<Args...>& signal, const clg::function& callback, const _<clg::lua_self>& luaSelf) {
                auto id = clg::light_userdata{const_cast<void*>(reinterpret_cast<const void*>(&signal))};
                auto callbacks = luaSelf->luaDataHolder().get_or_create(id, [&]() {
                    AObject::connect(signal, self, [self = self.get(), luaSelf = luaSelf.get(), id](Args... args) {
                        auto table = luaSelf->luaDataHolder()[id].as<clg::table_view>();
                        size_t cnt = table.raw_len();
                        for (size_t i = 1; i <= cnt;) {
                            auto func = table[i].ref();
                            if (func.isFunction()) {
                                auto result = clg::function(std::move(func)).call<std::optional<clg::ref>>(self->sharedPtr(), std::move(args)...);
                                if (result) {
                                    static auto SIGNAL_REMOVE = clg::state_interface(ExposeHelper::state()).global_variable("SIGNAL_REMOVE");
                                    if (*result == SIGNAL_REMOVE) {
                                        table.raw_set(i, nullptr, ExposeHelper::state());
                                        cnt -= 1;
                                        continue;
                                    }
                                }
                            }
                            ++i;
                        }
                    });
                    return clg::table{};
                }).template as<clg::table_view>();
                callbacks[callbacks.raw_len() + 1] = callback;
            }
        };
    }
}

template<auto signalField, typename ViewType = AView>
struct ForwardSignal {
    clg::builder_return_type operator()(const _<ViewType>& self, const clg::function& callback) {
        const auto& signal = signal::getSignal<signalField>(self);
        using Deducer = typename signal::ArgumentDeducer<std::decay_t<decltype(signal)>>;
        if (auto luaSelf = _cast<clg::lua_self>(self)) {
            Deducer::connectLuaSelf(self, signal, callback, luaSelf);
        } else {
            Deducer::connect(self, signal, callback);
        }
        return {};
    }
};

template<auto signalField, typename ViewType = AView>
struct DropSignal {
    clg::builder_return_type operator()(const _<ViewType>& self) {
        const auto& signal = signal::getSignal<signalField>(self);
        if (auto luaSelf = _cast<clg::lua_self>(self)) {
            auto id = clg::light_userdata{const_cast<void*>(reinterpret_cast<const void*>(&signal))};
            auto holder = luaSelf->luaDataHolder();
            auto value = holder[id];
            if (!value.ref().isNull()) {
                value = clg::table{};
            }
        }
        else {
            signal.clearAllOutgoingConnections();
        }
        return {};
    }
};
