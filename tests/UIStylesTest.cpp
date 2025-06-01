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
// Created by Admin on 7/24/2022.
//

#include <gtest/gtest.h>
#include <AUI/ASS/ASS.h>
#include "uiengine/UIEngine.h"

using namespace ass;

class UIStylesTest : public ::testing::Test {
protected:
    UIStylesTest() : mUiEngine(mFakeSurface) {}

    template <typename T>
    T fromLua(std::string_view luaCode) {
        auto val = mLua.do_string<std::shared_ptr<ass::prop::IPropertyBase>>("return " + std::string(luaCode));
        EXPECT_NE(val, nullptr);
        return std::dynamic_pointer_cast<ass::prop::Property<T>>(val)->value();
    }

protected:
    clg::vm mLua;
    AViewContainer mFakeSurface;
    UIEngine mUiEngine;
};

TEST_F(UIStylesTest, TextOverflow) {
    EXPECT_EQ(fromLua<ATextOverflow>("ATextOverflow.ELLIPSIS"), ATextOverflow::ELLIPSIS);
    EXPECT_EQ(fromLua<ATextOverflow>("ATextOverflow.CLIP"), ATextOverflow::CLIP);
    EXPECT_EQ(fromLua<ATextOverflow>("ATextOverflow.NONE"), ATextOverflow::NONE);
}

TEST_F(UIStylesTest, TextAlign) {
    EXPECT_EQ(fromLua<ATextAlign>("ATextAlign.LEFT"), ATextAlign::LEFT);
    EXPECT_EQ(fromLua<ATextAlign>("ATextAlign.CENTER"), ATextAlign::CENTER);
    EXPECT_EQ(fromLua<ATextAlign>("ATextAlign.RIGHT"), ATextAlign::RIGHT);
}

TEST_F(UIStylesTest, VerticalAlign) {
    EXPECT_EQ(fromLua<VerticalAlign>("VerticalAlign.DEFAULT"), VerticalAlign::DEFAULT);
    EXPECT_EQ(fromLua<VerticalAlign>("VerticalAlign.MIDDLE"), VerticalAlign::MIDDLE);
}

TEST_F(UIStylesTest, Padding) {
    auto asPadding = fromLua<Padding>("Padding(10, 20)");
    EXPECT_EQ(asPadding.top->getValueDp(), 10.f);
    EXPECT_EQ(asPadding.bottom->getValueDp(), 10.f);
    EXPECT_EQ(asPadding.left->getValueDp(), 20.f);
    EXPECT_EQ(asPadding.right->getValueDp(), 20.f);
}
TEST_F(UIStylesTest, PaddingUnset) {
    auto asPadding = fromLua<Padding>("Padding({}, 20)");
    EXPECT_FALSE(asPadding.top);
    EXPECT_FALSE(asPadding.bottom);
    EXPECT_EQ(asPadding.left->getValueDp(), 20.f);
    EXPECT_EQ(asPadding.right->getValueDp(), 20.f);
}

TEST_F(UIStylesTest, BackgroundSolid_White) {
    auto asBackgroundSolid = fromLua<BackgroundSolid>("BackgroundSolid('#fff')");
    EXPECT_EQ(asBackgroundSolid.color, AColor::WHITE);
}
TEST_F(UIStylesTest, BackgroundSolid_Null) {
    auto asBackgroundSolid = fromLua<BackgroundSolid>("BackgroundSolid(nil)");
    EXPECT_EQ(asBackgroundSolid.color, AColor(0.f));
}

// overloaded function substitution error: unimplemented converter for
TEST_F(UIStylesTest, BackgroundCropping) {
    auto asBackgroundCropping = fromLua<BackgroundCropping>("BackgroundCropping({1, 2}, {3, 4})");
    EXPECT_EQ(asBackgroundCropping.offset, glm::vec2(1, 2));
    EXPECT_EQ(asBackgroundCropping.size, glm::vec2(3, 4));
}
// как сравнить то
TEST_F(UIStylesTest, BackgroundImage) {
    auto asBackgroundImage = fromLua<BackgroundImage>("BackgroundImage(nil)");
    EXPECT_EQ(std::get<AString>(*asBackgroundImage.image), AString(""));
}

TEST_F(UIStylesTest, BackgroundImage2) {
    auto asBackgroundImage = fromLua<BackgroundImage>("BackgroundImage({}, '#f00', {}, {})");
    EXPECT_FALSE(asBackgroundImage.image);
    EXPECT_EQ(asBackgroundImage.overlayColor, AColor::RED);
}

TEST_F(UIStylesTest, Border_2args) {
    auto asBorder = fromLua<Border>("Border(10, '#fff')");
    EXPECT_EQ(asBorder.width.getValueDp(), 10.f);
    EXPECT_EQ(asBorder.color, AColor::WHITE);
}
TEST_F(UIStylesTest, Border_Null) {
    auto asBorder = fromLua<Border>("Border(nil)");
    EXPECT_EQ(asBorder.width.getValueDp(), 0.f);
    EXPECT_EQ(asBorder.color, AColor::BLACK);
}
// C++ exception with description "[string "return BorderBottom(0, '#000')"]:1: attempt to call global 'BorderBottom' (a
// nil value)" thrown in the test body.
TEST_F(UIStylesTest, BorderBottom) {
    auto asBorderBottom = fromLua<BorderBottom>("BorderBottom(0, '#000')");
    EXPECT_EQ(asBorderBottom.width.getValueDp(), 0.f);
    EXPECT_EQ(asBorderBottom.color, AColor::BLACK);
}

