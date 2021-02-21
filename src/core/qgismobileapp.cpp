/***************************************************************************
                            qgismobileapp.cpp
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

#include <QApplication>

#include <unistd.h>
#include <stdlib.h>

#include <proj.h>

#include <QFontDatabase>
#include <QStandardPaths>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlApplicationEngine>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog> // Until native looking QML dialogs are implemented (Qt5.4?)
#include <QtWidgets/QMenu> // Until native looking QML dialogs are implemented (Qt5.4?)
#include <QtWidgets/QMenuBar>
#include <QStandardItemModel>
#include <QPrinter>
#include <QPrintDialog>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QStyleHints>

#include <qgslayertreemodel.h>
#include <qgslocalizeddatapathregistry.h>
#include <qgsproject.h>
#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsrasterresamplefilter.h>
#include <qgsbilinearrasterresampler.h>
#include <qgssnappingutils.h>
#include <qgsunittypes.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsmapthemecollection.h>
#include <qgsprintlayout.h>
#include <qgslayoutmanager.h>
#include <qgslayoutpagecollection.h>
#include <qgslayoutitemmap.h>
#include <qgslocator.h>
#include <qgslocatormodel.h>
#include <qgsfield.h>
#include <qgsfieldconstraints.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgsexpressionfunction.h>
#include <qgslayertree.h>
#include <qgssinglesymbolrenderer.h>

#include "qgsquickmapsettings.h"
#include "qgsquickmapcanvasmap.h"
#include "qgsquickcoordinatetransformer.h"
#include "qgsquickmaptransform.h"

#include "qgsnetworkaccessmanager.h"

#include "qgismobileapp.h"

#include "appinterface.h"
#include "featurelistmodelselection.h"
#include "featurelistextentcontroller.h"
#include "modelhelper.h"
#include "rubberband.h"
#include "rubberbandmodel.h"
#include "qgsofflineediting.h"
#include "messagelogmodel.h"
#include "attributeformmodel.h"
#include "geometry.h"
#include "featuremodel.h"
#include "layertreemapcanvasbridge.h"
#include "identifytool.h"
#include "submodel.h"
#include "expressionvariablemodel.h"
#include "badlayerhandler.h"
#include "snappingutils.h"
#include "snappingresult.h"
#include "layertreemodel.h"
#include "legendimageprovider.h"
#include "featurelistmodel.h"
#include "qgsrelationmanager.h"
#include "distancearea.h"
#include "printlayoutlistmodel.h"
#include "vertexmodel.h"
#include "maptoscreen.h"
#include "projectinfo.h"
#include "projectsource.h"
#include "locatormodelsuperbridge.h"
#include "qgsgeometrywrapper.h"
#include "linepolygonhighlight.h"
#include "valuemapmodel.h"
#include "recentprojectlistmodel.h"
#include "referencingfeaturelistmodel.h"
#include "featurechecklistmodel.h"
#include "geometryeditorsmodel.h"
#include "geometryutils.h"
#include "trackingmodel.h"
#include "fileutils.h"
#include "featureutils.h"
#include "expressionevaluator.h"
#include "stringutils.h"
#include "urlutils.h"
#include "bluetoothreceiver.h"
#include "bluetoothdevicemodel.h"
#include "gnsspositioninformation.h"
#include "changelogcontents.h"
#include "layerresolver.h"

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string


QgisMobileapp::QgisMobileapp( QgsApplication *app, QObject *parent )
  : QQmlApplicationEngine( parent )
  , mIface( new AppInterface( this ) )
  , mFirstRenderingFlag( true )
{
#if 0
  QSurfaceFormat format;
  format.setSamples( 8 );
  setFormat( format );
  create();
#endif

  AppInterface::setInstance( mIface );

  //set the authHandler to qfield-handler
  std::unique_ptr<QgsNetworkAuthenticationHandler> handler;
  mAuthRequestHandler = new QFieldAppAuthRequestHandler();
  handler.reset( mAuthRequestHandler );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::move( handler ) );

  if ( !PlatformUtilities::instance()->qfieldDataDir().isEmpty() )
  {
    //set localized data paths
    QStringList localizedDataPaths;
    localizedDataPaths << PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "basemaps/" );
    QgsApplication::instance()->localizedDataPathRegistry()->setPaths( localizedDataPaths );

    // set fonts
    const QString fontsPath  = PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "fonts/" );
    QDir fontsDir( fontsPath );
    if ( fontsDir.exists() )
    {
      const QStringList fonts = fontsDir.entryList( QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF", QDir::Files );
      for ( auto font : fonts )
      {
        QFontDatabase::addApplicationFont( fontsPath + font );
      }
    }
  }

  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-Bold.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-BoldItalic.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-Condensed.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-Italic.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-Regular.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/Cadastra-Semibolditalic.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/CadastraSymbol-Mask.ttf" );
  QFontDatabase::addApplicationFont( ":/fonts/CadastraSymbol-Regular.ttf" );

  mProject = QgsProject::instance();
  mGpkgFlusher = qgis::make_unique<QgsGpkgFlusher>( mProject );
  mFlatLayerTree = new FlatLayerTreeModel( mProject->layerTreeRoot(), mProject, this );
  mLegendImageProvider = new LegendImageProvider( mFlatLayerTree->layerTreeModel() );
  mTrackingModel = new TrackingModel;

  // Transition from 1.8 to 1.8.1+
  QSettings settings;
  const QString deviceAddress = settings.value( QStringLiteral( "positioningDevice" ), QString() ).toString();
  if ( deviceAddress == QStringLiteral( "internal" ) )
  {
    settings.setValue( QStringLiteral( "positioningDevice" ), QString() );
  }

  // cppcheck-suppress leakReturnValNotUsed
  initDeclarative();

  if ( !PlatformUtilities::instance()->qfieldDataDir().isEmpty() )
  {
    // add extra proj search path to allow copying of transformation grid files
    QString path( proj_info().searchpath );
    QStringList paths;
#ifdef Q_OS_WIN
    paths = path.split( ';' );
#else
    paths = path.split( ':' );
#endif

    // thin out duplicates from paths -- see https://github.com/OSGeo/proj.4/pull/1498
    QSet<QString> existing;
    QStringList searchPaths;
    searchPaths.reserve( paths.count() );
    for ( QString &p : paths )
    {
      if ( existing.contains( p ) )
        continue;

      existing.insert( p );
      searchPaths << p;
    }

    searchPaths << QStringLiteral( "%1/proj/" ).arg( PlatformUtilities::instance()->qfieldDataDir() );
    char **newPaths = new char *[searchPaths.count()];
    for ( int i = 0; i < searchPaths.count(); ++i )
    {
      newPaths[i] = strdup( searchPaths.at( i ).toUtf8().constData() );
    }
    proj_context_set_search_paths( nullptr, searchPaths.count(), newPaths );
    for ( int i = 0; i < searchPaths.count(); ++i )
    {
      free( newPaths[i] );
    }
    delete [] newPaths;

    setenv( "PGSYSCONFDIR", PlatformUtilities::instance()->qfieldDataDir().toUtf8(), true );
  }

  PlatformUtilities::instance()->setScreenLockPermission( false );

  load( QUrl( "qrc:/qml/qgismobileapp.qml" ) );

  connect( this, &QQmlApplicationEngine::quit, app, &QgsApplication::quit );

  mMapCanvas = rootObjects().first()->findChild<QgsQuickMapCanvasMap *>();
  mMapCanvas->mapSettings()->setProject( mProject );

  mFlatLayerTree->layerTreeModel()->setLegendMapViewData( mMapCanvas->mapSettings()->mapSettings().mapUnitsPerPixel(),
      static_cast< int >( std::round( mMapCanvas->mapSettings()->outputDpi() ) ), mMapCanvas->mapSettings()->mapSettings().scale() );

  Q_ASSERT_X( mMapCanvas, "QML Init", "QgsQuickMapCanvasMap not found. It is likely that we failed to load the QML files. Check debug output for related messages." );

  connect( mProject, &QgsProject::readProject, this, &QgisMobileapp::onReadProject );

  mLayerTreeCanvasBridge = new LayerTreeMapCanvasBridge( mFlatLayerTree, mMapCanvas->mapSettings(), mTrackingModel, this );

  connect( this, &QgisMobileapp::loadProjectTriggered, mIface, &AppInterface::loadProjectTriggered );
  connect( this, &QgisMobileapp::loadProjectEnded, mIface, &AppInterface::loadProjectEnded );
  connect( this, &QgisMobileapp::setMapExtent, mIface, &AppInterface::setMapExtent );

  QTimer::singleShot( 1, this, &QgisMobileapp::onAfterFirstRendering );

  mOfflineEditing = new QgsOfflineEditing();

  mSettings.setValue( "/Map/searchRadiusMM", 5 );

  mAppMissingGridHandler = new AppMissingGridHandler( this );
}

void QgisMobileapp::initDeclarative()
{

#if defined(Q_OS_ANDROID) && QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QResource::registerResource( QStringLiteral( "assets:/android_rcc_bundle.rcc" ) );
#endif
  addImportPath( QStringLiteral( "qrc:/qml/imports" ) );

  // Register QGIS QML types
  qmlRegisterType<QgsSnappingUtils>( "org.qgis", 1, 0, "SnappingUtils" );
  qmlRegisterType<QgsMapLayerProxyModel>( "org.qgis", 1, 0, "MapLayerModel" );
  qmlRegisterType<QgsVectorLayer>( "org.qgis", 1, 0, "VectorLayer" );
  qmlRegisterType<QgsMapThemeCollection>( "org.qgis", 1, 0, "MapThemeCollection" );
  qmlRegisterType<QgsLocatorProxyModel>( "org.qgis", 1, 0, "QgsLocatorProxyModel" );
  qmlRegisterType<QgsVectorLayerEditBuffer>( "org.qgis", 1, 0, "QgsVectorLayerEditBuffer" );

  qRegisterMetaType<QgsGeometry>( "QgsGeometry" );
  qRegisterMetaType<QgsFeature>( "QgsFeature" );
  qRegisterMetaType<QgsPoint>( "QgsPoint" );
  qRegisterMetaType<QgsPointXY>( "QgsPointXY" );
  qRegisterMetaType<QgsPointSequence>( "QgsPointSequence" );
  qRegisterMetaType<QgsCoordinateTransformContext>( "QgsCoordinateTransformContext" );
  qRegisterMetaType<QgsWkbTypes::GeometryType>( "QgsWkbTypes::GeometryType" ); // could be removed since we have now qmlRegisterUncreatableType<QgsWkbTypes> ?
  qRegisterMetaType<QgsWkbTypes::Type>( "QgsWkbTypes::Type" ); // could be removed since we have now qmlRegisterUncreatableType<QgsWkbTypes> ?
  qRegisterMetaType<QgsMapLayerType>( "QgsMapLayerType" ); // could be removed since we have now qmlRegisterUncreatableType<QgsWkbTypes> ?
  qRegisterMetaType<QgsFeatureId>( "QgsFeatureId" );
  qRegisterMetaType<QgsAttributes>( "QgsAttributes" );
  qRegisterMetaType<QgsSnappingConfig>( "QgsSnappingConfig" );
  qRegisterMetaType<QgsUnitTypes::DistanceUnit>( "QgsUnitTypes::DistanceUnit" );
  qRegisterMetaType<QgsUnitTypes::AreaUnit>( "QgsUnitTypes::AreaUnit" );
  qRegisterMetaType<QgsRelation>( "QgsRelation" );
  qRegisterMetaType<QgsField>( "QgsField" );
  qRegisterMetaType<QVariant::Type>( "QVariant::Type" );
  qRegisterMetaType<QgsDefaultValue>( "QgsDefaultValue" );
  qRegisterMetaType<QgsFieldConstraints>( "QgsFieldConstraints" );
  qRegisterMetaType<QgsGeometry::OperationResult>( "QgsGeometry::OperationResult" );

  qmlRegisterUncreatableType<QgsProject>( "org.qgis", 1, 0, "Project", "" );
  qmlRegisterUncreatableType<QgsCoordinateReferenceSystem>( "org.qgis", 1, 0, "CoordinateReferenceSystem", "" );
  qmlRegisterUncreatableType<QgsUnitTypes>( "org.qgis", 1, 0, "QgsUnitTypes", "" );
  qmlRegisterUncreatableType<QgsRelationManager>( "org.qgis", 1, 0, "RelationManager", "The relation manager is available from the QgsProject. Try `qgisProject.relationManager`" );
  qmlRegisterUncreatableType<QgsWkbTypes>( "org.qgis", 1, 0, "QgsWkbTypes", "" );
  qmlRegisterUncreatableType<QgsMapLayer>( "org.qgis", 1, 0, "MapLayer", "" );
  qmlRegisterUncreatableType<QgsVectorLayer>( "org.qgis", 1, 0, "VectorLayerStatic", "" );
  qmlRegisterUncreatableType<QgsGeometry>( "org.qgis", 1, 0, "QgsGeometryStatic", "" );

  // Register QgsQuick QML types
  qmlRegisterType<QgsQuickMapCanvasMap>( "org.qgis", 1, 0, "MapCanvasMap" );
  qmlRegisterType<QgsQuickMapSettings>( "org.qgis", 1, 0, "MapSettings" );
  qmlRegisterType<QgsQuickCoordinateTransformer>( "org.qfield", 1, 0, "CoordinateTransformer" );

  REGISTER_SINGLETON( "Utils", QgsQuickUtils, "Utils" );

  qmlRegisterType<QgsQuickMapTransform>( "org.qgis", 1, 0, "MapTransform" );

  // Register QField QML types
  qmlRegisterType<MultiFeatureListModel>( "org.qgis", 1, 0, "MultiFeatureListModel" );
  qmlRegisterType<FeatureListModel>( "org.qgis", 1, 0, "FeatureListModel" );
  qmlRegisterType<FeatureListModelSelection>( "org.qgis", 1, 0, "FeatureListModelSelection" );
  qmlRegisterType<FeatureListExtentController>( "org.qgis", 1, 0, "FeaturelistExtentController" );
  qmlRegisterType<Geometry>( "org.qgis", 1, 0, "Geometry" );
  qmlRegisterType<ModelHelper>( "org.qgis", 1, 0, "ModelHelper" );
  qmlRegisterType<Rubberband>( "org.qgis", 1, 0, "Rubberband" );
  qmlRegisterType<RubberbandModel>( "org.qgis", 1, 0, "RubberbandModel" );
  qmlRegisterType<PictureSource>( "org.qgis", 1, 0, "PictureSource" );
  qmlRegisterType<ProjectInfo>( "org.qgis", 1, 0, "ProjectInfo" );
  qmlRegisterType<ProjectSource>( "org.qgis", 1, 0, "ProjectSource" );
  qmlRegisterType<ViewStatus>( "org.qgis", 1, 0, "ViewStatus" );
  qmlRegisterType<MessageLogModel>( "org.qgis", 1, 0, "MessageLogModel" );
  qmlRegisterType<AttributeFormModel>( "org.qfield", 1, 0, "AttributeFormModel" );
  qmlRegisterType<FeatureModel>( "org.qfield", 1, 0, "FeatureModel" );
  qmlRegisterType<IdentifyTool>( "org.qfield", 1, 0, "IdentifyTool" );
  qmlRegisterType<SubModel>( "org.qfield", 1, 0, "SubModel" );
  qmlRegisterType<ExpressionVariableModel>( "org.qfield", 1, 0, "ExpressionVariableModel" );
  qmlRegisterType<BadLayerHandler>( "org.qfield", 1, 0, "BadLayerHandler" );
  qmlRegisterType<SnappingUtils>( "org.qfield", 1, 0, "SnappingUtils" );
  qmlRegisterType<DistanceArea>( "org.qfield", 1, 0, "DistanceArea" );
  qmlRegisterType<FocusStack>( "org.qfield", 1, 0, "FocusStack" );
  qmlRegisterType<PrintLayoutListModel>( "org.qfield", 1, 0, "PrintLayoutListModel" );
  qmlRegisterType<VertexModel>( "org.qfield", 1, 0, "VertexModel" );
  qmlRegisterType<MapToScreen>( "org.qfield", 1, 0, "MapToScreen" );
  qmlRegisterType<LocatorModelSuperBridge>( "org.qfield", 1, 0, "LocatorModelSuperBridge" );
  qmlRegisterType<LocatorActionsModel>( "org.qfield", 1, 0, "LocatorActionsModel" );
  qmlRegisterType<LinePolygonHighlight>( "org.qfield", 1, 0, "LinePolygonHighlight" );
  qmlRegisterType<QgsGeometryWrapper>( "org.qfield", 1, 0, "QgsGeometryWrapper" );
  qmlRegisterType<ValueMapModel>( "org.qfield", 1, 0, "ValueMapModel" );
  qmlRegisterType<RecentProjectListModel>( "org.qgis", 1, 0, "RecentProjectListModel" );
  qmlRegisterType<ReferencingFeatureListModel>( "org.qgis", 1, 0, "ReferencingFeatureListModel" );
  qmlRegisterType<FeatureCheckListModel>( "org.qgis", 1, 0, "FeatureCheckListModel" );
  qmlRegisterType<GeometryEditorsModel>( "org.qfield", 1, 0, "GeometryEditorsModel" );
  qmlRegisterType<ExpressionEvaluator>( "org.qfield", 1, 0, "ExpressionEvaluator" );
  qmlRegisterType<BluetoothDeviceModel>( "org.qfield", 1, 0, "BluetoothDeviceModel" );
  qmlRegisterType<BluetoothReceiver>( "org.qfield", 1, 0, "BluetoothReceiver" );
  qmlRegisterType<ChangelogContents>( "org.qfield", 1, 0, "ChangelogContents" );
  qmlRegisterType<LayerResolver>( "org.qfield", 1, 0, "LayerResolver" );

  qRegisterMetaType<GnssPositionInformation>( "GnssPositionInformation" );

  REGISTER_SINGLETON( "org.qfield", GeometryEditorsModel, "GeometryEditorsModelSingleton" );
  REGISTER_SINGLETON( "org.qfield", GeometryUtils, "GeometryUtils" );
  REGISTER_SINGLETON( "org.qfield", FeatureUtils, "FeatureUtils" );
  REGISTER_SINGLETON( "org.qfield", FileUtils, "FileUtils" );
  REGISTER_SINGLETON( "org.qfield", StringUtils, "StringUtils" );
  REGISTER_SINGLETON( "org.qfield", UrlUtils, "UrlUtils" );

  qmlRegisterUncreatableType<AppInterface>( "org.qgis", 1, 0, "QgisInterface", "QgisInterface is only provided by the environment and cannot be created ad-hoc" );
  qmlRegisterUncreatableType<Settings>( "org.qgis", 1, 0, "Settings", "" );
  qmlRegisterUncreatableType<PlatformUtilities>( "org.qgis", 1, 0, "PlatformUtilities", "" );
  qmlRegisterUncreatableType<FlatLayerTreeModel>( "org.qfield", 1, 0, "FlatLayerTreeModel", "The FlatLayerTreeModel is available as context property `flatLayerTree`." );
  qmlRegisterUncreatableType<TrackingModel>( "org.qfield", 1, 0, "TrackingModel", "The TrackingModel is available as context property `trackingModel`." );
  qmlRegisterUncreatableType<QgsGpkgFlusher>( "org.qfield", 1, 0, "QgsGpkgFlusher", "The gpkgFlusher is available as context property `gpkgFlusher`" );

  qRegisterMetaType<SnappingResult>( "SnappingResult" );

  // Calculate device pixels
  qreal dpi = std::max( QApplication::desktop()->logicalDpiY(), QApplication::desktop()->logicalDpiY() );
  dpi *= QApplication::desktop()->devicePixelRatioF();

  // Register some globally available variables
  rootContext()->setContextProperty( "ppi", dpi );
  rootContext()->setContextProperty( "mouseDoubleClickInterval", QApplication::styleHints()->mouseDoubleClickInterval() );
  rootContext()->setContextProperty( "qgisProject", mProject );
  rootContext()->setContextProperty( "qgisProjet", mProject );
  rootContext()->setContextProperty( "iface", mIface );
  rootContext()->setContextProperty( "settings", &mSettings );
  rootContext()->setContextProperty( "appVersion", QString( "" APP_VERSION ) );
  rootContext()->setContextProperty( "appVersionStr", QString( QUOTE( APP_VERSION_STR ) ) );
  rootContext()->setContextProperty( "gitRev", QString( "" GIT_REV ) );
  rootContext()->setContextProperty( "flatLayerTree", mFlatLayerTree );
  rootContext()->setContextProperty( "platformUtilities", PlatformUtilities::instance() );
  rootContext()->setContextProperty( "CrsFactory", QVariant::fromValue<QgsCoordinateReferenceSystem>( mCrsFactory ) );
  rootContext()->setContextProperty( "UnitTypes", QVariant::fromValue<QgsUnitTypes>( mUnitTypes ) );
  rootContext()->setContextProperty( "ExifTools", QVariant::fromValue<QgsExifTools>( mExifTools ) );
  rootContext()->setContextProperty( "LocatorModelNoGroup", QgsLocatorModel::NoGroup );
  rootContext()->setContextProperty( "gpkgFlusher", mGpkgFlusher.get() );

  rootContext()->setContextProperty( "qfieldAuthRequestHandler", mAuthRequestHandler );

  rootContext()->setContextProperty( "trackingModel", mTrackingModel );

  addImageProvider( QLatin1String( "legend" ), mLegendImageProvider );
}

void QgisMobileapp::loadProjectQuirks()
{
  // force update of canvas, without automatic changes to extent and OTF projections
  bool autoEnableCrsTransform = mLayerTreeCanvasBridge->autoEnableCrsTransform();
  bool autoSetupOnFirstLayer = mLayerTreeCanvasBridge->autoSetupOnFirstLayer();
  mLayerTreeCanvasBridge->setAutoEnableCrsTransform( false );
  mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( false );

  mLayerTreeCanvasBridge->setCanvasLayers();

  if ( autoEnableCrsTransform )
    mLayerTreeCanvasBridge->setAutoEnableCrsTransform( true );

  if ( autoSetupOnFirstLayer )
    mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( true );
}

void QgisMobileapp::removeRecentProject( const QString &path )
{
  QList<QPair<QString, QString>> projects = recentProjects();
  for ( int idx = 0; idx < projects.count(); idx++ )
  {
    if ( projects.at( idx ).second == path )
    {
      projects.removeAt( idx );
      break;
    }
  }
  saveRecentProjects( projects );
}

QList<QPair<QString, QString>> QgisMobileapp::recentProjects()
{
  QSettings settings;
  QList<QPair<QString, QString>> projects;

  settings.beginGroup( "/qgis/recentProjects" );
  const QStringList projectKeysList = settings.childGroups();
  QList<int> projectKeys;
  // This is overdoing it since we're clipping the recent projects list to five items at the moment, but might as well be futureproof
  for ( const QString &key : projectKeysList )
  {
    projectKeys.append( key.toInt() );
  }
  for ( int i = 0; i < projectKeys.count(); i++ )
  {
    settings.beginGroup( QString::number( projectKeys.at( i ) ) );
    projects << qMakePair( settings.value( QStringLiteral( "title" ) ).toString(), settings.value( QStringLiteral( "path" ) ).toString() );
    settings.endGroup();
  }
  settings.endGroup();
  return projects;
}

void QgisMobileapp::saveRecentProjects( QList<QPair<QString, QString>> &projects )
{
  QSettings settings;
  settings.remove( QStringLiteral( "/qgis/recentProjects" ) );
  for ( int idx = 0; idx < projects.count() && idx < 5; idx++ )
  {
    settings.beginGroup( QStringLiteral( "/qgis/recentProjects/%1" ).arg( idx ) );
    settings.setValue( QStringLiteral( "title" ), projects.at( idx ).first );
    settings.setValue( QStringLiteral( "path" ), projects.at( idx ).second );
    settings.endGroup();
  }
}

void QgisMobileapp::onReadProject( const QDomDocument &doc )
{
  Q_UNUSED( doc )
  QMap<QgsVectorLayer *, QgsFeatureRequest> requests;

  const QList<QgsMapLayer *> mapLayers { mProject->mapLayers().values() };
  for ( QgsMapLayer *layer : mapLayers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( vl )
    {
      const QVariant itinerary = vl->customProperty( "qgisMobile/itinerary" );
      if ( itinerary.isValid() )
      {
        requests.insert( vl, QgsFeatureRequest().setFilterExpression( itinerary.toString() ) );
      }
    }
  }
  if ( requests.count() )
  {
    qDebug() << QString( "Loading itinerary for %1 layers." ).arg( requests.count() );
    mIface->openFeatureForm();
  }
}

void QgisMobileapp::onAfterFirstRendering()
{
  // This should get triggered exactly once, so we disconnect it right away
  // disconnect( this, &QgisMobileapp::afterRendering, this, &QgisMobileapp::onAfterFirstRendering );

  qFatal( "boom" );
  QObject *obj = nullptr;
  obj->deleteLater();
  if ( mFirstRenderingFlag )
  {
    if ( qApp->arguments().count() > 1 )
    {
      loadProjectFile( qApp->arguments().last() );
    }
    else if ( !PlatformUtilities::instance()->qgsProject().isNull() )
    {
      PlatformUtilities::instance()->checkWriteExternalStoragePermissions();
      loadProjectFile( PlatformUtilities::instance()->qgsProject() );
    }
    mFirstRenderingFlag = false;
  }
}

void QgisMobileapp::loadLastProject()
{
  QVariant lastProjectFile = QSettings().value( "/qgis/recentProjects/0/path" );
  if ( lastProjectFile.isValid() )
    loadProjectFile( lastProjectFile.toString() );
}

void QgisMobileapp::loadProjectFile( const QString &path, const QString &name )
{
  QFileInfo fi( path );
  if ( !fi.exists() )
    QgsMessageLog::logMessage( tr( "Project file \"%1\" does not exist" ).arg( path ), QStringLiteral( "QField" ), Qgis::Warning );

  const QString suffix = fi.suffix().toLower();
  if ( SUPPORTED_PROJECT_EXTENSIONS.contains( suffix ) ||
       SUPPORTED_VECTOR_EXTENSIONS.contains( suffix ) ||
       SUPPORTED_RASTER_EXTENSIONS.contains( suffix ) )
  {
    mAuthRequestHandler->clearStoredRealms();

    mProjectFilePath = path;
    mProjectFileName = !name.isEmpty() ? name : fi.completeBaseName();

    emit loadProjectTriggered( mProjectFilePath, mProjectFileName );
  }
}

void QgisMobileapp::reloadProjectFile()
{
  if ( mProjectFilePath.isEmpty() )
    QgsMessageLog::logMessage( tr( "No project file currently opened" ), QStringLiteral( "QField" ), Qgis::Warning );

  emit loadProjectTriggered( mProjectFilePath, mProjectFileName );
}

void QgisMobileapp::readProjectFile()
{
  QFileInfo fi( mProjectFilePath );
  if ( !fi.exists() )
    QgsMessageLog::logMessage( tr( "Project file \"%1\" does not exist" ).arg( mProjectFilePath ), QStringLiteral( "QField" ), Qgis::Warning );

  const QString suffix = fi.suffix().toLower();

  mProject->removeAllMapLayers();
  mTrackingModel->reset();

  // Load project file
  if ( SUPPORTED_PROJECT_EXTENSIONS.contains( suffix ) )
  {
    mProject->read( mProjectFilePath );

    // load fonts in same directory
    QDir fontDir = QDir::cleanPath( QFileInfo( mProjectFilePath ).absoluteDir().path() + QDir::separator() + ".fonts" );
    QStringList fontExts = QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF";
    const QStringList fontFiles = fontDir.entryList( fontExts, QDir::Files );
    for ( const QString &fontFile : fontFiles )
    {
      int id = QFontDatabase::addApplicationFont( QDir::cleanPath( fontDir.path() + QDir::separator() + fontFile ) );
      if ( id < 0 )
        QgsMessageLog::logMessage( tr( "Could not load font %1" ).arg( fontFile ) );
      else
        QgsMessageLog::logMessage( tr( "Loading font %1" ).arg( fontFile ) );
    }
  }
  else
  {
    if ( QFile::exists( PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "basemap.qgs" ) ) )
    {
      mProject->read( PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "basemap.qgs" ) );
    }
    else if ( QFile::exists( PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "basemap.qgz" ) ) )
    {
      mProject->read( PlatformUtilities::instance()->qfieldDataDir() + QStringLiteral( "basemap.qgz" ) );
    }
    else
    {
      mProject->clear();
    }
    mProject->setTitle( mProjectFileName );
  }

  QList<QPair<QString, QString>> projects = recentProjects();
  QPair<QString, QString> project = qMakePair( mProject->title().isEmpty() ? mProjectFileName : mProject->title(), mProjectFilePath );
  if ( projects.contains( project ) )
    projects.removeAt( projects.indexOf( project ) );
  projects.insert( 0, project );
  saveRecentProjects( projects );

  QList<QgsMapLayer *> vectorLayers;
  QList<QgsMapLayer *> rasterLayers;
  QgsCoordinateReferenceSystem crs;
  QgsRectangle extent;

  // Load vector dataset
  if ( SUPPORTED_VECTOR_EXTENSIONS.contains( suffix ) )
  {
    QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    options.loadDefaultStyle = true;

    QgsVectorLayer *layer = new QgsVectorLayer( mProjectFilePath, mProjectFileName, QLatin1String( "ogr" ), options );
    if ( layer->isValid() )
    {
      const QStringList sublayers = layer->dataProvider()->subLayers();
      if ( sublayers.count() > 1 )
      {
        for ( const QString &sublayerInfo : sublayers )
        {
          const QStringList info = sublayerInfo.split( QgsDataProvider::sublayerSeparator() );
          QgsVectorLayer *sublayer = new QgsVectorLayer( QStringLiteral( "%1|layerid=%2" ).arg( mProjectFilePath, info.at( 0 ) ),
              QStringLiteral( "%1: %2" ).arg( mProjectFileName, info.at( 1 ) ),
              QLatin1String( "ogr" ), options );
          if ( sublayer->isValid() )
          {
            if ( sublayer->crs().isValid() )
            {
              if ( !crs.isValid() )
                crs = sublayer->crs();

              if ( !sublayer->extent().isEmpty() )
              {
                if ( crs != sublayer->crs() )
                {
                  QgsCoordinateTransform transform( sublayer->crs(), crs, mProject->transformContext() );
                  if ( extent.isEmpty() )
                    extent = transform.transformBoundingBox( sublayer->extent() );
                  else
                    extent.combineExtentWith( transform.transformBoundingBox( sublayer->extent() ) );
                }
                else
                {
                  if ( extent.isEmpty() )
                    extent = sublayer->extent();
                  else
                    extent.combineExtentWith( sublayer->extent() );
                }
              }
            }
            vectorLayers << sublayer;
          }
          else
          {
            delete sublayer;
          }
        }
      }
      else
      {
        crs = layer->crs();
        extent = layer->extent();
        vectorLayers << layer;
      }

      for ( QgsMapLayer *l : vectorLayers )
      {
        QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( l );
        QgsSymbol *symbol = FeatureUtils::defaultSymbol( vlayer );
        if ( symbol )
        {
          QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( symbol );
          vlayer->setRenderer( renderer );
        }
      }

      if ( vectorLayers.size() > 1 )
      {
        std::sort( vectorLayers.begin(), vectorLayers.end(), []( QgsMapLayer * a, QgsMapLayer * b )
        {
          QgsVectorLayer *alayer = qobject_cast< QgsVectorLayer * >( a );
          QgsVectorLayer *blayer = qobject_cast< QgsVectorLayer * >( b );
          if ( alayer->geometryType() == QgsWkbTypes::PointGeometry && blayer->geometryType() != QgsWkbTypes::PointGeometry )
          {
            return true;
          }
          else if ( alayer->geometryType() == QgsWkbTypes::LineGeometry && blayer->geometryType() == QgsWkbTypes::PolygonGeometry )
          {
            return true;
          }
          else
          {
            return false;
          }
        } );
      }
    }
    else
    {
      delete layer;
    }
  }

  // Load raster dataset
  if ( SUPPORTED_RASTER_EXTENSIONS.contains( suffix ) )
  {
    QgsRasterLayer *layer = new QgsRasterLayer( mProjectFilePath, mProjectFileName, QLatin1String( "gdal" ) );
    if ( layer->isValid() )
    {
      const QStringList sublayers = layer->dataProvider()->subLayers();
      if ( sublayers.count() > 1 )
      {
        for ( const QString &sublayerInfo : sublayers )
        {
          const QStringList info = sublayerInfo.split( QgsDataProvider::sublayerSeparator() );
          QgsRasterLayer *sublayer = new QgsRasterLayer( QStringLiteral( "%1|layerid=%2" ).arg( mProjectFilePath, info.at( 0 ) ),
              QStringLiteral( "%1: %2" ).arg( mProjectFileName, info.at( 1 ) ),
              QLatin1String( "gdal" ) );
          if ( sublayer->isValid() )
          {
            if ( sublayer->crs().isValid() )
            {
              if ( !crs.isValid() )
                crs = sublayer->crs();

              if ( !sublayer->extent().isEmpty() )
              {
                if ( crs != sublayer->crs() )
                {
                  QgsCoordinateTransform transform( sublayer->crs(), crs, mProject->transformContext() );
                  if ( extent.isEmpty() )
                    extent = transform.transformBoundingBox( sublayer->extent() );
                  else
                    extent.combineExtentWith( transform.transformBoundingBox( sublayer->extent() ) );
                }
                else
                {
                  if ( extent.isEmpty() )
                    extent = sublayer->extent();
                  else
                    extent.combineExtentWith( sublayer->extent() );
                }
              }
            }
            rasterLayers << sublayer;
          }
          else
          {
            delete sublayer;
          }
        }
      }
      else
      {
        crs = layer->crs();
        extent = layer->extent();
        rasterLayers << layer;
      }

      // If the raster size is reasonably small, apply nicer resampling settings
      if ( fi.size() < 50000000 )
      {
        for ( QgsMapLayer *l : rasterLayers )
        {
          QgsRasterLayer *rlayer = qobject_cast< QgsRasterLayer * >( l );
          rlayer->resampleFilter()->setZoomedInResampler( new QgsBilinearRasterResampler() );
          rlayer->resampleFilter()->setZoomedOutResampler( new QgsBilinearRasterResampler() );
          rlayer->resampleFilter()->setMaxOversampling( 2.0 );
        }
      }
    }
  }

  if ( vectorLayers.size() > 0 || rasterLayers.size() > 0 )
  {
    if ( crs.isValid() )
      mProject->setCrs( crs );

    if ( mProject->fileName().isEmpty() )
    {
      // Add a default basemap
      QgsRasterLayer *layer = new QgsRasterLayer( QStringLiteral( "type=xyz&url=https://a.tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0&crs=EPSG3857" ), QStringLiteral( "OpenStreetMap" ), QLatin1String( "wms" ) );
      mProject->addMapLayers( QList<QgsMapLayer *>() << layer );
    }

    mProject->addMapLayers( rasterLayers );
    mProject->addMapLayers( vectorLayers );
    if ( suffix.compare( QLatin1String( "pdf" ) ) == 0 )
    {
      // GeoPDFs should have vector layers hidden by default
      for ( QgsMapLayer *layer : vectorLayers )
      {
        mProject->layerTreeRoot()->findLayer( layer->id() )->setItemVisibilityChecked( false );
      }
    }
  }

  loadProjectQuirks();

  // Restore last extent if present
  QSettings settings;
  const QStringList parts = settings.value( QStringLiteral( "/qgis/projectInfo/%1/extent" ).arg( mProjectFilePath ), QString() ).toString().split( '|' );
  if ( parts.size() == 4 &&
       ( SUPPORTED_PROJECT_EXTENSIONS.contains( fi.suffix().toLower() ) ||
         fi.size() == settings.value( QStringLiteral( "/qgis/projectInfo/%1/filesize" ).arg( mProjectFilePath ), 0 ).toLongLong() ) )
  {
    extent.setXMinimum( parts[0].toDouble() );
    extent.setXMaximum( parts[1].toDouble() );
    extent.setYMinimum( parts[2].toDouble() );
    extent.setYMaximum( parts[3].toDouble() );
  }

  if ( !extent.isEmpty() && extent.width() != 0.0 )
  {
    // Add a bit of buffer so elements don't touch the map edges
    emit setMapExtent( extent.buffered( extent.width() * 0.02 ) );
  }

  // Restored last map theme if present
  const QString mapTheme = settings.value( QStringLiteral( "/qgis/projectInfo/%1/maptheme" ).arg( mProjectFilePath ), QString() ).toString();
  if ( !mapTheme.isEmpty() )
    mFlatLayerTree->setMapTheme( mapTheme );

  emit loadProjectEnded( mProjectFilePath, mProjectFileName );
}

void QgisMobileapp::print( int layoutIndex )
{
  const QList<QgsPrintLayout *> projectLayouts( mProject->layoutManager()->printLayouts() );

  const auto &layoutToPrint = projectLayouts.at( layoutIndex );

  if ( layoutToPrint->pageCollection()->pageCount() == 0 )
    return;

  layoutToPrint->referenceMap()->zoomToExtent( mMapCanvas->mapSettings()->visibleExtent() );
  layoutToPrint->refresh();

  QString documentsLocation = QStringLiteral( "%1/QField" ).arg( QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) );
  QDir documentsDir( documentsLocation );
  if ( !documentsDir.exists() )
    documentsDir.mkpath( "." );
  const QString destination = documentsLocation  + '/' + layoutToPrint->name() + QStringLiteral( ".pdf" );

  QgsLayoutExporter::PdfExportSettings pdfSettings;
  pdfSettings.rasterizeWholeImage = layoutToPrint->customProperty( QStringLiteral( "rasterize" ), false ).toBool();
  pdfSettings.dpi = layoutToPrint->renderContext().dpi();
  pdfSettings.appendGeoreference = true;
  pdfSettings.exportMetadata = true;
  pdfSettings.simplifyGeometries = true;

  QgsLayoutExporter exporter = QgsLayoutExporter( layoutToPrint );
  exporter.exportToPdf( destination, pdfSettings );

  PlatformUtilities::instance()->open( destination );
}

bool QgisMobileapp::event( QEvent *event )
{
  if ( event->type() == QEvent::Close )
    QgsApplication::instance()->quit();

  return QQmlApplicationEngine::event( event );
}

QgisMobileapp::~QgisMobileapp()
{
  delete mOfflineEditing;
  mProject->removeAllMapLayers();
  // Reintroduce when created on the heap
  delete mProject;
  delete mAppMissingGridHandler;
}

