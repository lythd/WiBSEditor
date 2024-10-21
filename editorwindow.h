/* editorwindow.h
PURPOSE:
- Manages the graphical interface of the window
*/
#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include "editorwindow.h"
#include "tokenparser.h"
#include "syntaxhighlighter.h"
#include <QMainWindow>
#include <QTextEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSettings>
#include <QSplitter>

class EditorWindow : public QMainWindow {
    Q_OBJECT

public:
    EditorWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QTextEdit *textEdit;
    QFileSystemModel *fileModel;
    QTreeView *fileTree;
    QSplitter *splitter;
    QSettings *settings;

    SyntaxHighlighter *syntaxHighlighter;

    QString currentFilePath;

    const QString themeDir = ":/themes";

    void createMenu();
    void openFolder();
    void fileSelected(const QModelIndex &index);
    void newFile();
    void saveFile();
    void saveFileAs();
    void run();
    void changeTheme();
    #ifdef DEBUG
    void previewCompilation();
    #endif
    void loadTheme(const QString &themeFile);
    void saveSettings();
    void loadSettings();
    void applyTheme(const QJsonObject &themeData);
};

#endif // EDITORWINDOW_H
