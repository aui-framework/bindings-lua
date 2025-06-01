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
// Created by dervisdev on 3/6/2023.
//

#include "Animator.h"

#include <utility>

Animator::Animator(std::shared_ptr<AAnimator>  animator) : mAnimator(std::move(animator)) {}

std::shared_ptr<AAnimator> Animator::getAuiAnimator() {
    assert(("Invalid Animator", mAnimator != nullptr));
    return mAnimator;
}

void Animator::pause() {
    mAnimator->pause();
}
