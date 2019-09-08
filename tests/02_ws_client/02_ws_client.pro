QT     += core
CONFIG += c++11

TARGET  = 02_ws_client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../../libs/qconsolelistener.git/src/qconsolelistener.pri)
include($$PWD/../../libs/qdeferred.git/src/qdeferred.pri)
include($$PWD/../../libs/qlistclient/qlistclient.pri)
include($$PWD/../../libs/qlistproto/qlistproto.pri)

SOURCES += main.cpp

include($$PWD/../../libs/add_qt_path_win.pri)
