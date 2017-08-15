#include <QtGui>

#include "ipboxdelegate.h"


IpBoxDelegate::IpBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *IpBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent)   ;
    QRegExpValidator* ValIPAddr;
    QString Byte = "(?!0[0-9])(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp rxip;
    rxip.setPattern("^" + Byte + "\\." + Byte + "\\." + Byte + "\\." +
    Byte + "$");
    ValIPAddr = new QRegExpValidator(rxip, 0);
    editor->setValidator(ValIPAddr);

    /*
     * Input format
    editor->setInputMask("000.000.000.000;_");
     */
    editor->setCursorPosition(0);
    editor->setMinimumWidth(100);
    return editor;
}

void IpBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *ipBox = static_cast<QLineEdit*>(editor);
    ipBox->setText(value);
}

void IpBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    int pos = 0;
    QLineEdit *ipBox = static_cast<QLineEdit*>(editor);
    QString value = ipBox->text();
    if(ipBox->validator()->validate(value,pos) == QValidator::Acceptable)
        model->setData(index, value, Qt::EditRole);
    else
        QMessageBox::critical(editor, tr("Invalid IP value"),
                           tr("Invalid IP value"),tr("Accept"),0);

}

void IpBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

