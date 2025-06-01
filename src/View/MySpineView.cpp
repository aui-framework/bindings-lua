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
// Created by alex2772 on 4/15/25.
//

#if AUI_BINDINGS_LUA_SPINE
#include "MySpineView.h"

AMap<AString, MySpineView::SpineCache> MySpineView::ourCache;

const MySpineView::SpineCache& MySpineView::getSpineCache(const APath& prefix) {
    return ourCache.getOrInsert(prefix, [&] {
        auto atlas = [&] {
            auto buffer = AByteBuffer::fromStream(AUrl("{}.atlas"_format(prefix)).open());
            return _new<spine::Atlas>(buffer.data(), buffer.size(), prefix.parent().toStdString().c_str(), &ASpineView::TEXTURE_LOADER);
        }();
        auto binary = _new<spine::SkeletonBinary>(atlas.get());
        auto skeletonData = [&] {
            auto buffer = AByteBuffer::fromStream(AUrl("{}.skel"_format(prefix)).open());
            return aui::ptr::manage(
                binary->readSkeletonData(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size()));
        }();
        if (!binary->getError().isEmpty()) {
            throw AException("SkeletonBinary failed: {}"_format(binary->getError().buffer()));
        }

        return SpineCache{ .atlas = std::move(atlas), .skeletonData = std::move(skeletonData), .binary = std::move(binary) };
    });
}

MySpineView::MySpineView(APath prefix): MySpineView(getSpineCache(prefix)) {
    mPrefix = std::move(prefix);
}

MySpineView::MySpineView(const MySpineView::SpineCache& cacheEntry):
  ASpineView(cacheEntry.atlas, cacheEntry.skeletonData, _new<spine::AnimationStateData>(cacheEntry.skeletonData.get())) {
    animationStateData()->setDefaultMix(0.2f);
}

spine::Animation& MySpineView::getAnimation(std::string_view name) {
    auto animation = skeletonData()->findAnimation(name.data());
    if (animation == nullptr) {
        throw AException("{}: animation not found: {}"_format(mPrefix, name));
    }
    return *animation;
}
#endif