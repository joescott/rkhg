#include "settingdialog.h"
#include "settingstree.h"
#include <QtGui>

SettingDialog::SettingDialog(QWidget *parent, SettingsTree *tree) :
    QDialog(parent)
{
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    tree->setFallbacksEnabled(true);
    setWindowTitle(tr("Configuration"));
}
