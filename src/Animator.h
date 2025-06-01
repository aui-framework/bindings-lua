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

#pragma once

#include <AUI/Animator/AAnimator.h>
#include <clg.hpp>

class Animator : public clg::lua_self, public std::enable_shared_from_this<Animator> {
public:
    Animator() = default;
    explicit Animator(std::shared_ptr<AAnimator> animator);
    std::shared_ptr<AAnimator> getAuiAnimator();

    void setCurve(const clg::function& curve) {
        luaDataHolder()["cpp_curveCallback"] = curve.ref();
        mAnimator->setCurve([w = weak_from_this()](float x) -> float {
            auto self = w.lock();
            if (!self) {
                return 1.f;
            }

            if (auto callback = self->luaDataHolder()["cpp_curveCallback"].is<clg::function>()) {
                return callback->call<float, float>(static_cast<float&&>(x));
            }

            return 1.f;
        });
    }

    void pause();

    void setDuration(float period) {
        mAnimator->setDuration(period);
    }

    void setRepeating(bool repeating) {
        mAnimator->setRepeating(repeating);
    }

    template<class AnimatorType, typename... Args>
    static std::shared_ptr<Animator> create(Args ...args) {
        static_assert(("AnimatorType must be derived from AAnimator", std::is_base_of<AAnimator, AnimatorType>::value));
        return std::make_shared<Animator>(std::make_shared<AnimatorType>(args...));
    }

    static std::shared_ptr<Animator> fromCombination(const std::vector<std::shared_ptr<Animator>>& animators) {
        AVector<_<AAnimator>> auiAnimators;
        auiAnimators.reserve(animators.size());
        for (const auto& animator : animators) {
            auiAnimators.push_back(animator->getAuiAnimator());
        }

        return std::make_shared<Animator>(AAnimator::combine(auiAnimators));
    }

private:
    std::shared_ptr<AAnimator> mAnimator;
};