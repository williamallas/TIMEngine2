#ifndef FullScreenDock_H
#define FullScreenDock_H

#include <QDockWidget>

class FullScreenDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit FullScreenDock(QWidget* parent = 0);

protected:
    void keyPressEvent(QKeyEvent* event) override;

signals:

public slots:
    void switchFullScreen();

};

#endif // FullScreenDock_H
