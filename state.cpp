/**
 * Implementation of a state monad.
 * Not sure how to parametrise by the state type, so we just put a placeholder
 * `state_t` type as a proof-of-concept.
 */
#include "monad.hpp"

#include <iostream>
#include <string>

/* some kind of "global state" */
struct state_t
{
    int counter;
    std::string message;
};

template<typename X>
using eff_t = std::pair<X, state_t>;

template<typename X>
using Exec = std::function<eff_t<X>(state_t const &&)>;

/* Functoriality of state monad
 * A map f : X -> Y induces a map
 * [ S, X x S ] -> [ S, Y x S ]
 * via currying a map
 * [ S, X x S ] x S -> Y x S
 * */
template<typename X, typename Y>
struct functional::functor<Exec, X, Y>
{
    static std::function<Exec<Y>(Exec<X> const &&)>
    fmap(std::function<Y(X const &&)> morphism)
    {
        return [morphism](Exec<X> const &&exec)->Exec<Y>
        {
            return [morphism, exec=std::forward<decltype(exec)>(exec)](state_t const &&state)->eff_t<Y>
            {
                auto &&[ x, next_state ] = exec(std::forward<decltype(state)>(state));
                return { morphism(std::forward<X const>(x)), next_state };
            };
        };
    }
};

/* Monadicity of state monad */
template<typename X>
struct functional::monad<Exec, X>
{
    /* ret curries the identity map on X x S */
    static Exec<X> ret(X const &&val)
    {
        return [val=std::forward<X const>(val)](state_t const &&state)->eff_t<X>
        {
            return { val, state };
        };
    }
    /* join is given by evaluation [ S, [ S, X x S ] x S ] -> [ S, X x S ] */
    static Exec<X> join(Exec<Exec<X>> const &&ee)
    {
        return [ee=std::forward<decltype(ee)>(ee)](state_t const &&state)
        {
            auto &&[ exec, middle_state ] = ee(std::forward<decltype(state)>(state));
            return exec(std::forward<decltype(middle_state)>(middle_state));
        };
    }
};

int main()
{
    using functional::bind, functional::as_pure;

    auto result = (functional::monad<Exec, int>::ret(5)
        /bind<Exec, int, int>/ as_pure<Exec, int, int>([](int x)->int { return x + 3; })
        /bind<Exec, int, int>/ [](int x)->Exec<int> { return [x](state_t const &&state)->eff_t<int>
            {
                return { x, { state.counter + 1, state.message + "Int: " + std::to_string(x) + "\n" } };
            };}
        /bind<Exec, int, bool>/ [](int x)->Exec<bool> { return [x](state_t const &&state)->eff_t<bool>
            {
                if (!(x & (x - 1)))
                {
                    return { true, { state.counter + 1, state.message + "It's a power of 2!\n" } };
                }
                return { false, { state.counter, state.message + "It's not a power of 2...\n" } };
            };}
        /bind<Exec, bool, double>/ [](bool b)->Exec<double> { return [b](state_t const &&state)->eff_t<double>
            {
                if (b)
                {
                    return { 1.0, { state.counter + 1, state.message + "It's still true.\n" } };
                }
                return { 0.0, { state.counter + 1000, state.message + "It's still false.\n" } };
            };});

    auto &&[ val, state ] = result({ 10000, "Testing 1, 2, 3...\n" });
    std::cout << "Value: " << val
        << "\nState counter: " << state.counter
        << "\nState message: " << state.message << std::endl;
    return 0;
}
