// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "Bindings/Irrlicht/IrrlichtBindingsDLL.hpp"

namespace LM {
    class TypeSystem;
    class LangInterpreter;
}

namespace Bindings { namespace Irrlicht {

IRRLICHT_BINDINGS_API void add_values(LM::LangInterpreter&, LM::TypeSystem&);

}}
