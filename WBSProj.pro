TEMPLATE = app
CONFIG += c++20

QT += widgets

# Sources
SOURCES += main.cpp \
           editorwindow.cpp \
           syntaxhighlighter.cpp

# Headers
HEADERS += editorwindow.h \
           syntaxhighlighter.h

RESOURCES += resources.qrc

TARGET = wbsedit
