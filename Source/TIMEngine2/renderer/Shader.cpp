#include "Shader.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

Shader::Shader()
{

}

Shader::~Shader()
{
    openGL.unbindShader(_id);
    glDeleteProgram(_id);
}

std::string Shader::_lastLinkError = "";

Option<Shader*> Shader::combine(const Option<uint>& vs, const Option<uint>& fs, const Option<uint>& gs)
{
    if(!vs.hasValue() || !fs.hasValue())
    {
        _lastLinkError.clear();
        if(!vs.hasValue())
            _lastLinkError += "Vertex shader missing.\n";
        if(!fs.hasValue())
            _lastLinkError += "Fragment shader missing.\n";
        return Option<Shader*>();
    }

    uint id = glCreateProgram();
    glAttachShader(id, vs.value());
    glAttachShader(id, fs.value());

    if(gs.hasValue() && fs.value() != 0)
        glAttachShader(id, gs.value());

    glBindAttribLocation(id, 0, "vertex");
    glBindAttribLocation(id, 1, "normal");
    glBindAttribLocation(id, 2, "texCoord");
    glBindAttribLocation(id, 3, "tangent");
    glBindAttribLocation(id, 4, "drawId");

    glLinkProgram(id);

    int linkStatus = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);

    if(linkStatus != GL_TRUE)
    {
        int logSize;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);

        char* log = new char[logSize];
        glGetProgramInfoLog(id, logSize, &logSize, log);

        _lastLinkError = std::string(log);
        delete[] log;
        glDeleteProgram(id);
        return Option<Shader*>();
    }

    Shader* prog = new Shader;
    prog->_id = id;
    prog->loadEngineUniform();
    return Option<Shader*>(prog);
}

Option<Shader*> Shader::linkVertexShader(const Option<uint>& vs)
{
    if(!vs.hasValue())
    {
        _lastLinkError.clear();
        if(!vs.hasValue())
            _lastLinkError += "Vertex shader missing.\n";
        return Option<Shader*>();
    }

    uint id = glCreateProgram();
    glAttachShader(id, vs.value());

    glBindAttribLocation(id, 0, "vertex");
    glBindAttribLocation(id, 1, "normal");
    glBindAttribLocation(id, 2, "texCoord");
    glBindAttribLocation(id, 3, "tangent");
    glBindAttribLocation(id, 4, "drawId");

    glLinkProgram(id);

    int linkStatus = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);

    if(linkStatus != GL_TRUE)
    {
        int logSize;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);

        char* log = new char[logSize];
        glGetProgramInfoLog(id, logSize, &logSize, log);

        _lastLinkError = std::string(log);
        delete[] log;
        glDeleteProgram(id);
        return Option<Shader*>();
    }

    Shader* prog = new Shader;
    prog->_id = id;
    prog->loadEngineUniform();
    return Option<Shader*>(prog);
}

Option<Shader*> Shader::linkComputeShader(const Option<uint>& cs)
{
    if(!cs.hasValue())
    {
        _lastLinkError = "Compute Shader missing.\n";
        return Option<Shader*>();
    }

    uint id = glCreateProgram();
    glAttachShader(id, cs.value());
    glLinkProgram(id);

    int linkStatus = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);

    if(linkStatus != GL_TRUE)
    {
        int logSize;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);

        char* log = new char[logSize];
        glGetProgramInfoLog(id, logSize, &logSize, log);

        _lastLinkError = std::string(log);
        delete[] log;
        glDeleteProgram(id);
        return Option<Shader*>();
    }

    Shader* prog = new Shader;
    prog->_id = id;
    prog->loadEngineUniform();
    prog->setUniform(vec4(0,0,0,0), prog->_engineUniform[EngineUniform::CLIP_PLAN_0]);
    prog->setUniform(vec4(0,0,0,0), prog->_engineUniform[EngineUniform::CLIP_PLAN_1]);
    prog->setUniform(vec4(0,0,0,0), prog->_engineUniform[EngineUniform::CLIP_PLAN_2]);
    prog->setUniform(vec4(0,0,0,0), prog->_engineUniform[EngineUniform::CLIP_PLAN_3]);
    return prog;
}

