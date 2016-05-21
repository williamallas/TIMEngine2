#ifndef ASSETVIEWWIDGET_H
#define ASSETVIEWWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include "MeshEditorWidget.h"

class AssetViewWidget : public QListWidget
{
    using Material = MeshEditorWidget::Element;
public:
    AssetViewWidget(QWidget* parent);

    void setMeshEditorWidget(MeshEditorWidget* w) { _meshEditor = w; }

    struct Element
    {
        enum { MATERIAL, MESH };
        int type;
        QString name;

        QList<Material> materials;
    };

    void addElement(const Element&);

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

//    void dropEvent(QDropEvent*) override;
//    void dragMoveEvent(QDragMoveEvent*) override;
//    void dragEnterEvent(QDragEnterEvent*) override;

};

#endif // ASSETVIEWWIDGET_H
