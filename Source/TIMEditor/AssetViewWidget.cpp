#include "AssetViewWidget.h"
#include "core/StringUtils.h"
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDrag>
#include <QMimeData>

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

bool AssetViewWidget::getElement(QString name, Element* elemptr) const
{
    for(const ItemElement& elem : _items)
    {
        if(elem.elem.name == name)
        {
            *elemptr = elem.elem;
            return true;
        }
    }
    return false;
}

QIcon AssetViewWidget::getIcon(const Element&) const
{
    return _meshIcon;
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
            stream << "<MeshAsset name=\"" << e.elem.name << "\">\n";

            for(Material& m : e.elem.materials)
            {
                writeMaterial(m, stream, destDir);
            }

            stream << "</MeshAsset>\n\n";
        }
    }
}

void AssetViewWidget::writeMaterial(const Material& m, QTextStream& stream, QDir& destDir, QString prefix)
{
    stream << prefix << "<Element type=0>\n";
    stream << prefix << "\t<color>" << m.color.red() << "," << m.color.green() << "," << m.color.blue() << "</color>\n";
    stream << prefix << "\t<geometry>" << destDir.relativeFilePath(m.geometry) <<"</geometry>\n";
    stream << prefix << "\t<roughness>" << m.material[0] <<"</roughness>\n";
    stream << prefix << "\t<metallic>" << m.material[1] <<"</metallic>\n";
    stream << prefix << "\t<specular>" << m.material[2] <<"</specular>\n";
    stream << prefix << "\t<emissive>" << m.material[3] <<"</emissive>\n";
    if(!m.textures[0].isEmpty())
        stream << prefix << "\t<diffuseTex>" << destDir.relativeFilePath(m.textures[0]) <<"</diffuseTex>\n";
    if(!m.textures[1].isEmpty())
        stream << prefix << "\t<normalTex>" << destDir.relativeFilePath(m.textures[1]) <<"</normalTex>\n";
    if(!m.textures[2].isEmpty())
        stream << prefix << "\t<materialTex>" << destDir.relativeFilePath(m.textures[2]) <<"</materialTex>\n";
    stream << prefix << "</Element>\n";
}

void AssetViewWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    for(ItemElement& elem : _items)
    {
        if(elem.elem.name == item->text())
        {
            QList<Material> meshEditorElem;
            for(Material m : elem.elem.materials)
            {
                Material subMesh;
                subMesh.color = m.color;
                subMesh.geometry = m.geometry;
                subMesh.material = m.material;

                for(int i=0 ; i<MeshElement::NB_TEXTURES ; ++i)
                {
                    subMesh.textures[i] = m.textures[i];
                    subMesh.texturesIcon[i] = m.texturesIcon[i];
                }

                meshEditorElem.push_back(subMesh);
            }
            _meshEditor->setMesh(elem.elem.name, meshEditorElem);
            return;
        }
    }
}

void AssetViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if (currentItem() == NULL)
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> list;
    list.append(QUrl(currentItem()->text()));
    mimeData->setUrls(list);
    mimeData->setObjectName("AssetViewWidget");
    drag->setMimeData(mimeData);

    drag->start(Qt::CopyAction | Qt::MoveAction);
}

