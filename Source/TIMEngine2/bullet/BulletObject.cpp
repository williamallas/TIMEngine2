#include "BulletObject.h"
#include "BulletEngine.h"

namespace tim
{

BulletObject::BulletObject(btMotionState* ms, btCollisionShape* shape, float mass)
    : _motionState(ms)
{
    btVector3 inertia(0, 0, 0);

    if(mass > 0)
        shape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, _motionState, shape, inertia);
    _body = new btRigidBody(rigidBodyCI);
}

BulletObject::BulletObject(mat4 m, btCollisionShape* shape, float mass)
{
    btTransform worldTrans;
    worldTrans.setBasis(btMatrix3x3(m[0][0],m[0][1],m[0][2],
                                            m[1][0],m[1][1],m[1][2],
                                            m[2][0],m[2][1],m[2][2]));

    worldTrans.setOrigin(btVector3(m[0].w(), m[1].w(), m[2].w()));
    _motionState = new btDefaultMotionState(worldTrans);

    btVector3 inertia(0, 0, 0);

    if(mass > 0)
        shape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, _motionState, shape, inertia);
    _body = new btRigidBody(rigidBodyCI);
    _body->setMotionState(nullptr);
    delete _motionState;
    _motionState = nullptr;
}

BulletObject::~BulletObject()
{
    delete _motionState;
    _world->removeCollisionObject(_body);
    delete _body;
}

void BulletObject::swap(BulletObject& o)
{
    std::swap(*this, o);
}

void BulletObject::setMotionState(btMotionState* mt)
{
    _motionState = mt;
    if(_body)
        _body->setMotionState(mt);
}

vector<BulletObject::CollisionPoint> BulletObject::collideWorld() const
{
    struct CollideCallback : public btCollisionWorld::ContactResultCallback
    {

    CollideCallback(btRigidBody& target)
        : btCollisionWorld::ContactResultCallback(), body(target) { }

    btRigidBody& body;
    vector<CollisionPoint> contacts;

    bool needsCollision(btBroadphaseProxy* proxy) const
    {
        // superclass will check m_collisionFilterGroup and m_collisionFilterMask
        if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
            return false;
        // if passed filters, may also want to avoid contacts between constraints
        return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
    }

    btScalar addSingleResult(btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0,int /*partId0*/,int /*index0*/,
        const btCollisionObjectWrapper* colObj1,int /*partId1*/,int /*index1*/)
    {
        btVector3 pt, norm;
        CollisionPoint p;
        if(colObj0->m_collisionObject==&body)
        {
            pt = cp.m_localPointA;
            norm = cp.m_normalWorldOnB;
            p.with = const_cast<btCollisionObject*>(colObj1->m_collisionObject);
        }
        else
        {
            pt = cp.m_localPointB;
            norm = -cp.m_normalWorldOnB;
            p.with = const_cast<btCollisionObject*>(colObj0->m_collisionObject);
        }

        p.pos = {pt.x(), pt.y(), pt.z()};
        p.normal = {norm.x(), norm.y(), norm.z()};
        p.depth = fabs(cp.getDistance());

        contacts.push_back(p);

        return 0;
    }
    };

    CollideCallback callback(*_body);
    _world->contactTest(_body, callback);

    return std::move(callback.contacts);
}

vector<BulletObject::CollisionPoint> BulletObject::collideWith(btCollisionObject* obj) const
{
    struct CollideCallback : public btCollisionWorld::ContactResultCallback
    {

    CollideCallback(btRigidBody& target)
        : btCollisionWorld::ContactResultCallback(), body(target) { }

    btRigidBody& body;
    vector<CollisionPoint> contacts;

    btScalar addSingleResult(btManifoldPoint& cp,
                             const btCollisionObjectWrapper* colObj0,int /*partId0*/,int /*index0*/,
                             const btCollisionObjectWrapper* colObj1,int /*partId1*/,int /*index1*/)
    {
        btVector3 pt, norm;
        CollisionPoint p;
        if(colObj0->m_collisionObject==&body)
        {
            pt = cp.m_localPointA;
            norm = cp.m_normalWorldOnB;
            p.with = const_cast<btCollisionObject*>(colObj1->m_collisionObject);
        }
        else
        {
            pt = cp.m_localPointB;
            norm = -cp.m_normalWorldOnB;
            p.with = const_cast<btCollisionObject*>(colObj0->m_collisionObject);
        }

        p.pos = {pt.x(), pt.y(), pt.z()};
        p.normal = {norm.x(), norm.y(), norm.z()};
        p.depth = fabs(cp.getDistance());

        contacts.push_back(p);

        return 0;
    }
    };

    CollideCallback callback(*_body);
    _world->contactPairTest(_body, obj, callback);

    return std::move(callback.contacts);
}

