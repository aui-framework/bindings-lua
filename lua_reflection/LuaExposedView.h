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


#include "AUI/Render/ARenderContext.h"
#include <AUI/View/AView.h>
#include <uiengine/ILuaExposedView.h>

namespace performance {
    extern unsigned AUI_VIEW_RENDER;
}

template<typename View>
class LuaExposedView: public View, public ILuaExposedView {
public:
    template<typename... Args>
    LuaExposedView(UIEngine& uiEngine, Args&&... args): View(std::forward<Args>(args)...), ILuaExposedView(uiEngine) {}
    ~LuaExposedView() override {

    }

    [[nodiscard]]
    AView* view() noexcept override {
        return this;
    }

    void render(ARenderContext context) override;
    void drawStencilMask(ARenderContext context) override;
    void postRender(ARenderContext context) override;
    bool consumesClick(const glm::ivec2& pos) override;
    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;
    int getMinimumWidth() override;
    int getMinimumHeight() override;
    bool onGesture(const glm::ivec2& origin, const AGestureEvent& event) override;
    void onMouseEnter() override;
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onMouseLeave() override;
    void onDpiChanged() override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void onPointerDoubleClicked(const APointerDoubleClickedEvent& event) override;
    void onScroll(const AScrollEvent& event) override;
    void onKeyDown(AInput::Key key) override;
    void onKeyRepeat(AInput::Key key) override;
    void onKeyUp(AInput::Key key) override;
    void onFocusAcquired() override;
    void onFocusLost() override;
    void onCharEntered(char16_t c) override;
    bool handlesNonMouseNavigation() override;
    bool capturesFocus() override;
    void setGeometry(int x, int y, int width, int height) override;
    AMenuModel composeContextMenu() override { return {}; }

protected:
    void handle_lua_virtual_func_assignment(std::string_view name, clg::ref value) override {
        if (auto it = vtable().find(name); it != vtable().end()) {
            (this->*(it->second)) = !value.isNull();
        }
    }

private:
};