// C++ exception with description "[string "return BorderBottom(0, '#000')"]:1: attempt to call global 'BorderBottom' (a
// nil value)" thrown in the test body.
TEST_F(UIStylesTest, BorderLeft) {
    auto asBorderLeft = fromLua<BorderLeft>("BorderLeft(nil)");
    EXPECT_EQ(asBorderLeft.width.getValueDp(), 0.f);
    EXPECT_EQ(asBorderLeft.color, AColor::BLACK);
}

TEST_F(UIStylesTest, BoxShadow_4args) {
    auto asBoxShadow = fromLua<BoxShadow>("BoxShadow(1, 2, 3, '#fff')");
    EXPECT_EQ(asBoxShadow.offsetX.getValueDp(), 1.f);
    EXPECT_EQ(asBoxShadow.offsetY.getValueDp(), 2.f);
    EXPECT_EQ(asBoxShadow.blurRadius.getValueDp(), 3.f);
    EXPECT_EQ(asBoxShadow.color, AColor::WHITE);
}
TEST_F(UIStylesTest, BoxShadow_5args) {
    auto asBoxShadow = fromLua<BoxShadow>("BoxShadow(1, 2, 3, 4, '#fff')");
    EXPECT_EQ(asBoxShadow.offsetX.getValueDp(), 1.f);
    EXPECT_EQ(asBoxShadow.offsetY.getValueDp(), 2.f);
    EXPECT_EQ(asBoxShadow.blurRadius.getValueDp(), 3.f);
    EXPECT_EQ(asBoxShadow.spreadRadius.getValueDp(), 4.f);
    EXPECT_EQ(asBoxShadow.color, AColor::WHITE);
}

TEST_F(UIStylesTest, Expanding_2args) {
    auto asExpanding = fromLua<Expanding>("Expanding(1, 2)");
    EXPECT_EQ(asExpanding.expandingX, unset_wrap<unsigned int>(1));
    EXPECT_EQ(asExpanding.expandingY, unset_wrap<unsigned int>(2));
}
TEST_F(UIStylesTest, Expanding_1args) {
    auto asExpanding = fromLua<Expanding>("Expanding(1)");
    EXPECT_EQ(asExpanding.expandingX, unset_wrap<unsigned int>(1));
    EXPECT_EQ(asExpanding.expandingY, unset_wrap<unsigned int>(1));
}

TEST_F(UIStylesTest, FixedSize_2args) {
    auto asFixedSize = fromLua<FixedSize>("FixedSize(1, 2)");
    EXPECT_EQ(asFixedSize.width->getValueDp(), 1.0f);
    EXPECT_EQ(asFixedSize.height->getValueDp(), 2.0f);
}
TEST_F(UIStylesTest, FixedSize_1args) {
    auto asFixedSize = fromLua<FixedSize>("FixedSize(1)");
    EXPECT_EQ(asFixedSize.width->getValueDp(), 1.0f);
    EXPECT_EQ(asFixedSize.height->getValueDp(), 1.0f);
}

TEST_F(UIStylesTest, Font) {
    auto asFont = fromLua<Font>("Font('hehe')");
    EXPECT_EQ(asFont.url.path(), AString("hehe"));
}

TEST_F(UIStylesTest, FontFamily) {
    auto asFontFamily = fromLua<FontFamily>("FontFamily('hehe')");
    EXPECT_EQ(asFontFamily.family, AString("hehe"));
}

TEST_F(UIStylesTest, FontSize) {
    auto asFontSize = fromLua<FontSize>("FontSize(10)");
    EXPECT_EQ(asFontSize.size.getValueDp(), 10.0f);
}

TEST_F(UIStylesTest, LayoutSpacing) {
    auto asLayoutSpacing = fromLua<LayoutSpacing>("LayoutSpacing(10)");
    EXPECT_EQ(asLayoutSpacing.spacing.getValueDp(), 10.0f);
}

// Expected equality of these values:
// asLineHeight.spacing
//         Which is: 15
//           10.0
//         Which is: 10
TEST_F(UIStylesTest, LineHeight) {
    auto asLineHeight = fromLua<LineHeight>("LineHeight(10)");
    EXPECT_EQ(asLineHeight.spacing, 10.0f);
}

