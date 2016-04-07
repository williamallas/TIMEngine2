#ifndef BULLETENGINE_H_INCLUDED
#define BULLETENGINE_H_INCLUDED

#include "core.h"
#include <btBulletDynamicsCommon.h>

#include "BulletObject.h"

//#include "bullet/BulletTerrainObject.h"
//#include "bullet/GeometryShape.h"

namespace tim
{
    struct BulletEngine
    {
        static void tickCallback(btDynamicsWorld* world, btScalar time)
        {
            BulletEngine *w = static_cast<BulletEngine *>(world->getWorldUserInfo());
            for(uint i=0 ; i<w->_innerPhysicTick.size() ; ++i)
            {
                w->_innerPhysicTick[i](static_cast<float>(time), w);
            }
        }

        BulletEngine()
            : broadphase(new btDbvtBroadphase), collisionConfiguration(new btDefaultCollisionConfiguration),
              dispatcher(new btCollisionDispatcher(collisionConfiguration)), solver(new btSequentialImpulseConstraintSolver),
              dynamicsWorld(new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration))
        {
            dynamicsWorld->setGravity(btVector3(0, 0, -9.84));
            dynamicsWorld->setInternalTickCallback(tickCallback, static_cast<void *>(this), true);
            //dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration=0.0001f;
        }

        ~BulletEngine()
        {
            delete dynamicsWorld;
            delete solver;
            delete dispatcher;
            delete collisionConfiguration;
            delete broadphase;
        }

        void addObject(BulletObject* o)
        {
            dynamicsWorld->addRigidBody(o->body());
            o->_world = dynamicsWorld;
        }

        void removeObject(BulletObject* o) { dynamicsWorld->removeRigidBody(o->body()); }

        void addInnerPhysicTask(const std::function<void(float,BulletEngine*)>&);

        /** Attributes */
        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;

        btSequentialImpulseConstraintSolver* solver;

        btDiscreteDynamicsWorld* dynamicsWorld;

        vector<std::function<void(float,BulletEngine*)>> _innerPhysicTick;
    };

    inline void BulletEngine::addInnerPhysicTask(const std::function<void(float,BulletEngine*)>& f) { _innerPhysicTick.push_back(f); }
}

#endif // BULLETENGINE_H_INCLUDED
