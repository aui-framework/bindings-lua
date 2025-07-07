// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Logging/ALogger.h>
#include "ExposeHelper.h"
#include "AUI/Common/AException.h"
#include "AUI/Traits/concepts.h"
#include "StyleHelper.h"
#include "Animator.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Animator/AAnimator.h>
#include "SignalHelpers.h"
#include "clg.hpp"

using namespace ass;

namespace clg {
    template<>
    inline std::string class_name<AView>() {
        return "ILuaExposedView";
    }
}


static std::optional<std::tuple<int, _<AView>>> containerIterator(_<AView> view, int iterator) {
    if (auto c = _cast<AViewContainer>(view)) {
        if (iterator < c->getViews().size()) {
            return std::make_optional(std::make_tuple(iterator + 1, c->getViews()[iterator]));
        }
    }
    return std::nullopt;
}

template<auto signalField, typename ViewType = AView>
struct ForwardSignalAndEmit {

    clg::builder_return_type operator()(const _<AView>& self, const clg::function& callback) {
        onBeforeConnected(self, callback);
        return ForwardSignal<signalField, ViewType>()(self, callback);
    }

    std::function<void(const _<AView>& /* self*/, const clg::function& /* callback*/)> onBeforeConnected;
};


ExposeHelper::ExposeHelper(UIEngine& uiEngine): mUiEngine(uiEngine)  {
    clg::state_interface(clg::state()).register_function<containerIterator>("__uiengine_container_iter");
    auto setCursor = [] (const _<AView>& self, const clg::ref& image, int size = 16) {
        if (image.isNull()) {
            if (auto parent = self->getParent()) {
                self->setCursor(parent->getCursor());
            }
            else {
                self->setCursor(ACursor::DEFAULT);
            }
        } else {
            ACursor cursor= ACursor(AUrl(image.as<std::string_view>()), size);
            self->setCursor(cursor);
        }
        return clg::builder_return_type{};
    };

    clg::state_interface(clg::state()).register_class<AView>()
            .method("geometryChanged", ForwardSignal<&AView::geometryChanged>())
            .method("positionChanged", ForwardSignalAndEmit<&AView::position>{[](const _<AView>& view, const clg::function& callback) {
                callback(view, view->getPosition());
            }})
            .method("sizeChanged", ForwardSignalAndEmit<&AView::size>{[](const _<AView>& view, const clg::function& callback) {
                callback(view, view->getSize());
            }})
            .method("clicked", ForwardSignal<&AView::clicked>())
            .method("clickedRight", ForwardSignal<&AView::clickedRight>())
            .method("clickedRightOrLongPressed", ForwardSignal<&AView::clickedRightOrLongPressed>())
            .method("clickedButton", ForwardSignal<&AView::clickedButton>())
            .method("focused", ForwardSignal<&AView::focusAcquired>())
            .method("unfocused", ForwardSignal<&AView::focusLost>())
            .method("addOnKeyDown", ForwardSignal<&AView::keyPressed>())
            .method("addOnKeyUp", ForwardSignal<&AView::keyReleased>())
            .method("onChildFocused", ForwardSignal<&AView::childFocused>())
            .method("addOnScroll", ForwardSignal<&AView::scrolled>())
            .method("pressed", ForwardSignal<&AView::pressed>())
            .method("released", ForwardSignal<&AView::released>())
            .method("dropGeometryChanged", DropSignal<&AView::geometryChanged>())
            .method("dropClicked", DropSignal<&AView::clicked>())
            .method("dropClickedRight", DropSignal<&AView::clickedRight>())
            .method("dropClickedRightOrLongPressed", DropSignal<&AView::clickedRightOrLongPressed>())
            .method("dropClickedButton", DropSignal<&AView::clickedButton>())
            .method("dropFocused", DropSignal<&AView::focusAcquired>())
            .method("dropUnfocused", DropSignal<&AView::focusLost>())
            .method("dropAddOnKeyDown", DropSignal<&AView::keyPressed>())
            .method("dropAddOnKeyUp", DropSignal<&AView::keyReleased>())
            .method("dropOnChildFocused", DropSignal<&AView::childFocused>())
            .method("dropOnScroll", DropSignal<&AView::scrolled>())
            .method("dropPressed", DropSignal<&AView::pressed>())
            .method("dropReleased", DropSignal<&AView::released>())
            .method("focus", [&](const _<AView>& self) {
                self->focus(true);
                return clg::builder_return_type{};
            })
            .method("setStyle", [&](const _<AView>& view, clg::table table) {
                if (!view) {
                    return clg::builder_return_type{};
                }
                if (table.empty()) {
                    view->setCustomStyle({});
                    view->setExtraStylesheet(nullptr);
                    return clg::builder_return_type{};
                }
                StyleHelper sh;
                if (!sh.processDeclaration(table)) {
                    PropertyListRecursive ruleWithoutSelector;
                    StyleHelper::processDeclarations(ruleWithoutSelector, table.toArray());
                    view->setCustomStyle(std::move(ruleWithoutSelector));
                    return clg::builder_return_type{};
                }

                auto stylesheet = _new<AStylesheet>(AStylesheet(std::initializer_list<Rule>{}));
                stylesheet->setRules(std::move(static_cast<AVector<Rule>&>(sh.rules())));
                view->setExtraStylesheet(std::move(stylesheet));

                return clg::builder_return_type{};
            })
            .builder_method<&AView::setEnabled>("setEnabled")
            .builder_method<&AView::setVisibility>("setVisibility")
            .method("inflateView", [&] (const _<AView>& self, const _<AView>& wrapped) {
                if (auto c = _cast<AViewContainer>(self)) {
                    ALayoutInflater::inflate(*c, wrapped);
                    c->markMinContentSizeInvalid();
                }
                return clg::builder_return_type{};
            })
            .method("removeAllViews", [] (const _<AView>& self) {
                if (auto c = _cast<AViewContainer>(self)) {
                    c->removeAllViews();
                    c->markMinContentSizeInvalid();
                    UIEngine::removeAllChildren(c);
                }
                return clg::builder_return_type{};
            })
            .method("addViewCustomLayout", [] (const _<AView>& self, const _<AView>& view) {
                if (auto c = _cast<AViewContainer>(self)) {
                    c->addViewCustomLayout(view);
                    UIEngine::addChild(c, view);
                    self->invalidateAssHelper();
                    if(c->getParent()) c->applyGeometryToChildrenIfNecessary();
                    c->markMinContentSizeInvalid();
                }
                return clg::builder_return_type{};
            })
            .method("addView", [] (const _<AView>& self, const _<AView>& view) {
                if (view == nullptr) {
                    throw AException("addView(nil)?");
                }
                if (auto c = _cast<AViewContainer>(self)) {
                    c->addView(view);
                    UIEngine::addChild(c, view);
                    c->markMinContentSizeInvalid();
                } else {
                    throw AException("addView() called on non-container type");
                }
                return clg::builder_return_type{};
            })
            .method("addViewAtIndex", [](const _<AView>& self, const _<AView>& view, size_t index) {
                if (auto c = _cast<AViewContainer>(self)) {
                    if (index > c->getViews().size()) {
                        throw AException("addViewAtIndex: index cannot be larger than container size");
                    }
                    c->addView(index, view);
                    UIEngine::addChild(c, view);
                    c->markMinContentSizeInvalid();
                } else {
                    throw AException("addViewAtIndex() called on non-container type");
                }
                return clg::builder_return_type{};
            })
            .method("removeViewAtIndex", [](const _<AView>& self, size_t index) {
                if (auto c = _cast<AViewContainer>(self)) {
                    if (index == 0 || index > c->getViews().size()) {
                        throw AException("removeViewAtIndex: index cannot be larger than container size");
                    }
                    UIEngine::removeChild(c, c->getViews()[index - 1]);
                    c->removeView(index - 1);
                    c->markMinContentSizeInvalid();
                } else {
                    throw AException("removeViewAtIndex() called on non-container type");
                }
                return clg::builder_return_type{};
            })
            .method("getViewAtIndex", [](const _<AView>& self, size_t index) -> _<AView> {
                if (auto c = _cast<AViewContainerBase>(self)) {
                    auto& views = c->getViews();
                    if (index == 0 || index > views.size()) {
                        return nullptr;
                    }
                    return views[index - 1];
                }
                return nullptr;
            })
            .method("size", [](const _<AView>& self) -> std::optional<size_t> {
                if (auto c = _cast<AViewContainerBase>(self)) {
                    return c->getViews().size();
                }
                return std::nullopt;
            })
            .method("getParent", [] (const _<AView>& self) -> _<AView> {
                if (auto p = self->getParent()) {
                    return aui::ptr::shared_from_this(p);
                }

                return {};
            })
            .method("getViews", [] (const _<AView>& self) {
                if (auto c = _cast<AViewContainerBase>(self)) {
                    return c->getViews();
                }
                return AVector<_<AView>>{};
            })
            .method("visitViewsRecursive", [](const _<AView>& self, const clg::function& callback) {
                if (auto c = _cast<AViewContainerBase>(self)) {
                    c->visitsViewRecursive([&](const _<AView>& view){
                        callback(view);
                        return false;
                    });
                }
                return clg::builder_return_type{};
            })
            .method("updateLayout", [] (const _<AView>& self) {
                APerformanceSection updateLayout("layout update");
                if (auto c = _cast<AViewContainerBase>(self)) {
                    c->applyGeometryToChildrenIfNecessary();
                }
                return clg::builder_return_type{};
            })
            .method("mapLocalCoordsToGlobal", [] (const _<AView>& self, glm::ivec2 pos) {
                return self->getPositionInWindow() + pos;
            })
            .method("visitViewsUnderPos", [] (const _<AView>& self, glm::ivec2 pos, const clg::function& callback) {
                return self->getWindow()->getViewAtRecursive(pos, [&](const _<AView>& v){
                    if (auto c = _cast<ILuaExposedView>(v)) {
                        if (callback.call<bool>(c)) {
                            return true;
                        }
                    }

                    return false;
                }, AViewLookupFlags::ONLY_ONE_PER_CONTAINER);
            })
            .method("getViewAt", [] (const _<AView>& self, glm::ivec2 pos) {
                return self->getWindow()->getViewAt(pos);
            })
            .method("removeView", [] (const _<AView>& self, const _<AView>& view) {
                if (!view) {
                    return clg::builder_return_type{};
                }
                if (auto c = _cast<AViewContainer>(self)) {
                    c->removeView(view);
                    UIEngine::removeChild(c, view);
                    c->markMinContentSizeInvalid();
                } else {
                    throw AException("removeView() called on non-container type");
                }
                return clg::builder_return_type{};
            })
            .builder_method<&AView::setBlockClicksWhenPressed>("setBlockClicksWhenPressed")
            .builder_method<&AView::setPosition>("setPos")
            .method<&AView::getPosition>("getPos")
            .builder_method<&AView::setSize>("setSize")
            .method("enableRenderToTexture", [](const _<AView>& self) {
                IRenderViewToTexture::enableForView(AWindow::current()->getRenderingContext()->renderer(), *self);
            })
            .builder_method<&AView::addAssName>("addStylesheetName")
            .builder_method<&AView::removeAssName>("removeStylesheetName")
            .method<&AView::getSize>("getSize")
            .method<&AView::getMinimumSize>("getMinimumSize")
            .method("isPressed", [](const _<AView>& self) {
                return self->isPressed();
            })
            .method("expanding", [](const _<AView>& self) {
                self->setExpanding();
                return clg::builder_return_type{};
            })
            .method("dump", [](const _<AView>& self) {
                if (auto l = dynamic_cast<ILuaExposedView*>(self.get())) {
                    ALogger::info("Lua") << "dump(): " << l->luaDataHolder().debug_str();
                }
                return clg::builder_return_type{};
            })
            .method("setCursorWithSize", [&setCursor] (const _<AView>& self, const clg::ref& image, int size) {
                setCursor(self, image, size);
                return clg::builder_return_type{};
            })
            .method("setCursor", [&](const _<AView>& self, const clg::ref& image) {
                setCursor(self, image);
                return clg::builder_return_type{};
            })
            .method("iter", [&](const _<AView>& self) {
                return std::make_tuple(clg::state_interface(clg::state()).global_variable("__uiengine_container_iter"), self, 0);
            })
            .method("setAnimator", [&](const _<AView>& self, const std::shared_ptr<Animator>& animator) {
                self->setAnimator(animator->getAuiAnimator());
                return clg::builder_return_type{};
            })
            .constructor<>();


}

lua_State* ExposeHelper::state() {
    return clg::state();
}
