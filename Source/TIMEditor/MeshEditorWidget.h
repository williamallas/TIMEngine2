#ifndef MESHEDITORWIDGET_H
#define MESHEDITORWIDGET_H

#include <QWidget>
#include "MainRenderer.h"
#include <QColor>
#include <QModelindex>

class ResourceViewWidget;

namespace Ui {
class MeshEditor;
}

class MeshEditorWidget : public QWidget
{
    Q_OBJECT

public:
    MeshEditorWidget(QWidget* parent = nullptr);

    void setMainRenderer(tim::MainRenderer* r);
    void setResourceWidget(ResourceViewWidget* ptr) { _resourceWidget=ptr; }
    void addElement(QString geometry);

protected:
    Ui::MeshEditor *ui;
    tim::MainRenderer* _renderer;
    tim::interface::MeshInstance* _editedMesh;
    ResourceViewWidget* _resourceWidget;

    struct Element
    {
        QString name;
        QString geometry;
        QColor color = QColor(255,255,255);
        vec3 material = {0.5, 0, 0.2};
    };

    QList<Element> _meshData;
    int _curElementIndex = -1;

    void updateColorButton();

public slots:
    void dm_roughnessSlider_sliderMoved(int);
    void dm_metallicSlider_sliderMoved(int);
    void dm_specularSlider_sliderMoved(int);

    void dm_roughnessVal_valueChanged(double);
    void dm_metallicVal_valueChanged(double);
    void dm_specularVal_valueChanged(double);

    void dmSelectColor_clicked();
    void itemSelectionChanged(QModelIndex);
    void itemSelectionChanged(int);
    void updateMaterial();

    void selectGeometryFromResources();
    void removeElement();
};

#endif // MESHEDITORWIDGET_H
