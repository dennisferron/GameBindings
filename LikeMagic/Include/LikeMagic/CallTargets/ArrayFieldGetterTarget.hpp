// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "LikeMagic/Mirrors/CallTarget.hpp"
#include "LikeMagic/CallTargets/Delegate.hpp"

namespace LM {

template <typename R>
class ArrayFieldGetterTarget : public CallTarget
{
public:
    typedef R (Delegate::*F)[];

private:
    F const f_ptr;
    TypeIndex const actual_type;
    size_t array_size;

public:

    ArrayFieldGetterTarget(F f_ptr_, TypeIndex actual_type_, size_t array_size_)
        : f_ptr(f_ptr_), actual_type(actual_type_), array_size(array_size_) {}

    virtual ExprPtr call(ExprPtr target, ArgList args) const
    {
        ExprPtr ward;
        ExprPtr target_ward;
        Delegate const& target_obj = eval_as_const_target(target, actual_type, target_ward);
        return Term<R>::create(
            (target_obj.*f_ptr)[EvalAs<size_t>::value(args[0], ward)]);
    }

    virtual TypeInfoList const& get_arg_types() const
    {
        static TypeInfoList arg_types = MakeArgList<size_t>::value();
        return arg_types;
    }

    virtual TypeIndex get_return_type() const
    {
        typedef R RArray[];
        return TypId<RArray>::liberal();
    }

    virtual bool is_inherited() const { return true; }

    virtual void mark() const { /* do nothing */ }
};

}
