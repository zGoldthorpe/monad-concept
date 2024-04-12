/**
 * Implementation of list monad (using vector)
 */
#include "monad.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

template<typename X>
using List = std::vector<X>;

/* Functoriality of vector */
template<typename X, typename Y>
struct functional::functor<List, X, Y>
{
    static std::function<List<Y>(List<X> const &&)>
    fmap(std::function<Y(X const &&)> morphism)
    {
        return [morphism](List<X> const &&ls)->List<Y>
        {
            List<Y> res;
            res.reserve(ls.size());
            for (auto &&x : ls)
            {
                res.emplace_back(morphism(std::forward<X const>(x)));
            }
            return res;
        };
    }
};

/* Monadicity of vector */
template<typename X>
struct functional::monad<List, X>
{
    static List<X> ret(X const &&val)
    {
        return { val };
    }
    static List<X> join(List<List<X>> const &&lls)
    {
        List<X> res;
        for (auto &&ls : lls)
        {
            std::move(ls.begin(), ls.end(), std::back_inserter(res));
        }
        return res;
    }
};

int main()
{
    using functional::bind, functional::as_pure;

    auto result = (List<int> { 1, 2, 3, 4, 5 }
        /bind<List, int, char>/ as_pure<List, int, char>([](int x)->char { return x + '0'; })
        /bind<List, char, bool>/ [](char c)->List<bool> {
                if (c < '0')
                {
                    return {};
                }
                List<bool> res((int)(c-'0')+1, false);
                res[0] = true;
                return res;
            }
        /bind<List, bool, bool>/ as_pure<List, bool, bool>([](bool x)->bool { return !x; }));

    for (auto const &v : result)
    {
        std::cout << v << '\n';
    }
    return 0;
}
