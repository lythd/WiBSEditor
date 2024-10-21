TEMPLATE = app
CONFIG += c++20

QT += widgets

# Sources
SOURCES += main.cpp \
           editorwindow.cpp \
           syntaxhighlighter.cpp \
           tokenparser.cpp \
           intermediatenode.cpp

# Headers
HEADERS += editorwindow.h \
           syntaxhighlighter.h \
           tokenparser.h \
           intermediatenode.h \
           binarytreehelper.hpp \
           token.hpp \
           notimplementedexception.hpp

RESOURCES += resources.qrc

TARGET = wbsedit
