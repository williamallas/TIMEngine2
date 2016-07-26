#ifndef MESHELEMENT_H
#define MESHELEMENT_H

#include <QString>
#include <QIcon>
#include <QColor>
#include "core/Vector.h"
#include "renderer/DrawState.h"
#undef DrawState
using namespace tim;

struct MeshElement
{
    static const int NB_TEXTURES = 3;
    QString geometry;
    QColor color = QColor(255,255,255);
    core::vec4 material = {0.5, 0, 0.1, 0};
    float textureScale = 1;
    QString textures[NB_TEXTURES];
    QIcon texturesIcon[NB_TEXTURES];

    bool useAdvanced = false;
    QString advancedShader;
    renderer::DrawState advanced;

    bool operator<(const MeshElement& elem) const
    {
        if(geometry < elem.geometry) return true;
        else if(geometry > elem.geometry) return false;

        if(color.name() < elem.color.name()) return true;
        else if(color.name() > elem.color.name()) return false;

        if(material < elem.material) return true;
        else if(material > elem.material) return false;

        if(textureScale < elem.textureScale) return true;
        else if(textureScale > elem.textureScale) return false;

        if(useAdvanced < elem.useAdvanced) return true;
        else if(useAdvanced > elem.useAdvanced) return false;

        if(useAdvanced && elem.useAdvanced)
        {
            if(advanced < elem.advanced) return true;
            else if(elem.advanced < advanced) return false;

            if(advancedShader < elem.advancedShader) return true;
            else if(advancedShader > elem.advancedShader) return false;
        }

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
               textureScale == elem.textureScale &&
               material == elem.material && ct &&
               useAdvanced == elem.useAdvanced &&
               (!useAdvanced || (advanced == elem.advanced && advancedShader == elem.advancedShader));
    }
};

#endif // MESHELEMENT_H
