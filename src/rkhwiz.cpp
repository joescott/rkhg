#include <QtGui>
#include <QTranslator>

#include "definitions.h"
#include "rkhwiz.h"
#include "version.h"
#include "expert.h"
#include "settingstree.h"
#include "settingdialog.h"

#ifdef WIN32
#include <windows.h>
#endif

const int messageTimeout = 5000; //!< status bar message timeout in millisec.

MainWindow &MainWindow::instance()
{
  static MainWindow *theInstance = new MainWindow();
  return *theInstance;
}

MainWindow::MainWindow()
{
  QMenu *file = menuBar()->addMenu(tr("&File"));
  file->addAction(tr("&Quit"),  this, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
  file->addAction(tr("&Configure"),  this, SLOT(configure()), Qt::ALT+Qt::Key_C);



  for (int i = 0; i < MaxRecentFiles; ++i) {
         recentFileActs[i] = new QAction(this);
         recentFileActs[i]->setVisible(false);
         connect(recentFileActs[i], SIGNAL(triggered()),
                 this, SLOT(openRecentFile()));
     }


  separatorAct = file->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
      file->addAction(recentFileActs[i]);
  file->addSeparator();
  updateRecentFileActions();

  menuBar()->addSeparator();

  QMenu *help = menuBar()->addMenu(tr("&Help"));
  help->addAction(tr("Manual"),this, SLOT(manual()), Qt::Key_F1);
  help->addAction(tr("About"), this, SLOT(about()) );

  m_expert = new Expert;
  QWidget *topPart = new QWidget;
  QVBoxLayout *rowLayout = new QVBoxLayout(topPart);
  toolBar = new QToolBar(tr("File Generation"));

  m_xmlDefault = new QPushButton(tr("Open Default"));
  m_xmlDefault->setToolTip(tr("Open Default XML Configuration File"));
  m_xmlFileName = new QLineEdit();
  m_xmlFile = new QPushButton(tr("Open XML"));
  m_xmlFile->setToolTip(tr("Open XML Configuration File"));

  m_saveTo     = new QLineEdit();
  m_saveHeader = new QPushButton(tr("Generate Header"));
  m_saveHeader->setToolTip(tr("Generate Header File"));
  m_openHeader = new QPushButton(tr("Open Header"));
  m_openHeader->setToolTip(tr("Open Header File"));

  m_saveTo->setDisabled(true);
  m_saveHeader->setDisabled(true);
  m_openHeader->setDisabled(true);

  toolBar->addWidget(m_xmlDefault);
  toolBar->addWidget(m_xmlFileName);
  toolBar->addWidget(m_xmlFile);
  toolBar->addSeparator();
  toolBar->addWidget(m_saveTo);
  toolBar->addWidget(m_saveHeader);
  toolBar->addWidget(m_openHeader);
  toolBar->setMovable(true);
  toolBar->setFloatable(true);
  toolBar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);

  rowLayout->addWidget(m_expert);

  addToolBar(toolBar);
  setCentralWidget(topPart);
  statusBar()->showMessage(tr("Welcome to Hell"),messageTimeout);


  // connect signals and slots


  connect(m_saveHeader, SIGNAL(clicked()),  SLOT(saveHeader()));
  connect(m_openHeader, SIGNAL(clicked()),  SLOT(openHeader()));
  connect(m_xmlDefault, SIGNAL(clicked()),  SLOT(openConfigFromDefault()));
  connect(m_xmlFile,    SIGNAL(clicked()),  SLOT(openConfig()));
  connect(m_expert,     SIGNAL(changed()),  SLOT(configChanged()));

  m_modified = false;
  updateTitle();

}

void MainWindow::openHeader()
{
    QString fileName = m_saveTo->text();
    if( fileName.isEmpty())
    {
        QMessageBox::warning(this,tr("Open Error"),tr("Filename is empty!"));
        return;
    }
    openHeader(fileName);

}



