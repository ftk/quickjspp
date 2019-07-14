#pragma once

extern "C" {
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"
}

#include <algorithm>
#include <tuple>
#include <functional>
#include <unordered_map>
#include <cassert>
#include <memory>

namespace qjs {

    namespace detail {
        class exception {};

        template <typename R>
        struct js_traits
        {
            //static R unwrap(JSContext * ctx, JSValue v);
            //static JSValue wrap(JSContext * ctx, R value)
        };

        // identity
        template <>
        struct js_traits<JSValue>
        {
            static JSValue unwrap(JSContext * ctx, JSValue v) noexcept
            {
                return v;
            }

            static JSValue wrap(JSContext * ctx, JSValue v) noexcept
            {
                return v;
            }
        };

        template <>
        struct js_traits<int32_t>
        {
            static int32_t unwrap(JSContext * ctx, JSValue v)
            {
                int32_t r;
                if(JS_ToInt32(ctx, &r, v))
                    throw exception{};
                return r;
            }

            static JSValue wrap(JSContext * ctx, int32_t i) noexcept
            {
                return JS_NewInt32(ctx, i);
            }
        };

        template <>
        struct js_traits<bool>
        {
            static bool unwrap(JSContext * ctx, JSValue v) noexcept
            {
                return JS_ToBool(ctx, v);
            }

            static JSValue wrap(JSContext * ctx, bool i) noexcept
            {
                return JS_NewBool(ctx, i);
            }
        };

        template <>
        struct js_traits<double>
        {
            static double unwrap(JSContext * ctx, JSValue v)
            {
                double r;
                if(JS_ToFloat64(ctx, &r, v))
                    throw exception{};
                return r;
            }

            static JSValue wrap(JSContext * ctx, double i) noexcept
            {
                return JS_NewFloat64(ctx, i);
            }
        };

        class js_string : public std::string_view
        {
            using Base = std::string_view;
            JSContext * ctx = nullptr;

            friend struct js_traits<js_string>;
            js_string(JSContext * ctx, const char * ptr, std::size_t len) : Base(ptr, len), ctx(ctx) {}
        public:

            template <typename... Args>
            js_string(Args&&... args) : Base(std::forward<Args>(args)...), ctx(nullptr) {}

            js_string(const js_string& other) = delete;

            ~js_string()
            {
                if(ctx)
                    JS_FreeCString(ctx, this->data());
            }
        };

        template <>
        struct js_traits<js_string>
        {
            static js_string unwrap(JSContext * ctx, JSValue v)
            {
                int plen;
                const char * ptr = JS_ToCStringLen(ctx, &plen, v, 0);
                if(!ptr)
                    throw exception{};
                return js_string{ctx, ptr, (std::size_t)plen};
            }

            static JSValue wrap(JSContext * ctx, js_string str) noexcept
            {
                return JS_NewStringLen(ctx, str.data(), (int)str.size());
            }
        };

        template <> // slower
        struct js_traits<std::string>
        {
            static std::string unwrap(JSContext * ctx, JSValue v)
            {
                auto str_view = js_traits<js_string>::unwrap(ctx, v);
                return std::string{str_view.data(), str_view.size()};
            }

            static JSValue wrap(JSContext * ctx, const std::string& str) noexcept
            {
                return JS_NewStringLen(ctx, str.data(), (int) str.size());
            }
        };

            /*
        template <typename Arg1, typename... Args>
        std::tuple<Arg1, Args...> to_tuple_impl(JSContext * ctx, JSValue * argv)
        {
            if constexpr (sizeof...(Args) == 0)
                return std::make_tuple<Arg1>(js_traits<std::decay_t<Arg1>>::unwrap(ctx, *argv));
            else // todo:rewrite
                return std::tuple_cat(std::make_tuple<Arg1>(js_traits<std::decay_t<Arg1>>::unwrap(ctx, *argv)),
                                      to_tuple_impl<Args...>(ctx, argv + 1));
        }
*/
        template <class Tuple, std::size_t... I>
        Tuple unwrap_args_impl(JSContext * ctx, JSValue * argv, std::index_sequence<I...>)
        {
            return Tuple{js_traits<std::decay_t<std::tuple_element_t<I, Tuple>>>::unwrap(ctx, argv[I])...};
        }

