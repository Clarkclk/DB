TEMPLATE = app
CONFIG += console
CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-sign-compare
QMAKE_CXXFLAGS += -g
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    bufmanager/BufPageManager.cpp \
    bufmanager/FindReplace.cpp \
    fileio/FileManager.cpp \
    fileio/FileTable.cpp \
    utils/compare.cpp \
    utils/MyBitMap.cpp \
    utils/MyHashMap.cpp \
    utils/MyLinkList.cpp \
    utils/pagedef.cpp \
    database.cpp \
    table.cpp \
    yacc_lex/lex.yy.cpp \
    yacc_lex/parser.cpp \
    dbms.cpp \
    record.cpp \
    datanode.cpp

HEADERS += \
    bufmanager/BufPageManager.h \
    bufmanager/FindReplace.h \
    fileio/FileManager.h \
    fileio/FileTable.h \
    utils/compare.h \
    utils/MyBitMap.h \
    utils/MyHashMap.h \
    utils/MyLinkList.h \
    utils/pagedef.h \
    database.h \
    table.h \
    yacc_lex/SQL_Handler.l \
    yacc_lex/SQL_Handler.y \
    yacc_lex/parser.hpp \
    dbms.h \
    record.h \
    datanode.h