void MainWindow::closeEvent(QCloseEvent *event)
{
  if (discardUnsavedChanges())
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void MainWindow::quit()
{
  if (discardUnsavedChanges())
  {
  }
  QApplication::exit(0);
}

void MainWindow::setWorkingDir(const QString &dirName)
{
    QDir::setCurrent(dirName);
}

void MainWindow::openHeader(const QString &fileName)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::manual()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(directoryOf("help").absoluteFilePath(HELP_FILE)));
}

void MainWindow::configure()
{
#if 0
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open INI File"),
                               "", tr("INI Files (*.ini *.conf)"));

#endif
    QString fileName = directoryOf("help").absoluteFilePath(CF_PATH);
    QSettings *settings = new QSettings(fileName, QSettings::IniFormat);
    settingsTree = new SettingsTree;
    setSettingsObject(settings);
    SettingDialog d(this, settingsTree);
    d.exec();
}

QDir MainWindow::directoryOf(const QString &subdir)
{
    QDir dir(QApplication::applicationDirPath());
    dir.cd(subdir);
    return dir;
}

void MainWindow::about()
{
  QString msg;
  QTextStream t(&msg,QIODevice::WriteOnly);
  t << QString::fromAscii("<qt><center>This is a tool to generate RKH configuration header file</center>"
       "<center>Based on doxywizard work of Dimitri van Heesch. Written by Jorge Courett<br>&copy; 2012</center></qt>");
  QMessageBox::about(this,tr("RKH Configuration GUI"),msg);
}

void MainWindow::loadFile(QString file)
{
    setCurrentFile(file);
    m_xmlFileName->setText(file.toAscii());
    m_expert->loadXmlFile(file);
    m_saveTo->setDisabled(false);
    m_saveHeader->setDisabled(false);
    m_openHeader->setDisabled(false);
}

void MainWindow::openConfig()
{
  if (discardUnsavedChanges(false))
  {
      QString fn = QFileDialog::getOpenFileName(this,tr("Open Configuration XML File"),
                                                m_settings->value(CF_DEFAULTDIR).toString(),"*.xml");
    if (!fn.isEmpty())
    {
        loadFile(fn);
    }
  }
}

void MainWindow::openConfigFromDefault()
{
    loadFile(m_settings->value(CF_TEMPLATE).toString());
}

void MainWindow::updateConfigFileName(const QString &fileName)
{
  if (m_fileName!=fileName)
  {
    m_fileName = fileName;
    QString curPath = QFileInfo(fileName).path();
    setWorkingDir(curPath);
    updateTitle();
  }
}

void MainWindow::saveHeader(const QString &fileName)
{
  if (fileName.isEmpty())
      return;

  QFile f(fileName);
  if (!f.open(QIODevice::WriteOnly)) 
  {
    QMessageBox::warning(this,
        tr("Error saving"),
        tr("Error: cannot open the file ")+fileName+tr(" for writing!\n")+
        tr("Reason given: ")+f.error());
    return;
  }
  QTextStream t(&f);

  m_expert->writeConfig(QFileInfo(fileName).baseName(),t,false);
  QString fn = fileName;
  fn.replace(".h",".xml");

  m_expert->saveXmlFile(fn);
  m_saveTo->setText(fileName);

  updateConfigFileName(fileName);
  m_modified = false;
  updateTitle();
}

bool MainWindow::saveHeader()
{
  if (m_saveTo->text().isEmpty() ||
          ! this->matchExt("h",m_saveTo->text()))
  {
    return saveHeaderAs();
  }
  else
  {
    saveHeader(m_saveTo->text());
    return true;
  }
}

bool MainWindow::saveHeaderAs()
{
    QFileDialog *fd = new QFileDialog();
    fd->setDefaultSuffix(QString("h"));

    QString fileName = fd->getSaveFileName(this, QString(),
                                                  m_saveTo->text(),"*.h");
    if(fileName.isEmpty())
            fileName.append("dummy");
    if(!matchExt("h",fileName))
            fileName.append(".h");
    saveHeader(fileName);
  return true;
}

void MainWindow::configChanged()
{
  m_modified = true;
  updateTitle();
}

