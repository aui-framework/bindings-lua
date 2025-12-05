// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <clg.hpp>
#include <AUI/View/ACheckBox.h>
#include "AUI/Test/UI/Action/MouseMove.h"
#include "AUI/Test/UI/By.h"
#include "uiengine/UIEngine.h"
#include "AUI/Test/UI/Assertion/Color.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ATextField.h"
#include "View/MyTextField.h"
#include "View/MyScrollbar.h"

namespace {
class TestWindow : public AWindow {
public:
    TestWindow(clg::vm& lua, std::string_view luaCode) : AWindow("Test window", 200_dp, 100_dp), mUiEngine(*this) {
        lua.do_string(std::string(luaCode));
    }
    ~TestWindow() {}

private:
    UIEngine mUiEngine;
};

class UIEngineTest : public testing::UITest {
public:

protected:
    void SetUp() override { UITest::SetUp(); }

    void test(std::string_view luaCode) { _new<TestWindow>(mLua, luaCode)->show(); }

    clg::vm mLua;
};
}   // namespace

TEST_F(UIEngineTest, Basic) {
    test(R"(
UI.setSurface(Button("Hello"))
)");
    EXPECT_FALSE(By::text("Hello").toSet().empty());
}

TEST_F(UIEngineTest, Clicked) {
    bool called = false;
    mLua.register_function("handleClicked", [&] { called = true; });
    test(R"(
UI.setSurface(Button("Hello"):clicked(function() handleClicked() end))
)");
    By::text("Hello").perform(click());
    EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, ClickedSelf) {
    test(R"(
UI.setSurface(Button("Disable me"):clicked(function(self) self:setEnabled(false) end))
)");
    EXPECT_FALSE(By::text("Disable me").toSet().empty());
    By::text("Disable me").perform(click());

    EXPECT_FALSE(By::text("Disable me").one()->enabled());
}

TEST_F(UIEngineTest, Containers) {
    test(R"(
f = Vertical(
  Button("Hello"),
  Button("World")
)
UI.setSurface(f)
)");
    EXPECT_EQ(By::type<AButton>().toSet().size(), 2);
    By::text("Hello").check(isBottomAboveBottomOf(By::text("World")), "buttons are not aligned");
}

TEST_F(UIEngineTest, ContainersNested) {
    test(R"(
f = Vertical(
  Button("Top"),
  Horizontal(
    Button("Left"):expanding(),
    Button("Right"):expanding()
  ),
  Button("Bottom")
)
UI.setSurface(f)
)");
    EXPECT_EQ(By::type<AButton>().toSet().size(), 4);
}

TEST_F(UIEngineTest, CustomFormVariables1) {
    test(R"(
login = Input('')
password = Input(''):setPasswordMode(true)
function performLogin()
  game_login(login:text(), password:text())
end

form = Vertical(
  Label("Login:"),
  login,
  Label("Password:"),
  password,
  Button("Auth"):clicked(performLogin)
)
UI.setSurface(form)
)");
    bool called = false;

    mLua.register_function("game_login", [&](std::string login, std::string password) {
        called = true;
        EXPECT_EQ(login, "test");
        EXPECT_EQ(password, "123");
    });

    auto s = mLua.do_string<std::string>("return login:text()");
    EXPECT_TRUE(s.empty());

    s = mLua.do_string<std::string>("return password:text()");
    EXPECT_TRUE(s.empty());

    By::type<ATextField>().findNearestTo(By::text("Login:")).perform(type("test"));
    By::type<ATextField>().findNearestTo(By::text("Password:")).perform(type("123"));
    By::text("Auth").perform(click());

    EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, Centered) {
    test(R"(
inner = Label "Test" -- global variable
local cont = Centered(inner)

UI.setSurface(cont)
)");
    auto input = By::text("Test").one();
    uitest::frame();
}

TEST_F(UIEngineTest, setTextTest) {
    test(R"(
input = Input('')

form = Vertical(
  Label("Input:"),
  input
)
UI.setSurface(form)
)");
    auto input = By::type<MyTextField>().findNearestTo(By::text("Input:")).perform(click()).perform(type("test"));
    mLua.do_string("input:setText('test')");
    uitest::frame();
}

