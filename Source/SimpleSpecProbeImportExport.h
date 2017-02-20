#ifndef SIMPLESPECPROBEIMPORTEXPORT
#define SIMPLESPECPROBEIMPORTEXPORT

#include "core/Vector.h"
#include "interface/LightInstance.h"
#include <tinyxml.h>

struct LightProbeUtils
{
    tim::interface::LightInstance* light = nullptr;
    vec3 pos;
    float radius;
    std::string filename;

    int nbIterations = 1;
    float farDist = 1000;


    /** Static import/export methods */

    static vector<LightProbeUtils> importProbe(std::string filename)
    {
        vector<LightProbeUtils> lightsProbe;

        TiXmlDocument doc(filename);

        if(!doc.LoadFile())
            return {};

        TiXmlElement* elem=doc.FirstChildElement();

        while(elem)
        {
            if(elem->ValueStr() == std::string("LightProbe"))
            {
                LightProbeUtils l;
                l.filename = StringUtils::str(elem->Attribute("data"));
                elem->QueryFloatAttribute("radius", &(l.radius));
                elem->QueryFloatAttribute("farDist", &(l.farDist));
                elem->QueryIntAttribute("nbIterations", &(l.nbIterations));
                l.pos = toVec<3>(StringUtils::str(elem->Attribute("pos")));
                lightsProbe.push_back(l);
            }

            elem=elem->NextSiblingElement();
        }

        return lightsProbe;
    }

    static void exportProbe(std::string filename, const vector<LightProbeUtils>& lights)
    {
        std::ofstream f(filename);

        for(const LightProbeUtils& l : lights)
        {
            f << "<LightProbe data=\"" << l.filename << "\" pos=\"" << l.pos[0] << "," << l.pos[1] << "," << l.pos[2] << "\" radius=" << l.radius <<
                 " farDist=" << l.farDist << " nbIterations=" << l.nbIterations <<  " />\n";
        }
    }

    static tim::renderer::LightContextRenderer::Light genLightProbe(const LightProbeUtils& l)
    {
        tim::renderer::LightContextRenderer::Light liparam;
        liparam.tex = nullptr;

        std::ifstream input(l.filename, std::ios::binary|std::ios::ate );
        if(!input)
        {
            LOG("Unuable to load ", l.filename);
            return liparam;
        }

        std::streamsize size = input.tellg();
        ubyte* buf = new ubyte[size];
        input.seekg(0, std::ios::beg);
        input.read(reinterpret_cast<char*>(buf), size);

        tim::renderer::Texture::GenTexParam param;
        param.linear = true;
        param.trilinear = false;
        liparam.tex = tim::renderer::Texture::genTextureFromRawData(buf, param);

        liparam.position = l.pos;
        liparam.radius = l.radius;
        liparam.type = tim::renderer::LightContextRenderer::Light::SPECULAR_PROB;
        delete[] buf;

        return liparam;
    }
};




#endif // SIMPLESPECPROBEIMPORTEXPORT

