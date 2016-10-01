#ifndef COLLISIONMASK_H
#define COLLISIONMASK_H

#define BIT(x) (1<<(x))
enum CollisionTypes {
    COL_NOTHING = 0,
    COL_PADDLE = BIT(0),
    COL_PHYS   = BIT(1),
    COL_STATIC = BIT(2),
    COL_ROOM   = BIT(3),
    COL_IOBJ   = BIT(4)
};

static constexpr int PADDLE_COLLISION = COL_PHYS | COL_IOBJ;
static constexpr int PHYS_COLLISION = COL_PADDLE | COL_PHYS | COL_STATIC | COL_IOBJ;
static constexpr int STATIC_COLLISION = COL_PHYS | COL_IOBJ;
static constexpr int ROOMPATTERN_COLLISION = COL_IOBJ;
static constexpr int IOBJECT_COLLISION = COL_PADDLE | COL_PHYS | COL_STATIC | COL_ROOM | COL_IOBJ;

#endif // COLLISIONMASK_H

