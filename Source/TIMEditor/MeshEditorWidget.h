#ifndef MESHEDITORWIDGET_H
#define MESHEDITORWIDGET_H

#include <QWidget>
#include "MainRenderer.h"
#include <QColor>

namespace Ui {
class MeshEditor;
}

class MeshEditorWidget : public QWidget
{
    Q_OBJECT

public:
    MeshEditorWidget(QWidget* parent = nullptr);

    void setMainRenderer(tim::MainRenderer* r);
    void addElement(QString geometry);

protected:
    Ui::MeshEditor *ui;
    tim::MainRenderer* _renderer;
    tim::interface::MeshInstance* _editedMesh;

    struct Element
    {
        QString name;
        QString geometry;
        QColor color = QColor(255,255,255);
    };

    QList<Element> _meshData;

     void updateColorButton();

public slots:
    void updateMesh();

    void dm_roughnessSlider_sliderMoved(int);
    void dm_metallicSlider_sliderMoved(int);
    void dm_specularSlider_sliderMoved(int);

    void dm_roughnessVal_valueChanged(double);
    void dm_metallicVal_valueChanged(double);
    void dm_specularVal_valueChanged(double);

    void dmSelectColor_clicked();
};

#endif // MESHEDITORWIDGET_H
