#include "quickjspp.hpp"
#include <iostream>

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);

    auto simple_module_loader = [](char const * filename) -> qjs::Context::ModuleDataOpt {
        if (filename == std::string_view("some_module.js")) {
            return qjs::Context::ModuleData{R"xxx(
                import "folder/file1.js"
                log(import.meta.url);
            )xxx"};
        }
        if (filename == std::string_view("folder/file1.js")) {
            return qjs::Context::ModuleData{R"xxx(
                import "./file2.js"
                log(import.meta.url);
            )xxx"};
        }
        if (filename == std::string_view("folder/file2.js")) {
            return qjs::Context::ModuleData{R"xxx(
                log(import.meta.url);
            )xxx"};
        }
        return std::nullopt;
    };

    context.moduleLoader = simple_module_loader;
    context.global().add("log", [](std::string_view s) {
        std::cout << s << std::endl;
    });

    context.eval(R"xxx(
        import "./some_module.js";
    )xxx", "<eval>", JS_EVAL_TYPE_MODULE);

    try {
        context.eval(R"xxx(
            import "./invalid_module.js";
        )xxx", "<eval>", JS_EVAL_TYPE_MODULE);
        assert(false && "module import should have failed");
    }
    catch(qjs::exception e)
    {
        auto exc = e.get();
        assert(exc.isError() && "Exception should be a JS error");
        assert((std::string)exc == "ReferenceError: could not load module filename 'invalid_module.js'");
    }

    return 0;
}
