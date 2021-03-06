#ifndef INPUT_H
#define INPUT_H

#include <QVariant>


class DTree;
class QTextStream;
class QTextCodec;

class Input
{
  public:
    enum Kind
    {
      Bool,
      Int,
      String,
      StrList,
      Obsolete
    };
    enum Comp
    {
        QMajor,
        QMajorEqual,
        QMinor,
        QMinorEqual,
        QEqual
    };

    typedef struct {
        QString key_value;
        QString key_cond;
        QString key;
        QString cond;
        QString value;
    }depend;

    virtual ~Input() {}
    virtual QVariant &value() = 0;
    virtual void update() = 0;
    virtual Kind kind() const = 0;
    virtual QString docs() const = 0;
    virtual QString id() const = 0;
    virtual void addDependency(Input *option) = 0;
    virtual void addDependencyDTree(DTree *dt) = 0;
    virtual void setEnabled(bool) = 0;
    virtual void backToDefault() = 0;
    virtual void updateDependencies() = 0;
    virtual void updateDependencies(QVariant) = 0;
    virtual void reset() = 0;
    virtual void writeValue(QTextStream &t,QTextCodec *codec) = 0;

};


#endif
