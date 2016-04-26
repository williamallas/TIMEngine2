#include "FullScreenDock.h"
#include <QKeyEvent>
#include <iostream>

FullScreenDock::FullScreenDock(QWidget *parent) : QDockWidget(parent) {}

void FullScreenDock::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_F11)
    {
        event->accept();

        if(!this->isFullScreen())
            this->showFullScreen();
        else
            this->showNormal();
    }
    else
    {
        event->ignore();
        QDockWidget::keyPressEvent(event);
    }
}
