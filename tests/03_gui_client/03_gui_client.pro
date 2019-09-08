QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 03_gui_client
TEMPLATE = app

CONFIG += c++11

include($$PWD/../../libs/qdeferred.git/src/qdeferred.pri)
include($$PWD/../../libs/qlistclient/qlistclient.pri)
include($$PWD/../../libs/qlistproto/qlistproto.pri)

SOURCES += \
        main.cpp \
        qlistclientwidget.cpp

HEADERS += \
        qlistclientwidget.h

FORMS += \
        qlistclientwidget.ui

RESOURCES += \
		qlistclientwidget.qss

include($$PWD/../../libs/add_qt_path_win.pri)
