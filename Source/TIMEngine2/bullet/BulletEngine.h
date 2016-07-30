#ifndef BULLETENGINE_H_INCLUDED
#define BULLETENGINE_H_INCLUDED

#include "core.h"
#include <btBulletDynamicsCommon.h>

#include "BulletObject.h"
#include "HeightFieldShape.h"
//#include "bullet/BulletTerrainObject.h"

namespace tim
{
    struct BulletEngine
    {
        static const int NB_WORLD = 32;
        using TickCallback = std::function<void(float,BulletEngine*)>;

        static void tickCallback(btDynamicsWorld* world, btScalar time)
        {
            BulletEngine *w = static_cast<BulletEngine *>(world->getWorldUserInfo());
            int indexWorld=0;
            for(int j=0 ; j<NB_WORLD ; ++j)
            {
                if(w->dynamicsWorld[j] == world)
                {
                    indexWorld = j;
                    break;
                }
            }

            for(uint i=0 ; i<w->_innerPhysicTick.size() ; ++i)
            {

                if(w->_innerPhysicTick[i].second == indexWorld)
                    w->_innerPhysicTick[i].first(static_cast<float>(time), w);
            }
        }

        BulletEngine()
            : broadphase(new btDbvtBroadphase), collisionConfiguration(new btDefaultCollisionConfiguration),
              dispatcher(new btCollisionDispatcher(collisionConfiguration)), solver(new btSequentialImpulseConstraintSolver)
        {
            dynamicsWorld[0] = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
            dynamicsWorld[0]->setGravity(btVector3(0, 0, -9.87));
            dynamicsWorld[0]->setInternalTickCallback(tickCallback, static_cast<void *>(this), true);
			gContactBreakingThreshold = 0.005;
            //dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration=0.0001f;
        }

        ~BulletEngine()
        {
            for(int i=0 ; i<NB_WORLD ; ++i)
                delete dynamicsWorld[i];

            delete solver;
            delete dispatcher;
            delete collisionConfiguration;
            delete broadphase;
        }

        void addObject(BulletObject* o, int worldId = 0)
        {
            dynamicsWorld[worldId]->addRigidBody(o->body());
            o->_world = dynamicsWorld[worldId];
        }

        void createWorld(int index)
        {
            if(dynamicsWorld[index] == nullptr)
            {
                dynamicsWorld[index] = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
                dynamicsWorld[index]->setGravity(btVector3(0, 0, -9.87));
                dynamicsWorld[index]->setInternalTickCallback(tickCallback, static_cast<void *>(this), true);
            }
        }

        void removeObject(BulletObject* o) { o->_world->removeRigidBody(o->body()); }

        void addInnerPhysicTask(const TickCallback&, int);

        /** Attributes */
        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;

        btSequentialImpulseConstraintSolver* solver;

        btDiscreteDynamicsWorld* dynamicsWorld[NB_WORLD] = {nullptr};

        vector<std::pair<TickCallback, int>> _innerPhysicTick;
    };

    inline void BulletEngine::addInnerPhysicTask(const TickCallback& f, int worldIndex) { _innerPhysicTick.push_back({f, worldIndex}); }
}

#endif // BULLETENGINE_H_INCLUDED
