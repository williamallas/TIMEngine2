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

    bool operator<(const MeshElement& elem) const
    {
        if(geometry < elem.geometry) return true;
        else if(geometry > elem.geometry) return false;

        if(color.name() < elem.color.name()) return true;
        else if(color.name() > elem.color.name()) return false;

        if(material < elem.material) return true;
        else if(material > elem.material) return false;

        for(int i=0 ; i<NB_TEXTURES ; ++i)
        {
            if(textures[i] < elem.textures[i]) return true;
            else if(textures[i] > elem.textures[i]) return false;
        }

        return false;
    }

    bool operator==(const MeshElement& elem) const
    {
        bool ct = true;
        for(int i=0 ; i<NB_TEXTURES ; ++i)
            ct = ct && (textures[i] == elem.textures[i]);

        return geometry == elem.geometry &&
               color == elem.color &&
               material == elem.material && ct;
    }
};

#endif // MESHELEMENT_H