        template <typename... Args>
        std::tuple<Args...> unwrap_args(JSContext * ctx, JSValue * argv)
        {
            return unwrap_args_impl<std::tuple<Args...>>(ctx, argv, std::make_index_sequence<sizeof...(Args)>());
        }

        // free function

        template <auto F>
        struct fwrapper
        {
            const char * name = nullptr;
        };

        template <typename R, typename... Args, R (* F)(Args...)>
        struct js_traits<fwrapper<F>>
        {
            static JSValue wrap(JSContext * ctx, fwrapper<F> fw)
            {
                return JS_NewCFunction(ctx, [](JSContext * ctx, JSValue this_value, int argc,
                                               JSValue * argv) noexcept -> JSValue {
                    //if(argc < sizeof...(Args))
                    //return JS_EXCEPTION;
                    try
                    {
                        if constexpr (std::is_same_v<R, void>)
                        {
                            std::apply(F, detail::unwrap_args<Args...>(ctx, argv));
                            return JS_UNDEFINED;
                        } else
                            return js_traits<R>::wrap(ctx,
                                                      std::apply(F, detail::unwrap_args<Args...>(ctx, argv))
                            );
                    } catch(detail::exception) // todo: inspect exception
                    {
                        return JS_EXCEPTION;
                    }
                }, fw.name, sizeof...(Args));

            }
        };

        // class member function

        template <typename R, class T, typename... Args, R (T::*F)(Args...)>
        struct js_traits<fwrapper<F>>
        {
            static JSValue wrap(JSContext * ctx, fwrapper<F> fw)
            {
                return JS_NewCFunction(ctx, [](JSContext * ctx, JSValue this_value, int argc,
                                               JSValue * argv) noexcept -> JSValue {
                    //if(argc < sizeof...(Args))
                    //return JS_EXCEPTION;
                    try
                    {
                        const auto& this_sp = js_traits<std::shared_ptr<T>>::unwrap(ctx, this_value);

                        if constexpr (std::is_same_v<R, void>)
                        {
                            std::apply(F, std::tuple_cat(std::make_tuple(this_sp.get()),
                                                         detail::unwrap_args<Args...>(ctx, argv)));
                            return JS_UNDEFINED;
                        } else
                            return js_traits<R>::wrap(ctx,
                                                      std::apply(F, std::tuple_cat(std::make_tuple(this_sp.get()),
                                                                                   detail::unwrap_args<Args...>(ctx,
                                                                                                                argv)))
                            );
                    } catch(detail::exception)
                    {
                        return JS_EXCEPTION;
                    }
                }, fw.name, sizeof...(Args));

            }
        };


        // class constructor

        template <class T, typename... Args>
        struct ctor_wrapper
        {
            const char * name = nullptr;
        };

        template <class T, typename... Args>
        struct js_traits<ctor_wrapper<T, Args...>>
        {
            static JSValue wrap(JSContext * ctx, ctor_wrapper<T, Args...> cw)
            {
                return JS_NewCFunction2(ctx, [](JSContext * ctx, JSValue this_value, int argc,
                                                JSValue * argv) noexcept -> JSValue {
                    try
                    {

                        return js_traits<std::shared_ptr<T>>::wrap(ctx,
                                                                   std::apply(std::make_shared<T, Args...>,
                                                                              detail::unwrap_args<Args...>(ctx, argv))
                        );
                    } catch(detail::exception)
                    {
                        return JS_EXCEPTION;
                    }
                }, cw.name, sizeof...(Args), JS_CFUNC_constructor, 0);

            }
        };


        // SP<T> class

        template <class T>
        struct js_traits<std::shared_ptr<T>>
        {
            inline static JSClassID QJSClassId;

