TEMPLATE = app

CONFIG -= debug_and_release
CONFIG += release
# CONFIG += debug
QMAKE_CXXFLAGS_RELEASE += -O3

TARGET = pngDistImgViewer
QT += core gui
HEADERS += \
    ImgViewer.hpp
SOURCES += \
    main.cpp \
    ImgViewer.cpp

LIBS += \
   -lboost_filesystem \
   -lboost_regex

