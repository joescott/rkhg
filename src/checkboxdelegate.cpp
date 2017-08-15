#include <QtGui>

#include "checkboxdelegate.h"

#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>

static QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options) {
  QStyleOptionButton check_box_style_option;
  QRect check_box_rect = QApplication::style()->subElementRect(
      QStyle::SE_CheckBoxIndicator,
      &check_box_style_option);
  QPoint check_box_point(view_item_style_options.rect.x() +
                         view_item_style_options.rect.width() / 2 -
                         check_box_rect.width() / 2,
                         view_item_style_options.rect.y() +
                         view_item_style_options.rect.height() / 2 -
                         check_box_rect.height() / 2);
  return QRect(check_box_point, check_box_rect.size());
}

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
  : QStyledItemDelegate(parent) {
}

void CheckBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

  QStyleOptionButton check_box_style_option;
  check_box_style_option.state |= QStyle::State_Enabled;
  if (checked) {
    check_box_style_option.state |= QStyle::State_On;
  } else {
    check_box_style_option.state |= QStyle::State_Off;
  }
  check_box_style_option.rect = CheckBoxRect(option);

  QApplication::style()->drawControl(QStyle::CE_CheckBox,
                                     &check_box_style_option,
                                     painter);
}

bool CheckBoxDelegate::editorEvent(QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) {
  if ((event->type() == QEvent::MouseButtonRelease) ||
      (event->type() == QEvent::MouseButtonDblClick)) {
    QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
    if (mouse_event->button() != Qt::LeftButton ||
        !CheckBoxRect(option).contains(mouse_event->pos())) {
      return false;
    }
    if (event->type() == QEvent::MouseButtonDblClick) {
      return true;
    }
  } else if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
        static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
      return false;
    }
  } else {
    return false;
  }

  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
  return model->setData(index, !checked, Qt::EditRole);
}

