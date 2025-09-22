// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <uiengine/UIEngine.h>
#include <ExposeHelper.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/AGridSplitter.h>
#include <AUI/View/AScrollbar.h>
#include "AUI/Enum/ATextInputType.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/A2FingerTransformArea.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ATextField.h"
#include "StateHelper.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ADragArea.h"
#include "AUI/Util/ALayoutInflater.h"
#include "View/MyCircleProgressBar.h"
#include "View/MyTextField.h"
#include "View/MyNumberPicker.h"
#include "View/MyText.h"
#include "View/MySlider.h"
#include "View/MyScrollArea.h"
#include "View/MyProgressBar.h"
#include "View/MyPageView.h"
#include "View/MyTabView.h"
#include "View/MyScrollbar.h"
#include "View/MyCheckBox.h"

#if AUI_BINDINGS_LUA_SPINE
#include "View/MySpineView.h"
#endif

#include "View/MyDragArea.h"
#include "View/MyDrawableView.h"
#include "Animator.h"
#include <LuaExposedView.h>
#include <AUI/Animator/AFocusAnimator.h>
#include <AUI/Animator/ATranslationAnimator.h>
#include <AUI/Animator/ARotationAnimator.h>
#include <AUI/Animator/APlaceholderAnimator.h>
#include <AUI/Animator/ASizeAnimator.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AClipboard.h>
#include "SignalHelpers.h"
#include "clg.hpp"
#include "View/MyTextArea.h"
#include "View/MyForEachUI.h"

static constexpr auto LOG_TAG = "UIEngine";
unsigned performance::AUI_VIEW_RENDER = 0;

class UI {};
class Window {};

static _<AWindow> currentWindow() {
    return _cast<AWindow>(aui::ptr::shared_from_this(AWindow::current()));
}

