#include "QtTextureLoader.h"
#include <QImage>
#include <QGLWidget>

#include "MemoryLoggerOn.h"
namespace tim
{

ubyte* QtTextureLoader::loadImage(const std::string& file, ImageFormat& format) const
{
    QImage textureImg(file.c_str());

    if(textureImg.isNull())
        return nullptr;

    textureImg = QGLWidget::convertToGLFormat(textureImg);
    ubyte* b = new ubyte[4*textureImg.byteCount()];
    memcpy(b, textureImg.bits(), 4*textureImg.byteCount());
    format.size.x() = textureImg.size().width();
    format.size.y() = textureImg.size().height();
    format.nbComponent = 4;
    return b;
}

}
#include "MemoryLoggerOff.h"