TEST_F(UIEngineTest, CustomFormVariables2) {
    test(R"(
function createLoginForm()
    login_label = Label("Login:")
    login_input = Input(''):expanding()

    password_label = Label("Password:")
    password_input = Input(''):setPasswordMode(true):expanding()

    button_ok = Button("Ok")
    button_cancel = Button("Cancel")

    login_horizontal = {
        login_label,
        login_input
    }

    password_horizontal = {
        password_label,
        password_input
    }

    button_horizontal = Horizontal(
        button_ok,
        button_cancel
    )

    form = Vertical(
        Grid{
            login_horizontal,
            password_horizontal
        },
        Horizontal(
            Spacer(),
            Checkbox()
        ),
        Centered(
            button_horizontal
        )
    )

    return form
end

loginForm = createLoginForm()
UI.setSurface(loginForm)

)");
    bool called = false;

    mLua.register_function("game_login", [&](std::string login, std::string password) {
        called = true;
        EXPECT_EQ(login, "test");
        EXPECT_EQ(password, "123");
    });

    By::type<MyTextField>().findNearestTo(By::text("Login:")).perform(type("test"));
    By::type<MyTextField>().findNearestTo(By::text("Password:")).perform(type("123"));
    // By::text("Auth").perform(click());

    // EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, Styling) {
    test(R"(

loginStyle = {
    ["Button"] = {
        BackgroundSolid('#f00'),
        ["hover"] = {
            BackgroundSolid('#0f0')
        },
    },
    ["Input"] = {
        BackgroundSolid('#00f')
    },
}

function createLoginForm()
    login_label = Label("Login:")
    login_input = Input(''):expanding()

    password_label = Label("Password:")
    password_input = Input(''):setPasswordMode(true):expanding()

    button_ok = Button("Ok")
    button_cancel = Button("Cancel")

    login_horizontal = {
        login_label,
        login_input
    }

    password_horizontal = {
        password_label,
        password_input
    }

    button_horizontal = Horizontal(
        button_ok,
        button_cancel
    )
    c = Centered(button_horizontal)
    c:setStyle({
        Padding(32)
    })

    form = Vertical(
        Grid{
            login_horizontal,
            password_horizontal
        },
        Horizontal(
            Spacer(),
            Checkbox()
        ),
        c
    )

    form:setStyle(loginStyle)

    return form
end

loginForm = createLoginForm()
UI.setSurface(loginForm)

)");
    bool called = false;

    mLua.register_function("game_login", [&](std::string login, std::string password) {
        called = true;
        EXPECT_EQ(login, "test");
        EXPECT_EQ(password, "123");
    });

    By::type<MyTextField>().findNearestTo(By::text("Login:")).perform(type("test"));
    By::type<MyTextField>().findNearestTo(By::text("Password:")).perform(type("123"));
    By::text("Ok").check(averageColor(0xff0000_rgb), "the styles were not applied");

    auto d = uitest::verticalDistanceBetween(By::text("Cancel").one(), By::type<ACheckBox>().one());
    EXPECT_LE(32, d);

    // EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, UnsetWrap) {
    test("");
    {
        auto val = mLua.do_string<ass::unset_wrap<float>>("return {}");
        EXPECT_FALSE(bool(val));
    }
    {
        auto val = mLua.do_string<ass::unset_wrap<float>>("return 1.0");
        EXPECT_TRUE(bool(val));
        float v = val;
        EXPECT_EQ(v, 1.0f);
    }
}

TEST_F(UIEngineTest, Drag) {
    test(R"(
f = Stacked(
    DragHandle(),
    Label("Hello")
)
UI.setSurface(DragArea(f))
)");
    EXPECT_FALSE(By::text("Hello").toSet().empty());
}

TEST_F(UIEngineTest, EmptyContainer) {
    test(R"(
UI.setSurface(Vertical())
)");
}

TEST_F(UIEngineTest, InnerCheckboxColoring) {
    test(R"(
UI.setSurface(Checkbox():setStyle {
  Checkbox = {
    BackgroundSolid('#f00')
  }
})
)");
    By::type<ACheckBox>().check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, AddStylesheetName) {
    test(R"(
Cell = Centered {
    Checkbox():addStylesheetName("CheckboxStyle")
}

root = Stacked {
    Vertical {
        Cell
    }
}:setStyle({
    CheckboxStyle = {
        BackgroundSolid('#f00')
    }
})

UI.setSurface(root)
)");
    By::name("CheckboxStyle").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, InnerCheckboxChecked) {
    bool checked = false;
    mLua.register_function("handleChecked", [&] { checked = true; });
    test(R"(
checkbox = Checkbox()
UI.setSurface(checkbox:checked(function() handleChecked() end):setStyle {
  Checkbox = {
    checked = {
      BackgroundSolid('#f00'),
      BackgroundImage(nil),
    }
  }
})
)");
    EXPECT_FALSE(mLua.do_string<bool>("return checkbox:isChecked()"));
    By::type<ACheckBox>().perform(click()).check(averageColor(0xff0000_rgb));
    EXPECT_TRUE(checked);
    EXPECT_TRUE(mLua.do_string<bool>("return checkbox:isChecked()"));
}

