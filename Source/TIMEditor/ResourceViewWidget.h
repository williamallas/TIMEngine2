#ifndef RESOURCEVIEWWIDGET_H
#define RESOURCEVIEWWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>

class ResourceViewWidget : public QListWidget
{
public:
    ResourceViewWidget(QWidget* parent);

    struct Element
    {
        enum { Texture, Geometry };
        QString path;
        int type;
    };

    void addElement(Element);
    void addDir(QString, bool rec=false);
    void addFile(QString);

    QIcon getResourceIconForPath(QString path);

    QList<QString> selectResources(int type, QWidget*, bool singleSelection);

protected:
    struct ItemElement
    {
        Element elem;
        QIcon icon;
    };
    QList<ItemElement> _items;
    QIcon _objIcon, _timIcon;

    QIcon getIcon(Element) const;

    void dropEvent(QDropEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;

    bool isTexture(QString) const;
    bool isGeometry(QString) const;

};

#endif // RESOURCEVIEWWIDGET_H
