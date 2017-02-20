#include "BulletEngine.h"

namespace tim
{

BulletEngine::BulletEngine()
{
    createWorld(0);
    gContactBreakingThreshold = 0.005;
    //dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration=0.0001f;
}

BulletEngine::~BulletEngine()
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

void BulletEngine::addObject(BulletObject* o, int worldId)
{
    dynamicsWorld[worldId]->addRigidBody(o->body());
    o->body()->setDamping(0.3, 0.5);
    o->_world = dynamicsWorld[worldId];
    o->_indexWorld = worldId;
}

void BulletEngine::addObject(BulletObject* o, int worldId, int colMask, int colWithMask)
{
    dynamicsWorld[worldId]->addRigidBody(o->body(), colMask, colWithMask);
    o->body()->setDamping(0.3, 0.5);
    o->_world = dynamicsWorld[worldId];
    o->_indexWorld = worldId;
    o->_collisionMask = colMask;
    o->_collisionWithMask = colWithMask;
}

void BulletEngine::createWorld(int index)
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

}
