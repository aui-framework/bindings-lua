//
// Created by alex2772 on 11/18/25.
//

#include "MyButton.h"
#include <AUI/Platform/AWindow.h>

MyButton::MyButton() {

}


void MyButton::setDefault(bool isDefault) {
  mDefault.set(this, isDefault);
  connect(AWindow::current()->keyDown, this, [&](AInput::Key k) {
      if (!mDefault) {
          AObject::disconnect();
      } else {
          if (k == AInput::RETURN) {
              emit clicked();
          }
      }
  });

}

bool MyButton::consumesClick(const glm::ivec2& pos) {
  return true;
}
