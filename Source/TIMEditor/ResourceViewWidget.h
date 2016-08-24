#ifndef RESOURCEVIEWWIDGET_H
#define RESOURCEVIEWWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>

class ResourceViewWidget : public QListWidget
{
    Q_OBJECT
public:
    ResourceViewWidget(QWidget* parent);
    virtual ~ResourceViewWidget() {}

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

    QList<QString> selectResources(int type, QWidget*, bool singleSelection, bool& emptySelected);

protected slots:
    void showContextMenu(const QPoint& pos);

protected:
    struct ItemElement
    {
        Element elem;
        QIcon icon;
        QListWidgetItem* item;
    };
    QList<ItemElement> _items;
    QIcon _objIcon, _timIcon;

    QIcon getIcon(Element) const;

    void dropEvent(QDropEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    bool isTexture(QString) const;
    bool isGeometry(QString) const;

    void onGeometryRightClicked(const ItemElement&, const QPoint&);

};

#endif // RESOURCEVIEWWIDGET_H
