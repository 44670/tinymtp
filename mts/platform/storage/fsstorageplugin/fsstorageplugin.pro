include(../../../common.pri)

TEMPLATE = lib
TARGET = fsstorage

CONFIG += plugin debug

QT += xml
QT -= gui



DEPENDPATH += . \
              .. \
              ../../.. \
              ../../../protocol \
              ../../../transport \
              ../../../common

INCLUDEPATH += . \
               .. \
               ../../.. \
               ../../../protocol \
               ../../../transport \
               ../../../common \


# Input
HEADERS += fsstorageplugin.h \
           storagetracker.h \
           ../storageplugin.h \
           fsinotify.h \
           storageitem.h

SOURCES += fsstorageplugin.cpp \
           fsstoragepluginfactory.cpp \
           fsinotify.cpp \
           storageitem.cpp \
    storagetracker.cpp

LIBPATH += ../../..
LIBS    += -lmeegomtp -lblkid

#install
target.path = /usr/lib/mtp

configuration.path = /etc/fsstorage.d
configuration.files = phone-memory.xml homedir-blacklist.conf sd-card.xml

INSTALLS += target configuration

#clean
QMAKE_CLEAN += $(TARGET)
