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

#include "inputint.h"
#include "helplabel.h"

#include <QtGui>

InputInt::InputInt( QGridLayout *layout,int &row,
                    const QString & id, 
                    int defVal, int minVal,int maxVal,
                    const QString & docs )
  : m_default(defVal), m_minVal(minVal), m_maxVal(maxVal), m_docs(docs), m_id(id)
{
  m_lab = new HelpLabel(id);
  m_sp  = new QSpinBox;
  m_sp->setMinimum(minVal);
  m_sp->setMaximum(maxVal);
  m_sp->setSingleStep(1);
  m_val=defVal-1; // force update
  setValue(defVal);


  m_qcomp[">"]  = QMajor;
  m_qcomp[">="] = QMajorEqual;
  m_qcomp["=>"] = QMajorEqual;
  m_qcomp["<"]  = QMinor;
  m_qcomp["<="] = QMinorEqual;
  m_qcomp["=<"] = QMinorEqual;
  m_qcomp["="]  = QEqual;

  layout->addWidget( m_lab, row, 0 );
  layout->addWidget( m_sp, row, 1 );

  connect(m_sp, SIGNAL(valueChanged(int)), 
          this, SLOT(setValue(int)) );
  connect( m_lab, SIGNAL(enter()), SLOT(help()) );
  connect( m_lab, SIGNAL(reset()), SLOT(reset()) );
  row++;

}

void InputInt::updateDependencies()
{
  for (int i=0;i<m_dependencies.count();i++)
  {
    m_dependencies[i]->setEnabled(m_enabled);
  }
}

int InputInt::parseCondition(QString value, QString cond, QVariant val)
{
    switch(m_qcomp.value(cond.toAscii()))
    {
    case QMajor:
        if(val.toInt() > value.toInt())
            return 1;
        break;
    case QMajorEqual:
        if(val.toInt() >= value.toInt())
            return 1;
        break;
    case QMinor:
        if(val.toInt() <  value.toInt())
            return 1;
        break;
    case QMinorEqual:
        if(val.toInt() <= value.toInt())
            return 1;
        break;
    case QEqual:
    default:
        if(value.toInt() == val.toInt())
            return 1;
    }
    return 0;
}

void InputInt::updateDependencies(QVariant val)
{   

    for (int i=0;i<m_dt.count();i++)
    {
        m_enabled = m_dt[i]->EvalRPN();
        if(!m_enabled)
            m_dependencies[i]->backToDefault();
        m_dependencies[i]->setEnabled(m_enabled);
    }
}

void InputInt::backToDefault()
{
    setValue(m_default);
}

void InputInt::help()
{
  showHelp(this);
}


void InputInt::setValue(int val)
{
  val = qMax(m_minVal,val);
  val = qMin(m_maxVal,val);
  if (val!=m_val) 
  {
    m_val = val;
    m_sp->setValue(val);
    m_value = m_val;
    if (m_val==m_default)
    {
      m_lab->setText(QString::fromAscii("<qt>")+m_id+QString::fromAscii("</qt"));
    }
    else
    {
      m_lab->setText(QString::fromAscii("<qt><font color='red'>")+m_id+QString::fromAscii("</font></qt>"));
    }
    updateDependencies(QVariant(val));
    emit changed();
  }
}

void InputInt::setEnabled(bool state)
{
  m_lab->setEnabled(state);
  m_sp->setEnabled(state);
}

QVariant &InputInt::value() 
{
  return m_value;
}

void InputInt::update()
{
  setValue(m_value.toInt());
}

void InputInt::reset()
{
  setValue(m_default);
}

void InputInt::writeValue(QTextStream &t,QTextCodec *)
{
  t << m_val;
}