vector<BulletObject::CollisionPoint> BulletObject::continuousCollide(const mat4& dest) const
{
    struct ContactCallback : public btCollisionWorld::ConvexResultCallback
    {
        btRigidBody& myself;
        vector<CollisionPoint> points;

        ContactCallback(btRigidBody& me) : btCollisionWorld::ConvexResultCallback(), myself(me) {}
        bool needsCollision	(btBroadphaseProxy* proxy) const
        {
            if(proxy->m_clientObject == &myself) return false;
            if(!btCollisionWorld::ConvexResultCallback::needsCollision(proxy))
                return false;

            return myself.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
        }

        btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool /*normalInWorldSpace*/)
        {
            CollisionPoint point;
            point.normal = vec3(convexResult.m_hitNormalLocal.x(), convexResult.m_hitNormalLocal.y(), convexResult.m_hitNormalLocal.z());
            point.depth = convexResult.m_hitFraction;
            points.push_back(point);
            return 0;
        }
    };

    ContactCallback callback(*_body);
    _world->convexSweepTest(reinterpret_cast<btConvexShape*>(_body->getCollisionShape()),
                            _body->getWorldTransform(), toBtTransform(dest), callback, 0);
    return std::move(callback.points);
}

bool BulletObject::firstCollision(const mat4& dest, CollisionPoint& output) const
{
    struct ContactCallback : public btCollisionWorld::ClosestConvexResultCallback
    {
        btRigidBody& myself;
        bool hasHit=false;
        CollisionPoint point;

        ContactCallback(btRigidBody& me, const vec3& to)
         : btCollisionWorld::ClosestConvexResultCallback(me.getWorldTransform().getOrigin(), btVector3(to[0], to[1], to[2])), myself(me) {}

        bool needsCollision	(btBroadphaseProxy* proxy) const
        {
            if(proxy->m_clientObject == &myself) return false;
            if(!btCollisionWorld::ClosestConvexResultCallback::needsCollision(proxy))
                return false;

            return myself.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
        }

        btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool /*normalInWorldSpace*/)
        {
            point.normal = vec3(convexResult.m_hitNormalLocal.x(), convexResult.m_hitNormalLocal.y(), convexResult.m_hitNormalLocal.z());
            point.depth = convexResult.m_hitFraction;
            hasHit=true;
            point.with = const_cast<btCollisionObject*>(convexResult.m_hitCollisionObject);
            return 0;
        }
    };

    ContactCallback callback(*_body, dest.translation());
    _world->convexSweepTest(reinterpret_cast<btConvexShape*>(_body->getCollisionShape()),
                            _body->getWorldTransform(), toBtTransform(dest), callback, 0);
    output = callback.point;

    return callback.hasHit;
}

bool BulletObject::rayCastNotMe(const vec3& from, const vec3& to, CollisionPoint& point) const
{
    class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        ClosestNotMe (btRigidBody* me, const vec3& from, const vec3& to)
            : btCollisionWorld::ClosestRayResultCallback(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2]))
        { _me = me; }

        virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
        {
            if (rayResult.m_collisionObject == _me)
                return 1.0;

            return ClosestRayResultCallback::addSingleResult (rayResult, normalInWorldSpace);
        }

    protected:
        btRigidBody* _me;
    };

    ClosestNotMe rayCallback(_body, from, to);
    _world->rayTest(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2]), rayCallback);

    point.depth = rayCallback.m_closestHitFraction;
    point.normal = vec3(rayCallback.m_hitNormalWorld.m_floats[0],
                        rayCallback.m_hitNormalWorld.m_floats[1],
                        rayCallback.m_hitNormalWorld.m_floats[2]);

    return rayCallback.m_closestHitFraction < 1;
}

bool BulletObject::rayCast(const vec3& from, const vec3& to, CollisionPoint& point) const
{
    class Closest : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        Closest (btRigidBody* me, const vec3& from, const vec3& to)
            : btCollisionWorld::ClosestRayResultCallback(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2]))
        { _me = me; }

        virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
        {
            if (rayResult.m_collisionObject == _me)
                return ClosestRayResultCallback::addSingleResult (rayResult, normalInWorldSpace);

            return 1;
        }

    protected:
        btRigidBody* _me;
    };

    Closest rayCallback(_body, from, to);
    _world->rayTest(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2]), rayCallback);

    point.depth = rayCallback.m_closestHitFraction;
    point.normal = vec3(rayCallback.m_hitNormalWorld.m_floats[0],
                        rayCallback.m_hitNormalWorld.m_floats[1],
                        rayCallback.m_hitNormalWorld.m_floats[2]);
    point.pos = vec3(rayCallback.m_hitPointWorld.m_floats[0],
                     rayCallback.m_hitPointWorld.m_floats[1],
                     rayCallback.m_hitPointWorld.m_floats[2]);

    return rayCallback.m_closestHitFraction < 1;
}

bool BulletObject::rayCastFirst(const vec3& from, const vec3& to, CollisionPoint& point, btDiscreteDynamicsWorld& world)
{
    class Closest : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        Closest (const vec3& from, const vec3& to)
            : btCollisionWorld::ClosestRayResultCallback(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2])){}

        virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
        {
            return ClosestRayResultCallback::addSingleResult (rayResult, normalInWorldSpace);
        }
    };

    Closest rayCallback(from, to);
    world.rayTest(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2]), rayCallback);

    point.depth = rayCallback.m_closestHitFraction;
    point.normal = vec3(rayCallback.m_hitNormalWorld.m_floats[0],
                        rayCallback.m_hitNormalWorld.m_floats[1],
                        rayCallback.m_hitNormalWorld.m_floats[2]);
    point.pos = vec3(rayCallback.m_hitPointWorld.m_floats[0],
                     rayCallback.m_hitPointWorld.m_floats[1],
                     rayCallback.m_hitPointWorld.m_floats[2]);

    return rayCallback.m_closestHitFraction < 1;
}

}
