TEMPLATE = lib
TARGET = qgsquick

include( ../../qfield.pri )
include( ../../qgis.pri )
include( ../../version.pri )

#QT += widgets concurrent xml positioning printsupport svg sql opengl sensors quick quickcontrols2
QT += widgets xml positioning quick printsupport

SOURCES += mapsettings.cpp \
           qgsquickutils.cpp

HEADERS += mapsettings.h \
           qgsquickutils.h



