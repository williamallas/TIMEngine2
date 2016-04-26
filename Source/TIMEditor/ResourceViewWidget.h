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
    void dragEnterEvent(QDragEnterEvent*) override;

};

#endif // RESOURCEVIEWWIDGET_H
