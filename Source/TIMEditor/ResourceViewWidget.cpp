#include "ResourceViewWidget.h"
#include "SelectResourcesDialog.h"
#include <iostream>
#include <QMessageBox>
#include <QHeaderView>
#include <QResizeEvent>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include "core/StringUtils.h"

ResourceViewWidget::ResourceViewWidget(QWidget* parent) : QListWidget(parent), _objIcon(":/icons/Icons/objIcon.png"), _timIcon(":/icons/Icons/timIcon")
{
}

void ResourceViewWidget::addElement(Element e)
{
    if(!QFileInfo(e.path).exists())
        return;

    for(auto elem : _items)
    {
        if(elem.elem.path == e.path)
            return;
    }

    QString name="";

    if(e.type == Element::Geometry)
        name = QFileInfo(e.path).baseName();

    QIcon ic = getIcon(e);
    QListWidgetItem* ol = new QListWidgetItem(ic, name, this);
    ol->setSizeHint(QSize(100,100));

    _items += {e, ic, ol};
}

QList<QString> ResourceViewWidget::selectResources(int type, QWidget* parent, bool singleSelection)
{
    SelectResourcesDialog dialog(parent);
    if(singleSelection)
        dialog.singleSelection();

    for(auto elem : _items)
    {
        if(elem.elem.type == type)
        {
            QListWidgetItem* item = new QListWidgetItem(elem.icon, QFileInfo(elem.elem.path).baseName(), dialog.listElement());
            item->setSizeHint(QSize(100,100));
            dialog.registerItem(item, elem.elem.path);
        }
    }
    dialog.exec();
    return dialog.selectedItems();
}

void ResourceViewWidget::addDir(QString dirStr, bool rec)
{
    QDirIterator it(dirStr, rec ? QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags);
    while (it.hasNext())
        addFile(it.next());
}

void ResourceViewWidget::addFile(QString file)
{
    if(isTexture(file))
        addElement({file, Element::Texture});
    else if(isGeometry(file))
        addElement({file, Element::Geometry});
}

QIcon ResourceViewWidget::getResourceIconForPath(QString path)
{
    for (const auto& item : _items) {
        if (item.elem.path == path) {
            return item.icon;
        }
    }
    return QIcon();
}

bool ResourceViewWidget::isTexture(QString file) const
{
    if(file.endsWith(".jpg",Qt::CaseInsensitive) ||
       file.endsWith(".png",Qt::CaseInsensitive) ||
       file.endsWith(".bmp",Qt::CaseInsensitive) ||
       file.endsWith(".gif",Qt::CaseInsensitive))
        return true;
    else return false;
}

bool ResourceViewWidget::isGeometry(QString file) const
{
    if(file.endsWith(".obj",Qt::CaseInsensitive) ||
       file.endsWith(".tim",Qt::CaseInsensitive))
        return true;
    else return false;
}

QIcon ResourceViewWidget::getIcon(Element e) const
{
    switch(e.type)
    {
    case Element::Geometry:
    {
        std::string ext = tim::core::StringUtils(e.path.toStdString()).extension();
        if(ext == "obj")
            return _objIcon;
        else if(ext == "tim")
            return _timIcon;
        else return QIcon();
    }
    case Element::Texture:
        return QIcon(e.path);
    }
    return QIcon();
}

void ResourceViewWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    event->acceptProposedAction();
    event->accept();

   if(mimeData->hasUrls())
   {
       QList<QUrl> urlList = event->mimeData()->urls();
       for(QUrl& url : urlList)
       {
           addFile(url.toLocalFile());
       }
   }
}

void ResourceViewWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urlList = event->mimeData()->urls();

        for(QUrl& url : urlList)
        {
            if(isTexture(url.toLocalFile()) || isGeometry(url.toLocalFile()))
            {
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void ResourceViewWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

void ResourceViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if (currentItem() == NULL)
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> list;
    for(auto elem : _items)
    {
        if(elem.item == currentItem())
        {
            list.append(QUrl(elem.elem.path));
            list.append(QUrl(currentItem()->text()));

            if(elem.elem.type == Element::Geometry)
                mimeData->setObjectName("ResourceViewWidget::Geometry");
            else
                mimeData->setObjectName("ResourceViewWidget::Texture");
            break;
        }
    }

    mimeData->setUrls(list);

    drag->setMimeData(mimeData);
    drag->start(Qt::CopyAction | Qt::MoveAction);
}


