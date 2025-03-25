TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

HEADERS +=

SUBDIRS += \
    ../rlt/rlt.pro \
    rlt.pro \
    rlt.pro

FORMS +=

DISTFILES +=
