/* main.cpp
PURPOSE:
- Launches the application
*/
#include <QApplication>
#include "editorwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    EditorWindow window;
    window.resize(800, 600);
    window.show();

    return app.exec();
}