TEST_F(UIEngineTest, Scrollbar) {
    test(R"(
UI.setSurface(Scrollbar(ALayoutDirection.VERTICAL):setStyle {
  Scrollbar = {
    BackgroundSolid('#f00')
  }
})
)");
    By::name("Scrollbar").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, ScrollAreaScrollbar) {
    test(R"(
local scroll_inner = Vertical()
scroll_inner:addView(Button("Кнопка"):setStyle {
    FixedSize(500, 50)
})

local scroll_area = ScrollArea(scroll_inner):setStyle{
    Scrollbar = {
        BackgroundSolid("#0000ff")
    }
}

UI.setSurface(scroll_area)
)");
    By::exactType<MyScrollbar>().check(averageColor(0x0000ff_rgb));
}

TEST_F(UIEngineTest, ScrollbarButton) {
    test(R"(
UI.setSurface(Scrollbar(ALayoutDirection.VERTICAL):setStyle {
  ScrollbarButton = {
    BackgroundSolid('#f00')
  }
})
)");
    By::name("ScrollbarButton").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, addViewCustomLayout) {
    test(R"(
local root = Stacked():setStyle{ BackgroundSolid("#2e3440") }
local hint = Centered(
   Label("Test with label"):setStyle {
    Margin(8),
    TextColor("#d8dee9")
   }):setStyle {
   BackgroundSolid("#4c566a"),
   Padding(2),
  }
local hint2 =Centered(
   Text({"Test with text"}):setStyle {
    Margin(8),
    TextColor("#d8dee9")
   }):setStyle {
   BackgroundSolid("#4c566a"),
   Padding(2),
  }
root:addViewCustomLayout(hint)
root:addViewCustomLayout(hint2)
  hint:setPos {5, 50}
hint2:setPos {100, 5}
UI.setSurface(root)
)");
    uitest::frame();
}

TEST_F(UIEngineTest, Progressbar) {
    test(R"(
pb = Progressbar()
pb:setValue(1.0)
pb:setStyle {
  ProgressbarInner = {
    BackgroundSolid('#f00'),
    MinSize(100, {}),
  }
}
UI.setSurface(pb)
)");
    By::name("Progressbar").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, CustomViewOverride) {
    test(R"(
called = false

view = Button('Gavno'):expanding():addStylesheetName(".target")

function view:onPointerPressed()
  called = true
end


UI.setSurface(view)
)");
    By::name(".target").perform(click());

    EXPECT_TRUE(mLua.global_variable("called").as<bool>()) << "not called";
}

TEST_F(UIEngineTest, CustomViewOverrideTwice) {
    test(R"(
called = false

view = Button('Gavno'):expanding():addStylesheetName(".target")

function view:onPointerPressed()

end

function view:onPointerPressed()
  called = true
end


UI.setSurface(view)
)");
    By::name(".target").perform(click());

    EXPECT_TRUE(mLua.global_variable("called").as<bool>()) << "not called";
}

TEST_F(UIEngineTest, RemoveView) {
    test(R"(
v = View():setStyle({
  BackgroundSolid('#f00')
}):expanding()

container = Centered(v):addStylesheetName(".container"):setStyle {
  MinSize(20, 20)
}

UI.setSurface(container)
)");
    By::name(".container").check(averageColor(0xff0000_rgb));   // check for item exists

    test(R"(
container:removeView(v)
)");
    By::name(".container").check(averageColor(0xffffff_rgb));   // check for item disappeared
}

