TEMPLATE = app
CONFIG += c++20

QT += widgets

# Sources
SOURCES += src/main.cpp \
           src/editorwindow.cpp \
           src/syntaxhighlighter.cpp \
           src/buildermanager.cpp

# Headers
HEADERS += src/editorwindow.h \
           src/syntaxhighlighter.h \
           src/buildermanager.h \
           src/binarytreehelper.hpp \
           src/syntaxerror.hpp \
           src/notimplementedexception.hpp

RESOURCES += resources.qrc

TARGET = wibsedit
