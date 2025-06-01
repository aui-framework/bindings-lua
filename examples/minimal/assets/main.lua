function HelloButton()
    return Button "Click me":clicked(function()
        print("Clicked")
    end)
end

UI.setSurface(Centered {
    Vertical {
        Label "Hello world!",
        HelloButton(),
    }
})
