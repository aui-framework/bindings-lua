// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#pragma once


#include "AUI/ASS/unset.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/ADesktop.h"
#include <AUI/Platform/AWindowBase.h>
#include <AUI/Platform/AWindow.h>
#include "AUI/Traits/values.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Render/ARenderContext.h"
#include "AUI/View/A2FingerTransformArea.h"
#include "clg.hpp"
#include "converter.hpp"
#include "dynamic_result.hpp"
#include "lua.h"
#include "table.hpp"
#include <memory>
#include <optional>
#include <uiengine/ILuaExposedView.h>
#include <AUI/Common/AColor.h>
#include <AUI/ASS/ASS.h>
#include <uiengine/ILuaExposedView.h>

extern std::optional<glm::vec2> vec2_from_lua(lua_State* l, int n);
extern std::optional<glm::vec3> vec3_from_lua(lua_State* l, int n);
extern std::optional<glm::vec4> vec4_from_lua(lua_State* l, int n);

extern int vec2_to_lua(lua_State* l, glm::vec2 v);
extern int vec3_to_lua(lua_State* l, glm::vec3 v);
extern int vec4_to_lua(lua_State* l, glm::vec4 v);

/**
 * @brief Блокирует стандартный конвертер _<AView>.
 */
struct IAmNotAView {};

namespace clg {

    template<typename T>
    struct converter<AVector<T>>: converter_derived<std::vector<T>, AVector<T>> {};

