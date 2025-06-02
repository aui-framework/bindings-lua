function HelloButton()
    return Button "Click me":clicked(function()
        print("Clicked")
    end)
end

local win = Window("Test1", 854, 500)

win:inflateView(Centered (
    Vertical {
        Label "Hello world!",
        HelloButton(),
    }
))

print("Win dpi scale", win:getDpiRatio())
win:show()