function HelloButton()
    return Button "Click me":clicked(function()
        print("Clicked")
    end)
end

local win = Window("Test1")

win:inflateView(Centered (
    Vertical {
        Label "Hello world!",
        HelloButton(),
    }
))

win:show()