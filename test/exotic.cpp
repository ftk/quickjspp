/* Example of how to register a class without class_registrar and how to use exotic methods */

#include <iostream>
#include <stdio.h>
#include "quickjspp.hpp"

std::string atom_to_string(JSContext * ctx, JSAtom atom)
{
    JSValue v = JS_AtomToString(ctx, atom);
    auto str = qjs::js_traits<std::string>::unwrap(ctx, v);
    JS_FreeValue(ctx, v);
    return str;
}

class ExoticClass : public qjs::enable_shared_from_this<ExoticClass>
{
public:
    std::function<bool(JSContext * ctx, JSAtom atom)> has_property;
    std::function<qjs::Value(JSContext * ctx, JSAtom atom, JSValueConst receiver)> get_property;

    static void register_class(JSContext * ctx)
    {
        // we are passing pointer to exotic - requires static lifetime
        static JSClassExoticMethods exotic{
            // see JSClassExoticMethods for more methods
                .has_property = [](JSContext * ctx, JSValueConst obj, JSAtom atom) -> int {
                    try
                    {
                        auto ptr = qjs::js_traits<qjs::shared_ptr<ExoticClass>>::unwrap(ctx, obj);
                        return ptr->has_property(ctx, atom);
                    }
                    catch(qjs::exception)
                    {
                        return -1;
                    }
                },
                .get_property = [](JSContext * ctx, JSValueConst obj, JSAtom atom,
                                   JSValueConst receiver) -> JSValue {
                    try
                    {
                        auto ptr = qjs::js_traits<qjs::shared_ptr<ExoticClass>>::unwrap(ctx, obj);
                        return ptr->get_property(ctx, atom, receiver);
                    }
                    catch(qjs::exception)
                    {
                        return JS_EXCEPTION;
                    }
                }
        };
        qjs::js_traits<qjs::shared_ptr<ExoticClass>>::register_class(ctx, "ExoticClass", JS_NULL, nullptr, &exotic);
    }
};


void println(qjs::rest<std::string> args)
{
    for(auto const& arg: args) std::cout << arg << " ";
    std::cout << "\n";
}

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);
    try
    {
        ExoticClass::register_class(context.ctx);

        auto e = qjs::make_shared<ExoticClass>(context.ctx);
        e->has_property = [](JSContext * ctx, JSAtom) { return true; };
        e->get_property = [](JSContext * ctx, JSAtom prop, JSValueConst receiver) {
            return qjs::Value{ctx, "got " + atom_to_string(ctx, prop)};
        };

        context.global()["e"] = e;
        context.global()["println"] = qjs::fwrapper<&println>{"println"};

        // prints: got property1
        context.eval("println(e.property1);");

        // prints: got property2
        context.eval("println(e.property2);");

    } catch(qjs::exception)
    {
        auto exc = context.getException();
        std::cerr << (std::string) exc << std::endl;
        if((bool) exc["stack"]) std::cerr << (std::string) exc["stack"] << std::endl;
        return 1;
    }
}
