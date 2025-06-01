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

#include <AUI/Spine/ASpineView.h>

class MySpineView: public ASpineView {
public:
    explicit MySpineView(APath prefix);
    ~MySpineView() override = default;

    void setPos(glm::vec2 v) {
        skeleton().setX(v.x);
        skeleton().setY(v.y);
    }

    void setScale(glm::vec2 v) {
        skeleton().setScaleX(v.x);
        skeleton().setScaleY(v.y);
    }

    void addAnimation(size_t trackIndex, std::string_view name, bool loop, float delay) {
        animationState().addAnimation(trackIndex, &getAnimation(name), loop, delay);
    }

    void setAnimation(size_t trackIndex, std::string_view name, bool loop) {
        animationState().setAnimation(trackIndex, &getAnimation(name), loop);
    }

    void clearTrack(size_t trackIndex) {
        animationState().clearTrack(trackIndex);
    }

private:
    APath mPrefix;
    struct SpineCache {
        _<spine::Atlas> atlas;
        _<spine::SkeletonData> skeletonData;
        _<spine::SkeletonBinary> binary;
    };

    explicit MySpineView(const SpineCache& cacheEntry);

    spine::Animation& getAnimation(std::string_view name);

    static const SpineCache& getSpineCache(const APath& prefix);
    static AMap<AString, SpineCache> ourCache;
};
