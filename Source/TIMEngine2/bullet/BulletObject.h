#ifndef BULLETOBJECT_H
#define BULLETOBJECT_H

#include "core.h"
#include "bullet/SceneMotionState.h"
#include <btBulletDynamicsCommon.h>

namespace tim
{
using namespace core;

    class BulletObject
    {
        friend struct BulletEngine;
    public:
        BulletObject(btMotionState*, btCollisionShape*, float mass = 0);
        BulletObject(mat4, btCollisionShape*, float mass = 0);
        virtual ~BulletObject();

        void addConstraintToWorld(btTypedConstraint*);

        void setMotionState(btMotionState* mt);

        btRigidBody* body() { return _body; }
        const btRigidBody* body() const { return _body; }

        int indexWorld() const { return _indexWorld; }
        int colMask() const { return _collisionMask; }
        int colWithMask() const { return _collisionWithMask; }

       struct CollisionPoint
       {
           vec3 pos;
           vec3 normal;
           float depth=1;
           btCollisionObject* with;
       };

       vector<CollisionPoint> collideWorld() const;
       vector<CollisionPoint> collideWith(btCollisionObject*) const;
       vector<CollisionPoint> continuousCollide(const mat4& dest) const;
       bool firstCollision(const mat4& dest, CollisionPoint&) const;
       bool rayCastNotMe(const vec3& from, const vec3& to, CollisionPoint&) const;
       bool rayCast(const vec3& from, const vec3& to, CollisionPoint&) const;
       static bool rayCastFirst(const vec3& from, const vec3& to, CollisionPoint&, btDiscreteDynamicsWorld&);

       static btTransform toBtTransform(const mat4& m)
       {
           btTransform worldTrans;
           worldTrans.setBasis(btMatrix3x3(m[0][0],m[0][1],m[0][2],
                                           m[1][0],m[1][1],m[1][2],
                                           m[2][0],m[2][1],m[2][2]));

           worldTrans.setOrigin(btVector3(m[0].w(), m[1].w(), m[2].w()));
           return worldTrans;
       }

       static mat4 fromBtTransform(const btTransform& worldTrans)
       {
           mat4 m;
           btMatrix3x3 m33 = worldTrans.getBasis();
           m[0] = {m33[0].m_floats[0], m33[0].m_floats[1], m33[0].m_floats[2], 0};
           m[1] = {m33[1].m_floats[0], m33[1].m_floats[1], m33[1].m_floats[2], 0};
           m[2] = {m33[2].m_floats[0], m33[2].m_floats[1], m33[2].m_floats[2], 0};
           m[3] = {0,0,0,1};
           m.setTranslation(vec3(worldTrans.getOrigin().m_floats[0], worldTrans.getOrigin().m_floats[1], worldTrans.getOrigin().m_floats[2]));
           return m;
       }

    private:
        btMotionState* _motionState = nullptr;
        btRigidBody* _body = nullptr;
        btDiscreteDynamicsWorld* _world = nullptr;
        int _indexWorld = -1;

        int _collisionMask = 0, _collisionWithMask = 0;

        void swap(BulletObject&);
    };
}

#endif // BULLETOBJECT_H
