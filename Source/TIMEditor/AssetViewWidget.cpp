#include "AssetViewWidget.h"
#include "core/StringUtils.h"
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>

AssetViewWidget::AssetViewWidget(QWidget* parent) : QListWidget(parent), _meshIcon(":/icons/Icons/mesh.png")
{
}

void AssetViewWidget::addElement(const Element& e)
{
    for(ItemElement& elem : _items)
    {
        if(elem.elem.name == e.name)
        {
            elem.elem = e;
            return;
        }
    }

    _items += {e, getIcon(e)};
    auto ol = new QListWidgetItem(getIcon(e), e.name, this);
    ol->setSizeHint(QSize(100,100));
}

QIcon AssetViewWidget::getIcon(const Element& elem) const
{
    if(elem.type == Element::MESH)
        return _meshIcon;
    else return _materialIcon;
}

void AssetViewWidget::exportMesh(QString filePath, QString relativeSource)
{
    QFile file(filePath);
    QDir destDir(relativeSource);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        for(ItemElement& e : _items)
        {
            if(e.elem.type == Element::MESH)
            {
                stream << "<MeshAsset name=\"" << e.elem.name << "\">\n";

                for(Material& m : e.elem.materials)
                {
                    stream << "\t<Element type=0>\n";
                    stream << "\t\t<color>" << m.color.red() << "," << m.color.green() << "," << m.color.blue() << "</color>\n";
                    stream << "\t\t<geometry>" << destDir.relativeFilePath(m.geometry) <<"</geometry>\n";
                    stream << "\t\t<roughness>" << m.material[0] <<"</roughness>\n";
                    stream << "\t\t<metallic>" << m.material[1] <<"</metallic>\n";
                    stream << "\t\t<specular>" << m.material[2] <<"</specular>\n";
                    stream << "\t\t<emissive>" << m.material[3] <<"</emissive>\n";
                    if(!m.textures[0].isEmpty())
                        stream << "\t\t<diffuseTex>" << destDir.relativeFilePath(m.textures[0]) <<"</diffuseTex>\n";
                    if(!m.textures[1].isEmpty())
                        stream << "\t\t<normalTex>" << destDir.relativeFilePath(m.textures[1]) <<"</normalTex>\n";
                    if(!m.textures[2].isEmpty())
                        stream << "\t\t<materialTex>" << destDir.relativeFilePath(m.textures[2]) <<"</materialTex>\n";
                    stream << "\t</Element>\n";
                }

                stream << "</MeshAsset>\n\n";
            }
        }
    }
}

//void ResourceViewWidget::dropEvent(QDropEvent* event)
//{
//    const QMimeData* mimeData = event->mimeData();
//    event->acceptProposedAction();
//    event->accept();

//   if(mimeData->hasUrls())
//   {
//       QList<QUrl> urlList = event->mimeData()->urls();
//       for(QUrl& url : urlList)
//       {
//           addFile(url.toLocalFile());
//       }
//   }
//}

//void ResourceViewWidget::dragEnterEvent(QDragEnterEvent* event)
//{
//    if(event->mimeData()->hasUrls())
//    {
//        QList<QUrl> urlList = event->mimeData()->urls();

//        for(QUrl& url : urlList)
//        {
//            if(isTexture(url.toLocalFile()) || isGeometry(url.toLocalFile()))
//            {
//                event->acceptProposedAction();
//                break;
//            }
//        }
//    }
//}

//void ResourceViewWidget::dragMoveEvent(QDragMoveEvent* event)
//{
//    event->accept();
//}


