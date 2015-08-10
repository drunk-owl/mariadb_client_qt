TEMPLATE = lib
TARGET = mariadb_client_qt
VERSION = 0.1.0
QT -= gui

HEADERS += \
    mariadb_result.h \
    mariadb_conn_params.h \
    mariadb_conn_pool.h \
    mariadb_conn.h \
    mariadb_conn_block.h

SOURCES += \
    mariadb_result.cpp \
    mariadb_conn_pool.cpp \
    mariadb_conn.cpp \
    mariadb_conn_block.cpp

DEFINES += QT_FORCE_ASSERTS

LIBS += -lmysqlclient

QMAKE_CXXFLAGS += -std=gnu++11
QMAKE_LFLAGS += -Wl,--as-needed

OBJECTS_DIR = ./build
MOC_DIR = ./build
DESTDIR = ./build
