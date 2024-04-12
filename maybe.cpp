/**
 * Implementation of maybe monad (using optional)
 */
#include "monad.hpp"

#include <iostream>
#include <optional>

template<typename X>
using Maybe = std::optional<X>;

/* Functoriality of optional */
template<typename X, typename Y>
struct functional::functor<Maybe, X, Y>
{
    static std::function<Maybe<Y>(Maybe<X> const &&)>
    fmap(std::function<Y(X const &&)> morphism)
    {
        return [morphism](Maybe<X> const &&mb)->Maybe<Y>
        {
            return mb ? Maybe<Y> (morphism(std::forward<X const>(*mb))) : std::nullopt;
        };
    }
};

/* Monadicity of optional */
template<typename X>
struct functional::monad<Maybe, X>
{
    static Maybe<X> ret(X const &&val)
    {
        return { val };
    }
    static Maybe<X> join(Maybe<Maybe<X>> const &&mm)
    {
        return mm ? *mm : std::nullopt;
    }
};

int main()
{
    using functional::bind, functional::as_pure;

    auto result = (Maybe<int> { 5 }
        /bind<Maybe, int, int>/ as_pure<Maybe, int, int>([](int x)->int { return x / 2; })
        /bind<Maybe, int, bool>/ as_pure<Maybe, int, bool>([](int x)->bool { return x == 0; })
        /bind<Maybe, int, int>/ as_pure<Maybe, bool, int>([](bool b)->int { return b ? 1 : 0; })
        /bind<Maybe, int, double>/ [](int x)->Maybe<double> { return x ? std::make_optional(1.0 / x) : std::nullopt; }
        /bind<Maybe, double, int>/ [](double x)->Maybe<int> { return x > 0.0 ? std::make_optional(15) : std::nullopt; });

    if (result)
    {
        std::cout << *result << std::endl;
    }
    else
    {
        std::cout << "nullopt" << std::endl;
    }

    return 0;
}
