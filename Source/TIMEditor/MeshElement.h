#ifndef MESHELEMENT_H
#define MESHELEMENT_H

#include <QString>
#include <QIcon>
#include <QColor>
#include "core/Vector.h"

using namespace tim;

struct MeshElement
{
    static const int NB_TEXTURES = 3;
    QString geometry;
    QColor color = QColor(255,255,255);
    core::vec4 material = {0.5, 0, 0.2, 0};
    QString textures[NB_TEXTURES];
    QIcon texturesIcon[NB_TEXTURES];
};

#endif // MESHELEMENT_H
