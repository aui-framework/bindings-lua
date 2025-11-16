// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <range/v3/all.hpp>
#include <AUI/View/AForEachUI.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <uiengine/Converters.h>
#include "MyForEachUI.h"

using namespace declarative;
using namespace ass;

template<>
struct std::hash<clg::ref> {
    std::size_t operator()(const clg::ref& r) {
        clg::stack_integrity_check c;
        r.push_value_to_stack();
        auto value = lua_topointer(clg::state(), -1);
        std::uintptr_t hash = 0;
        hash = reinterpret_cast<std::uintptr_t>(value);
        if (!hash) {
            hash = lua_tointeger(clg::state(), -1);
        }
        lua_pop(clg::state(), 1);
#if AUI_DEBUG
        ALogger::info("Test") << "Value: " << r.debug_str() << " hash: " << hash;
        AUI_ASSERT(hash != 0);
#endif
        return hash;
    }
};

struct MyForEachUI::ListAdapter {
public:
    struct iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = clg::ref;

        iterator() = default;
        iterator(MyForEachUI* parent, size_t index) : parent(parent), index(index) {}

        auto operator<=>(const iterator&) const = default;

        auto operator*() const {
            return parent->model()[index].as<clg::ref>();
        }

        auto& operator++() {
            index += 1;
            return *this;
        }

        auto& operator--() {
            index -= 1;
            index = glm::max(size_t(1), index);
            return *this;
        }

        auto operator++(int) {
            auto copy = *this;
            operator++();
            return copy;
        }

        auto operator--(int) {
            auto copy = *this;
            operator++();
            return copy;
        }

    private:
        MyForEachUI* parent = nullptr;
        size_t index = 0;
    };

    ListAdapter(MyForEachUI* parent): parent(parent) {

    }

    iterator begin() const {
        aui::react::DependencyObserverScope::addDependency(parent->notified);
        return {parent, 1};
    }

    iterator end() const {
        return {parent, parent->model().raw_len() + 1 };
    }

private:
    MyForEachUI* parent;
};


MyForEachUI::MyForEachUI() {
    static_assert(ranges::input_iterator<MyForEachUI::ListAdapter::iterator>);
    static_assert(ranges::range<MyForEachUI::ListAdapter>);
}

void MyForEachUI::setModel(clg::ref model) {
    asLuaSelf(this)->luaDataHolder()["cpp_model"] = std::move(model);
    tryFullUpdate();
}

void MyForEachUI::setFactory(clg::function factory) {
    asLuaSelf(this)->luaDataHolder()["cpp_factory"] = std::move(factory);
    tryFullUpdate();
}

void MyForEachUI::tryFullUpdate() {
    auto self = asLuaSelf(this)->luaDataHolder();

    if (!self["cpp_model"].is<clg::table>()) {
        return;
    }
    if (!self["cpp_factory"].is<clg::function>()) {
        return;
    }
    auto list = ListAdapter(this);
    setContents(Centered { AUI_DECLARATIVE_FOR(i, list, AVerticalLayout) {
        return self["cpp_factory"].as<clg::function>().call<_<AView>>(i);
    } AUI_WITH_STYLE { Expanding() } });
}

clg::table_view MyForEachUI::model() {
    return asLuaSelf(this)->luaDataHolder()["cpp_model"].as<clg::table_view>();
}

void MyForEachUI::notify() {
    emit notified();
}