            static void register_class(JSContext * ctx, const char * name, JSValue proto)
            {
                JSClassDef def{
                        name,
                        // destructor
                        [](JSRuntime * rt, JSValue obj) noexcept {
                            auto pptr = reinterpret_cast<std::shared_ptr<T> *>(JS_GetOpaque(obj, QJSClassId));
                            if(!pptr)
                            {
                                assert(false && "bad destructor");
                            }
                            delete pptr;
                        }
                };
                if(QJSClassId == 0)
                {
                    int e = JS_NewClass(JS_GetRuntime(ctx), JS_NewClassID(&QJSClassId), &def);
                    if(e < 0)
                        throw exception{};
                }
                JS_SetClassProto(ctx, QJSClassId, proto);
            }

            static JSValue wrap(JSContext * ctx, std::shared_ptr<T> ptr)
            {
                if(QJSClassId == 0) // not registered
                    throw exception{};
                auto pptr = new std::shared_ptr<T>(std::move(ptr));
                auto jsobj = JS_NewObjectClass(ctx, QJSClassId);
                JS_SetOpaque(jsobj, pptr);
                return jsobj;
            }

            static const std::shared_ptr<T>& unwrap(JSContext * ctx, JSValue v)
            {
                auto ptr = reinterpret_cast<std::shared_ptr<T> *>(JS_GetOpaque2(ctx, v, QJSClassId));
                if(!ptr)
                    throw exception{};
                return *ptr;
            }
        };

        //          properties

        template <typename Key>
        struct js_property_traits
        {
            // static void set_property(JSContext * ctx, JSValue this_obj, R key, JSValue value)
            // static JSValue get_property(JSContext * ctx, JSValue this_obj, R key)
        };

        template <>
        struct js_property_traits<const char *>
        {
            static void set_property(JSContext * ctx, JSValue this_obj, const char * name, JSValue value)
            {
                int err = JS_SetPropertyStr(ctx, this_obj, name, value);
                if(err < 0)
                    throw exception{};
            }

            static JSValue get_property(JSContext * ctx, JSValue this_obj, const char * name) noexcept
            {
                return JS_GetPropertyStr(ctx, this_obj, name);
            }
        };

        template <>
        struct js_property_traits<uint32_t>
        {
            static void set_property(JSContext * ctx, JSValue this_obj, uint32_t idx, JSValue value)
            {
                int err = JS_SetPropertyUint32(ctx, this_obj, idx, value);
                if(err < 0)
                    throw exception{};
            }

            static JSValue get_property(JSContext * ctx, JSValue this_obj, uint32_t idx) noexcept
            {
                return JS_GetPropertyUint32(ctx, this_obj, idx);
            }
        };

        template <typename Key>
        struct property_proxy
        {
            JSContext * ctx;
            JSValue this_obj;
            Key key;

            template <typename Value>
            operator Value() const
            {
                return js_traits<Value>::unwrap(ctx, js_property_traits<Key>::get_property(ctx, this_obj, key));
            }

            template <typename Value>
            property_proxy& operator =(Value value)
            {
                js_property_traits<Key>::set_property(ctx, this_obj, key,
                                                      js_traits<Value>::wrap(ctx, std::move(value)));
                return *this;
            }
        };

    }

    class Value
    {
    public:
        JSValue v;
        JSContext * ctx = nullptr;

    public:
        template <typename T>
        Value(JSContext * ctx, T val) : ctx(ctx)
        {
            v = detail::js_traits<T>::wrap(ctx, val);
        }

        Value(const Value& rhs)
        {
            ctx = rhs.ctx;
            v = JS_DupValue(ctx, rhs.v);
        }

        Value(Value&& rhs)
        {
            std::swap(ctx, rhs.ctx);
            v = rhs.v;
        }

        ~Value()
        {
            if(ctx) JS_FreeValue(ctx, v);
        }


        template <typename T>
        T cast() const
        {
            return detail::js_traits<T>::unwrap(ctx, v);
        }

