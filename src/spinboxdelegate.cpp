#include <QtGui>

#include "spinboxdelegate.h"


SpinBoxDelegate::SpinBoxDelegate(QObject *parent,
    int min,int max,int dec,QString suff)
    : QItemDelegate(parent), m_min(min),m_max(max),m_dec(dec),m_suff(suff)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //QSpinBox *editor = new QSpinBox(parent);
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setMinimum(m_min);
    editor->setMaximum(m_max);
    editor->setDecimals(m_dec);
    editor->setSuffix(m_suff);
    if(m_dec != 0)
    {
        double step = 1/(10*(double)m_dec);
        editor->setSingleStep(step);
    }

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::UserRole).toDouble();

    //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    QVariant val;

    model->setData(index, spinBox->textFromValue(value), Qt::DisplayRole);
    if(m_dec)
        val = value;
    else
        val = (int) value;

    model->setData(index, val, Qt::UserRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
