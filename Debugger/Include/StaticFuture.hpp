#pragma once

#include "ChainBuilder.hpp"

namespace Iocaste {
    namespace Debugger {

/*

After I wrote several input/output components, arranging the code in such a way
that it clearly presents what kind of chain the code is building was obfuscated
by the fact that constructor parameters to the objects necessitated instantiating
the objects in a different order than the order of the chain.

I wanted to be able to write input/output chains with syntax like:

chain.to<A>().to<B>().to<C>()...

But there is a problem: I need to pass C to B's constructor, and pass B to A's constructor,
however with a syntax like the above, A() would be evaluated before B() and B() before C().

My solution is to make the expression "to<X>()" return not the real "X" but a promise to create
"X" in the future, after all the constructor dependencies are available.  This allows me to use a uniform
syntax to create chains of input / output components, without regard to needs of instantiation
order, and then instantiate everything only after the complete chain is described.

*/

template <typename T, typename ConstructorPolicy, typename LeftFuture, typename HeadType, typename... Args>
struct StaticFuture
{
    T* self;
    LeftFuture lhs;
    std::tuple<Args&&...> args;

    StaticFuture(LeftFuture& lhs_, Args&&... args_)
        : self(nullptr), lhs(lhs_), args(std::forward_as_tuple(args_...))
    {
    }

    std::pair<HeadType*, T*> force()
    {
        HeadType& head = _unwind(UnusedType());
        T& temp = *self;
        return std::make_pair(&head, &temp);
    }

    template <typename RHS>
    HeadType& _unwind(RHS rhs)
    {
        typedef typename MakeIndexPack<sizeof...(Args)>::type IPack;
        typedef decltype(get(ChainPolicy(), *(T*)0)) ConstructorPolicyType;

        if (!self)
            self = ChainBuilder<T, decltype(lhs), decltype(rhs),
                ConstructorPolicyType>::create(lhs, rhs, args, IPack());

        return lhs._unwind(*self);
    }

    typedef StaticFuture ThisFuture;

    template <typename Next, typename... NextArgs>
    StaticFuture<Next, ConstructorPolicy, ThisFuture, HeadType, NextArgs...> to(NextArgs&&... args_)
    {
        return StaticFuture<Next, ConstructorPolicy, ThisFuture, HeadType, NextArgs...>(*this, std::forward<NextArgs>(args_)...);
    }
};

struct StopUnwind
{
    template <typename RHS>
    RHS& _unwind(RHS& rhs) { return rhs; }
};

template <typename ConstructorPolicy>
struct BeginFuture
{
    template <typename T, typename... NextArgs>
    StaticFuture<T, ConstructorPolicy, StopUnwind, T, NextArgs...> to(NextArgs&&... args_)
    {
        StopUnwind& lhs = *new StopUnwind();
        return StaticFuture<T, ConstructorPolicy, StopUnwind, T, NextArgs...>(lhs, std::forward<NextArgs>(args_)...);
    }
};

    }
}
