#-------------------------------------------------
#
# Project created by QtCreator 2012-03-21T17:38:12
#
#-------------------------------------------------

QT       += testlib \
    network

QT       -= gui

TARGET = tst_connectiontest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_connectiontest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../src/api/release/ -lvkit
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../src/api/debug/ -lvkit
else:symbian: LIBS += -lvkit
else:unix: LIBS += -L$$OUT_PWD/../../../src/api/ -lvkit

INCLUDEPATH += $$PWD/../../../src/api \
    $$PWD/../../

DEPENDPATH += $$PWD/../../../src/api
