TARGET = main
TEMPLATE = app
INCLUDEPATH += /opt/ace/include
LIBS += -L/opt/ace/lib -lACE -lACE_QtReactor
SOURCES += main.cpp
HEADERS += main.h

