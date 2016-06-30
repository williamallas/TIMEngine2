#ifndef ASSETVIEWWIDGET_H
#define ASSETVIEWWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include "MeshEditorWidget.h"

class AssetViewWidget : public QListWidget
{
    Q_OBJECT
public:
    using Material = MeshElement;

    AssetViewWidget(QWidget* parent);
    ~AssetViewWidget() = default;

    void setMeshEditorWidget(MeshEditorWidget* w) { _meshEditor = w; }

    struct Element
    {
        enum { MATERIAL, MESH };
        int type;
        QString name;

        QList<Material> materials;
    };

    void addElement(const Element&);
    bool getElement(QString name, Element*) const;

    void exportMesh(QString file, QString relativeSource);

protected:
    MeshEditorWidget* _meshEditor = nullptr;
    struct ItemElement
    {
        Element elem;
        QIcon icon;
    };
    QList<ItemElement> _items;
    QIcon _meshIcon, _materialIcon;

    QIcon getIcon(const Element&) const;

protected slots:
    void onItemDoubleClicked(QListWidgetItem*);
    void confirmAssetDrop(QString, QDragEnterEvent*);

    void mouseMoveEvent(QMouseEvent *event) override;

};

#endif // ASSETVIEWWIDGET_H
