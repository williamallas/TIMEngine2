#ifndef MESHEDITORWIDGET_H
#define MESHEDITORWIDGET_H

#include <QWidget>
#include "MainRenderer.h"
#include <QColor>
#include <QIcon>
#include <QModelindex>

class ResourceViewWidget;

namespace Ui {
class MeshEditor;
}

class MeshEditorWidget : public QWidget
{
    Q_OBJECT

public:

    struct Element
    {
        QString geometry;
        QColor color = QColor(255,255,255);
        vec4 material = {0.5, 0, 0.2, 0};
        QString textures[3];
        QIcon texturesIcon[3];
    };

    MeshEditorWidget(QWidget* parent = nullptr);

    void setMainRenderer(tim::MainRenderer* r);
    void setResourceWidget(ResourceViewWidget* ptr) { _resourceWidget=ptr; }
    void addElement(QString geometry);

    QString currentMeshName() const;
    const QList<Element>& currentMesh() const { return _meshData; }

protected:
    Ui::MeshEditor *ui;
    tim::MainRenderer* _renderer;
    tim::interface::MeshInstance* _editedMesh;
    ResourceViewWidget* _resourceWidget;

    float _rz=0, _ry=0;

    QList<Element> _meshData;
    int _curElementIndex = -1;

    void updateColorButton();
    void updateTexture(int);

public slots:
    void dm_roughnessSlider_sliderMoved(int);
    void dm_metallicSlider_sliderMoved(int);
    void dm_specularSlider_sliderMoved(int);
    void dm_emissiveSlider_sliderMoved(int);

    void dm_roughnessVal_valueChanged(double);
    void dm_metallicVal_valueChanged(double);
    void dm_specularVal_valueChanged(double);
    void dm_emissiveVal_valueChanged(double);

    void dmSelectColor_clicked();
    void itemSelectionChanged(QModelIndex);
    void itemSelectionChanged(int);
    void updateMaterial();

    void selectGeometryFromResources();
    void removeElement();

    void rotateEditedMesh(int, int);
    void resetView();

    void on_diffuseTex_clicked();
    void on_normalTex_clicked();
    void on_materialTex_clicked();

    void on_saveMeshButton_pressed();

signals:
    void saveMeshClicked();
};

#endif // MESHEDITORWIDGET_H