TEST_F(UIEngineTest, DragNDropTest) {
    test(R"(

root = Centered()

function ItemCell(view)
  local cell = Centered(view):setStyle({
    FixedSize(32),
    Border(1, '#000')
  })
  cell.view = view
  cell.drag = nil

  function cell:onPointerPressed(event)
    pos = event.position
    if self.view ~= nil then
      local viewPosition = self.view
      self.drag = {
        view = self.view,
        offset = pos
      }

      self:removeView(self.view)
      root:addViewCustomLayout(self.view)
      self.view = nil
      self:focus() -- см. cell:capturesFocus
      self:onPointerMove(pos) -- после перемещения view в другой контейнер нужно сразу же выставить ему новые координаты
    end
  end

  function cell:capturesFocus()
    return true -- нужно, чтобы контейнер отловил фокус. это позволит получать события mouse move и released даже если курсор находится за пределами контейнера
  end

  function cell:onPointerMove(pos)
    if self.drag ~= nil then
      local cellLocalCoords = { pos[1] - self.drag.offset[1],
                                pos[2] - self.drag.offset[2] }

      self.drag.view:setPos(self:mapLocalCoordsToGlobal(cellLocalCoords))
    end
  end

  function cell:onPointerReleased(event)
    pos = event.position
    if self.drag ~= nil then
      success = root:visitViewsUnderPos(self:mapLocalCoordsToGlobal(pos), function(v)
        if v.acceptDrop ~= nil then
          v:acceptDrop(self.drag.view)
          return true
        end
        return false
      end)
      if not success then
        self:acceptDrop(self.drag.view)
      end
      self.drag = nil
    end
  end

  function cell:acceptDrop(view)
    root:removeView(view)
    self.view = view
    self:addView(view)
    self:updateLayout() -- дать просраться менеджеру компоновки и корректно выставить ItemView
  end

  return cell
end

function ItemView()
  v = View():setStyle({
    BackgroundSolid('#f00'),
    FixedSize(30),
  }):addStylesheetName(".item")
  return v
end

root:addView(Horizontal {
    ItemCell(ItemView()):addStylesheetName(".from"),
    ItemCell():addStylesheetName(".to"),
})

UI.setSurface(root)
)");
    By::name(".from")
        .check(averageColor(0xff0000_rgb))   // check for item in the first cell
        .perform(mousePress());

    By::name(".to").perform(mouseMove());

    mLua.collectGarbage();

    By::name(".to").perform(mouseRelease());

    By::name(".to").check(averageColor(0xff0000_rgb), "visual appear check");   // check item appeared in the second
                                                                                // cell
    By::name(".from").check(averageColor(0xffffff_rgb), "visual disappear check");   // check item disappeared from the
                                                                                     // first cell

    By::name(".to").check(isParentOf(By::name(".item").one()), "parent check");   // check item is actual child of the
                                                                                  // second cell
}

TEST_F(UIEngineTest, TextColor) {
    test(R"(
UI.setSurface(Vertical {
  Text({'!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'}):addStylesheetName(".target"):setStyle {
    TextColor('#f00'),
  }
})
)");
    By::name(".target").check(averageColor(0xff8888_rgb));   // check for color appliance
}

TEST_F(UIEngineTest, OnChildFocused) {
    test(R"(
elements = Vertical {
  Input(""),
}

function callback(self, c)
    self:setVisibility(Visibility.GONE)
    c:setVisibility(Visibility.GONE)
end

elements:onChildFocused(callback)
UI.setSurface(elements)
)");

    auto inputView = mLua.do_string<_<AView>>("return elements:getViews()[1]");
    auto containerView = mLua.do_string<_<AView>>("return elements");
    EXPECT_EQ(inputView->getVisibility(), Visibility::VISIBLE);
    EXPECT_EQ(containerView->getVisibility(), Visibility::VISIBLE);

    inputView->focus();

    EXPECT_EQ(inputView->getVisibility(), Visibility::GONE);
    EXPECT_EQ(containerView->getVisibility(), Visibility::GONE);
}

TEST_F(UIEngineTest, MouseEnterLeave) {
    test(R"(
enterCalled = false
leaveCalled = false

probe = Input(""):addStylesheetName(".probe")

function probe:onMouseEnter()
  enterCalled = true
end

function probe:onMouseLeave()
  leaveCalled = true
end

UI.setSurface(Vertical {
  probe,
  Button("Another Button")
})
)");

    By::name(".probe").perform(mouseMove());
    EXPECT_TRUE(mLua.global_variable("enterCalled").as<bool>());
    By::text("Another Button").perform(mouseMove());
    EXPECT_TRUE(mLua.global_variable("leaveCalled").as<bool>());
}

TEST_F(UIEngineTest, Grid) {
    test(R"(
UI.setSurface(Grid{
  { Label("1"), Label("2") },
  { Label("3"), Label("4") },
})
)");

    (By::text("1") | By::text("2")).check(isBottomAboveBottomOf(By::text("3") | By::text("4")));
}

TEST_F(UIEngineTest, ScrollStickToEnd1) {
    test(R"(
elements = Vertical {
  Button("Hello"),
  Button("Hello"),
  Button("Hello"),
  Button("Hello"),
}
UI.setSurface(ScrollArea(
  elements
):setStyle{
  FixedSize({}, 100)
}:addStylesheetName("scrollarea"))
)");
    By::name("scrollarea").check(averageColor(0xff'ff'ff_rgb));

    mLua.do_string(R"(
elements:addView(View():setStyle{
  BackgroundSolid('#f00'),
  FixedSize({}, 100),
})
)");

    By::name("scrollarea").check(averageColor(0xff'ff'ff_rgb));
}

TEST_F(UIEngineTest, ScrollStickToEnd2) {
    test(R"(
elements = Vertical {
  Button("Hello"),
  Button("Hello"),
  Button("Hello"),
  Button("Hello"),
}
UI.setSurface(ScrollArea(
  elements
):setStyle{
  FixedSize(100, 100)
}:addStylesheetName("scrollarea"):setStickToEnd(true))
)");
    By::name("scrollarea").check(averageColor(0xff'ff'ff_rgb));

    mLua.do_string(R"(
elements:addView(View():setStyle{
  BackgroundSolid('#f00'),
  FixedSize({}, 100),
})
)");

    By::name("scrollarea").check(averageColor(0xff'00'00_rgb));
}

