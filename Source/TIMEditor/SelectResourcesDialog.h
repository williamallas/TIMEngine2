#ifndef SELECTRESOURCESDIALOG_H
#define SELECTRESOURCESDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class SelectResourcesDialog;
}

class QListWidget;
class QListWidgetItem;

class SelectResourcesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectResourcesDialog(QWidget *parent = 0);
    ~SelectResourcesDialog();

    void singleSelection();

    QListWidget* listElement() const;
    QList<QString> selectedItems() const;

    void registerItem(QListWidgetItem* item, QString path) { _itemToPath[item]=path; }

protected slots:
    void confirmSelection();
    void itemDoubleClicked(QListWidgetItem * item);

private:
    Ui::SelectResourcesDialog *ui;
    QList<QString> _selectedItems;

    QMap<QListWidgetItem*, QString> _itemToPath;

};

#endif // SELECTRESOURCESDIALOG_H