TEST_F(UIStylesTest, Margin_1arg) {
    auto asMargin = fromLua<Margin>("Margin(10)");
    EXPECT_EQ(asMargin.top->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.left->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.right->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.bottom->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, Margin_2arg) {
    auto asMargin = fromLua<Margin>("Margin(10, 20)");
    EXPECT_EQ(asMargin.top->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.left->getValueDp(), 20.0f);
    EXPECT_EQ(asMargin.right->getValueDp(), 20.0f);
    EXPECT_EQ(asMargin.bottom->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, Margin_3arg) {
    auto asMargin = fromLua<Margin>("Margin(10, 20, 30)");
    EXPECT_EQ(asMargin.top->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.left->getValueDp(), 20.0f);
    EXPECT_EQ(asMargin.right->getValueDp(), 20.0f);
    EXPECT_EQ(asMargin.bottom->getValueDp(), 30.0f);
}
TEST_F(UIStylesTest, Margin_4arg) {
    auto asMargin = fromLua<Margin>("Margin(10, 20, 30, 40)");
    EXPECT_EQ(asMargin.top->getValueDp(), 10.0f);
    EXPECT_EQ(asMargin.left->getValueDp(), 40.0f);
    EXPECT_EQ(asMargin.right->getValueDp(), 20.0f);
    EXPECT_EQ(asMargin.bottom->getValueDp(), 30.0f);
}

TEST_F(UIStylesTest, MaxSize_1arg) {
    auto asMaxSize = fromLua<MaxSize>("MaxSize(10)");
    EXPECT_EQ(asMaxSize.width->getValueDp(), 10.0f);
    EXPECT_EQ(asMaxSize.height->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, MaxSize_2arg) {
    auto asMaxSize = fromLua<MaxSize>("MaxSize(10, 40)");
    EXPECT_EQ(asMaxSize.width->getValueDp(), 10.0f);
    EXPECT_EQ(asMaxSize.height->getValueDp(), 40.0f);
}

TEST_F(UIStylesTest, MinSize_1arg) {
    auto asMinSize = fromLua<MinSize>("MinSize(10)");
    EXPECT_EQ(asMinSize.width->getValueDp(), 10.0f);
    EXPECT_EQ(asMinSize.height->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, MinSize_2arg) {
    auto asMinSize = fromLua<MinSize>("MinSize(10, 40)");
    EXPECT_EQ(asMinSize.width->getValueDp(), 10.0f);
    EXPECT_EQ(asMinSize.height->getValueDp(), 40.0f);
}

TEST_F(UIStylesTest, Opacity) {
    auto asOpacity = fromLua<Opacity>("Opacity(10)");
    EXPECT_EQ(asOpacity.opacity, 10.0f);
}

TEST_F(UIStylesTest, Padding_1arg) {
    auto asPadding = fromLua<Padding>("Padding(10)");
    EXPECT_EQ(asPadding.top->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.left->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.right->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.bottom->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, Padding_2arg) {
    auto asPadding = fromLua<Padding>("Padding(10, 20)");
    EXPECT_EQ(asPadding.top->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.left->getValueDp(), 20.0f);
    EXPECT_EQ(asPadding.right->getValueDp(), 20.0f);
    EXPECT_EQ(asPadding.bottom->getValueDp(), 10.0f);
}
TEST_F(UIStylesTest, Padding_3arg) {
    auto asPadding = fromLua<Padding>("Padding(10, 20, 30)");
    EXPECT_EQ(asPadding.top->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.left->getValueDp(), 20.0f);
    EXPECT_EQ(asPadding.right->getValueDp(), 20.0f);
    EXPECT_EQ(asPadding.bottom->getValueDp(), 30.0f);
}
TEST_F(UIStylesTest, Padding_4arg) {
    auto asPadding = fromLua<Padding>("Padding(10, 20, 30, 40)");
    EXPECT_EQ(asPadding.top->getValueDp(), 10.0f);
    EXPECT_EQ(asPadding.left->getValueDp(), 40.0f);
    EXPECT_EQ(asPadding.right->getValueDp(), 20.0f);
    EXPECT_EQ(asPadding.bottom->getValueDp(), 30.0f);
}

TEST_F(UIStylesTest, TextBorder) {
    auto asTextBorder = fromLua<TextBorder>("TextBorder('#fff')");
    EXPECT_EQ(asTextBorder.borderColor, AColor::WHITE);
}

TEST_F(UIStylesTest, TextColor) {
    auto asTextColor = fromLua<TextColor>("TextColor('#fff')");
    EXPECT_EQ(asTextColor.color, AColor::WHITE);
}

TEST_F(UIStylesTest, TextShadow) {
    auto asTextShadow = fromLua<TextShadow>("TextShadow('#fff')");
    EXPECT_EQ(asTextShadow.shadowColor, AColor::WHITE);
}

TEST_F(UIStylesTest, TransformScale_1arg) {
    auto asTransformScale = fromLua<TransformScale>("TransformScale(1)");
    EXPECT_EQ(asTransformScale.scale.x, 1.0f);
    EXPECT_EQ(asTransformScale.scale.y, 1.0f);
}

TEST_F(UIStylesTest, TransformScale_2arg) {
    auto asTransformScale = fromLua<TransformScale>("TransformScale(1, 2)");
    EXPECT_EQ(asTransformScale.scale.x, 1.0f);
    EXPECT_EQ(asTransformScale.scale.y, 2.0f);
}
