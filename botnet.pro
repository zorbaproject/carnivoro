TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    Thread.cpp \
    IRCSocket.cpp \
    IRCHandler.cpp \
    IRCClient.cpp \
    Command.cpp

HEADERS += \
    Thread.h \
    IRCSocket.h \
    IRCHandler.h \
    IRCClient.h

win32 {
LIBS += -lwsock32
}

unix {
LIBS += -lpthread
}
