// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)


#pragma once

#include "LikeMagic/TypeConv/AbstractTypeConverter.hpp"
#include "boost/intrusive_ptr.hpp"

// Forward declarations.
namespace LikeMagic { namespace Exprs {

class AbstractExpression;

typedef boost::intrusive_ptr<AbstractExpression> ExprPtr;

}}

namespace LikeMagic { namespace TypeConv {

using namespace LikeMagic::Exprs;

template <typename To>
class ConvertibleTo : public virtual AbstractTypeConverter
{
public:
    virtual std::string description() const { return BetterTypeInfo::create_index<decltype(this)>().description(); }
};

}}