    template<>
    struct converter<AString> {
        static converter_result<AString> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<std::string>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return *r;
        }
        static int to_lua(lua_State* l, const AString& v) {
            clg::push_to_lua(l, v.toStdString());
            return 1;
        }
    };

    template<>
    struct converter<APath> {
        static converter_result<APath> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<AString>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return APath(*r);
        }
        static int to_lua(lua_State* l, const APath& v) {
            clg::push_to_lua(l, v.toStdString());
            return 1;
        }
    };

    template<>
    struct converter<AMetric> {
        static converter_result<AMetric> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<float>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return AMetric{ *r, AMetric::T_DP };
        }
        static int to_lua(lua_State* l, const AMetric& v) {
            return clg::push_to_lua(l, v.getValueDp());
        }
    };

    template<>
    struct converter<APointerIndex> {
        static int to_lua(lua_State* l, const APointerIndex& v) {
            return clg::push_to_lua(l, int(v.rawValue()));
        }
    };

    template<>
    struct converter<APointerMoveEvent> {
        static int to_lua(lua_State* l, const APointerMoveEvent& v) {
            return clg::push_to_lua(l, clg::table{
                {"finger", clg::ref::from_cpp(l, v.pointerIndex.finger().valueOr(-1))},
            });
        }
    };

    template<>
    struct converter<APointerPressedEvent> {
        static int to_lua(lua_State* l, const APointerPressedEvent& v) {
            return clg::push_to_lua(l, clg::table{
                {"position", clg::ref::from_cpp(l, v.position)},
                {"button", clg::ref::from_cpp(l, v.asButton)},
                {"finger", clg::ref::from_cpp(l, v.pointerIndex.finger().valueOr(-1))},
            });
        }
    };

    template<>
    struct converter<APointerReleasedEvent> {
        static int to_lua(lua_State* l, const APointerReleasedEvent& v) {
            return clg::push_to_lua(l, clg::table{
                {"position", clg::ref::from_cpp(l, v.position)},
                {"button", clg::ref::from_cpp(l, v.asButton)},
                {"triggerClick", clg::ref::from_cpp(l, v.triggerClick)},
                {"finger", clg::ref::from_cpp(l, v.pointerIndex.finger().valueOr(-1))},
            });
        }
    };

    template<>
    struct converter<ALongPressEvent> {
        static int to_lua(lua_State* l, const ALongPressEvent& v) {
            return clg::push_to_lua(l, clg::table{
            });
        }
    };

    template<>
    struct converter<AScrollEvent> {
        static int to_lua(lua_State* l, const AScrollEvent& v) {
            return clg::push_to_lua(l, clg::table{
                {"origin", clg::ref::from_cpp(l, v.origin)},
                {"delta", clg::ref::from_cpp(l, v.delta)},
                {"kinetic", clg::ref::from_cpp(l, v.kinetic)},
                {"button", clg::ref::from_cpp(l, v.pointerIndex.button().valueOr(AInput::LBUTTON))},
                {"finger", clg::ref::from_cpp(l, v.pointerIndex.finger().valueOr(-1))},
            });
        }
    };

    template<>
    struct converter<AStringVector> {
        static converter_result<AStringVector> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<AVector<AString>>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return *r;
        }
        static int to_lua(lua_State* l, const AStringVector& v) {
            clg::table_array tableArray;
            tableArray.reserve(v.size());

            for (const auto& string : v) {
                clg::stack_integrity_check check(l);
                clg::push_to_lua(l, string);
                auto asRef = clg::ref::from_stack(l);
                tableArray.push_back(std::move(asRef));
            }
            return clg::push_to_lua(l, tableArray);
        }
    };




    template<typename T>
    struct converter_shared_ptr<T, std::enable_if_t<(std::is_base_of_v<AView, T> || std::is_base_of_v<ILuaExposedView, T>) && !std::is_same_v<ILuaExposedView, T> && !std::is_base_of_v<IAmNotAView, T>>> {
        static converter_result<std::shared_ptr<T>> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<std::shared_ptr<ILuaExposedView>>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            auto& v = *r;
            if (!v) return nullptr;
            return _cast<T>(std::move(v));
        }
        static int to_lua(lua_State* l, const std::shared_ptr<T>& v) {
            if (!v) {
                lua_pushnil(l);
                return 1;
            }
            if (auto lua = std::dynamic_pointer_cast<ILuaExposedView>(v)) {
                return clg::push_to_lua(l, lua);
            }
            assert(("non lua-compatible view pushed to lua", false));
            return 0;
            //return converter_shared_ptr_impl<T>::to_lua(l, v);
        }
    };


    template<typename T>
    struct converter_shared_ptr<_<T>>: converter_shared_ptr<T> {};

    template<typename T>
    struct converter<_<T>, std::enable_if_t<std::is_base_of_v<AView, T> || std::is_base_of_v<ILuaExposedView, T>>> {
        static converter_result<_<T>> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<std::shared_ptr<T>>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return *r;
        }
        static int to_lua(lua_State* l, const _<T>& v) {
            return clg::push_to_lua<std::shared_ptr<T>>(l, v);
        }
    };

    template<size_t N>
    struct vector_helper {
        using vec = glm::vec<N, float, glm::defaultp>;

        static std::optional<vec> from(lua_State* l, int n) {
            return std::nullopt;
        }

        static int to(lua_State* l, vec v) {
            return 0;
        }
    };


    template<>
    struct vector_helper<2> {
        static auto from(lua_State* l, int n) {
            return vec2_from_lua(l, n);
        }

        static int to(lua_State* l, glm::vec2 v) {
            return vec2_to_lua(l, v);
        }
    };

    template<>
    struct vector_helper<3> {
        static auto from(lua_State* l, int n) {
            return vec3_from_lua(l, n);
        }

        static int to(lua_State* l, glm::vec3 v) {
            return vec3_to_lua(l, v);
        }
    };

    template<>
    struct vector_helper<4> {
        static auto from(lua_State* l, int n) {
            return vec4_from_lua(l, n);
        }

        static int to(lua_State* l, glm::vec4 v) {
            return vec4_to_lua(l, v);
        }
    };


    namespace detail {

        template<typename T, int N, glm::qualifier Q>
        struct array_like_converter_glm_vec_helper {
            using vec = glm::vec<N, T, Q>;
            static bool set(vec& dst, size_t index, T value) {
                if (index < N) {
                    dst[index] = value;
                }
                // always return true, as it allows LUASHERS to oversize the vector size.
                return true;
            }

            static size_t size(const vec& v) {
                return N;
            }
        };
    }

    template<typename T, int N, glm::qualifier Q>
    struct converter<glm::vec<N, T, Q>> {
        using vec = glm::vec<N, T, Q>;
        using my_array_like_converter = array_like_converter<vec, detail::array_like_converter_glm_vec_helper<T, N, Q>>;

        static converter_result<vec> from_lua(lua_State* l, int n) {
            if (lua_istable(l, n)) {
                auto len = lua_rawlen(l, n);
                if (len < N) {
                    goto error;
                }

                return my_array_like_converter::from_lua(l, n);
            }
            error:
            static std::string e = "expected table of size " + std::to_string(N) + " (vec" + std::to_string(N) + ")";
            return converter_error{e.c_str()};
        }
        static int to_lua(lua_State* l, vec v) {
            return my_array_like_converter::to_lua(l, v);
        }
    };

    template<>
    struct converter<AColor> {

        template<typename vec>
        static converter_result<vec> convert_vec(lua_State* l, int n) {
            auto r = converter<vec>::from_lua(l, n);
            if (r.is_error()) {
                return r.error();
            }
            auto& v = *r;
            glm::clamp(v, 0.f, 255.f);
            v /= 255;
            return v;
        }

        static converter_result<AColor> from_lua(lua_State* l, int n) {
            if (lua_isstring(l, n)) {
                return AColor{ lua_tostring(l, n) };
            }

            if (auto r = convert_vec<glm::vec4>(l, n); r.is_ok()) {
                auto& vec = *r;
                return AColor{vec.r, vec.g, vec.b, vec.a};
            }

            if (auto r = convert_vec<glm::vec3>(l, n); r.is_ok()) {
                auto& vec = *r;
                return AColor{vec.r, vec.g, vec.b};
            }

            return clg::converter_error{"bad color"};
        }
        static int to_lua(lua_State* l, const AColor& v) {
            return clg::push_to_lua(l, v.toString());
        }
    };

    template<>
    struct converter<AAngleRadians> {
        static converter_result<AAngleRadians> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<double>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return operator ""_deg(static_cast<long double>(*r));
        }
        static int to_lua(lua_State* l, AAngleRadians v) {
            return clg::push_to_lua(l, v.degrees());
        }
    };

    template<typename T>
    struct converter<ass::unset_wrap<T>> {
        static converter_result<ass::unset_wrap<T>> from_lua(lua_State* l, int n) {
            if (lua_istable(l, n)) {
                clg::stack_integrity_check check(l);
                lua_pushnil(l);
                if (n < 0) {  // refers to the top of stack
                    n -= 1;
                }
                if (lua_next(l, n) == 0) {
                    // table is empty
                    return ass::unset_wrap<T>{}; // unset
                }
                lua_pop(l, 2);
            }
            auto r = clg::get_from_lua_raw<T>(l, n);
            if (r.is_error()) {
                return r.error();
            }
            return *r;
        }
        static int to_lua(lua_State* l, const ass::unset_wrap<T>& v) {
            if (v) {
                return clg::push_to_lua(l, *v);
            }
            lua_createtable(l, 0, 0);
            return 1;
        }
    };


    template<typename T>
    struct converter<std::optional<T>> {
        static converter_result<std::optional<T>> from_lua(lua_State* l, int n) {
            if (n == 0) {
                return std::nullopt;
            }
            if (lua_isnil(l, n)) {
                return std::nullopt;
            }
            return clg::converter_derived<T, std::optional<T>>::from_lua(l, n);
        }

        static int to_lua(lua_State* l, const std::optional<T>& look) {
            if (look) {
                return clg::push_to_lua(l, *look);
            }
            return 0;
        }
    };

    template<>
    struct converter<ARenderContext> {
        static converter_result<ARenderContext> from_lua(lua_State* l, int n) {
            return converter_error{"ARenderContext is not convertible"};
        }

        static int to_lua(lua_State* l, const ARenderContext& look) {
            // луашерам это незачем
            lua_pushnil(l);
            return 1;
        }
    };


    template<>
    struct converter<A2DTransform> {
        static converter_result<A2DTransform> from_lua(lua_State* l, int n) {
            return A2DTransform{}; 
        }

        static int to_lua(lua_State* l, const A2DTransform& look) {
            return clg::push_to_lua(l, clg::table{
                {"offset", clg::ref::from_cpp(l, look.offset)},
                {"rotation", clg::ref::from_cpp(l, look.rotation)},
                {"scale", clg::ref::from_cpp(l, look.scale)},
            });
        }
    };

    template<typename T, auto min, auto max>
    struct converter<aui::ranged_number<T, min, max>> { // новое безполезное говно от aui
        static converter_result<aui::ranged_number<T, min, max>> from_lua(lua_State* l, int n) {
            auto r = clg::get_from_lua_raw<T>(l);
            if (r.is_error()) {
                return r.error();
            }
            return aui::ranged_number<T, min, max>{*r}; 
        }

        static int to_lua(lua_State* l, const aui::ranged_number<T, min, max>& look) {
            return clg::push_to_lua(l, T(look));
        }
    };

    template<typename T>
    struct converter<AFuture<T>> {
        static void when(const std::shared_ptr<clg::any_wrap>& self, clg::function callback) {
            auto callbackShared = std::make_shared<clg::function>(std::move(callback));
            const auto& future = std::any_cast<AFuture<T>>(*self);
            if constexpr (std::is_same_v<T, void>) {
                future.onSuccess([callback = callbackShared]() {
                    AThread::main()->enqueue([callback]() {
                        (*callback)(true, clg::ref());
                    });
                });
            }
            else {
                future.onSuccess([callback = callbackShared](T result) {
                    AThread::main()->enqueue([callback, result = std::move(result)] {
                        (*callback)(true, clg::ref::from_cpp(clg::state(), result));
                    });
                });
            }
            future.onError([callback = callbackShared](const AException& result) {
                AThread::main()->enqueue([callback, result = std::move(result.getMessage())] {
                    (*callback)(false, clg::ref::from_cpp(clg::state(), result));
                });
            });
        }

        static int to_lua(clg::state_interface l, AFuture<T> f) {
            clg::push_to_lua(l, std::make_shared<clg::any_wrap>(std::move(f)));
            clg::push_to_lua(l, clg::cfunction<when>("when"));
            lua_setfield(l, -2, "when");
            return 1;
        }
    };

    template<>
    struct converter<ADesktop::FileExtension> {
        static converter_result<ADesktop::FileExtension> from_lua(lua_State* l, int n) {
            if (!lua_istable(l, n)) {
                return converter_error{"expected table for FileExtension"};
            }

            ADesktop::FileExtension result;

            lua_getfield(l, n, "name");
            if (!lua_isnil(l, -1)) {
                result.name = get_from_lua<AString>(l, -1);
            }
            lua_pop(l, 1);

            lua_getfield(l, n, "extension");
            if (!lua_isnil(l, -1)) {
                result.extension = get_from_lua<AString>(l, -1);
            }
            lua_pop(l, 1);

            return result;
        }

        static int to_lua(lua_State* l, const ADesktop::FileExtension& v) {
            lua_createtable(l, 0, 2);

            push_to_lua(l, v.name);
            lua_setfield(l, -2, "name");

            push_to_lua(l, v.extension);
            lua_setfield(l, -2, "extension");

            return 1;
        }
    };

    template<>
    struct converter<AWindowBase*> {
        static converter_result<AWindowBase*> from_lua(lua_State* l, int n) {
            return AWindow::current();
        }
    };

}
