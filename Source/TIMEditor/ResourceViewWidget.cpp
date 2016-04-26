#include "ResourceViewWidget.h"
#include <iostream>
#include <QMessageBox>
#include <QHeaderView>
#include <QResizeEvent>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDropEvent>
#include <QMimeData>
#include "core/StringUtils.h"

ResourceViewWidget::ResourceViewWidget(QWidget* parent) : QListWidget(parent), _objIcon("objIcon.png"), _timIcon("timIcon")
{
    setAcceptDrops(true);
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

    _items += {e, getIcon(e)};
    auto ol = new QListWidgetItem(_items.back().icon, name, this);
    ol->setSizeHint(QSize(100,100));
}

void ResourceViewWidget::addDir(QString dirStr, bool rec)
{
    QDirIterator it(dirStr, rec ? QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString fileName = it.next();

        if(fileName.endsWith(".jpg",Qt::CaseInsensitive) ||
           fileName.endsWith(".png",Qt::CaseInsensitive) ||
           fileName.endsWith(".bmp",Qt::CaseInsensitive) ||
           fileName.endsWith(".gif",Qt::CaseInsensitive))
            addElement({fileName, Element::Texture});
        else if(fileName.endsWith(".obj",Qt::CaseInsensitive) ||
                fileName.endsWith(".tim",Qt::CaseInsensitive))
            addElement({fileName, Element::Geometry});
    }
}

void ResourceViewWidget::addFile(QString file)
{
    if(file.endsWith(".jpg",Qt::CaseInsensitive) ||
       file.endsWith(".png",Qt::CaseInsensitive) ||
       file.endsWith(".bmp",Qt::CaseInsensitive) ||
       file.endsWith(".gif",Qt::CaseInsensitive))
        addElement({file, Element::Texture});
    else if(file.endsWith(".obj",Qt::CaseInsensitive) ||
            file.endsWith(".tim",Qt::CaseInsensitive))
        addElement({file, Element::Geometry});
    else QMessageBox::warning(this, "File extension not recognized", file + " is not a valid resource file.");

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

   if (mimeData->hasUrls())
   {
     QStringList pathList;
     QList<QUrl> urlList = mimeData->urls();

     // extract the local paths of the files
     for (int i = 0; i < urlList.size() && i < 32; ++i)
     {
         std::cout << urlList[i].toString().toStdString() << std::endl;
       //pathList.append(urlList.at(i).toLocalFile());
     }

     // call a function to open the files
     //openFiles(pathList);
   }
}

void ResourceViewWidget::dragEnterEvent(QDragEnterEvent* event)
{
    std::cout <<"Enter drag" << std::endl;
    if(event->mimeData()->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = event->mimeData()->urls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size() && i < 32; ++i)
        {
            std::cout << urlList[i].toString().toStdString() << std::endl;
          //pathList.append(urlList.at(i).toLocalFile());
        }


    }
}