TEST_F(UIEngineTest, CompositeSelector) {
    test(R"(
local target = View():setStyle {
    FixedSize(4)
}
target:addStylesheetName("Target")
UI.setSurface(Horizontal {
    View():addStylesheetName "HoverMe" :setStyle {
        FixedSize(100)
    },
    target
}:setStyle {
    ViewContainer = {
        Target = {
            BackgroundSolid('#0f0'),
        }
    }
})
)");
    By::name("HoverMe").check(averageColor(AColor::WHITE));
    By::name("Target").check(averageColor(AColor::GREEN));
}

TEST_F(UIEngineTest, CompositeSelectorHover) {
    test(R"(
local target = View():setStyle {
    FixedSize(4)
}
target:addStylesheetName("Target")
UI.setSurface(Horizontal {
    View():addStylesheetName "HoverMe" :setStyle {
        FixedSize(100)
    },
    target
}:setStyle {
    ViewContainer = {
        hover = {
            Target = {
                BackgroundSolid('#0f0'),
            }
        }
    }
})
)");
    By::name("Target").check(averageColor(AColor::WHITE));
    By::name("HoverMe").perform(mouseMove());
    By::name("HoverMe").check(averageColor(AColor::WHITE));
    By::name("Target").check(averageColor(AColor::GREEN));
}

TEST_F(UIEngineTest, GetViews) {
    test(R"(
elements = Vertical {
  Button("1"),
  Button("2"),
}
UI.setSurface(elements)
)");
    auto button1View = mLua.do_string<_<AView>>("return elements:getViews()[1]");
    auto button2View = mLua.do_string<_<AView>>("return elements:getViews()[2]");

    EXPECT_NE(button1View, nullptr);
    EXPECT_NE(button2View, nullptr);

    auto button1 = _cast<AButton>(button1View);
    auto button2 = _cast<AButton>(button2View);

    EXPECT_NE(button1, nullptr);
    EXPECT_NE(button2, nullptr);
}

TEST_F(UIEngineTest, AddStylesheet1) {
    test(R"(
cell = Centered {
    Label("Cell")
}:addStylesheetName("CellStyle")

root = Stacked {
    cell,
}:setStyle({
    CellStyle = {
        BackgroundSolid('#f00'),
    },
})

UI.setSurface(root)
)");

    By::name("CellStyle").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, AddStylesheet2) {
    test(R"(
cell = Centered {
    Label("Cell")
}:addStylesheetName("CellStyle")

root = Stacked {
    Stacked {
        cell,
    }:setStyle({
        ViewContainer = {
            BackgroundSolid('#f00'),
        },
    })
}:setStyle({
    CellStyle = {
        BackgroundSolid('#fff'),
    },
})

UI.setSurface(root)
)");

    By::name("CellStyle").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, AddStylesheet3) {
    test(R"(
cell = Centered {
    Label("Cell")
}:addStylesheetName("CellStyle")

root = Stacked {
    Stacked {
        cell:setStyle {
            BackgroundSolid('#f00'),
        },
    }:setStyle({
        ViewContainer = {
            BackgroundSolid('#00f'),
        },
    })
}:setStyle({
    CellStyle = {
        BackgroundSolid('#fff'),
    },
})

UI.setSurface(root)
)");

    By::name("CellStyle").check(averageColor(0xff0000_rgb));
}

TEST_F(UIEngineTest, Text) {
    test(R"(
UI.setSurface(Vertical {
  Text {
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud",
    Button("Button"),
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.",
  }:addStylesheetName(".text")
})
)");

    By::text("Button").check(isBottomAboveBottomOf(By::name(".text")));
}

TEST_F(UIEngineTest, ViewChangeSize) {
    glm::ivec2 pos(0), size(0);

    mLua.register_function("test", [&](glm::ivec2 p, glm::ivec2 s) {
        pos = p;
        size = s;
    });

    test(R"(
function onGeometryChanged(view, pos, size)
  print(clgDump(size))
  test(pos, size)
end

UI.setSurface(Vertical {
  Button("Hello"):geometryChanged(onGeometryChanged)
})
)");

    uitest::frame();

    EXPECT_GE(pos.x, 0);
    EXPECT_GE(pos.y, 0);

    EXPECT_GE(size.x, 0);
    EXPECT_GE(size.y, 0);
}

