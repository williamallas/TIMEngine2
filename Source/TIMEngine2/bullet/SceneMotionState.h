#ifndef SCENEMOTIONSTATE_H_INCLUDED
#define SCENEMOTIONSTATE_H_INCLUDED

#include "core.h"
#include "Matrix.h"
#include <btBulletDynamicsCommon.h>

namespace tim
{
using namespace core;

    inline static btTransform toBtTransform(const mat4& m)
    {
        btTransform worldTrans;
        worldTrans.setBasis(btMatrix3x3(m[0][0],m[0][1],m[0][2],
                                        m[1][0],m[1][1],m[1][2],
                                        m[2][0],m[2][1],m[2][2]));

        worldTrans.setOrigin(btVector3(m[0].w(), m[1].w(), m[2].w()));
        return worldTrans;
    }

    inline static mat4 fromBtTransform(const btTransform& worldTrans)
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

    template<class T>
    class SceneMotionState : public btMotionState
    {
    public:
        SceneMotionState() : _sceneObject(nullptr) {}
        SceneMotionState(T& obj, float scale=1) : _sceneObject(&obj), _scale(scale) {}
        ~SceneMotionState() = default;

        void setSceneObject(T& obj) { _sceneObject = &obj; }

        void getWorldTransform(btTransform &worldTrans) const override
        {
            if(_sceneObject == nullptr)
            {
                worldTrans = btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0));
                return;
            }

            mat4 m = _sceneObject->matrix();
            worldTrans.setBasis(btMatrix3x3(m[0][0] / _scale,m[0][1],m[0][2],
                                            m[1][0],m[1][1] / _scale,m[1][2],
                                            m[2][0],m[2][1],m[2][2] / _scale));

            worldTrans.setOrigin(btVector3(m[0].w(), m[1].w(), m[2].w()));
        }

        void setWorldTransform(const btTransform &worldTrans) override
        {
            if(_sceneObject == nullptr)
                return;

            _sceneObject->setMatrix(fromBtTransform(worldTrans) * mat4::Scale(vec3::construct(_scale)));
        }

    private:
        T* _sceneObject;
        float _scale = 1;

    };
}

#endif // SCENEMOTIONSTATE_H_INCLUDED