void MainWindow::updateTitle()
{
  QString title = tr("RKH GUI frontend");
  if (m_modified)
  {
    title+=QString::fromAscii(" +");
  }
  if (!m_fileName.isEmpty())
  {
    title+=QString::fromAscii(" (")+m_fileName+QString::fromAscii(")");
  }
  setWindowTitle(title);
}

bool MainWindow::discardUnsavedChanges(bool saveOption)
{
  if (m_modified)
  {
    QMessageBox::StandardButton button;
    if (saveOption)
    {
      button = QMessageBox::question(this,
          tr("Unsaved changes"),
          tr("Unsaved changes will be lost! Do you want to save the configuration file?"),
          QMessageBox::Save    |
          QMessageBox::Discard |
          QMessageBox::Cancel
          );
      if (button==QMessageBox::Save)
      {
        return saveHeader();
      }
    }
    else
    {
      button = QMessageBox::question(this,
          tr("Unsaved changes"),
          tr("Unsaved changes will be lost! Do you want to continue?"),
          QMessageBox::Discard |
          QMessageBox::Cancel
          );
    }
    return button==QMessageBox::Discard;
  }
  return true;
}

void MainWindow::setSettings(QSettings *qs)
{
    m_settings = qs;
}

bool MainWindow::matchExt(QString ext,QString fileName)
{
    QRegExp rx("*."+ext.toAscii());
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName.toAscii());
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}


void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}

void MainWindow::setSettingsObject(QSettings *settings)
{
    settings->setFallbacksEnabled(false);
    settingsTree->setSettingsObject(settings);

    QString niceName = settings->fileName();
    niceName.replace("\\", "/");
    int pos = niceName.lastIndexOf("/");
    if (pos != -1)
        niceName.remove(0, pos + 1);

    if (!settings->isWritable())
        niceName = tr("%1 (read only)").arg(niceName);

    setWindowTitle(tr("%1 - %2").arg(niceName).arg(tr("Settings Editor")));
}

/**
 * Main
 */

void MessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg:
            fprintf(stdout, "Debug: %s\n", msg);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", msg);
            abort();
    }
}

void
valSettings(QSettings *pqs)
{
    pqs->setValue(CF_STYLE,
            pqs->value(CF_STYLE,DFTCF_STYLE));
    pqs->setValue(CF_LANGUAGE,
            pqs->value(CF_LANGUAGE,DFTCF_LANGUAGE));
    pqs->setValue(CF_DEFAULTDIR,
            pqs->value(CF_DEFAULTDIR,DFTCF_DEFAULTDIR));
    pqs->setValue(CF_TEMPLATE,
            pqs->value(CF_TEMPLATE,DFTCF_TEMPLATE));

}

int main(int argc,char **argv)
{
    qInstallMsgHandler(MessageOutput);

    QTextCodec *Codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(Codec);
    QTextCodec::setCodecForCStrings(Codec);
    QTextCodec::setCodecForLocale(Codec);

    QSettings settings( CF_PATH, QSettings::IniFormat );

    valSettings(&settings);

    QApplication app(argc,argv);
    app.setWindowIcon(QIcon(":icon"));

    QTranslator qtTranslator;
    qtTranslator.load("qt_"+settings.value(CF_LANGUAGE).toString(),LANG_PATH);
    app.installTranslator(&qtTranslator);

    QTranslator translator;
    if (!translator.load(settings.value(CF_LANGUAGE).toString(),
                         LANG_PATH))
        qDebug() << "Using Default Language";
    app.installTranslator(&translator);


    QPixmap pixmap(":splash");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->setWindowFlags(splash->windowFlags() | Qt::WindowStaysOnTopHint);
    splash->show();
    app.processEvents();
    splash->showMessage(QObject::tr("\n\nLoading ..."),Qt::AlignCenter,Qt::white);

    QFile f(":theme.css");
    if (f.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(f.readAll());
        f.close();
    }

    QApplication::setStyle(QStyleFactory::create(
           settings.value(CF_STYLE).toString()));

    MainWindow &main = MainWindow::instance();
    main.setSettings(&settings);
    main.show();
    splash->finish(&main);

    return app.exec();
}

