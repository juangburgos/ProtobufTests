QT += core
QT -= gui

CONFIG += c++11

TARGET = 00_proto_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../../libs/qlistproto/qlistproto.pri)

SOURCES += main.cpp

include($$PWD/../../libs/add_qt_path_win.pri)

