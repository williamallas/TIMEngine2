#ifndef ASSETVIEWWIDGET_H
#define ASSETVIEWWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QDir>
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
        QString name;
        QList<Material> materials;
    };

    void addElement(const Element&);
    bool getElement(QString name, Element*) const;

    void exportMesh(QString file, QString relativeSource);
    static void writeMaterial(const Material&, QTextStream&, QDir& destDir, QString prefix = "\t");

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

    void mouseMoveEvent(QMouseEvent *event) override;

};

#endif // ASSETVIEWWIDGET_H
