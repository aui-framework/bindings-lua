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
#include "LiveData.h"

class StateHelper {
public:
    static void initLua(clg::state_interface lua);

    static std::shared_ptr<LiveData<std::string>> string() {
        return std::make_shared<LiveData<std::string>>();
    }
};


namespace clg {
    template<>
    inline std::string class_name<StateHelper>() {
        return "State";
    }
}