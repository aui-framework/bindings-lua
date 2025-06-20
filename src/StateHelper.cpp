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
// Created by Alex2772 on 7/6/2022.
//

#include "StateHelper.h"

void StateHelper::initLua(clg::state_interface lua) {
    lua.register_class<StateHelper>()
            .staticFunction<StateHelper::string>("string")
                    ;
}

