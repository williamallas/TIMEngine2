#include "Game.h"

using namespace tim;
using namespace tim::interface;

Game::Game()
{

}

bool Game::rayCast(tim::BulletObject* obj, const Camera& camera, vec2 mousePos, BulletObject::CollisionPoint & collision)
{
    mousePos.y() = 1-mousePos.y();
    mat4 proj = mat4::Projection(camera.fov, camera.ratio, camera.clipDist.x(), camera.clipDist.y());
    mat4 view = mat4::View(camera.pos, camera.dir, camera.up);

    mat4 inv = (proj*view).inverted();
    vec4 screen = vec4(mousePos*2-1, 0.5, 1);
    vec4 wPos = inv*screen;
    wPos /= wPos.w();

    vec3 pos = camera.pos;
    vec3 dir = (vec3(wPos)-camera.pos).normalized();

    bool b = obj->rayCast(pos, pos+dir*camera.clipDist.y(), collision);
    return b;
}
