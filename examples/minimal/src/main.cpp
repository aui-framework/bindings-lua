// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "uiengine/UIEngine.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Entry.h>

AUI_ENTRY {
  static clg::vm vm;
  // auto w = _new<AWindow>("Example", 500_dp, 400_dp);
  static UIEngine uiEngine;
  auto main = AString::fromUtf8(AByteBuffer::fromStream(":main.lua"_url.open())).toStdString();
  clg::state_interface(clg::state()).do_string(main);
  // w->show();

  return 0;
};