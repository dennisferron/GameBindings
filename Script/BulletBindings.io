method(type_system, io_vm, Bullet_Bindings, LikeMagic,

    Bullet_Bindings lexicalDo(

        // This is actually creating an object of type "Namespace" that happens to be in the
        // LikeMagic scope called "namespace".  It's not as confusing as it looks...
        // It has to create a namespace-object to tell the initial add_bindings for bullet
        // what LikeMagic namespace to add all the other add_bindings functions to.
        ns_bullet_bindings := LikeMagic namespace Namespace global(type_system) subspace("Bindings") subspace("Bullet")
        add_bindings(ns_bullet_bindings)

        ns_bullet := LikeMagic namespace Namespace global(type_system) subspace("Bullet")
        add_bindings_btBroadphaseProxy(ns_bullet)
        add_bindings_btBroadphaseInterface(ns_bullet)
        add_bindings_btCollisionConfiguration(ns_bullet)
        add_bindings_btCollisionObject(ns_bullet)
        add_bindings_btCollisionShape(ns_bullet)
        add_bindings_btCollisionWorld(ns_bullet)
        add_bindings_btConcaveShape(ns_bullet)
        add_bindings_btConstraintSolver(ns_bullet)
        add_bindings_btDispatcher(ns_bullet)
        add_bindings_btMatrix3x3(ns_bullet)
        add_bindings_btMotionState(ns_bullet)
        add_bindings_btQuaternion(ns_bullet)
        add_bindings_btSoftBody(ns_bullet)
        add_bindings_btStridingMeshInterface(ns_bullet)
        add_bindings_btTransform(ns_bullet)
        add_bindings_btTypedConstraint(ns_bullet)
        add_bindings_btVector3(ns_bullet)
        add_protos(io_vm)
    )
)

