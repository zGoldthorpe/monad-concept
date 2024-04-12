/**
 * Implementation of a writer monad.
 * Not sure how to parametrise by a monoid concept, so we just put a placeholder
 * monoid as a proof-of-concept.
 */
#include "monad.hpp"

#include <iostream>
#include <string>

/* monoid of strings and concatenation */
template<typename X>
using Writer = std::pair<std::string, X>;

/* Functoriality of writer */
template<typename X, typename Y>
struct functional::functor<Writer, X, Y>
{
    static std::function<Writer<Y>(Writer<X> const &&)>
    fmap(std::function<Y(X const &&)> morphism)
    {
        return [morphism](Writer<X> const &&writer)->Writer<Y>
        {
            auto &&[ content, x ] = writer;
            return { content, morphism(std::forward<X const>(x)) };
        };
    }
};

/* Monadicity of writer */
template<typename X>
struct functional::monad<Writer, X>
{
    static Writer<X> ret(X const &&val)
    {
        return { "", val };
    }

    static Writer<X> join(Writer<Writer<X>> const &&ww)
    {
        // ww = [ lhs, [ rhs, x ] ]
        auto &&[ lhs, w ] = ww;
        auto &&[ rhs, x ] = w;
        return { lhs + rhs, x };
    }
};

int main()
{
    using functional::bind, functional::as_pure;

    auto result = (Writer<int> { "Test.\n", 15 }
        /bind<Writer, int, int>/ as_pure<Writer, int, int>([](int x)->int { return x/3; })
        /bind<Writer, int, int>/ [](int x)->Writer<int> { return { std::to_string(x) + "\n", 4 }; }
        /bind<Writer, int, std::string>/ [](int x)->Writer<std::string> { return { "Forget about " + std::to_string(x) + ".\n", "Hello!" }; }
        /bind<Writer, std::string, int>/ [](std::string s)->Writer<int> { return { s, s.size() }; });

    auto &&[ message, value ] = result;
    std::cout << "Message:\n" << message << "\n---\nValue: " << value << std::endl;
    return 0;
}
