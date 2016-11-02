#ifndef CONFIGSPECPROBE_H
#define CONFIGSPECPROBE_H

#include <QDialog>

namespace Ui {
class ConfigSpecProbe;
}

class ConfigSpecProbe : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigSpecProbe(QWidget *parent = 0);
    ~ConfigSpecProbe();

    int nbIterations() const { return _nbIterations; }
    QString pathRawData() const { return _pathRawData; }
    QString pathSkybox() const { return _pathSkybox; }
    int resolution() const { return _resolution; }
    bool addToScene() const { return _addToScene; }
    bool centerOnSlection() const { return _centerOnSelection; }
    float radius() const { return _radius; }
    float farDist() const { return _farDist; }
    bool exportAsRawData() const { return _exportAsRawData; }
    bool exportAsSkybox() const { return _exportAsSkybox; }

    bool isRenderClicked() const { return _renderClicked; }

private:
    Ui::ConfigSpecProbe *ui;

    int _nbIterations;
    QString _pathRawData, _pathSkybox;
    int _resolution;
    bool _addToScene;
    bool _centerOnSelection;
    float _radius, _farDist;
    bool _exportAsRawData, _exportAsSkybox;
    bool _renderClicked = false;

private slots:
    void on_render_clicked();
};

#endif // CONFIGSPECPROBE_H
