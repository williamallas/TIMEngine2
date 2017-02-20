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

        BulletEngine();
        ~BulletEngine();

        void addObject(BulletObject* o, int worldId = 0);
        void addObject(BulletObject* o, int worldId, int colMask, int colWithMask);
        void removeObject(BulletObject* o) { o->_world->removeRigidBody(o->body()); }

        void createWorld(int index);

        void setGravity(int index, vec3 gravity) { dynamicsWorld[index]->setGravity(btVector3(gravity.x(), gravity.y(), gravity.z())); }

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
