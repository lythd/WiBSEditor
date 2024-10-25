/* editorwindow.cpp
PURPOSE:
- Manages the graphical interface of the window
*/
#include "editorwindow.h"
#include <QApplication>
#include <QPainter>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QAction>
#include <QDir>
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QHeaderView>
#include <QScrollBar>
#include <cmath>

EditorWindow::EditorWindow() {
    // Main window setup
    setWindowTitle("Simple Code Editor");

    // Initialize settings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini";
    settings = new QSettings(configPath, QSettings::IniFormat, this);

    // Create text editor area
    textEdit = new QTextEdit(this);
    syntaxHighlighter = new SyntaxHighlighter(textEdit->document());
    textEdit->setInputMethodHints(Qt::ImhNone);

    // Create file tree view
    fileTree = new QTreeView(this);
    fileModel = new QFileSystemModel;
    fileModel->setRootPath(QDir::homePath());
    fileTree->setModel(fileModel);
    fileTree->setRootIndex(fileModel->index(QDir::homePath()));
    connect(fileTree, &QTreeView::clicked, this, &EditorWindow::fileSelected);
    fileTree->header()->hide();
    fileTree->setColumnWidth(0, 250);
    fileTree->setColumnHidden(1, true);  // Hide the "File Size" column
    fileTree->setColumnHidden(2, true);  // Hide the "Type" column
    fileTree->setColumnHidden(3, true);  // Hide the "Date Modified" column

    // Create splitter
    splitter = new QSplitter(this);
    splitter->addWidget(fileTree);
    splitter->addWidget(textEdit);
    setCentralWidget(splitter);

    // Set header style for the tree view
    fileTree->header()->setStretchLastSection(true);

    // Set scrollbar style
    fileTree->setStyleSheet("QScrollBar:vertical { border: 2px solid #CCCCCC; background: #F5F5F5; width: 10px; }"
                            "QScrollBar::handle:vertical { background: #888888; min-height: 20px; }");

    // Create menu
    createMenu();

    // Load settings (including theme, window size, and splitter position)
    loadSettings();
}

void EditorWindow::createMenu() {
    QMenu *fileMenu = menuBar()->addMenu("File");
    QMenu *editMenu = menuBar()->addMenu("Edit");
    #ifdef DEBUG
    QMenu *debugMenu = menuBar()->addMenu("Debug");
    #endif

    // Open project
    QAction *openFolderAction = new QAction("Open Folder", this);
    connect(openFolderAction, &QAction::triggered, this, &EditorWindow::openFolder);
    fileMenu->addAction(openFolderAction);

    // New File
    QAction *newFileAction = new QAction("New File", this);
    connect(newFileAction, &QAction::triggered, this, &EditorWindow::newFile);
    fileMenu->addAction(newFileAction);

    // Save file
    QAction *saveFileAction = new QAction("Save", this);
    connect(saveFileAction, &QAction::triggered, this, &EditorWindow::saveFile);
    fileMenu->addAction(saveFileAction);

    // Generate
    QAction *runAction = new QAction("Generate", this);
    connect(runAction, &QAction::triggered, this, &EditorWindow::run);
    fileMenu->addAction(runAction);

    // Change theme
    QAction *changeThemeAction = new QAction("Change Theme", this);
    connect(changeThemeAction, &QAction::triggered, this, &EditorWindow::changeTheme);
    editMenu->addAction(changeThemeAction);

    // Preview compilation tree
    #ifdef DEBUG
    QAction *previewCompilationAction = new QAction("Preview Compilation Tree", this);
    connect(previewCompilationAction, &QAction::triggered, this, &EditorWindow::previewCompilation);
    debugMenu->addAction(previewCompilationAction);
    #endif
}

void EditorWindow::openFolder() {
    QString folder = QFileDialog::getExistingDirectory(this, "Open Project Folder", QDir::homePath());
    if (!folder.isEmpty()) {
        fileTree->setRootIndex(fileModel->index(folder));
    }
}

void EditorWindow::fileSelected(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        currentFilePath = filePath;
        textEdit->setPlainText(file.readAll());
        syntaxHighlighter->rehighlight();
        file.close();
    }
}

void EditorWindow::newFile() {
    // Clear current text and set as new unsaved file
    textEdit->clear();
    currentFilePath.clear();
    textEdit->document()->setModified(true);
}

void EditorWindow::saveFile() {
    if (currentFilePath.isEmpty()) {
        saveFileAs();  // If it's a new file, ask to save as
        return;
    }
    QFile file(currentFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(textEdit->toPlainText().toUtf8());
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Could not save the file.");
    }
}

