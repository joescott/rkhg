/******************************************************************************
 *
 * 
 *
 * Copyright (C) 1997-2012 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 */

#ifndef _INPUTINT_H
#define _INPUTINT_H

#include "input.h"
#include "dtree.h"
#include <QObject>

class QGridLayout;
class QLabel;
class QSpinBox;

class InputInt : public QObject, public Input
{
  Q_OBJECT

  public:
    InputInt( QGridLayout *layout,int &row,
              const QString &id, int defVal, 
              int minVal, int maxVal,
              const QString &docs );
    ~InputInt(){}

    // Input
    QVariant &value();
    void update();
    Kind kind() const { return Int; }
    QString docs() const { return m_docs; }
    QString id() const { return m_id; }
    void addDependency(Input *option) {
        m_dependencies+=option;
    }
    void addDependencyCond(Input::depend &cond) {
        m_dependencies_data+=cond;
    }
    void addDependencyDTree(DTree *dt) {
        m_dt += dt;
    }
    void setEnabled(bool);
    void updateDependencies(QVariant);
    void updateDependencies();
    void backToDefault();
    void writeValue(QTextStream &t,QTextCodec *codec);
    int  parseCondition(QString value, QString cond, QVariant val);

  public slots:
    void reset();
    void setValue(int val); 

  private slots:
    void help();

  signals:
    void changed();
    void showHelp(Input *);

  private:
    QLabel   *m_lab;
    QSpinBox *m_sp;
    int       m_val;
    int       m_default;
    int       m_minVal;
    int       m_maxVal;
    bool      m_enabled;
    QVariant  m_value;
    QString   m_docs;
    QString   m_id;
    QList<Input   *>     m_dependencies;
    QList<Input::depend> m_dependencies_data;
    QMap<QString,int> m_qcomp;
    QList<DTree *>m_dt;

};

#endif
