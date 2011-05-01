// LikeMagic C++ Binding Library
// Copyright 2008-2010 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "AbstractClass.hpp"
#include "CallTargetSelector.hpp"
#include "ConstructorCallTarget.hpp"
#include "LikeMagic/TypeConv/NumberConv.hpp"
#include "LikeMagic/TypeConv/ImplicitConv.hpp"
#include "LikeMagic/TypeConv/BaseConv.hpp"
#include "LikeMagic/TypeConv/AddrOfConv.hpp"

#include "FieldGetterTarget.hpp"
#include "FieldSetterTarget.hpp"
#include "ArrayFieldGetterTarget.hpp"
#include "ArrayFieldSetterTarget.hpp"
#include "CustomFieldGetterTarget.hpp"
#include "CustomFieldSetterTarget.hpp"

#include "LikeMagic/SFMO/ClassExpr.hpp"

// Used in friend declaration.
namespace LikeMagic { class RuntimeTypeSystem; }

namespace LikeMagic { namespace Marshaling {

using LikeMagic::AbstractTypeSystem;

template <typename T>
class DummyClass : public AbstractClass
{
protected:
    friend class LikeMagic::RuntimeTypeSystem;
    DummyClass(TypeIndex type_, std::string name_, AbstractTypeSystem& type_system_, NamespacePtr namespace_) : AbstractClass(name_, type_system_, namespace_), type(type_)
    {
    }

    TypeIndex type;

public:

    virtual TypeIndex get_type() const { return type; }

    virtual AbstractCppObjProxy* create_class_proxy() const
    {
        return
            // Whereas regular class returns a proxy on type T by reference,
            // DummyClass uses call-by-pointer.  This is so that the void
            // type will result in "void*" because a "void&" is illegal.
            // It doesn't matter whether we use call by reference or
            // call by pointer, because LikeMagic supports both.
            CppObjProxy<T*, true>::create
            (
                ClassExpr<T*>::create(),
                type_system
            );
    }
};


}}