TEST_F(UIEngineTest, ViewContainerForeach) {
    glm::ivec2 pos(0), size(0);

    mLua.register_function("test", [&](glm::ivec2 p, glm::ivec2 s) {
        pos = p;
        size = s;
    });

    test(R"(
c = Vertical {
  Button("1"),
  Button("2"),
}

for _,view in c:iter() do
  view:setStyle {
    BackgroundSolid('#f00')
  }
end

UI.setSurface(c)
)");
    auto s = (By::text("1") | By::text("2")).check(averageColor(0xff0000_rgb)).toSet();
    EXPECT_EQ(s.size(), 2);
}

TEST_F(UIEngineTest, SliderStyling) {
    bool called = false;
    auto callMe = [&](float v) { called = true; };
    mLua.register_function("test", callMe);
    test(R"(
slider = Slider():setStyle {
  SliderBackground = {
    BackgroundSolid('#f00')
  },
  SliderForeground = {
    BackgroundSolid('#00f')
  }
}

slider:setValue(0)
slider:onValueChangingCallback(test)

UI.setSurface(Vertical {
  slider
})
)");
    By::name("SliderBackground").check(averageColor(0xff0000_rgb));
    mLua.do_string("slider:setValue(1)");

    By::name("SliderBackground").check(averageColor(0x0000ff_rgb));
    EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, AbsoluteLayout) {
    test(R"(
UI.setSurface(AbsoluteLayout {
    {
        view = Button("1"),
        pos = {0, 0},
    },
    {
        view = Button("2"),
        pos = {20, 20},
    },
})
)");
    EXPECT_EQ(By::type<AButton>().toSet().size(), 2);
}

TEST_F(UIEngineTest, ConsumedClick) {
    test(R"(
overlay = Vertical {}:setStyle {
  Expanding(),
}

function overlay:consumesClick()
  return true
end

root = Stacked {
    Button("Don't click me"),
    overlay,
}

UI.setSurface(root)
)");
    auto dontClickMe = By::text("Don't click me").one();
    AObject::connect(dontClickMe->clicked, dontClickMe, [] { FAIL() << "\"Don't click me\" button clicked"; });
    By::value(dontClickMe).perform(click());
}

/**
 * Checks for the bug when stylesheet names are sorted by alphabet, thus causing issues with order.
 *
 * In this example, abuduba's green color should not be overwritten by fuk's red color just because 'f'
 * appears later than 'a' in alphabet.
 */
TEST_F(UIEngineTest, ValidApplianceOrder) {
    test(R"(
root = Vertical {
  View():addStylesheetName("abuduba"):addStylesheetName("fuk")
}
root:setStyle {
    fuk = {
        FixedSize(20),
        BackgroundSolid('#f00')
    },
    abuduba = {
        BackgroundSolid('#0f0')
    }
}

UI.setSurface(root)
)");
    By::name("abuduba").check(averageColor(AColor::GREEN));
}

TEST_F(UIEngineTest, Vec1) {
    mLua.register_function("test", [](glm::ivec2 v) { return v + 1; });
    mLua.do_string(R"(
v = test({1, 2})
assert(v[1] == 2)
assert(v[2] == 3)
    )");
}

TEST_F(UIEngineTest, Vec2) {
    mLua.register_function("test", [](std::vector<int> v) {
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 2);
        return v;
    });
    mLua.do_string(R"(
v = test({1, 2})
assert(v[1] == 1)
assert(v[2] == 2)
    )");
}

TEST_F(UIEngineTest, Vec3) {
    mLua.register_function("test", [](std::vector<glm::ivec2> v) {
        EXPECT_EQ(v.size(), 2);
        EXPECT_EQ(v[0].x, 1);
        EXPECT_EQ(v[0].y, 2);
        EXPECT_EQ(v[1].x, 3);
        EXPECT_EQ(v[1].y, 4);
        return v;
    });
    mLua.do_string(R"(
v = test({{1, 2}, {3, 4}})
assert(v[1][1] == 1)
assert(v[1][2] == 2)
assert(v[2][1] == 3)
assert(v[2][2] == 4)
    )");
}

TEST_F(UIEngineTest, Vec4) {
    mLua.register_function("test", [](std::vector<glm::ivec2> v) {
        EXPECT_EQ(v.size(), 1);
        EXPECT_EQ(v[0].x, 1);
        EXPECT_EQ(v[0].y, 2);
        return v;
    });
    mLua.do_string(R"(
v = test({{1, 2}})
assert(v[1][1] == 1)
assert(v[1][2] == 2)
    )");
}

