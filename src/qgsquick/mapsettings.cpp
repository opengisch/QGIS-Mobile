/***************************************************************************
  mapsettings.cpp
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qgsmaplayer.h>
#include <qgsproject.h>
#include <qgsmessagelog.h>

#include <qgsmaplayerstylemanager.h>

#include "mapsettings.h"

MapSettings::MapSettings( QObject *parent )
  : QObject( parent )
{
  // Connect signals for derived values
  connect( this, &MapSettings::destinationCrsChanged, this, &MapSettings::mapUnitsPerPixelChanged );
  connect( this, &MapSettings::destinationCrsChanged, this, &MapSettings::abbreviatedStringOfDistanceUnitChanged );
  connect( this, &MapSettings::extentChanged, this, &MapSettings::mapUnitsPerPixelChanged );
  connect( this, &MapSettings::outputSizeChanged, this, &MapSettings::mapUnitsPerPixelChanged );
  connect( this, &MapSettings::extentChanged, this, &MapSettings::visibleExtentChanged );
  connect( this, &MapSettings::rotationChanged, this, &MapSettings::visibleExtentChanged );
  connect( this, &MapSettings::outputSizeChanged, this, &MapSettings::visibleExtentChanged );
  connect( QgsProject::instance(), &QgsProject::readProject, this, &MapSettings::onReadProject );
}

QgsCoordinateTransformContext MapSettings::transformContext() const
{
  return mMapSettings.transformContext();
}

QgsRectangle MapSettings::extent() const
{
  return mMapSettings.extent();
}

void MapSettings::setExtent( const QgsRectangle &extent )
{
  if ( mMapSettings.extent() == extent )
    return;

  mMapSettings.setExtent( extent );
  emit extentChanged();
}

void MapSettings::setCenter( const QgsPoint &center )
{
  QgsVector delta = QgsPointXY( center ) - mMapSettings.extent().center();

  QgsRectangle e = mMapSettings.extent();
  e.setXMinimum( e.xMinimum() + delta.x() );
  e.setXMaximum( e.xMaximum() + delta.x() );
  e.setYMinimum( e.yMinimum() + delta.y() );
  e.setYMaximum( e.yMaximum() + delta.y() );

  setExtent( e );
}

double MapSettings::mapUnitsPerPixel() const
{
  return mMapSettings.mapUnitsPerPixel();
}

QgsRectangle MapSettings::visibleExtent() const
{
  return mMapSettings.visibleExtent();
}

QPointF MapSettings::coordinateToScreen( const QgsPoint &point ) const
{
  QgsPointXY pt( point.x(), point.y() );
  QgsPointXY pp = mMapSettings.mapToPixel().transform( pt );
  return pp.toQPointF();
}

QgsPoint MapSettings::screenToCoordinate( const QPointF &point ) const
{
  const QgsPointXY pp = mMapSettings.mapToPixel().toMapCoordinatesF( point.x(), point.y() );
  return QgsPoint( pp );
}

QgsMapSettings MapSettings::mapSettings() const
{
  return mMapSettings;
}

QSize MapSettings::outputSize() const
{
  return mMapSettings.outputSize();
}

void MapSettings::setOutputSize( const QSize &outputSize )
{
  if ( mMapSettings.outputSize() == outputSize )
    return;

  mMapSettings.setOutputSize( outputSize );
  emit outputSizeChanged();
}

double MapSettings::outputDpi() const
{
  return mMapSettings.outputDpi();
}

void MapSettings::setOutputDpi( double outputDpi )
{
  if ( mMapSettings.outputDpi() == outputDpi )
    return;

  mMapSettings.setOutputDpi( outputDpi );
  emit outputDpiChanged();
}

QgsCoordinateReferenceSystem MapSettings::destinationCrs() const
{
  return mMapSettings.destinationCrs();
}

void MapSettings::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( mMapSettings.destinationCrs() == destinationCrs )
    return;

  mMapSettings.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
}

QList<QgsMapLayer *> MapSettings::layers() const
{
  return mMapSettings.layers();
}

void MapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  mMapSettings.setLayers( layers );
  emit layersChanged();
}

QString MapSettings::abbreviatedStringOfDistanceUnit() const
{
  return QgsUnitTypes::toAbbreviatedString( mMapSettings.destinationCrs().mapUnits() );
}

#if 0
void MapSettings::setMapTheme( QgsProject* project, const QString& mapThemeName )
{
  QStringList layerIds;

  QgsMapThemeCollection::MapThemeRecord mapTheme = project->mapThemeCollection()->mapThemeState( mapThemeName );

  Q_FOREACH( const QgsMapThemeCollection::MapThemeLayerRecord& record, mapTheme.layerRecords() )
  {
    record.layer()->styleManager()->setCurrentStyle( mapTheme.perLayerCurrentStyle().value( layerId ) );

    layerIds << layerId;
  }
}

mMapSettings.setLayers( layerIds );
emit layersChanged();
}
#endif

void MapSettings::onReadProject( const QDomDocument &doc )
{
  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );

    mMapSettings.readXml( node );

    if ( mMapSettings.rotation() != 0 )
      QgsMessageLog::logMessage( tr( "Map Canvas rotation is not supported. Resetting from %1 to 0." ).arg( mMapSettings.rotation() ) );

    mMapSettings.setRotation( 0 );

    emit extentChanged();
    emit destinationCrsChanged();
    emit outputSizeChanged();
    emit outputDpiChanged();
    emit layersChanged();
  }
}

double MapSettings::rotation() const
{
  return mMapSettings.rotation();
}

void MapSettings::setRotation( double rotation )
{
  if ( rotation == mMapSettings.rotation() )
    return;

  mMapSettings.setRotation( rotation );
  emit rotationChanged();
}
