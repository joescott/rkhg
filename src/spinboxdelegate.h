#ifndef SPINBOXDELEGATE_H
#define SPINBOXDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>

class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent,int min,int max,int dec = 0,QString suff ="");

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
           const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    int m_min;
    int m_max;
    int m_dec;
    QString m_suff;
};
#endif
