/**
 * Monad concept
 *
 * Allows for any suitable templated type to be "enhanced" to a functor or monad
 * by providing the appropriate basic operations (functoriality via `fmap`, and
 * monadicity via `ret` and `join`).
 *
 * Inspired by https://stackoverflow.com/a/39730419
 */

#include <concepts>
#include <functional>
#include <utility>

namespace
{
    /**
     * hidden types for checking that templated types implement the desired
     * functionality of a functor or monad
     */
    struct GenericType {};
    struct GenericTarget {};
}

namespace functional
{

    template<template<typename, typename...> class F, typename X, typename Y>
    struct functor
    {};

    /**
     * To realise a templated type F<X> as functorial in X, you need to provide
     * a specialisation of functional::functor that implements
     *
     * static std::function<F<Y>(F<X> &&)>
     * functional::functor<F, X, Y>::fmap(
     *      std::function<Y(X &&)>);
     *
     */
    template<template<typename, typename...> class F>
    concept functorial
    = requires(std::function<GenericTarget(GenericType const &&)> &&morphism)
    {
        { ::functional::functor<F, GenericType, GenericTarget>::fmap(
                std::forward<decltype(morphism)>(morphism)) }
            -> std::convertible_to<
                std::function<F<GenericTarget>(F<GenericType> const &&)>>;
    };

    template<template<typename, typename...> class F, typename X>
        requires functorial<F>
    struct monad
    {};

    /**
     * To realise a functor F<X> as monadic, you need to provide a
     * specialisation of functional::monad that implements
     *
     * static F<X>
     * functional::monad<F, X>::ret(X &&);
     *
     * static F<X>
     * functional::monad<F, X>::join(F<F<X>> &&);
     * 
     */
    template<template<typename, typename...> class F>
    concept monadic
    = requires (GenericType const &&generic, F<F<GenericType>> const &&ff)
    {
        { ::functional::monad<F, GenericType>::ret(
                std::forward<decltype(generic)>(generic)) }
            -> std::same_as<F<GenericType>>;
        { ::functional::monad<F, GenericType>::join(
                std::forward<decltype(ff)>(ff)) }
            -> std::same_as<F<GenericType>>;
    };

    /* Now, some monadic structure */

    /* bind : [X -> TY] -> [TX -> TY] */
    template<template<typename, typename...> class Monad, typename X, typename Y>
        requires monadic<Monad>
    std::function<Monad<Y>(Monad<X> const &&)>
    bind(std::function<Monad<Y>(X const &&)> map)
    {
        return [map](Monad<X> const &&x)->Monad<Y>
        {
            return monad<Monad, Y>::join(
                    functor<Monad, X, Monad<Y>>::fmap(map)(
                        std::forward<Monad<X> const>(x)));
        };
    }

    /* view pure functions as having trivial computational effect */
    template<template<typename, typename...> class Monad, typename X, typename Y>
        requires monadic<Monad>
    std::function<Monad<Y>(X const &&)> as_pure(std::function<Y(X const &&)> pure)
    {
        return [pure](X const &&x)->Monad<Y>
        {
            return monad<Monad, Y>::ret(pure(std::forward<X const>(x)));
        };
    }
}

/* syntax sugar for infixing a binary function */
namespace
{
    template<typename Left, typename BinaryFunction>
    struct infix
    {
        Left &&left;
        BinaryFunction const &op;
        infix(Left &&left, BinaryFunction const &op) :
            left(std::forward<Left>(left)), op(op) {}
    };
}

template<typename Left, typename BinaryFunction>
auto operator/(Left &&left, BinaryFunction &op)
{
    return infix<Left, BinaryFunction>(std::forward<Left>(left), op);
}

template<typename Left, typename BinaryFunction, typename Right>
auto operator/(infix<Left, BinaryFunction> left, Right &&right)
{
    return left.op(std::forward<Right>(right))(std::forward<Left>(left.left));
}
