#ifndef LIGHTCOMPONENT_H
#define LIGHTCOMPONENT_H

#include <QWidget>

namespace Ui {
class LightComponent;
}

class LightComponent : public QWidget
{
    Q_OBJECT

public:
    explicit LightComponent(QWidget *parent = 0);
    ~LightComponent();

private:
    Ui::LightComponent *ui;
};

#endif // LIGHTCOMPONENT_H
