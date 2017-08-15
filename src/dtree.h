#ifndef DTREE_H
#define DTREE_H

#include "input.h"
#include <QList>
#include <QVariant>
#include <QtCore>

class DTree
{
public:
    enum PSTATE{
        POPERAND,
        POPERATOR,
        PSEPOPEN,
        PSEPCLOSE,
        PSPACE
    };
    enum OPERATOR{
        OP_None,
        OP_Major,
        OP_MajorEqual,
        OP_Minor,
        OP_MinorEqual,
        OP_Equal,
        OP_AND,
        OP_OR
    };

    QStringList stack;
    QStringList output;
    QStringList depencies;
    PSTATE pstate;

    DTree(QString str,QHash<QString,Input*> *options);
    bool isOperator(QChar   *data);
    bool isOperator(QString *data);
    OPERATOR getOperator(QString *data);
    bool isNumeric(QString  *data);
    void ParseSY(QString str);
    bool EvalRPN();

    int getPrepend(QString op);
    bool compPrepend(QString op1,QString op2);   
    QString testCondition(QString value1, OPERATOR cond, QString value2);

private:
    static QMap<QString, OPERATOR> initOpMap();

    static OPERATOR getOpMap(QString str)
    {
           OPMAP = initOpMap();
           if(OPMAP.contains(str))
               return OPMAP.value(str);
           else
               return OP_None;
    }
    void changeParseState(PSTATE nst,QString *token,QChar *data);
    static QMap<QString, OPERATOR> OPMAP;
    QHash<QString, Input*> *m_options;

};


#endif // DTREE_H


