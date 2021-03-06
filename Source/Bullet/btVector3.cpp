// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#include "btBulletDynamicsCommon.h"

#include "LikeMagic/BindingMacros.hpp"

using namespace LM;

namespace Bindings { namespace Bullet {

void add_bindings_btVector3()
{
    TypeMirror& global_ns = type_system->global_namespace();
    TypeMirror& ns_bullet = register_namespace("Bullet", global_ns);

    LM_CLASS(ns_bullet, btVector3)
    LM_CONSTR(btVector3,"new")
    LM_CONSTR(btVector3, "new", btVector3 const&)
    LM_CONSTR(btVector3, "new", btScalar const&, btScalar const&, btScalar const&)
    LM_FUNC(btVector3, (absolute)(angle)(closestAxis)(cross)(deSerialize)(deSerializeDouble)(deSerializeFloat)(distance)
            (distance2)(dot)(furthestAxis)(fuzzyZero)(getSkewSymmetricMatrix)(getX)(getY)(getZ)(isZero)(length)(length2)
            (lerp)(maxAxis)(minAxis)(normalize)(normalized)(rotate)(serializeDouble)(serializeFloat)(setInterpolate3)
            (setMax)(setMin)(setW)(setValue)(setX)(setY)(setZ)(setZero)(triple)(w)(x)(y)(z))

    LM_OP(btVector3, (+=)(-=))
}

}}

