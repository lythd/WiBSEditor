TEMPLATE = app
CONFIG += c++20

QT += widgets

# Sources
SOURCES += main.cpp \
           editorwindow.cpp \
           syntaxhighlighter.cpp \
           tokenparser.cpp

# Headers
HEADERS += editorwindow.h \
           syntaxhighlighter.h \
           tokenparser.h \
           binarytreehelper.hpp

RESOURCES += resources.qrc

TARGET = wbsedit
