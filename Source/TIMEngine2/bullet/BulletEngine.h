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
                if(w->_innerPhysicTick[i].second == indexWorld || w->_innerPhysicTick[i].second < 0)
                    w->_innerPhysicTick[i].first(static_cast<float>(time), w);
            }
        }

        BulletEngine()
        {
            createWorld(0);
			gContactBreakingThreshold = 0.005;
            //dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration=0.0001f;
        }

        ~BulletEngine()
        {
            for(int i=0 ; i<NB_WORLD ; ++i)
            {
                delete dynamicsWorld[i];
                delete solver[i];
                delete dispatcher[i];
                delete collisionConfiguration[i];
                delete broadphase[i];
            }
        }

        void addObject(BulletObject* o, int worldId = 0)
        {
            dynamicsWorld[worldId]->addRigidBody(o->body());
            o->body()->setDamping(0.3, 0.5);
            o->_world = dynamicsWorld[worldId];
            o->_indexWorld = worldId;
        }

        void addObject(BulletObject* o, int worldId, int colMask, int colWithMask)
        {
            dynamicsWorld[worldId]->addRigidBody(o->body(), colMask, colWithMask);
            o->body()->setDamping(0.3, 0.5);
            o->_world = dynamicsWorld[worldId];
            o->_indexWorld = worldId;
            o->_collisionMask = colMask;
            o->_collisionWithMask = colWithMask;
        }

        void reinstance(BulletObject* o, int colMask, int colWithMask)
        {
            auto* mt = o->body()->getMotionState();
            o->setMotionState(nullptr);

            BulletObject* b = new BulletObject(mt, o->body()->getCollisionShape(), 1.f / o->body()->getInvMass());
            b->body()->setFriction(o->body()->getFriction());
            b->body()->setRestitution(o->body()->getRestitution());
            b->body()->setRollingFriction(o->body()->getRollingFriction());
            b->body()->setCcdMotionThreshold(o->body()->getCcdMotionThreshold());
            b->body()->setCcdSweptSphereRadius(o->body()->getCcdSweptSphereRadius());

            b->body()->setLinearVelocity(o->body()->getLinearFactor());
            b->body()->setAngularVelocity(o->body()->getAngularVelocity());
            b->body()->setUserIndex(o->body()->getUserIndex());


            addObject(b, o->_indexWorld, colMask, colWithMask);

            o->swap(*b);
            delete b;
        }

        void createWorld(int index)
        {
            if(dynamicsWorld[index] == nullptr)
            {
                broadphase[index] = new btDbvtBroadphase;
                collisionConfiguration[index] = new btDefaultCollisionConfiguration;
                dispatcher[index] = new btCollisionDispatcher(collisionConfiguration[index]);
                solver[index] = new btSequentialImpulseConstraintSolver;

                dynamicsWorld[index] = new btDiscreteDynamicsWorld(dispatcher[index], broadphase[index],
                                                                   solver[index], collisionConfiguration[index]);
                dynamicsWorld[index]->setGravity(btVector3(0, 0, -9.87));
                dynamicsWorld[index]->setInternalTickCallback(tickCallback, static_cast<void *>(this), true);
                //dynamicsWorld[index]->stepSimulation(0.00001, 1);
            }
        }

        void removeObject(BulletObject* o) { o->_world->removeRigidBody(o->body()); }

        void addInnerPhysicTask(const TickCallback&, int);

        /** Attributes */
        btBroadphaseInterface* broadphase[NB_WORLD] = {nullptr};
        btDefaultCollisionConfiguration* collisionConfiguration[NB_WORLD] = {nullptr};
        btCollisionDispatcher* dispatcher[NB_WORLD] = {nullptr};
        btSequentialImpulseConstraintSolver* solver[NB_WORLD] = {nullptr};

        btDiscreteDynamicsWorld* dynamicsWorld[NB_WORLD] = {nullptr};

        vector<std::pair<TickCallback, int>> _innerPhysicTick;
    };

    inline void BulletEngine::addInnerPhysicTask(const TickCallback& f, int worldIndex) { _innerPhysicTick.push_back({f, worldIndex}); }
}

#endif // BULLETENGINE_H_INCLUDED
