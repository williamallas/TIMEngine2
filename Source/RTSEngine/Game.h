#ifndef GAME_H
#define GAME_H

#include "Graphic/TerrainRenderer.h"
#include "Graphic/RTSCamera.h"
#include "interface/Pipeline.h"
#include "bullet/BulletEngine.h"

class Game
{
    using Scene = tim::interface::Pipeline::SceneEntity<tim::interface::SimpleScene>;
public:
    Game();

    //static bool rayCast(tim::BulletObject*, const tim::core::Camera&, vec2 mousePos, tim::BulletObject::CollisionPoint&);
protected:


    // Graphic
    //tim::BulletEngine _physic;

    tim::interface::Pipeline::SceneView _camera;
    RTSCamera _cameraController;

    Scene _scene;
    TerrainRenderer* _terrain;
};

#endif // GAME_H
