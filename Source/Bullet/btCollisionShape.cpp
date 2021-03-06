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

void add_bindings_btCollisionShape()
{
    TypeMirror& global_ns = type_system->global_namespace();
    TypeMirror& ns_bullet = register_namespace("Bullet", global_ns);

    LM_CLASS(ns_bullet, btCollisionShape)
    LM_FUNC(btCollisionShape,
            (calculateLocalInertia)
            (calculateSerializeBufferSize)(calculateTemporalAabb)(getAabb)(getAngularMotionDisc)(getBoundingSphere)(getContactBreakingThreshold)
            (getLocalScaling)(getMargin)(getName)(getShapeType)(getUserPointer)(isCompound)(isConcave)(isConvex)(isConvex2d)(isInfinite)
            (isPolyhedral)(isSoftBody)(serialize)(setLocalScaling)(setMargin)(setUserPointer)
    )

    LM_CLASS(ns_bullet, btStaticPlaneShape)
    LM_BASE(btStaticPlaneShape, btCollisionShape)
    LM_CONSTR(btStaticPlaneShape, "new",btVector3 const&, btScalar)

    LM_CLASS(ns_bullet, btSphereShape)
    LM_BASE(btSphereShape, btCollisionShape)
    LM_CONSTR(btSphereShape, "new", btScalar const&)

    LM_CLASS(ns_bullet, btBoxShape)
    LM_BASE(btBoxShape, btCollisionShape)
    LM_CONSTR(btBoxShape, "new", btVector3 const&)

    LM_CLASS(ns_bullet, btCylinderShape)
    LM_BASE(btCylinderShape, btCollisionShape)
    LM_CONSTR(btCylinderShape, "new", btVector3 const&)

    LM_CLASS(ns_bullet, btCylinderShapeX)
    LM_BASE(btCylinderShapeX, btCollisionShape)
    LM_CONSTR(btCylinderShapeX, "new", btVector3 const&)

    LM_CLASS(ns_bullet, btCylinderShapeZ)
    LM_BASE(btCylinderShapeZ, btCollisionShape)
    LM_CONSTR(btCylinderShapeZ, "new", btVector3 const&)

    LM_CLASS(ns_bullet, btConeShape)
    LM_BASE(btConeShape, btCollisionShape)
    LM_CONSTR(btConeShape, "new", btScalar, btScalar)

    LM_CLASS(ns_bullet, btConeShapeX)
    LM_BASE(btConeShapeX, btCollisionShape)
    LM_CONSTR(btConeShapeX, "new", btScalar, btScalar)

    LM_CLASS(ns_bullet, btConeShapeZ)
    LM_BASE(btConeShapeZ, btCollisionShape)
    LM_CONSTR(btConeShapeZ, "new", btScalar, btScalar)

    LM_CLASS(ns_bullet, btCapsuleShape)
    LM_BASE(btCapsuleShape, btCollisionShape)
    LM_CONSTR(btCapsuleShape, "new", btScalar, btScalar)

    LM_CLASS(ns_bullet, btCompoundShape)
    LM_BASE(btCompoundShape, btCollisionShape)
    LM_CONSTR(btCompoundShape, "new", bool)
    LM_FUNC(btCompoundShape, (addChildShape)(calculateLocalInertia)(calculatePrincipalAxisTransform)(calculateSerializeBufferSize))
    LM_FUNC(btCompoundShape, (getAabb)(getChildList))

    LM_FUNC_OVERLOAD_BOTH(btCompoundShape, getDynamicAabbTree, btDbvt*)

    LM_FUNC(btCompoundShape, (getLocalScaling)(getMargin))
    LM_FUNC(btCompoundShape, (getName)(getNumChildShapes)(getUpdateRevision))
    LM_FUNC(btCompoundShape, (recalculateLocalAabb)(removeChildShape)(removeChildShapeByIndex)(serialize)(setLocalScaling)(setMargin)(updateChildTransform))

    LM_FUNC_OVERLOAD_BOTH(btCompoundShape, getChildShape, btCollisionShape*, int)
    LM_FUNC_OVERLOAD_BOTH(btCompoundShape, getChildTransform, btTransform&, int)
}

}}

