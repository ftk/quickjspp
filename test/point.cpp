#include "quickjspp.hpp"
#include <iostream>
#include <cmath>

class Point : public qjs::enable_shared_from_this<Point>
{
public:
    int x, y;
    Point(int x, int y) : x(x), y(y) {}

    double norm()
    {
        try {
            auto js_norm = shared_from_this().evalThis("this.norm.bind(this)").as<std::function<double ()>>();
            return js_norm();
        } catch(qjs::exception e) {}
        return std::sqrt((double) x * x + double(y) * y);
    }
};


int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);
    try
    {
        // export classes as a module
        auto& module = context.addModule("MyModule");
        module.class_<Point>("Point")
                .constructor<int, int>()
                .fun<&Point::x>("x")
                .fun<&Point::y>("y")
                .fun<&Point::norm>("norm");
        // import module
        context.eval(R"xxx(
import { Point } from "MyModule";

function assert(b, str)
{
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}

class ColorPoint extends Point {
    constructor(x, y, color) {
        super(x, y);
        this.color = color;
    }
    get_color() {
        return this.color;
    }
    norm() {
        assert(this.color);
        return 1.0;
    }
};

function main()
{
    var pt, pt2;

    pt = new Point(2, 3);
    assert(pt.x === 2);
    assert(pt.y === 3);
    pt.x = 4;
    assert(pt.x === 4);
    assert(pt.norm() == 5);

    pt2 = new ColorPoint(2, 3, 0xffffff);
    assert(pt2.x === 2);
    assert(pt2.color === 0xffffff);
    assert(pt2.get_color() === 0xffffff);
    assert(pt2.norm() === 1.0);
    globalThis.pt2 = pt2;
}

main();
)xxx", "<eval>", JS_EVAL_TYPE_MODULE);
        auto pt2 = context.eval("pt2").as<qjs::shared_ptr<Point>>();
        assert(pt2->x == 2);
        assert(pt2->y == 3);
        assert(pt2->norm() == 1);
    }
    catch(qjs::exception)
    {
        auto exc = context.getException();
        std::cerr << (std::string) exc << std::endl;
        if((bool) exc["stack"])
            std::cerr << (std::string) exc["stack"] << std::endl;
        return 1;
    }
}