void EditorWindow::saveFileAs() {
    // Open the save dialog in the current directory of the file tree
    QString initialDir = fileModel->filePath(fileTree->rootIndex());
    QString filePath = QFileDialog::getSaveFileName(this, "Save File", initialDir, "Text Files (*.txt);;All Files (*)");

    if (filePath.isEmpty()) return;

    currentFilePath = filePath;  // Update current file path
    saveFile();  // Save the file
}

void EditorWindow::run() {
    if (fileModel->rootPath().isEmpty()) {
        QMessageBox::warning(this, "Error", "No project folder is opened.");
        return;
    }

    QString outputPath = fileModel->rootPath() + "/website.php";
    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write("<?php // This is an empty generated file. ?>");
        file.close();
        QMessageBox::information(this, "Run", "Generated website.php in the project directory.");
    }
}

void EditorWindow::changeTheme() {
    QStringList themes = {"Default"};

    // Access the resources folder to list all theme files
    QDir dir(themeDir);
    if (dir.exists()) {
        // Get all .json files in the directory
        QStringList themeFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
        themes.append(themeFiles);
    } else {
        QMessageBox::warning(this, "Error", "Themes directory not found.");
        return;
    }

    // Prompt the user to select a theme after formatting the name
    bool ok;
    for (int i = 0; i < themes.size(); ++i) {
        themes[i] = themes[i].remove(".json");
        themes[i].replace(0, 1, themes[i][0].toUpper());
    }

    QString selectedTheme = QInputDialog::getItem(this, "Select Theme", "Theme:", themes, 0, false, &ok);

    if (ok && !selectedTheme.isEmpty()) {
        // Unformat name and then load theme
        selectedTheme = selectedTheme.toLower() + ".json";
        loadTheme(selectedTheme);
        settings->setValue("theme", selectedTheme);
        saveSettings();  // Save the chosen theme to settings
    }
}

#ifdef DEBUG
void EditorWindow::previewCompilation() {
    // Custom QWidget to represent each node, I was trying to draw lines which is the whole purpose this class exists but I couldn't get them working properly and it aint that important
    class TreeLabel : public QWidget {
    public:
        TreeLabel(const QString &text, QWidget *parent = nullptr)
            : QWidget(parent), label(new QLabel(text, this)) {
            label->setAlignment(Qt::AlignCenter);
            setFixedSize(label->sizeHint());
        }

        #ifdef DEBUG_DRAWLINES
        void setParentPositionLC(const TreeLabel& parent) {
            parentStart = label->rect().topRight();
            parentEnd = parent.label->rect().bottomLeft();
            update();
        }

        void setParentPositionRC(const TreeLabel& parent) {
            parentStart = label->rect().topLeft();
            parentEnd = parent.label->rect().bottomRight();
            update();
        }
        #endif

    protected:
        #ifdef DEBUG_DRAWLINES
        void paintEvent(QPaintEvent *) override {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            // Draw line to parent
            if (!parentStart.isNull() && !parentEnd.isNull()) {
                painter.setPen(QPen(Qt::black, 2));
                painter.drawLine(parentStart, parentEnd);
            }
        }
        #endif

        QSize sizeHint() const override {
            return label->sizeHint();
        }

    private:
        QLabel *label;
        #ifdef DEBUG_DRAWLINES
        QPoint parentStart, parentEnd;
        #endif
    };

    // Parse
    TokenParser parser;
    auto toks = parser.parse(textEdit->toPlainText().toStdString());
    IntermediateNode *inter = new IntermediateNode();
    inter->generateTree(toks);
    while (inter->getParent() != nullptr) inter = inter->getParent();
    std::vector<std::string> tokens;
    inter->getAsVector(tokens);

    // Create a popup window
    QDialog *popup = new QDialog(nullptr);
    popup->setWindowTitle("Binary Tree Representation of Tokens");
    QGridLayout *layout = new QGridLayout(popup);

    // Display the tokens in a binary tree form
    std::vector<TreeLabel*> treeLabels;
    for (size_t i = 0; i < tokens.size(); ++i) {
        int maxDepth = BinaryTreeHelper::getDepth(tokens.size() - 1);
        int depth = BinaryTreeHelper::getDepth(i);
        int column = BinaryTreeHelper::getColumn(i, depth);
        int rowColumns = BinaryTreeHelper::getMaxColumnsForDepth(depth);
        int maxColumns = BinaryTreeHelper::getMaxColumnsForDepth(maxDepth+1);
        
        // Calculate column by centering the tokens within the max column space for the depth
        int baseCol = (int) std::round(((double)column+0.5)/(double)rowColumns * maxColumns);

        // Create a custom widget for the token
        TreeLabel *treeLabel = new TreeLabel(QString::fromStdString(tokens[i]));
        treeLabels.push_back(treeLabel);
        layout->addWidget(treeLabel, depth, baseCol);

        // Draw line to parent based on either being right child or left child
        #ifdef DEBUG_DRAWLINES
        if(i%2==1) treeLabel->setParentPositionLC(*(treeLabels[i/2]));
        else if(i>0) treeLabel->setParentPositionRC(*(treeLabels[i/2-1]));
        #endif
    }
    treeLabels.clear();

    popup->setLayout(layout);
    popup->exec();
}
#endif

