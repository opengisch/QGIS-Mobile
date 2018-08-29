/***************************************************************************
                            qgismobileapp.h
                              -------------------
              begin                : Wed Apr 04 10:48:28 CET 2012
              copyright            : (C) 2012 by Marco Bernasocchi
              email                : marco@bernawebdesign.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGISMOBILEAPP_H
#define QGISMOBILEAPP_H

// Qt includes
#include <QtQml/QQmlApplicationEngine>

// QGIS includes
#include <qgsapplication.h>
#include <qgsmaplayerproxymodel.h>

// QGIS mobile includes
#include "multifeaturelistmodel.h"
#include "settings.h"
#include "focusstack.h"

#include "platformutilities.h"
#if defined(Q_OS_ANDROID)
#include "androidplatformutilities.h"
#endif

class AppInterface;
class QgsOfflineEditing;
class QgsQuickMapCanvasMap;
class LayerTreeMapCanvasBridge;
class LayerTreeModel;
class LegendImageProvider;
class QgsProject;

class QgisMobileapp : public QQmlApplicationEngine
{
    Q_OBJECT
  public:
    QgisMobileapp( QgsApplication *app , QObject* parent = nullptr );
    ~QgisMobileapp() override;

    /**
     * When called loads the project file found at path.
     *
     * @param path The project file to load
     */
    void loadProjectFile( const QString& path );
    void print( int layoutIndex );

    bool event( QEvent* event ) override;

  signals:
    /**
     * Emitted when a project file is being loaded
     *
     * @param filename The filename of the project that is being loaded
     */
    void loadProjectStarted( const QString& filename );

    /**
     * Emitted when the project is fully loaded
     */
    void loadProjectEnded();

  private slots:

    /**
     * When called loads the last project
     */
    void loadLastProject();

    /**
     * Is called when a project is read.
     * Saves the last project location for auto-load on next start.
     * @param doc The xml content
     */
    void onReadProject( const QDomDocument& doc );

    void onAfterFirstRendering();

  private:
    void initDeclarative();

    void loadProjectQuirks();

    QgsOfflineEditing* mOfflineEditing;
    LayerTreeMapCanvasBridge* mLayerTreeCanvasBridge;
    LayerTreeModel* mLayerTree;
    QgsMapLayerProxyModel* mLayerList;
    AppInterface* mIface;
    Settings mSettings;
    QgsQuickMapCanvasMap* mMapCanvas;
    bool mFirstRenderingFlag;
    LegendImageProvider* mLegendImageProvider;

    QgsProject* mProject;

    // Dummy objects. We are not able to call static functions from QML, so we need something here.
    QgsCoordinateReferenceSystem mCrsFactory;
    QgsUnitTypes mUnitTypes;

#if defined(Q_OS_ANDROID)
    AndroidPlatformUtilities mPlatformUtils;
#else
    PlatformUtilities mPlatformUtils;
#endif
};

Q_DECLARE_METATYPE( QgsWkbTypes::GeometryType )
Q_DECLARE_METATYPE( QgsFeatureId )
Q_DECLARE_METATYPE( QgsAttributes )

#endif // QGISMOBILEAPP_H
