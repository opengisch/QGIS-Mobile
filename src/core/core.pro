TEMPLATE = lib
TARGET = core

QT += widgets concurrent xml positioning printsupport svg sql opengl sensors quick quickcontrols2 qml
# MISSING in apk: printsupport svg sql opengl sensors

android {
    QT += androidextras
    HEADERS += androidplatformutilities.h \
               androidpicturesource.h
    SOURCES += androidplatformutilities.cpp \
               androidpicturesource.cpp
}

include( ../../qfield.pri )
include( ../../qgis.pri )
include( ../../version.pri )

HEADERS += \
    appinterface.h \
    featurelistextentcontroller.h \
    multifeaturelistmodel.h \
    featurelistmodelhighlight.h \
    featurelistmodelselection.h \
    featuremodel.h \
    maptransform.h \
    qgismobileapp.h \
    qgsquickmapcanvasmap.h \
    qgssggeometry.h \
    sgrubberband.h \
    settings.h \
    platformutilities.h \
    geometry.h \
    modelhelper.h \
    rubberbandmodel.h \
    rubberband.h \
    picturesource.h \
    messagelogmodel.h \
    attributeformmodelbase.h \
    layertreemapcanvasbridge.h \
    identifytool.h \
    submodel.h \
    attributeformmodel.h \
    expressionvariablemodel.h \
    badlayerhandler.h \
    snappingutils.h \
    snappingresult.h \
    layertreemodel.h \
    legendimageprovider.h \
    featurelistmodel.h \
    distancearea.h \
    coordinatetransformer.h \
    expressioncontextutils.h \
    focusstack.h \
    printlayoutlistmodel.h \
    vertexmodel.h \
    maptoscreen.h

SOURCES += \
    appinterface.cpp \
    featurelistextentcontroller.cpp \
    multifeaturelistmodel.cpp \
    featurelistmodelhighlight.cpp \
    featurelistmodelselection.cpp \
    featuremodel.cpp \
    maptransform.cpp \
    qgismobileapp.cpp \
    qgsquickmapcanvasmap.cpp \
    qgssggeometry.cpp \
    sgrubberband.cpp \
    settings.cpp \
    platformutilities.cpp \
    geometry.cpp \
    modelhelper.cpp \
    rubberbandmodel.cpp \
    rubberband.cpp \
    picturesource.cpp \
    messagelogmodel.cpp \
    attributeformmodelbase.cpp \
    layertreemapcanvasbridge.cpp \
    identifytool.cpp \
    submodel.cpp \
    attributeformmodel.cpp \
    expressionvariablemodel.cpp \
    badlayerhandler.cpp \
    snappingutils.cpp \
    snappingresult.cpp \
    layertreemodel.cpp \
    legendimageprovider.cpp \
    featurelistmodel.cpp \
    distancearea.cpp \
    coordinatetransformer.cpp \
    expressioncontextutils.cpp \
    focusstack.cpp \
    printlayoutlistmodel.cpp \
    vertexmodel.cpp \
    maptoscreen.cpp

INCLUDEPATH += ../../3rdparty/tessellate \
               ../qgsquick \

DEPENDPATH += ../qgsquick

LIBS += ../../3rdparty/tessellate/libtessellate.a \
       -L$$OUT_PWD/../qgsquick -lqgsquick



