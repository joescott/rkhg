#ifndef DOXYWIZARD_H
#define DOXYWIZARD_H

#include <QMainWindow>
#include <QSettings>
#include <QStringList>
#include <QToolBar>

class Expert;
class Wizard;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QMenu;
class QProcess;
class QTimer;
class QGroupBox;
class QToolBar;
class QDir;
class SettingsTree;


class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    static MainWindow &instance();
    //void loadSettings();
    //void saveSettings();
    void closeEvent(QCloseEvent *event);
    QString configFileName() const { return m_fileName; }
    void updateTitle();
    void setSettings(QSettings *qs);

  public slots:
    void manual();
    void about();
    bool saveHeader();
    bool saveHeaderAs();
    void openConfig();
    void openConfigFromDefault();
    void openHeader();
    void configure();
    void quit();


  private slots:
    void configChanged();
    void openRecentFile();
    
  private:
    MainWindow();
    void setSettingsObject(QSettings *settings);

    SettingsTree *settingsTree;
    void saveHeader(const QString &fileName);
    void openHeader(const QString &fileName);
    void addRecentFile(const QString &fileName);
    void updateConfigFileName(const QString &fileName);
    void setWorkingDir(const QString &dirName);
    bool discardUnsavedChanges(bool saveOption=true);
    QDir directoryOf(const QString &subdir);
    bool matchExt(QString ext,QString fileName);


    void loadFile(QString file);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);


    QString curFile;
    QLineEdit *m_saveTo;
    QLineEdit *m_xmlFileName;
    QPushButton *m_saveHeader;
    QPushButton *m_openHeader;
    QPushButton *m_xmlFile;
    QPushButton *m_xmlDefault;
    Expert *m_expert;
    QString m_fileName;
    bool m_modified;
    QToolBar *toolBar;
    QSettings *m_settings;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    QAction *separatorAct;


};

#endif
