#ifndef RENDERERTHREAD_H
#define RENDERERTHREAD_H

#include "core/core.h"
#include "renderer/renderer.h"
#include "resource/TextureLoader.h"
#include "MainRenderer.h"

#include <QThread>
#include <QGLContext>
#include <QMutex>

class EditorWindow;

class RendererThread : public QThread {
    Q_OBJECT
public:
    explicit RendererThread(RendererWidget* editor);
    bool isInitialized() const;

    tim::MainRenderer* mainRenderer() { return _main; }

protected:
    virtual void run();

private:
    RendererWidget* _renderer;
    QGLWidget* _contextCreator;
    bool _init;

    tim::MainRenderer* _main = nullptr;

    void initContext();

};

#endif // RENDERERTHREAD_H
