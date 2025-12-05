#pragma once
#include "AUI/View/AAbstractLabel.h"

/**
 * @brief Button with text, which can be pushed to make some action.
 *
 * ![](imgs/views/MyButton.png)
 *
 * @ingroup views_actions
 * @details
 * Button is a fundamental view which can be pushed to make some action.
 *
 * <figure markdown="span">
 * ![](imgs/MyButton.gif)
 * <figcaption>The button view being pressed.</figcaption>
 * </figure>
 *
 * Button is styled with background, box shadow, and a border that highlights on
 * hover. When pushed, the shadow disappears, making an illusion of pressing.
 *
 * Button can be made default. In such case, it is colored to user's accent
 * color, making it stand out. Also, when the user presses `Enter`, the button
 * is pushed automatically.
 *
 * Button usually contains text only, but in practice any view can be put in it.
 *
 * <!-- aui:include examples/ui/button/src/main.cpp -->
 *
 */
class API_AUI_VIEWS MyButton : public AAbstractLabel {
public:
    MyButton();

    explicit MyButton(AString text) noexcept: AAbstractLabel(std::move(text)) {}

    virtual ~MyButton() = default;

    [[nodiscard]]
    bool isDefault() const noexcept {
        return mDefault;
    }

    void setDefault(bool isDefault = true);

    bool consumesClick(const glm::ivec2& pos) override;

signals:
    emits<bool> defaultState;
    emits<> becameDefault;
    emits<> noLongerDefault;

private:
    AFieldSignalEmitter<bool> mDefault = AFieldSignalEmitter<bool>(defaultState, becameDefault, noLongerDefault);
};

/**
 * @brief Unlike MyButton, MyButtonEx is a container which looks like a button.
 */
class MyButtonEx : public AViewContainer {
public:
    MyButtonEx() {
        addAssName(".btn");
    }

    ~MyButtonEx() override = default;
};

