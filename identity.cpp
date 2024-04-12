/**
 * Implementation of the identity monad.
 */
#include "monad.hpp"

#include <iostream>

template<typename X>
struct Id
{
    X const val;
};

/* Functoriality of Id */
template<typename X, typename Y>
struct functional::functor<Id, X, Y>
{
    static std::function<Id<Y>(Id<X> const &&)>
    fmap(std::function<Y(X const &&)> morphism)
    {
        return [morphism](Id<X> const &&x)->Id<Y>
        {
            return { morphism(std::forward<X const>(x.val)) };
        };
    }
};

/* Monadicity of Id */
template<typename X>
struct functional::monad<Id, X>
{
    static Id<X> ret(X const &&val)
    {
        return { std::forward<X const>(val) };
    }
    static Id<X> join(Id<Id<X>> const &&ii)
    {
        return ii.val;
    }
};

int main()
{
    using functional::bind, functional::as_pure;

    auto result = (Id<int> {5}
        /bind<Id, int, int>/ as_pure<Id, int, int>([](int x)->int { return x - 3; })
        /bind<Id, int, bool>/ as_pure<Id, int, bool>([](int x)->bool { return x > 0; })
        /bind<Id, bool, double>/ [](bool tv)->Id<double> { return { tv ? 1.0 : 0.0 }; }
        ).val;

    std::cout << result << std::endl;
    return 0;
}
