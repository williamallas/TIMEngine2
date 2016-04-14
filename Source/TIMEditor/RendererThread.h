#ifndef RENDERERTHREAD_H
#define RENDERERTHREAD_H

#include "core/core.h"
#include "renderer/renderer.h"
#include <QThread>
#include <QGLContext>
#include "RendererWidget.h"

class EditorWindow;

class RendererThread : public QThread {
    Q_OBJECT
public:
    explicit RendererThread(RendererWidget* editor);
    bool isInitialized() const;

protected:
    virtual void run();

private:
    RendererWidget* _renderer;
    QGLWidget* _contextCreator;
    bool _init;

};

#endif // RENDERERTHREAD_H