UIEngine::UIEngine(AViewContainer& surface):
        mSurface(surface)
{
    using namespace declarative;

    ExposeHelper expose(*this);

    clg::state_interface lua(clg::state());
    lua.set_global_value("IS_32BIT", bool(sizeof(void *) == 4));
    lua.set_global_value("IS_64BIT", bool(sizeof(void *) == 8));

    lua.set_global_value("AUI_PLATFORM_WIN", bool(AUI_PLATFORM_WIN));

    lua.set_global_value("AUI_PLATFORM_ANDROID", bool(AUI_PLATFORM_ANDROID));
    lua.set_global_value("AUI_PLATFORM_LINUX", bool(AUI_PLATFORM_LINUX));
    lua.set_global_value("AUI_PLATFORM_UNIX", bool(AUI_PLATFORM_UNIX));

    lua.set_global_value("AUI_PLATFORM_MACOS", bool(AUI_PLATFORM_MACOS));
    lua.set_global_value("AUI_PLATFORM_IOS", bool(AUI_PLATFORM_IOS));

    lua.set_global_value("SIGNAL_REMOVE", clg::table{});
    lua.register_enum<ATouchscreenKeyboardPolicy>("TouchscreenKeyboardPolicy");

    lua.register_class<UI>()
        .staticFunction("setSurface", [this](const _<AView>& wrapper) {
            ALayoutInflater::inflate(mSurface, wrapper);
        })
        .staticFunction("stealSurface", [this]() {
            auto wrapper = _new<LuaExposedView<AViewContainer>>(*this);
            auto oldSurface = mSurface.getViews().first();
            mSurface.removeView(oldSurface);
            ALayoutInflater::inflate(wrapper, oldSurface);
            return wrapper;
        });

    lua.register_function<currentWindow>("currentWindow");

    lua.register_class<AClipboard>()
        .staticFunction<AClipboard::isEmpty>("isEmpty")
        .staticFunction<AClipboard::copyToClipboard>("copyToClipboard")
        .staticFunction<AClipboard::pasteFromClipboard>("pasteFromClipboard");

    lua.register_class<Window>()
        .staticFunction("focusNextView", []() {
            AUI_NULLSAFE(currentWindow())->focusNextView();
        })
        .staticFunction("onKeyDown", [](clg::function callback) {
            auto window = AWindow::current();
            AObject::connect(window->keyDown, window, [callback = std::move(callback)](AInput::Key key) {
                callback(key);
            });
        })
        .staticFunction("hideTouchscreenKeyboard", []() {
            AUI_NULLSAFE(currentWindow())->requestHideTouchscreenKeyboard();
        })
        .staticFunction("showTouchscreenKeyboard", []() {
            AUI_NULLSAFE(currentWindow())->requestShowTouchscreenKeyboard();
        })
        .staticFunction("setTouchscreenKeyboardPolicy", [](ATouchscreenKeyboardPolicy policy) {
            AUI_NULLSAFE(currentWindow())->setTouchscreenKeyboardPolicy(policy);
        })
        .staticFunction("setScalingParams", [](float scalingFactor, std::optional<glm::uvec2> minWindowSizeDp) {
            auto w = currentWindow();
            if (!w) {
                return;
            }
            AWindowBase::ScalingParams params;
            params.scalingFactor = scalingFactor;
            if (!minWindowSizeDp) {
                params.minimalWindowSizeDp = std::nullopt;
            }
            else {
                params.minimalWindowSizeDp = *minWindowSizeDp;
            }
            w->setScalingParams(params);
        })
        .staticFunction("getSize", []() -> glm::ivec2 {
            if (auto w = currentWindow()) {
                return w->getSize();
            }
            return {0, 0};
        })
        .staticFunction("getDpiRatio", []() -> float {
            if (auto w = currentWindow()) {
                return w->getDpiRatio();
            }
            return 0;
        });

    lua.register_function<clgDump>("clgDump");

    lua.register_class<AAnimator>().constructor<>();
    lua.register_class<Animator>()
        .method<&Animator::pause>("pause")
        .builder_method<&Animator::setCurve>("setCurve")
        .builder_method<&Animator::setDuration>("setDuration")
        .builder_method<&Animator::setRepeating>("setRepeating")
        .staticFunction<Animator::fromCombination>("fromCombination")
        .staticFunction<Animator::create<AFocusAnimator>>("focus")
        .staticFunction<Animator::create<APlaceholderAnimator>>("placeholder")
        .staticFunction<Animator::create<ARotationAnimator, AAngleRadians, AAngleRadians>>("rotation")
        .staticFunction<Animator::create<ASizeAnimator, const glm::ivec2&, const glm::ivec2&>>("size")
        .staticFunction<Animator::create<ATranslationAnimator, const glm::vec2&, const glm::vec2&>>("translation")
        .constructor<std::shared_ptr<AAnimator>>();

    expose.view<AView>("View")
            .ctor<>();

    expose.view<AButton>("Button")
            .method<&AButton::setText>("setText")
            .ctor<std::string>();

    expose.view<MyProgressBar>("Progressbar")
            .method<&MyProgressBar::value>("value")
            .builder<&MyProgressBar::setValue>("setValue")
            .ctor<>();

    expose.view<MyCircleProgressBar>("CircleProgressbar")
            .method<&MyCircleProgressBar::value>("value")
            .builder<&MyCircleProgressBar::setValue>("setValue")
            .ctor<>();


    expose.view<ASpacerExpanding>("Spacer")
            .ctor<>();

    expose.view<ASpacerFixed>("SpacerFixed")
            .ctor<AMetric>();

    expose.view<ALabel>("Label")
            .method<&ALabel::setText>("setText")
            .ctor<AString>()
            ;

    expose.view<MyPageView>("PageView")
            .method<&MyPageView::setPageId>("setPageId")
            .ctor<clg::table_array>()
            ;

    expose.view<MyTabView>("TabView")
            .method<&MyTabView::setTabId>("setTabId")
            .ctor<clg::table_array>()
            ;

    expose.view<MyCheckBox>("Checkbox")
            .builder<&ACheckBox::setChecked>("setChecked")
            .method("isChecked", [](const _<MyCheckBox>& c) {
                return bool(c->checked());
            })
            .method("checked", ForwardSignal<&MyCheckBox::checked, MyCheckBox>())
            .method("disableCheckingOnClick", DropSignal<&MyCheckBox::checked, MyCheckBox>())
            .ctor<>()
            ;


    expose.view<MyNumberPicker>("NumberPicker")
            .builder<&MyNumberPicker::setValue>("setValue")
            .builder<&MyNumberPicker::setMin>("setMin")
            .builder<&MyNumberPicker::setMax>("setMax")
            .builder<&MyNumberPicker::onValueChangedCallback>("onValueChangedCallback")
            .builder<&MyNumberPicker::onValueChangingCallback>("onValueChangingCallback")
            .method<&MyNumberPicker::value>("value")
            .method<&MyNumberPicker::text>("text")
            .ctor<int>()
            ;


    lua.register_enum<ATextInputType>("TextInputType");
    lua.register_enum<ATextInputActionIcon>("TextInputAction");
    expose.view<MyTextField>("Input")
            .builder<&MyTextField::onTextChangedCallback>("onTextChangedCallback")
            .builder<&MyTextField::onTextChangingCallback>("onTextChangingCallback")
            .builder<&MyTextField::onEnterPressedCallback>("onEnterPressed")
            .builder<&MyTextField::isValidTextPredicate>("isValidTextPredicate")
            .builder<&MyTextField::setPasswordMode>("setPasswordMode")
            .builder<&MyTextField::selectAll>("selectAll")
            .builder<&MyTextField::setTextInputType>("setTextInputType")
            .builder<&MyTextField::setTextInputActionIcon>("setTextInputAction")
            .method("actionButtonPressed", ForwardSignal<&ATextField::actionButtonPressed, MyTextField>())
            .method("dropActionButtonPressed", DropSignal<&ATextField::actionButtonPressed, MyTextField>())
            .method("text", [](const _<MyTextField>& t) { return *t->text(); })
            .method<&MyTextField::setText>("setText")
            .ctor<std::string_view>();

    expose.view<MyTextArea>("TextArea")
            .builder<&MyTextArea::onTextChangedCallback>("onTextChangedCallback")
            .builder<&MyTextArea::onTextChangingCallback>("onTextChangingCallback")
            .builder<&MyTextArea::onEnterPressedCallback>("onEnterPressed")
            .builder<&MyTextArea::selectAll>("selectAll")
            .builder<&MyTextArea::setTextInputActionIcon>("setTextInputAction")
            .method("actionButtonPressed", ForwardSignal<&ATextArea::actionButtonPressed, MyTextArea>())
            .method("dropActionButtonPressed", DropSignal<&ATextArea::actionButtonPressed, MyTextArea>())
            .method("text", [](const _<MyTextArea>& t) { return *t->text(); })
            .method<&MyTextArea::setText>("setText")
            .ctor<std::string_view>();


    expose.view<MyText>("Text")
            .builder<&MyText::setText>("setText")
            .ctor<clg::table_array>();

    expose.view<AScrollbarButton>("ScrollbarButton")
            .ctor<>();

    expose.view<MyScrollbar>("Scrollbar")
            .method("scrolled", ForwardSignal<&AScrollbar::scrolled, AScrollbar>())
            .method("updatedMaxScroll", ForwardSignal<&AScrollbar::updatedMaxScroll, AScrollbar>())
            .method("triggeredManually", ForwardSignal<&AScrollbar::triggeredManually, AScrollbar>())
            .method<&MyScrollbar::setScrollRatio>("setScrollRatio")
            .ctor<ALayoutDirection>();

    expose.view<MyScrollbar>("VerticalScrollbar")
            .method("scrolled", ForwardSignal<&AScrollbar::scrolled, AScrollbar>())
            .method("updatedMaxScroll", ForwardSignal<&AScrollbar::updatedMaxScroll, AScrollbar>())
            .method("triggeredManually", ForwardSignal<&AScrollbar::triggeredManually, AScrollbar>())
            .method<&MyScrollbar::setScrollRatio>("setScrollRatio")
            .ctor<>();

    expose.view<A2FingerTransformArea>("A2FingerTransformArea")
            .method("transformed", ForwardSignal<&A2FingerTransformArea::transformed, A2FingerTransformArea>())
            .ctor<>();

    expose.view<MySlider>("Slider")
            .builder<&MySlider::setValue>("setValue")
            .builder<&MySlider::onValueChangingCallback>("onValueChangingCallback")
            .builder<&MySlider::onValueChangedCallback>("onValueChangedCallback")
            .method<&MySlider::value>("value")
            .ctor<>();

    expose.view<MyScrollArea>("CustomScrollArea")
            .builder<&MyScrollArea::setStickToEnd>("setStickToEnd")
            .builder<&MyScrollArea::setScrollRatioX>("setScrollRatioX")
            .builder<&MyScrollArea::setScrollRatioY>("setScrollRatioY")
            .builder<&MyScrollArea::scrollTo2>("scrollTo")
            .builder<&MyScrollArea::setContent>("setContent")
            .builder<&AScrollArea::setWheelScrollable>("setWheelScrollable")
            .builder<&MyScrollArea::setAllowUserScroll>("setAllowUserScroll")
            .method("scroll", [](const _<MyScrollArea>& area, glm::ivec2 delta) { area->scroll(delta); })
            .method("setScroll", [](const _<MyScrollArea>& area, glm::ivec2 abs) { area->setScroll(abs); })
            .ctor<_<AView>, _<MyScrollbar>, _<MyScrollbar>>();

    expose.view<MyScrollArea>("ScrollArea")
            .builder<&MyScrollArea::setStickToEnd>("setStickToEnd")
            .builder<&MyScrollArea::setScrollRatioX>("setScrollRatioX")
            .builder<&MyScrollArea::setScrollRatioY>("setScrollRatioY")
            .builder<&MyScrollArea::scrollTo2>("scrollTo")
            .builder<&MyScrollArea::setContent>("setContent")
            .builder<&MyScrollArea::setAllowUserScroll>("setAllowUserScroll")
            .builder<&AScrollArea::setWheelScrollable>("setWheelScrollable")
            .method("scroll", [](const _<MyScrollArea>& area, glm::ivec2 delta) { area->scroll(delta); })
            .method("setScroll", [](const _<MyScrollArea>& area, glm::ivec2 abs) { area->setScroll(abs); })
            .ctor<_<AView>>();

    expose.view<MyDrawableView>("Drawable")
            .builder<&MyDrawableView::animationFinished>("animationFinished")
            .ctor<std::string_view>();

    expose.container<AVerticalLayout>("Vertical");
    expose.container<AStackedLayout>("Centered");
    expose.container<AStackedLayout>("Stacked");
    expose.container<AHorizontalLayout>("Horizontal");
    expose.view<AViewContainer>("ViewContainer")
            .method<&AViewContainer::getViews>("getViews")
            .ctor<>();

    // drag
    expose.container<MyDragArea>("DragArea");
    expose.view<ADragArea::ADraggableHandle>("DragHandle")
            .ctor<>();

#if AUI_BINDINGS_LUA_SPINE
    expose.view<MySpineView>("SpineView")
        .builder<&MySpineView::setPos>("setPos")
        .builder<&MySpineView::setScale>("setScale")
        .builder<&MySpineView::addAnimation>("addAnimation")
        .builder<&MySpineView::setAnimation>("setAnimation")
        .builder<&MySpineView::clearTrack>("clearTrack")
        .builder<&MySpineView::setUsePma>("setUsePma")
        .ctor<const AString&>()
            ;
    spine::Bone::setYDown(true);
#endif

    expose.view<MyForEachUI>("ForEachUI")
        .builder<&MyForEachUI::setModel>("setModel")
        .builder<&MyForEachUI::setFactory>("setFactory")
        .method<&MyForEachUI::notify>("notify")
        .ctor<>()
        ;


    lua.register_function("AbsoluteLayout", [this](const clg::table_array& array) {
        auto layout = std::make_unique<AAbsoluteLayout>();
        for (const auto& v : array) {
            auto item = v.as<clg::table_view>();
            auto view = item["view"].ref().as<_<AView>>();
            auto pos = item["pos"].ref().as<glm::vec2>();

            layout->add({
                .view = std::move(view),
                .pivotX = AMetric(pos.x, AMetric::T_DP),
                .pivotY = AMetric(pos.y, AMetric::T_DP),
            });
        }
        auto container = _new<LuaExposedView<AViewContainer>>(*this);
        container->setLayout(std::move(layout));
        return container;
    });

    lua.register_function("Draggable", [this](const _<AView>& wrapper) {
        return wrapViewWithLuaWrapper(ADragArea::convertToDraggable(wrapper, false));
    });

    lua.register_function("GridSplitter", [this](clg::table_array luaItems) {
        auto view = AGridSplitter::Builder().withItems(AVector<AVector<_<AView>>>::fromRange(aui::range(luaItems), [](const clg::ref& luaRow) {
            return AVector<_<AView>>::fromRange(aui::range(luaRow.as<clg::table_array>()), [](const clg::ref& r) -> _<AView> {
                try {
                    return r.as<_<AView>>();
                } catch(...) {
                    return _new<ASpacerExpanding>();
                }
            });
        })).noDefaultSpacers().build() AUI_WITH_STYLE { Expanding{} };

        return wrapViewWithLuaWrapper(view);
    });


    // Styles ==========================================================================================================
    lua.register_class<ass::prop::IPropertyBase>();
    using namespace ass;

    expose.rule<BackgroundCropping>()
            .ctor<glm::vec2, unset_wrap<glm::vec2>>();

    expose.rule<BackgroundGradient>()
            .ctor<std::nullptr_t>()
            .ctor<AColor, AColor, ALayoutDirection>();

    lua.register_enum<ALayoutDirection>();

    expose.rule<BackgroundImage>()
            .ctor<>()
            .ctor<AString>()
            .ctor<std::nullptr_t>()
            .ctor<unset_wrap<AString>,
                  unset_wrap<AColor>,
                  unset_wrap<Repeat>>()
            .ctor<unset_wrap<AString>,
                  unset_wrap<AColor>,
                  unset_wrap<Repeat>,
                  unset_wrap<Sizing>>()
            .ctor<unset_wrap<AString>,
                  unset_wrap<AColor>,
                  unset_wrap<Repeat>,
                  unset_wrap<Sizing>,
                  unset_wrap<glm::vec2>>()
            .ctor<unset_wrap<AString>,
                  unset_wrap<AColor>,
                  unset_wrap<Repeat>,
                  unset_wrap<Sizing>,
                  unset_wrap<glm::vec2>,
                  unset_wrap<float>>()
                  ;

    lua.register_enum<Repeat>();
    lua.register_enum<Sizing>();
    lua.register_enum<ALayoutDirection>();

    lua.register_function("BackdropBlur", [](AMetric radius) -> std::shared_ptr<ass::prop::IPropertyBase> {
        return std::make_shared<ass::prop::Property<ass::Backdrop>>(ass::Backdrop {
          { Backdrop::GaussianBlur { radius } },
        });
   });

    expose.rule<BackgroundSolid>()
            .ctor<AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<Border>()
            .ctor<AMetric, AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<BorderRadius>()
            .ctor<AMetric>()
            ;

    expose.rule<BorderBottom>()
            .ctor<AMetric, AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<BorderLeft>()
            .ctor<AMetric, AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<BorderTop>()
            .ctor<AMetric, AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<BorderRight>()
            .ctor<AMetric, AColor>()
            .ctor<std::nullptr_t>()
            ;

    expose.rule<BoxShadow>()
            .ctor<std::nullptr_t>()
            .ctor<AMetric, AMetric, AMetric, AColor>()
            .ctor<AMetric, AMetric, AMetric, AMetric, AColor>();
 
    expose.rule<BoxShadowInner>()
            .ctor<std::nullptr_t>()
            .ctor<AMetric, AMetric, AMetric, AColor>()
            .ctor<AMetric, AMetric, AMetric, AMetric, AColor>()
            ;

    expose.rule<ACursor>("Cursor")
            .ctor<AString>()
            .ctor<AString, int>();

    expose.rule<Expanding>()
            .ctor<>()
            .ctor<unset_wrap<unsigned>>()
            .ctor<unset_wrap<unsigned>, unset_wrap<unsigned>>()
            ;

    expose.rule<FixedSize>()
           .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>>()
           .ctor<AMetric>()
           ;

    expose.rule<Font>()
           .ctor<AString>()
           ;

    expose.rule<FontFamily>()
           .ctor<AString>()
           ;

    expose.rule<FontRendering>();
    expose.rule<FontSize>()
           .ctor<AMetric>()
           ;

    expose.rule<ImageRendering>();

    expose.rule<LayoutSpacing>()
            .ctor<AMetric>()
            ;

    expose.rule<LineHeight>()
            .ctor<float>()
            ;

    expose.rule<Margin>()
            .ctor<unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>>()
            ;

    expose.rule<MaxSize>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<AMetric>()
            ;

    expose.rule<MinSize>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<AMetric>()
            ;

    expose.rule<Opacity>().ctor<float>();

    expose.rule<AOverflow>();
    expose.rule<AFloat>();

    expose.rule<Padding>()
            .ctor<unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>>()
            .ctor<unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>, unset_wrap<AMetric>>()
            ;

    expose.rule<ATextAlign>("TextAlign");

    expose.rule<ATextOverflow>("TextOverflow");

    lua.register_enum<ScrollbarAppearance::AxisValue>("ScrollbarAppearanceAxis");
    lua.register_enum<ScrollbarAppearance::AxisValue>("AxisValue"); // fallback

    expose.rule<ScrollbarAppearance>()
            .ctor<ScrollbarAppearance::AxisValue, ScrollbarAppearance::AxisValue>()
            .ctor<ScrollbarAppearance::AxisValue>();

    expose.rule<TextBorder>().ctor<AColor>();

    expose.rule<TextColor>().ctor<AColor>();

    expose.rule<TextShadow>().ctor<AColor>();

    expose.rule<TextTransform>();

    expose.rule<TransformOffset>().ctor<AMetric, AMetric>();
    expose.rule<TransformRotate>()
            .ctor<AAngleRadians>();

    expose.rule<TransformScale>()
            .ctor<float>()
            .ctor<float, float>()
            ;

    expose.rule<VerticalAlign>();

    expose.rule<Visibility, false>("VisibilityStyle")
            .ctor<Visibility>();

    lua.register_enum<Visibility>();


    lua.register_function("Grid", [this](clg::table_array args) {
        std::size_t rows = args.size();
        if (rows == 0) {
            auto container = _new<LuaExposedView<AViewContainer>>(*this);
            container->setLayout(std::make_unique<AAdvancedGridLayout>(1, 1));
            return container;
        }
        std::size_t columns = -1;

        AVector<_<AView>> views;
        for (std::size_t r = 0; r < rows; ++r) {
            auto row = args[r].as<clg::table_array>();
            if (columns == -1) {
                columns = row.size();
                views.reserve(columns * rows);
            } else if (columns != row.size()) {
                throw std::runtime_error("row " + std::to_string(r + 1) + " has different column count: expected " + std::to_string(columns) + ", got " + std::to_string(row.size()));
            }
            for (std::size_t c = 0; c < columns; ++c) {
                views << row[c].as<_<AView>>();
            }
        }

        auto container = _new<LuaExposedView<AViewContainer>>(*this);
        {
            // for data holder initializating
            clg::push_to_lua(clg::state(), container);
            clg::pop_from_lua<decltype(container)>(clg::state());
        }
        container->setLayout(std::make_unique<AAdvancedGridLayout>(columns, rows));
        auto childrenTable = UIEngine::luaChildrenTable(container);
        for (auto v : views) {
            childrenTable[v] = true;
        }
        container->addViews(std::move(views));
        return container;
    });

    StateHelper::initLua(lua);
}

