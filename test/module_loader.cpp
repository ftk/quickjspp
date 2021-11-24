#include "quickjspp.hpp"
#include <iostream>
#include <string_view>

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);

    auto simple_module_loader = [](std::string_view filename) -> qjs::Context::ModuleData {
        if (filename == "some_module.js") {
            return {R"xxx(
                import "folder/file1.js"
                log(import.meta.url);
            )xxx"};
        }
        if (filename == "folder/file1.js") {
            return {R"xxx(
                import "./file2.js"
                log(import.meta.url);
            )xxx"};
        }
        if (filename == "folder/file2.js") {
            return {R"xxx(
                import "http://localhost/script1.js";
                log(import.meta.url);
            )xxx"};
        }
        if (filename == "http://localhost/script1.js") {
            return {R"xxx(
                import "./script2.js";
                log(import.meta.url);
            )xxx"};
        }
        if (filename == "http://localhost/script2.js") {
            return {R"xxx(
                log(import.meta.url);
            )xxx"};
        }
        return {};
    };

    context.moduleLoader = simple_module_loader;
    context.global().add("log", [](std::string_view s) {
        std::cout << s << std::endl;
    });

    context.eval(R"xxx(
        import "./some_module.js";
    )xxx", "<eval>", JS_EVAL_TYPE_MODULE);

    try
    {
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
