#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H


#include <QDialog>

class SettingsTree;

class SettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent, SettingsTree * tree);

signals:

public slots:

};

#endif // SETTINGDIALOG_H
