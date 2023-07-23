// ShowDecl - Pretty-print an expression's decltype.
// Works with popular C++20 compilers.
// ref: Arthur O'Dwyer https://quuxplusone.github.io/blog/2018/08/22/puts-pretty-function/

#pragma once

#define SHOW_DECL(expr) \
    dprintf("decltype(" #expr ") == %s\n", std::string(getDecl<decltype(expr)>()).c_str());

template<typename T>
consteval std::string_view getDecl()
{
#if defined(__clang__)
    auto declC = __PRETTY_FUNCTION__;
    std::string_view head = "[T = ";
    std::string_view tail = "]";
#elif defined(__GNUC__)
    auto declC = __PRETTY_FUNCTION__;
    std::string_view head = "[with T = ";
    std::string_view tail = "; std::string_view = ";
#elif defined(_MSC_VER)
    auto declC = __FUNCSIG__;
    std::string_view head = "getDecl<";
    std::string_view tail = ">(void)";
#else
    #error Compiler not supported
#endif
    // remove boilerplate text
    std::string_view decl = declC;
    size_t c1 = decl.find(head);
    size_t c2 = decl.rfind(tail);
    if (c1 == std::string::npos || c2 == std::string::npos || c1 > c2) {
        return decl; // just in case
    } else {
        c1 += head.length();
        return decl.substr(c1, c2 - c1);
    }
}