const std::string& Shader::lastLinkError()
{
    return _lastLinkError;
}

void Shader::loadEngineUniform()
{
    _engineUniform[EngineUniform::VIEW] = glGetUniformLocation(_id, "view");
    _engineUniform[EngineUniform::PROJ] = glGetUniformLocation(_id, "projection");
    _engineUniform[EngineUniform::PROJVIEW] = glGetUniformLocation(_id, "projView");
    _engineUniform[EngineUniform::INV_VIEW] = glGetUniformLocation(_id, "invView");
    _engineUniform[EngineUniform::INV_PROJ] = glGetUniformLocation(_id, "invProj");
    _engineUniform[EngineUniform::INV_PROJVIEW] = glGetUniformLocation(_id, "invProjView");
    _engineUniform[EngineUniform::INV_PROJVIEW_WORLD_ORIGIN] = glGetUniformLocation(_id, "worldOriginInvProjView");
    _engineUniform[EngineUniform::WORLD_ORIGIN] = glGetUniformLocation(_id, "worldOrigin");

    _engineUniform[EngineUniform::NB_DRAW] = glGetUniformLocation(_id, "nbDraw");
    _engineUniform[EngineUniform::TIME] = glGetUniformLocation(_id, "time");
    _engineUniform[EngineUniform::FRAME_TIME] = glGetUniformLocation(_id, "timeFrame");
    _engineUniform[EngineUniform::CAMERA_WORLD] = glGetUniformLocation(_id, "cameraWorld");
    _engineUniform[EngineUniform::CAMERA_UP] = glGetUniformLocation(_id, "cameraUp");
    _engineUniform[EngineUniform::CAMERA_DIR] = glGetUniformLocation(_id, "cameraDir");

    _engineUniform[EngineUniform::CLIP_PLAN_0] = glGetUniformLocation(_id, "clipPlan0");
    _engineUniform[EngineUniform::CLIP_PLAN_1] = glGetUniformLocation(_id, "clipPlan1");
    _engineUniform[EngineUniform::CLIP_PLAN_2] = glGetUniformLocation(_id, "clipPlan2");
    _engineUniform[EngineUniform::CLIP_PLAN_3] = glGetUniformLocation(_id, "clipPlan3");

    openGL.bindShader(_id);

    uint nbSimple=0;
    for(uint i=0 ; i<MAX_TEXTURE_UNIT ; ++i)
    {
        int tmp = glGetUniformLocation(_id, (std::string("texture")+StringUtils(i).str()).c_str());
        if(tmp >= 0)
        {
            nbSimple = i+1;
            _uniformTextureId[i] = tmp;
            glUniform1i(_uniformTextureId[i], (int)i);
        }
    }

    for(uint i=0 ; i<MAX_TEXTURE_UNIT-nbSimple ; ++i)
    {
        _uniformTextureId[nbSimple+i] = glGetUniformLocation(_id, (std::string("textures[")+StringUtils(i).str()+"]").c_str());
        if(_uniformTextureId[nbSimple+i] >= 0)
            glUniform1i(_uniformTextureId[nbSimple+i], (int)(nbSimple+i));
    }



    bool findOne=false;
    for(uint i=0 ; i<MAX_IMAGE_UNIT ; ++i)
    {
        _uniformImageId[i] = glGetUniformLocation(_id, (std::string("image")+StringUtils(i).str()).c_str());
        findOne |= (_uniformImageId[i]>=0);
        if(_uniformImageId[i] >= 0)
            glUniform1i(_uniformImageId[i], (int)i);
    }

    if(!findOne)
    {
        for(uint i=0 ; i<MAX_IMAGE_UNIT ; ++i)
        {
            _uniformImageId[i] = glGetUniformLocation(_id, (std::string("image[")+StringUtils(i).str()+"]").c_str());
            if(_uniformImageId[i] >= 0)
                glUniform1i(_uniformImageId[i], (int)i);
        }
    }
}

}
}