void EditorWindow::loadTheme(const QString &themeName) {
    // Load theme
    if(themeName == "default.json") {
        qApp->setPalette(qApp->style()->standardPalette());
        return;
    }
    QString themeFile = themeDir + "/" + themeName;
    QFile file(themeFile);
    if (!file.exists()) {
        QMessageBox::warning(this, "Error", QString("Theme file does not exist: %1").arg(themeFile));
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", QString("Could not open theme file: %1").arg(themeFile));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    // Apply theme
    if (doc.isObject()) {
        applyTheme(doc.object());
    } else {
        QMessageBox::warning(this, "Error", "Invalid theme file format.");
    }
}

void EditorWindow::applyTheme(const QJsonObject &themeData) {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(themeData["Window"].toString()));
    palette.setColor(QPalette::WindowText, QColor(themeData["WindowText"].toString()));
    palette.setColor(QPalette::Base, QColor(themeData["Base"].toString()));
    palette.setColor(QPalette::Text, QColor(themeData["Text"].toString()));
    palette.setColor(QPalette::Highlight, QColor(themeData["Highlight"].toString()));
    palette.setColor(QPalette::HighlightedText, QColor(themeData["HighlightedText"].toString()));
    qApp->setPalette(palette);

    // Set the header style for the file tree view
    fileTree->header()->setStyleSheet(QString("QHeaderView::section { background-color: %1; }")
                                      .arg(themeData["TreeViewHeader"].toString()));

    // Set the background color for the file tree view immediately
    fileTree->setStyleSheet(QString("QTreeView { background-color: %1; color: %2; }")
                            .arg(themeData["Base"].toString())   // Background color
                            .arg(themeData["Text"].toString()));  // Text color

    // Set the style for the horizontal scrollbar
    QScrollBar *horizontalScrollBar = fileTree->horizontalScrollBar();
    horizontalScrollBar->setStyleSheet(QString("QScrollBar:horizontal { background-color: %1; }")
                                       .arg(themeData["TreeViewHeader"].toString()));
}

void EditorWindow::saveSettings() {
    settings->setValue("theme", settings->value("theme", "light").toString());

    // Save window geometry
    settings->setValue("windowGeometry", saveGeometry());
    settings->setValue("windowState", saveState());

    // Save splitter positions
    settings->setValue("splitterState", splitter->saveState());

    // Save the last opened directory
    QString currentDir = fileModel->filePath(fileTree->rootIndex());
    settings->setValue("lastOpenedDir", currentDir);

    settings->sync();
}

void EditorWindow::loadSettings() {
    QString theme = settings->value("theme", "light").toString();
    loadTheme(theme);

    // Restore window geometry and state, if there is no data or it fails to restore then use default
    QByteArray geometryData = settings->value("windowGeometry").toByteArray();
    if (geometryData.isEmpty() || !restoreGeometry(geometryData)) {
        resize(800, 600);
        move(100, 100);
    }

    // Restore window state
    restoreState(settings->value("windowState").toByteArray());

    // Restore splitter state
    splitter->restoreState(settings->value("splitterState").toByteArray());

    // Restore last opened directory (if saved)
    QString lastOpenedDir = settings->value("lastOpenedDir", QDir::homePath()).toString();
    fileTree->setRootIndex(fileModel->index(lastOpenedDir));
}

void EditorWindow::closeEvent(QCloseEvent *event) {
    // Check if the document is modified
    if (textEdit->document()->isModified()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved Changes", 
                                      "You have unsaved changes. Do you want to save them?",
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            // If it's a new file save as, otherwise just save
            currentFilePath.isEmpty() ? saveFileAs() : saveFile();
            event->accept();  // Proceed with closing the window
        } else if (reply == QMessageBox::Discard) {
            event->accept();  // Proceed with closing the window without saving
        } else {
            event->ignore();  // Prevent the window from closing if the user cancels
            return;
        }
    } else event->accept();  // No unsaved changes, proceed with closing the window

    // Save settings before closing
    saveSettings();
    QMainWindow::closeEvent(event);
}
