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
// Created by Alex2772 on 3/23/2023.
//

#include "MySlider.h"

#include "AUI/Util/UIBuildingHelpers.h"

using namespace ass;
using namespace declarative;

MySlider::MySlider() {
    setContents(Stacked {
        mProgress = _new<AProgressBar>() AUI_OVERRIDE_STYLE {
            Expanding{1, 0},
        },
        mHandle = _new<Handle>()
    });

    connect(mProgress->value().changed, [this](aui::float_within_0_1 v) {
        emit valueChanging(v); // we would emit ed signal in pointer release method
        updateHandlePosition();
    });

    setValue(0.f);

    addAssName("Slider");
    progressbar()->addAssName("SliderBackground");
    progressbar()->innerView()->addAssName("SliderForeground");
    handle()->addAssName("SliderHandle");
}

void MySlider::onPointerPressed(const APointerPressedEvent& event) {
    AView::onPointerPressed(event); // we do not want to gain focus to some of our childs; so calling AView directly
    updateSliderWithPosition(event.position);
}

void MySlider::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainerBase::onPointerMove(pos, event);
    if (isDragging()) {
        updateSliderWithPosition(pos);
    }
}

void MySlider::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainerBase::onPointerReleased(event);
    updateSliderWithPosition(event.position);

    emit valueChanged(value());
}

void MySlider::updateSliderWithPosition(glm::ivec2 pointerPosition) {
    setValue(float(pointerPosition.x) / float(getContentWidth()));
}

void MySlider::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
    updateHandlePosition();
}

void MySlider::updateHandlePosition() {
    const auto& progressInner = mProgress->innerView();
    const auto progressRightCornerAbsolutePos = progressInner->getPositionInWindow().x + progressInner->getWidth();
    const auto handleCenteringOffset = mHandle->getWidth() / 2;
    mHandle->setPosition({getPadding().left + progressRightCornerAbsolutePos - getPositionInWindow().x - handleCenteringOffset,
                          mHandle->getPosition().y });
}

bool MySlider::capturesFocus() {
    return true; // we want to receive onPointerMove events even when pointer is outside
}
