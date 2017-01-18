
QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = Editor
CONFIG += c++11
CONFIG += console

INCLUDEPATH += .
INCLUDEPATH += TIMEngine2\
#INCLUDEPATH += Game\
INCLUDEPATH += TIM_SDL\
INCLUDEPATH += TIMEngine2\core\
INCLUDEPATH += ../Lib/boost
INCLUDEPATH += ../Lib/glew-1.10.0/include/
INCLUDEPATH += ../Lib/assimp-3.1.1/include/
INCLUDEPATH += ../Lib/SDL2-2.0.3/include/
INCLUDEPATH += ../Lib/bullet/include/
INCLUDEPATH += ../Lib/tinyxml/

LIBS += -L./../Lib/lib/
LIBS += -lboost_thread-mgw47-mt-1_55.dll
LIBS += -lboost_atomic-mgw47-mt-1_55.dll
LIBS += -lboost_system-mgw47-mt-1_55.dll
LIBS += -lboost_timer-mgw47-mt-1_55.dll
LIBS += -lopengl32
LIBS += -lmingw32
LIBS += -lzlib
LIBS += -lassimp.dll
LIBS += -lSDL2main
LIBS += -lSDL2.dll
LIBS += -lSDL2_image.dll
LIBS += -lBulletDynamics
LIBS += -lBulletCollision -lLinearMath


QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_RELEASE += -fomit-frame-pointer
QMAKE_CXXFLAGS_RELEASE += -fexpensive-optimizations
QMAKE_CXXFLAGS_RELEASE += -flto

QMAKE_LFLAGS += -O3
QMAKE_LFLAGS += -flto

DEFINES += GLEW_STATIC GLEW_NO_GLU TIXML_USE_STL USE_SDL NO_OPEN_AL
debug: DEFINES += TIM_DEBUG

# Input
HEADERS += TIM_SDL/*.h TIMEngine2/core/*.h TIMEngine2/renderer/*.h TIMEngine2/resource/*.h TIMEngine2/interface/*.h TIMEngine2/interface/pipeline/*.h TIMEngine2/scene/*.h \
    TIMEditor/*.h \
    DebugCamera.h \
    AssimpLoader.h\
    TIMEngine2/bullet/*.h \
    TIMEditor/EditorWindow.h \
    TIMEditor/RendererThread.h \
    TIMEditor/RendererWidget.h \
    TIMEditor/QtTextureLoader.h \
    TIMEditor/FullPipeline.h \
    TIMEditor/MainRenderer.h \
    TIMEditor/ResourceViewWidget.h \
    TIMEditor/AssetViewWidget.h \
    TIMEditor/FullScreenDock.h \
    TIMEditor/MeshEditorWidget.h \
    TIMEditor/SceneEditorWidget.h \
    TIMEditor/SelectResourcesDialog.h \
    TIMEditor/SelectSkyboxDialog.h \
    MultipleSceneHelper.h \
    TIMEditor/LightComponent.h \
    TIMEditor/ConfigSpecProbe.h \
    SimpleSpecProbeImportExport.h \
    TIMEditor/ObjectInstancingDialog.h
    TIMEditor/MeshElement.h

SOURCES += mainEditor.cpp \
           TIM_SDL/SDLInputManager.cpp \
           TIM_SDL/SDLTextureLoader.cpp \
           ../Lib/tinyxml/tinyxml.cpp \
           ../Lib/tinyxml/tinystr.cpp \
           ../Lib/tinyxml/tinyxmlerror.cpp \
           ../Lib/tinyxml/tinyxmlparser.cpp \
           ../Lib/glew-1.10.0/src/glew.c \
            AssimpLoader.cpp\
    TIMEngine2/renderer/GLState.cpp \
    TIMEngine2/renderer/IndexBuffer.cpp \
    TIMEngine2/renderer/renderer.cpp \
    TIMEngine2/renderer/Shader.cpp \
    TIMEngine2/renderer/ShaderCompiler.cpp \
    TIMEngine2/renderer/Texture.cpp \
    TIMEngine2/renderer/FrameBuffer.cpp \
    TIMEngine2/renderer/MeshRenderer.cpp \
    TIMEngine2/renderer/DeferredRenderer.cpp \
    TIMEngine2/renderer/LightContextRenderer.cpp \
    TIMEngine2/renderer/IndirectLightRenderer.cpp \
    TIMEngine2/renderer/TiledLightRenderer.cpp \
    TIMEngine2/renderer/PostReflexionRenderer.cpp \
    TIMEngine2/core/StringUtils.cpp \
    TIMEngine2/core/Box.cpp \
    TIMEngine2/core/core.cpp \
    TIMEngine2/core/MemoryLogger.cpp \
    TIMEngine2/core/OrientedBox.cpp \
    TIMEngine2/core/Rand.cpp \
    TIMEngine2/core/Sphere.cpp \
    TIMEngine2/core/Frustum.cpp \
    TIMEngine2/resource/MeshLoader.cpp \
    TIMEngine2/resource/TextureLoader.cpp \
    TIMEngine2/resource/Image.cpp \
    TIMEngine2/interface/Mesh.cpp \
    TIMEngine2/interface/MeshInstance.cpp \
    TIMEngine2/interface/ShaderPool.cpp \
    TIMEngine2/interface/FullPipeline.cpp \
    TIMEngine2/interface/XmlMeshAssetLoader.cpp \
    TIMEngine2/interface/Pipeline.cpp \
    TIMEngine2/interface/pipeline/DeferredRendererNode.cpp \
    TIMEngine2/interface/pipeline/OnScreenRenderer.cpp \
    TIMEngine2/interface/pipeline/InBufferRenderer.cpp \
    TIMEngine2/interface/pipeline/BloomNode.cpp \
    TIMEngine2/interface/pipeline/DirLightShadowNode.cpp \
    TIMEngine2/interface/pipeline/SimpleFilter.cpp \
    TIMEngine2/bullet/BulletEngine.cpp \
    TIMEngine2/bullet/BulletObject.cpp \
    TIMEngine2/bullet/GeometryShape.cpp \
    TIMEditor/EditorWindow.cpp \
    TIMEditor/RendererThread.cpp \
    TIMEditor/RendererWidget.cpp \
    TIMEditor/QtTextureLoader.cpp \
    TIMEditor/MainRenderer.cpp \
    TIMEditor/ResourceViewWidget.cpp \
    TIMEditor/AssetViewWidget.cpp \
    TIMEditor/FullScreenDock.cpp \
    TIMEditor/MeshEditorWidget.cpp \
    TIMEditor/SceneEditorWidget.cpp \
    TIMEditor/SelectResourcesDialog.cpp \
    TIMEditor/SelectSkyboxDialog.cpp \
    TIMEngine2/renderer/PooledBuffer.cpp \
    TIMEngine2/renderer/TextureBufferPool.cpp \
    MultipleSceneHelper.cpp \
    TIMEditor/LightComponent.cpp \
    TIMEditor/ConfigSpecProbe.cpp \
    TIMEditor/ObjectInstancingDialog.cpp

FORMS += \
    TIMEditor/EditorWindow.ui \
    TIMEditor/MeshEditor.ui \
    TIMEditor/SceneEditor.ui \
    TIMEditor/SelectResourcesDialog.ui \
    TIMEditor/SelectSkyboxDialog.ui \
    TIMEditor/lightcomponent.ui \
    TIMEditor/ConfigSpecProbe.ui \
    TIMEditor/ObjectInstancingDialog.ui

RESOURCES += \
    ../EditorResources.qrc