        JSValue release() // dont call freevalue
        {
            ctx = nullptr;
            return v;
        }

        operator JSValue()&&
        {
            return release();
        }


        // access properties
        template <typename Key>
        detail::property_proxy<Key> operator [](Key key)
        {
            return {ctx, v, std::move(key)};
        }


    };


    class Runtime
    {
    public:
        JSRuntime * rt;

        Runtime()
        {
            rt = JS_NewRuntime();
            if(!rt)
                throw detail::exception{};
        }

        Runtime(const Runtime&) = delete;

        ~Runtime()
        {
            JS_FreeRuntime(rt);
        }
    };


    class Context
    {
    public:
        JSContext * ctx;

        class Module
        {
            friend class Context;

            JSModuleDef * m;
            JSContext * ctx;
            const char * name;

            std::vector<std::pair<const char *, JSValue>> exports;
        public:
            Module(JSContext * ctx, const char * name) : ctx(ctx), name(name)
            {
                m = JS_NewCModule(ctx, name, [](JSContext * ctx, JSModuleDef * m) noexcept {
                    auto context = reinterpret_cast<Context *>(JS_GetContextOpaque(ctx));
                    if(!context)
                        return -1;
                    auto it = std::find_if(context->modules.cbegin(), context->modules.cend(),
                                           [m](const Module& module) { return module.m == m; });
                    if(it == context->modules.end())
                        return -1;
                    for(const auto& e : it->exports)
                    {
                        if(JS_SetModuleExport(ctx, m, e.first, e.second) != 0)
                            return -1;
                    }
                    return 0;
                });
                if(!m)
                    throw detail::exception{};
            }

            Module& add(const char * name, JSValue value)
            {
                exports.push_back({name, value});
                JS_AddModuleExport(ctx, m, name);
                return *this;
            }

            Module& add(const char * name, Value value)
            {
                return add(name, value.release());
            }

            template <typename T>
            Module& add(const char * name, T value)
            {
                return add(name, detail::js_traits<T>::wrap(ctx, std::move(value)));
            }

            //Module(const Module&) = delete;
        };

        std::vector<Module> modules;
    private:
    public:
        Context(Runtime& rt) : Context(rt.rt)
        {}

        Context(JSRuntime * rt)
        {
            ctx = JS_NewContext(rt);
            if(!ctx)
                throw detail::exception{};
            JS_SetContextOpaque(ctx, this);
        }

        Context(JSContext * ctx) : ctx{ctx}
        {
            JS_SetContextOpaque(ctx, this);
        }

        Context(const Context&) = delete;

        ~Context()
        {
            JS_FreeContext(ctx);
        }

        Module& addModule(const char * name)
        {
            modules.emplace_back(ctx, name);
            return modules.back();
        }

        Value global()
        {
            return Value{ctx, JS_GetGlobalObject(ctx)};
        }

        Value newObject()
        {
            return Value{ctx, JS_NewObject(ctx)};
        }

        template <class T>
        void registerClass(const char * name, JSValue proto)
        {
            detail::js_traits<std::shared_ptr<T>>::register_class(ctx, name, proto);
        }

        Value eval(std::string_view buffer, const char * filename, unsigned eval_flags = 0)
        {
            JSValue v = JS_Eval(ctx, buffer.data(), buffer.size(), filename, eval_flags);
            //if(JS_IsException(v))
            //throw detail::exception{};
            return Value{ctx, v};
        }

        Value evalFile(const char * filename, unsigned eval_flags = 0)
        {
            size_t buf_len;
            auto deleter = [this](void * p) { js_free(ctx, p); };
            auto buf = std::unique_ptr<uint8_t, decltype(deleter)>{js_load_file(ctx, &buf_len, filename), deleter};
            if(!buf)
                throw std::runtime_error{std::string{"evalFile: can't read file: "} + filename};
            return eval({reinterpret_cast<char *>(buf.get()), buf_len}, filename, eval_flags);
        }

    };


}