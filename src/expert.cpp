#include "expert.h"
#include "inputbool.h"
#include "inputstring.h"
#include "inputint.h"
#include "inputstring.h"
#include "inputstrlist.h"
#include <QtGui>
#include <QtXml>
#include "version.h"

#undef  SA
#define SA(x) QString::fromAscii(x)
#define COMMENT_MAX_LONG 75
#define __DISABLE_NOPRINT_DEPEND__

static QString convertToComment(const QString &s)
{
  if (s.isEmpty()) 
  {
    return QString();
  }
  else
  {
#if 0
    QString rs;
    QStringList sl;
    rs.append(SA("/**\n"));
    sl.append(s.split("\n"));
    foreach (QString str, sl) {
        rs.append(SA("* "));
        rs.append(str.simplified());
        rs.append(SA("\n"));
    }
    rs.append(SA("*/\n"));
    return rs;
#else
      QString rs;
      QStringList sl;
      int cnt;
      rs.append(SA("/**\n"));
      sl.append(s.simplified().split(QRegExp("\\s+")));
      while(!sl.isEmpty())
      {
        QString rsl;
        rsl.append(SA("* "));
        cnt -= rsl.count();
        for(cnt=COMMENT_MAX_LONG;cnt > 0;)
        {
            if(sl.isEmpty())
                break;
            QString str = sl.first();
            str.append(" ");
            rsl.append(str);
            sl.removeAt(0);
            cnt -= str.count();
        }
        rsl.append(SA("\n"));
        rs.append(rsl);
      }

      rs.append(SA("*/\n"));
      return rs;

#endif
  }
}




//------------------------------------------------------------------------------------

Expert::Expert()
{
  m_treeWidget = new QTreeWidget;
  m_treeWidget->setColumnCount(1);
  m_topicStack = new QStackedWidget;
  m_inShowHelp = FALSE;

  m_helper = new QTextEdit;
  m_helper->setObjectName("Help");
  m_helper->setReadOnly(true);
  m_helper->verticalScrollBar()->hide();
  m_splitter = new QSplitter(Qt::Vertical);
  m_splitter->addWidget(m_treeWidget);
  m_splitter->addWidget(m_helper);

  m_rightSide = new QWidget;
  QGridLayout *grid = new QGridLayout(m_rightSide);
  m_prev = new QPushButton(QIcon(QString::fromAscii(":back")),tr("Previous"));
  m_prev->setEnabled(false);
  m_next = new QPushButton(QIcon(QString::fromAscii(":next")),tr("Next"));
  grid->addWidget(m_topicStack,0,0,1,2);
  grid->addWidget(m_prev,1,0,Qt::AlignLeft);
  grid->addWidget(m_next,1,1,Qt::AlignRight);
  grid->setColumnStretch(0,1);
  grid->setRowStretch(0,1);

  m_splitter->setHidden(true);
  m_rightSide->setHidden(true);
  addWidget(m_splitter);
  addWidget(m_rightSide);
  connect(m_next,SIGNAL(clicked()),SLOT(nextTopic()));
  connect(m_prev,SIGNAL(clicked()),SLOT(prevTopic()));
  setMinimumSize(600,400);
}

Expert::~Expert()
{
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext()) 
  {
    i.next();
    delete i.value();
  }
}

void Expert::createTopics(const QDomElement &rootElem)
{
  QList<QTreeWidgetItem*> items;
  QDomElement childElem = rootElem.firstChildElement();
  m_treeWidget->clear();

  while (!childElem.isNull())
  {
    if (childElem.tagName()==SA("group"))
    {
      QString name = childElem.attribute(SA("name"));
      items.append(new QTreeWidgetItem((QTreeWidget*)0,QStringList(name)));
      QWidget *widget = createTopicWidget(childElem);
      m_topics[name] = widget;
      m_topicStack->addWidget(widget);
    }
    childElem = childElem.nextSiblingElement();
  }
  m_treeWidget->setHeaderLabels(QStringList() << SA("Topics"));
  m_treeWidget->insertTopLevelItems(0,items);
  connect(m_treeWidget,
          SIGNAL(currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)),
          this,
          SLOT(activateTopic(QTreeWidgetItem *,QTreeWidgetItem *)));
}