TEST_F(UIEngineTest, ParentTest) {
    mLua.register_function("expect_equal", [](const _<AView>& a, const _<AView>& b) { EXPECT_EQ(a, b); });

    mLua.register_function("expect_not_equal", [](const _<AView>& a, const _<AView>& b) { EXPECT_EQ(a, b); });

    mLua.register_function("expect_null", [](const _<AView>& a) { EXPECT_EQ(a, nullptr); });

    test(R"(
        cont1 = Vertical{}
        cont2 = Vertical{}
        view = View()
        UI.setSurface(Vertical{
            cont1,
            cont2
        })
        cont1:addView(view)
        expect_equal(view:getParent(), cont1)
        cont1:removeView(view)
        expect_null(view:getParent())
    )");
}

TEST_F(UIEngineTest, ParentTestInflateView) {
    mLua.register_function("expect_equal", [](const _<AView>& a, const _<AView>& b) { EXPECT_EQ(a, b); });

    mLua.register_function("expect_not_equal", [](const _<AView>& a, const _<AView>& b) { EXPECT_EQ(a, b); });

    mLua.register_function("expect_null", [](const _<AView>& a) { EXPECT_EQ(a, nullptr); });

    test(R"(
        cont = Vertical{}
        view1 = View()
        view2 = View()
        UI.setSurface(Vertical{
            cont
        })
        cont:addView(view1)
        expect_equal(view1:getParent(), cont)
        cont:inflateView(view2)
        expect_null(view1:getParent())
        expect_equal(view2:getParent(), cont)
    )");
}

TEST_F(UIEngineTest, LuaStateTest1) {
    bool called = false;
    mLua.register_function("test", [&](lua_State* l) {
        called = true;
        EXPECT_EQ(mLua, l);
    });

    mLua.do_string("test()");
    EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, LuaStateTest2) {
    bool called = false;
    mLua.register_function("test", [&](lua_State* l, int extraValue) {
        called = true;
        EXPECT_EQ(mLua, l);
        EXPECT_EQ(extraValue, 228);
    });

    mLua.do_string("test(228)");
    EXPECT_TRUE(called);
}

TEST_F(UIEngineTest, CoroutineView) {
    test(R"(
local co = coroutine.create(function()
  local v = View()
  v:addStylesheetName("TestView")
  return v
end)

local _, v = coroutine.resume(co)
UI.setSurface(Centered{ v })
    )");

    EXPECT_EQ(By::name("TestView").toSet().size(), 1);
}

TEST_F(UIEngineTest, CoroutineLabel1) {
    test(R"(
local co = coroutine.create(function()
  local v = Label("Hello")
  v:addStylesheetName("TestView")
  return v
end)

local _, v = coroutine.resume(co)
UI.setSurface(Centered{ v })
    )");

    auto views = By::name("TestView").toSet();
    ASSERT_EQ(views.size(), 1);

    auto label = _cast<ALabel>(*views.begin());
    ASSERT_TRUE(label);
    EXPECT_EQ(label->text(), "Hello");
}

TEST_F(UIEngineTest, CoroutineLabel2) {
    test(R"(
local co = coroutine.create(function()
  local v = Label("Hello")
  return v
end)

local _, v = coroutine.resume(co)
v:addStylesheetName("TestView")
UI.setSurface(Centered{ v })
    )");

    auto views = By::name("TestView").toSet();
    ASSERT_EQ(views.size(), 1);

    auto label = _cast<ALabel>(*views.begin());
    ASSERT_TRUE(label);
    EXPECT_EQ(label->text(), "Hello");
}

TEST_F(UIEngineTest, CoroutineLabel3) {
    test(R"(
called = false
local co = coroutine.create(function()
  local v = Label("Hello")
  v:clicked(function()
    called = true
  end)
  return v
end)

local _, v = coroutine.resume(co)
v:addStylesheetName("TestView")
UI.setSurface(Centered{ v })
    )");

    auto views = By::name("TestView").toSet();
    ASSERT_EQ(views.size(), 1);

    auto label = _cast<ALabel>(*views.begin());
    ASSERT_TRUE(label);
    EXPECT_EQ(label->text(), "Hello");

    EXPECT_FALSE(mLua.global_variable("called").as<bool>());
    By::value(label).perform(click());
    EXPECT_TRUE(mLua.global_variable("called").as<bool>());
}

TEST_F(UIEngineTest, CoroutineGetParent) {
    mLua.register_function("assertNonNull", [](const _<AView>& v) { ASSERT_TRUE(v != nullptr); });
    mLua.register_function("assertEq", [](const _<AView>& v1, const _<AView>& v2) { ASSERT_EQ(v1, v2); });
    test(R"(
called = false
local co = coroutine.create(function()
  local v = Label("Hello")
  local p = Vertical { v }
  assertNonNull(v:getParent())
  assertEq(p, v:getParent())
  return p, v
end)

local _, p, v = coroutine.resume(co)
UI.setSurface(Centered{ p })
assertEq(p, v:getParent())
    )");
    auto views = By::text("Hello").toSet();
    EXPECT_GE(views.size(), 0);
}

