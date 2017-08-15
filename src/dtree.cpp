#include "dtree.h"
#include "input.h"
#include <QString>


QMap<QString, DTree::OPERATOR> DTree::initOpMap() {
    QMap<QString, OPERATOR> map;
    map.insert(" ",DTree::OP_None);
    map.insert("&",DTree::OP_AND);
    map.insert("&&",DTree::OP_AND);
    map.insert("|",DTree::OP_OR);
    map.insert("||",DTree::OP_OR);
    map.insert("=",DTree::OP_Equal);
    map.insert("==",DTree::OP_Equal);
    map.insert(">",DTree::OP_Major);
    map.insert(">=",DTree::OP_MajorEqual);
    map.insert("=>",DTree::OP_MajorEqual);
    map.insert("<",DTree::OP_Minor);
    map.insert("<=",DTree::OP_MinorEqual);
    map.insert("=<",DTree::OP_MinorEqual);
    return map;
}

QMap<QString,DTree::OPERATOR> DTree::OPMAP = initOpMap();

bool DTree::isOperator(QChar *data)
{
    return ( data->unicode() == '<' ||
             data->unicode() == '>' ||
             data->unicode() == '=' ||
             data->unicode() == '|' ||
             data->unicode() == '&'
             );
}

bool DTree::isOperator(QString *data)
{
    return (data->contains(QRegExp("(>|>=|=>|<|<=|=<|=|==)")) ||
                     data->contains(QRegExp("(&|&&|\\||\\|\\|)")));
}

DTree::OPERATOR DTree::getOperator(QString *data)
{
    QRegExp rx;
    QString op;
    rx.setPattern("(>|>=|=>|<|<=|=<|=|==|&|&&|\\||\\|\\|)");
    if (rx.indexIn(*data) != -1)
    {
        op = rx.cap(1);
        return getOpMap(op);
    }
    else
        return OP_None;
}

QString DTree::testCondition(QString value1, OPERATOR cond, QString value2)
{
    int val1 = value1.toInt();
    int val2 = value2.toInt();
    bool rsp;
    switch(cond)
    {
    case DTree::OP_AND:
        rsp = (val1 && val2 );
        break;
    case DTree::OP_OR:
        rsp = (val1 || val2 );
        break;
    case DTree::OP_Equal:
        rsp = (val1 == val2 );
        break;
    case   DTree::OP_Major:
        rsp = (val1 > val2 );
        break;
    case  DTree::OP_MajorEqual:
        rsp = (val1 >= val2 );
        break;
    case DTree::OP_Minor:
        rsp = (val1 < val2 );
        break;
    case DTree::OP_MinorEqual:
        rsp = (val1 <= val2 );
        break;
    case DTree::OP_None:
    default:
        rsp =  0;
    }

    return (rsp)?QString("1"):QString("0");
}

bool DTree::isNumeric(QString *data)
{
    return (data->contains(QRegExp("\\d+")));
}

int DTree::getPrepend(QString op)
{
    if(op.contains(QRegExp("(>|>=|=>|<|<=|=<|=|==)")))
        return 2;
    if(op.contains(QRegExp("(&|&&|\\||\\|\\|)")))
        return 1;
   return 0;
}

bool DTree::compPrepend(QString op1, QString op2)
{
    return (getPrepend(op1) <= getPrepend(op2));
}

void DTree::changeParseState(PSTATE nst,QString *token,QChar *data)
{
    switch(pstate)
    {
    case PSEPOPEN:
        stack.push_front("(");
        break;
    case PSEPCLOSE:
        while(!stack.isEmpty() &&
              (stack.at(0).compare("(") != 0))
        {
            output.push_front(stack.at(0));
            stack.removeFirst();
        }
        stack.removeFirst();
        break;
    case POPERAND:
        if(nst != pstate)
        {
            if(!token->contains(QRegExp("^\\d+$")))
                depencies.append(*token);
            output.push_front(*token);
            token->clear();
            if(nst == POPERATOR)
                token->append(*data);
        }else
            token->append(*data);
        break;
    case POPERATOR:
        if(nst != pstate)
        {
            while(!stack.isEmpty() &&
                  (stack.at(0).compare("(") != 0) &&
                  compPrepend(*token,stack.at(0)))
            {
                output.push_front(stack.at(0));
                stack.removeFirst();
            }
            stack.push_front(*token);
            token->clear();
            if(nst == POPERAND)
                token->append(*data);
        }else
            token->append(*data);
        break;
    case PSPACE:
        if(nst == POPERAND || nst == POPERATOR)
            token->append(*data);
        break;
    default:
        break;
    }
    pstate = nst;

}

void DTree::ParseSY(QString str)
{
    QString token;
    QChar *data = str.data();
    pstate = PSPACE;
    while (!data->isNull())
    {
        if( data->unicode() == '(' )
        {
            changeParseState(PSEPOPEN,&token,data);
            qDebug() << "(" ;
        }else if( data->unicode() == ')' )
        {
            changeParseState(PSEPCLOSE,&token,data);
            qDebug() << ")" ;
        }else if( isOperator(data))
        {
            changeParseState(POPERATOR,&token,data);
            qDebug() << "OP" << data->toAscii();
        }else if( data->unicode() == ' ' )
        {
            changeParseState(PSPACE,&token,data);
            qDebug() << "SPACE";
        }else{
            changeParseState(POPERAND,&token,data);
            qDebug() << "VALUE" << data->toAscii();
        }
        data++;
    }
    if(!token.isEmpty())
        changeParseState(PSPACE,&token,data);

    while(!stack.isEmpty())
    {
        output.push_front(stack.at(0));
        stack.removeFirst();
    }
}

/*
A stack-based algorithm for evaluation of postfix expressions is:
    1.- Get a token (operator or operand) from the input text.
    2.- If the token is an operand, push it onto the top of the stack.
    3.- If the token is an operator, pop the top two operands from the stack,
        perform the operation, and push the result onto the top of the stack.
    4.- Repeat 1...3 until there are no more tokens.
*/
bool DTree::EvalRPN()
{
    QString opnd1,opnd2;
    stack.clear();
    for (int i = output.size(), idx = i-1;i; --i, --idx)
    {
       QString token = output.at(idx).toAscii();
       if(isOperator(&token))
       {
           opnd2 = stack.at(0);
           stack.removeFirst();
           opnd1 = stack.at(0);
           stack.removeFirst();
           stack.push_front(testCondition(opnd1,getOpMap(token),opnd2));
           qDebug() << opnd2 + token + opnd1 << "Operator" << endl;
       }else if(isNumeric(&token)){
           stack.push_front(token);
           qDebug() << token << "Numeric" << endl;
       }else{
           QHash<QString,Input*>::const_iterator i = m_options->find(token);
           while (i != m_options->end() && i.key() == token )
           {
               Input *in = i.value();
               stack.push_front(in->value().toString());
               ++i;
           }
           if(i == m_options->begin())
               stack.push_front(QString("0"));

           qDebug() << token << "Symbolic" << endl;
       }
    }
    return stack.at(0).toInt();
}

DTree::DTree(QString str,QHash<QString, Input *> *options)
{
        m_options = options;
        ParseSY(str);
}
