#ifndef MESHEDITORWIDGET_H
#define MESHEDITORWIDGET_H

#include <QWidget>
#include "MainRenderer.h"
#include "MeshElement.h"
#include "interface/XmlMeshAssetLoader.h"

#include <QColor>
#include <QModelindex>
#include <functional>

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
    void addElement(MeshElement);

    QString currentMeshName() const;
    const QList<MeshElement>& currentMesh() const { return *_editedMaterials; }

    void setMesh(QString, const QList<MeshElement>&);
    void setEditedMesh(tim::interface::MeshInstance*, tim::interface::MeshInstance*, QList<MeshElement>*, QString);
    void activeEditMode();

    static tim::interface::Mesh::Element constructMeshElement(const MeshElement&);
    static tim::interface::Mesh highlightMesh(const tim::interface::Mesh&);

    QList<MeshElement> convertFromEngine(const vector<interface::XmlMeshAssetLoader::MeshElementModel>&);

protected:
    Ui::MeshEditor *ui;
    tim::MainRenderer* _renderer;

    ResourceViewWidget* _resourceWidget;

    tim::interface::MeshInstance* _editedMesh = nullptr;
    tim::interface::MeshInstance* _highlightedMesh = nullptr;
    QList<MeshElement>* _editedMaterials = nullptr;
    int _curElementIndex = -1;

    /* Mesh editor scene part */
    tim::interface::MeshInstance* _meshEditorNode;
    QList<MeshElement> _meshEditorMaterials;
    float _rz=0, _ry=0;

    void updateColorButton();
    void updateTexture(int);

    void addUiElement(const MeshElement&);
    void setUi(const MeshElement&);

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
