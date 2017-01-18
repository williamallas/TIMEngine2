#ifndef OBJECTINSTANCINGDIALOG_H
#define OBJECTINSTANCINGDIALOG_H

#include <QDialog>
#include "core/Vector.h"

namespace Ui {
class ObjectInstancingDialog;
}

class ObjectInstancingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectInstancingDialog(QWidget *parent = 0);
    ~ObjectInstancingDialog();

    struct ObjectInstancingConfig
    {
        QString name;
        bool prefix = false;
        tim::core::vec3 size = {200,200,200};
        tim::core::ubvec3 randRot = {false,false,false};
        tim::core::vec2 randScale = {1,1};
        int number = 100;
        int seed = 0;
    };

    ObjectInstancingConfig getConfig() const;
    int state() const { return _state;  }

private:
    Ui::ObjectInstancingDialog *ui;
    int _state = -1; // 0->quit 1->generate 2->reset

private slots:
    void on_generate_clicked();
    void on_cancel_clicked();
    void on_remLast_clicked();

};

#endif // OBJECTINSTANCINGDIALOG_H
