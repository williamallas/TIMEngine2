#include "SelectResourcesDialog.h"
#include "ResourceViewWidget.h"
#include "ui_SelectResourcesDialog.h"

SelectResourcesDialog::SelectResourcesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectResourcesDialog)
{
    ui->setupUi(this);
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
}

SelectResourcesDialog::~SelectResourcesDialog()
{
    delete ui;
}

QListWidget* SelectResourcesDialog::listElement() const
{
    return ui->listWidget;
}

QList<QString> SelectResourcesDialog::selectedItems() const
{
    return _selectedItems;
}

void SelectResourcesDialog::confirmSelection()
{
    QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
    for(QListWidgetItem* item : list)
    {
        _selectedItems.push_back(_itemToPath[item]);
    }
    close();
}

void SelectResourcesDialog::singleSelection()
{
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void SelectResourcesDialog::itemDoubleClicked(QListWidgetItem * item)
{
    _selectedItems.push_back(_itemToPath[item]);
    close();
}