void Expert::loadXmlFile(QString xmlFile)
{
    QFile file(xmlFile);
    QString err;
    int errLine,errCol;
    xmlDoc.clear();

    if (file.open(QIODevice::ReadOnly))
    {
      if (!xmlDoc.setContent(&file,false,&err,&errLine,&errCol))
      {
        QString msg = tr("Error parsing internal config.xml at line %1 column %2.\n%3").
                    arg(errLine).arg(errCol).arg(err);
        QMessageBox::warning(this, tr("Error"), msg);
        exit(1);
      }
    }
    m_rootElement = xmlDoc.documentElement();

    m_rightSide->setHidden(false);
    m_splitter->setHidden(false);

    createTopics(m_rootElement);

}


void Expert::saveXmlFile(QString xmlFile)
{
    QFile file(xmlFile);
    if (file.open(QIODevice::WriteOnly))
    {
     file.write(xmlDoc.toString().toUtf8());
    }

}




QWidget *Expert::createTopicWidget(QDomElement &elem)
{
  QScrollArea *area   = new QScrollArea;
  QWidget     *topic  = new QWidget;
  QGridLayout *layout = new QGridLayout(topic);
  QDomElement child   = elem.firstChildElement();
  int row=0;
  while (!child.isNull())
  {
    QString type = child.attribute(SA("type"));
    if (type==SA("bool"))
    {
      InputBool *boolOption = 
          new InputBool(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval"))==SA("1"),
            child.attribute(SA("docs"))
           );
      m_options.insert(
          child.attribute(SA("id")),
          boolOption
         );
      connect(boolOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(boolOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("string"))
    {
      InputString::StringMode mode;
      QString format = child.attribute(SA("format"));
      if (format==SA("dir"))
      {
        mode = InputString::StringDir;
      }
      else if (format==SA("file"))
      {
        mode = InputString::StringFile;
      }
      else // format=="string"
      {
        mode = InputString::StringFree;
      }
      InputString *stringOption = 
          new InputString(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")),
            mode,
            child.attribute(SA("docs")),
            child.attribute(SA("abspath"))
           );
      m_options.insert(
          child.attribute(SA("id")),
          stringOption
         );
      connect(stringOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(stringOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("enum"))
    {
      InputString *enumList = new InputString(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")),
            InputString::StringFixed,
            child.attribute(SA("docs"))
           );
      QDomElement enumVal = child.firstChildElement();
      while (!enumVal.isNull())
      {
        enumList->addValue(enumVal.attribute(SA("name")));
        enumVal = enumVal.nextSiblingElement();
      }
      enumList->setDefault();

      m_options.insert(child.attribute(SA("id")),enumList);
      connect(enumList,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(enumList,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("int"))
    {
      InputInt *intOption = 
          new InputInt(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")).toInt(),
            child.attribute(SA("minval")).toInt(),
            child.attribute(SA("maxval")).toInt(),
            child.attribute(SA("docs"))
          );
      m_options.insert(
          child.attribute(SA("id")),
          intOption
        );
      connect(intOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(intOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("list"))
    {
      InputStrList::ListMode mode;
      QString format = child.attribute(SA("format"));
      if (format==SA("dir"))
      {
        mode = InputStrList::ListDir;
      }
      else if (format==SA("file"))
      {
        mode = InputStrList::ListFile;
      }
      else if (format==SA("filedir"))
      {
        mode = InputStrList::ListFileDir;
      }
      else // format=="string"
      {
        mode = InputStrList::ListString;
      }
      QStringList sl;
      QDomElement listVal = child.firstChildElement();
      while (!listVal.isNull())
      {
        sl.append(listVal.attribute(SA("name")));
        listVal = listVal.nextSiblingElement();
      }
      InputStrList *listOption = 
          new InputStrList(
            layout,row,
            child.attribute(SA("id")),
            sl,
            mode,
            child.attribute(SA("docs"))
          );
      m_options.insert(
          child.attribute(SA("id")),
          listOption
        );
      connect(listOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(listOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("obsolete"))
    {
      // ignore
    }
    else // should not happen
    {
      printf("Unsupported type %s\n",qPrintable(child.attribute(SA("type"))));
    }
    child = child.nextSiblingElement();
  }

  // compute dependencies between options
  child = elem.firstChildElement();
  while (!child.isNull())
  {
    QString dependsOn = child.attribute(SA("depends"));
    QString id        = child.attribute(SA("id"));
    Input *parentOption;
    if (dependsOn.isEmpty())
    {
        child = child.nextSiblingElement();
        continue;
    }

    DTree *dt = new DTree(dependsOn,&m_options);
    Input *thisOption   = m_options[id];
    foreach(QString po, dt->depencies)
    {
        parentOption = m_options[po];
        if (parentOption && thisOption)
        {
            parentOption->addDependencyDTree(dt);
            parentOption->addDependency(thisOption);
        }
    }

    child = child.nextSiblingElement();
  }

  // set initial dependencies
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext()) 
  {
    i.next();
    if (i.value())
    {
      i.value()->updateDependencies(i.value()->value());
    }
  }

  layout->setRowStretch(row,1);
  layout->setColumnStretch(1,2);
  layout->setSpacing(5);
  topic->setLayout(layout);
  area->setWidget(topic);
  area->setWidgetResizable(true);
  return area;
}

void Expert::activateTopic(QTreeWidgetItem *item,QTreeWidgetItem *)
{
  if (item)
  {
    QWidget *w = m_topics[item->text(0)];
    m_topicStack->setCurrentWidget(w);
    m_prev->setEnabled(m_topicStack->currentIndex()!=0); 
    m_next->setEnabled(m_topicStack->currentIndex()!=m_topicStack->count()-1); 
  }
}

void Expert::loadSettings(QSettings *s)
{
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext()) 
  {
    i.next();
    QVariant var = s->value(SA("config/")+i.key());
    if (i.value())
    {
      //printf("Loading key %s: type=%d value='%s'\n",qPrintable(i.key()),var.type(),qPrintable(var.toString()));
      i.value()->value() = var;
      i.value()->update();
    }
  }
}
#if 0
void Expert::saveSettings(QSettings *s)
{
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext()) 
  {
    i.next();
    //printf("Saving key %s: type=%d value='%s'\n",qPrintable(i.key()),i.value()->value().type(),qPrintable(i.value()->value().toString()));
    if (i.value())
    {
      s->setValue(SA("config/")+i.key(),i.value()->value());
    }
  }
}
#endif
void Expert::saveTopic(QTextStream &t,QDomElement &elem,QTextCodec *codec,
                       bool brief)
{
  // write group header

  t << endl;
  t << convertToComment(elem.attribute(SA("docs")));
  t << endl;

  // write options...
  QDomElement childElem = elem.firstChildElement();

  while (!childElem.isNull())
  {
    QString type = childElem.attribute(SA("type"));
    QString name = childElem.attribute(SA("id"));

#ifndef __DISABLE_NOPRINT_DEPEND__

    QString did = childElem.attribute(SA("depends"));
    QHash<QString,Input*>::const_iterator k = m_options.find(did);
    if (k !=m_options.end())
    {
        Input *dop = k.value();
        if(dop->value() == 0 || dop->value() == false)
        {
            t << endl;
            t << "/* " + name +" Depends on "+ did + " */";
            t << endl;
            childElem = childElem.nextSiblingElement();
            continue;
        }
    }
#endif

    QHash<QString,Input*>::const_iterator i = m_options.find(name);
    if (i!=m_options.end())
    {

      Input *option = i.value();
      if (!brief)
      {
        t << endl;
        t << convertToComment(childElem.attribute(SA("docs")));
        t << endl;
      }
      t << "#define ";
      t << name.leftJustified(23) << " ";
      if (option)
      {
        option->writeValue(t,codec);
      }
      t << endl;
    }
    childElem = childElem.nextSiblingElement();
  }

}

bool Expert::writeConfig(QString fileName, QTextStream &t,bool brief)
{
  if (!brief)
  {
          // write global header
          t << "#ifndef __" << fileName.toUpper().toAscii() << "_H__\n";
          t << "#define __" << fileName.toUpper().toAscii() << "_H__\n";
  }

  QTextCodec *codec = 0;
#if 0
  Input *option = m_options[QString::fromAscii("DOXYFILE_ENCODING")];
  if (option)
  {
    codec = QTextCodec::codecForName(option->value().toString().toAscii());
    if (codec==0) // fallback: use UTF-8
    {
      codec = QTextCodec::codecForName("UTF-8");
    }
  }
#endif
  codec = QTextCodec::codecForName("UTF-8");
  QDomElement childElem = m_rootElement.firstChildElement();
  while (!childElem.isNull())
  {
    saveTopic(t,childElem,codec,brief);
    childElem = childElem.nextSiblingElement();
  }
  if (!brief)
      t << "#endif\n";
  return true;
}

#if 0
QByteArray Expert::saveInnerState () const
{
  return m_splitter->saveState();
}

bool Expert::restoreInnerState ( const QByteArray & state )
{
  return m_splitter->restoreState(state);
}
#endif

void Expert::showHelp(Input *option)
{
  if (!m_inShowHelp)
  {
    m_inShowHelp = TRUE;
    m_helper->setText(
        QString::fromAscii("<qt><b>")+option->id()+
        QString::fromAscii("</b><br>")+
        option->docs().
        replace(QChar::fromAscii('\n'),QChar::fromAscii(' '))+
        QString::fromAscii("<qt>")
        );
    m_inShowHelp = FALSE;
  }
}

void Expert::nextTopic()
{
  m_topicStack->setCurrentIndex(m_topicStack->currentIndex()+1);
  m_next->setEnabled(m_topicStack->count()!=m_topicStack->currentIndex()+1);
  m_prev->setEnabled(m_topicStack->currentIndex()!=0);
  m_treeWidget->setCurrentItem(m_treeWidget->invisibleRootItem()->child(m_topicStack->currentIndex()));
}

void Expert::prevTopic()
{
  m_topicStack->setCurrentIndex(m_topicStack->currentIndex()-1);
  m_next->setEnabled(m_topicStack->count()!=m_topicStack->currentIndex()+1);
  m_prev->setEnabled(m_topicStack->currentIndex()!=0);
  m_treeWidget->setCurrentItem(m_treeWidget->invisibleRootItem()->child(m_topicStack->currentIndex()));
}

#if 0
void Expert::resetToDefaults()
{
  //printf("Expert::makeDefaults()\n");
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext()) 
  {
    i.next();
    if (i.value())
    {
      i.value()->reset();
    }
  }
}
#endif

static bool stringVariantToBool(const QVariant &v)
{
  QString s = v.toString().toLower();
  return s==QString::fromAscii("yes") || s==QString::fromAscii("true") || s==QString::fromAscii("1");
} 

static bool getBoolOption(
    const QHash<QString,Input*>&model,const QString &name)
{
  Input *option = model[name];
  Q_ASSERT(option!=0);
  return stringVariantToBool(option->value());
} 

static QString getStringOption(
    const QHash<QString,Input*>&model,const QString &name)
{
  Input *option = model[name];
  Q_ASSERT(option!=0);
  return option->value().toString();
}

#if 0
bool Expert::htmlOutputPresent(const QString &workingDir) const
{
  bool generateHtml = getBoolOption(m_options,QString::fromAscii("GENERATE_HTML"));
  if (!generateHtml || workingDir.isEmpty()) return false;
  QString indexFile = getHtmlOutputIndex(workingDir);
  QFileInfo fi(indexFile);
  return fi.exists() && fi.isFile();
}
#endif

#if 0
QString Expert::getHtmlOutputIndex(const QString &workingDir) const
{
  QString outputDir = getStringOption(m_options,QString::fromAscii("OUTPUT_DIRECTORY"));
  QString htmlOutputDir = getStringOption(m_options,QString::fromAscii("HTML_OUTPUT"));
  //printf("outputDir=%s\n",qPrintable(outputDir));
  //printf("htmlOutputDir=%s\n",qPrintable(htmlOutputDir));
  QString indexFile = workingDir;
  if (QFileInfo(outputDir).isAbsolute()) // override
  {
    indexFile = outputDir;
  }
  else // append
  { 
    indexFile += QString::fromAscii("/")+outputDir;
  }
  if (QFileInfo(htmlOutputDir).isAbsolute()) // override
  {
    indexFile = htmlOutputDir;
  }
  else // append
  {
    indexFile += QString::fromAscii("/")+htmlOutputDir;
  }
  indexFile+=QString::fromAscii("/index.html");
  return indexFile;
}


bool Expert::pdfOutputPresent(const QString &workingDir) const
{
  bool generateLatex = getBoolOption(m_options,QString::fromAscii("GENERATE_LATEX"));
  bool pdfLatex = getBoolOption(m_options,QString::fromAscii("USE_PDFLATEX"));
  if (!generateLatex || !pdfLatex) return false;
  QString latexOutput = getStringOption(m_options,QString::fromAscii("LATEX_OUTPUT"));
  QString indexFile;
  if (QFileInfo(latexOutput).isAbsolute())
  {
    indexFile = latexOutput+QString::fromAscii("/refman.pdf");
  }
  else
  {
    indexFile = workingDir+QString::fromAscii("/")+
                latexOutput+QString::fromAscii("/refman.pdf");
  }
  QFileInfo fi(indexFile);
  return fi.exists() && fi.isFile();
}
#endif