TEST_F(UIEngineTest, CoroutineStacktrace) {
    mLua.register_function("acceptsInt", [](int) {

    });
    test(R"(
local co = coroutine.create(function()
    acceptsInt('sosi zhopy i give you a string')
end)

local _, p, v = coroutine.resume(co)
    )");
}

TEST_F(UIEngineTest, SetStyle) {
    test(R"(
target = View():setStyle {
    BackgroundSolid('red'),
}
target:addStylesheetName("Target")
target:expanding()

UI.setSurface(Stacked {
    target
}:setStyle {
    FixedSize(100),
    BackgroundSolid('green'),
})
)");
    By::name("Target").check(averageColor(AColor::RED));
    mLua.do_string(R"(
target:setStyle {}
    )");
    By::name("Target").check(averageColor(AColor::GREEN));
}

TEST_F(UIEngineTest, ProgressbarRangedFloat) {
    test(R"(
pg = Progressbar()
pg:setValue(0.5)
UI.setSurface(Horizontal {
    pg
})
)");
    EXPECT_DOUBLE_EQ(mLua.do_string<float>("return pg:value()"), 0.5);
}

TEST_F(UIEngineTest, TextGone) {
    test(R"(
function makeCell(name)
  return Label(name):setStyle {
    BackgroundSolid('green'),
    FixedSize(100),
    Margin(0),
  }:addStylesheetName(name)
end
first = makeCell("First")
second = makeCell("Second")
third = makeCell("Third")
UI.setSurface(Centered {
  Text { first, second, third }:setStyle {
    FixedSize(200),
  }
})
)");
    auto textAndThird = By::name("Text") | By::name("Third");
    EXPECT_EQ(textAndThird.toSet().size(), 2);
    mLua.do_string("second:setVisibility(Visibility.GONE)");
    textAndThird.check(areTopAligned(), "Text and Third should be aligned");
}

TEST_F(UIEngineTest, ClickedRemoval) {
    bool called = false;
    mLua.register_function("callMeOnlyOnce", [&] {
        EXPECT_FALSE(called);
        called = true;
    });
    test(R"(
btn = Button("Test")
btn:clicked(function()
  callMeOnlyOnce()
  return SIGNAL_REMOVE
end)
UI.setSurface(Centered { btn })
)");
    By::text("Test").perform(click()).perform(click());
}

TEST_F(UIEngineTest, Merging1) {
    test(R"(
UI.setSurface(Centered {
  View():addStylesheetName "test1" :addStylesheetName "test2"
}:setStyle {
  ViewContainer = {
    BackgroundSolid('#000'),
  },
  test1 = {
    BackgroundImage(':uni/caption/close.svg', {}, {}, Sizing.COVER),
    FixedSize(32),
  },
  test2 = {
    BackgroundImage({}, '#f00', {}, {}),
  },
})
)");

    uitest::frame();
    auto selector = By::name("test1");
    auto testView = selector.one();
    ASSERT_TRUE(testView->getAssNames().contains("test1"));
    ASSERT_TRUE(testView->getAssNames().contains("test2"));
    selector.check(pixelColorAt({ 0.5f, 0.5f }, AColor::RED));
    saveScreenshot("");
}

TEST_F(UIEngineTest, Merging2) {
    test(R"(
UI.setSurface(Centered {
  View():addStylesheetName "test1":setStyle {
    BackgroundImage({}, '#f00', {}, {}),
  }
}:setStyle {
  ViewContainer = {
    BackgroundSolid('#000'),
  },
  test1 = {
    BackgroundImage(':uni/caption/close.svg', {}, {}, Sizing.COVER),
    FixedSize(32),
  },
})
)");

    uitest::frame();
    auto selector = By::name("test1");
    auto testView = selector.one();
    ASSERT_TRUE(testView->getAssNames().contains("test1"));
    selector.check(pixelColorAt({ 0.5f, 0.5f }, AColor::RED));
    saveScreenshot("");
}


TEST_F(UIEngineTest, Callable) {
    bool called = false;

    mLua.register_function("test", [&](clg::table_view table, _<AView> view) {
        called = true;
        EXPECT_TRUE(table["govno"].as<bool>());
        EXPECT_TRUE(view->getParent() != nullptr);
    });

    test(R"(
view = View()

view:addStylesheetName "Test"

callable = { govno=true }
setmetatable(callable, {
  __call = function(self, view)
    test(self, view)
    return 228
  end
})

view.getContentMinimumWidth = callable

assert(view.getContentMinimumWidth.govno)

UI.setSurface(Centered { view })
)");
    EXPECT_EQ(By::name("Test").one()->getContentMinimumWidth(), 228);
    EXPECT_TRUE(called);
}