_<AView> UIEngine::loadForm(std::string_view file) {
    /*
    APath fullpath = APath("ui") / file;
    if (fullpath.isRegularFileExists()) {
        try {
            mState.do_file(fullpath.toStdString());
        } catch (const std::exception& e) {
            ALogger::err(LOG_TAG) << "Unable to load form: " << e;
        }
    }*/
    return nullptr;
}

const char* UIEngine::anyToString(const clg::ref& r) {
    static std::string s;
    s = r.debug_str();
    return s.c_str();
}

const char* UIEngine::anyToString(lua_State* L, int n) {
    static std::string s;
    s = clg::any_to_string(L, n);
    return s.c_str();
}


std::string UIEngine::clgDump(clg::ref any) {
    return any.debug_str();
}

clg::table_view UIEngine::luaChildrenTable(const _<AViewContainer>& cont) {
    if (auto luaView = _cast<ILuaExposedView>(cont)) {
        return luaView->luaDataHolder()
                    .get_or_create("cpp_children", []() { return clg::table{}; })
                    .as<clg::table_view>();
    }

    return clg::ref(nullptr);
}

void UIEngine::addChild(const _<AViewContainer>& cont, const _<AView>& view) {
    assert(view != nullptr);
    AUI_NULLSAFE(luaChildrenTable(cont))[view] = true;
}

void UIEngine::removeChild(const _<AViewContainer>& cont, const _<AView>& view) {
    assert(view != nullptr);
    AUI_NULLSAFE(luaChildrenTable(cont))[view] = clg::ref(nullptr);
}

void UIEngine::removeAllChildren(const _<AViewContainer>& cont) {
    if (auto luaView = _cast<ILuaExposedView>(cont)) {
        luaView->luaDataHolder()["cpp_children"] = clg::ref(nullptr);
    }
}

_<AView> UIEngine::wrapViewWithLuaWrapper(const _<AView>& v) {
    auto container = _new<LuaExposedView<AViewContainer>>(*this);
    v->setExpanding();
    container->setLayout(std::make_unique<AStackedLayout>());
    container->addView(v);
    return container;
}
