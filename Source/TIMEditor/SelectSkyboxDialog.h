#ifndef SELECTSKYBOXDIALOG_H
#define SELECTSKYBOXDIALOG_H

#include <QDialog>
#include "ResourceViewWidget.h"

namespace Ui {
class SelectSkyboxDialog;
}

class SelectSkyboxDialog : public QDialog
{
    Q_OBJECT

public:

    enum SKY_DIR_INDEX {X = 0, NX = 1, Y = 2, NY = 3, Z = 4, NZ = 5};

    explicit SelectSkyboxDialog(QWidget* parent, ResourceViewWidget* resourceWidget);
    ~SelectSkyboxDialog();

    QList<QString> getSkydirPaths();

private slots:

    void on_xButton_clicked();

    void on_yButton_clicked();

    void on_zButton_clicked();

    void on_nxButton_clicked();

    void on_nyButton_clicked();

    void on_nzButton_clicked();

    void on_okButton_clicked();

private:

    void selectFileFor(QPushButton* button, int pathIndex);

    QString _skydirpaths[6];
    QList<QString> _returnPaths;

    ResourceViewWidget* _resourceWidget;
    Ui::SelectSkyboxDialog *ui;
};

#endif // SELECTSKYBOXDIALOG_H
