/***************************************************************************
  snappingutils.cpp

 ---------------------
 begin                : 8.10.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snappingutils.h"
#include "mapsettings.h"

#include "qgsvectorlayer.h"

SnappingUtils::SnappingUtils( QObject* parent )
  : QgsSnappingUtils( parent )
  , mSettings( nullptr )
{
}

void SnappingUtils::onMapSettingsUpdated()
{
  QgsSnappingUtils::setMapSettings( mSettings->mapSettings() );

  snap();
}

void SnappingUtils::snap()
{
  QgsPointLocator::Match match = snapToMap( QPoint( mInputCoordinate.x(), mInputCoordinate.y() ) );
  mSnappingResult = SnappingResult( match );

  emit snappingResultChanged();
}

QPointF SnappingUtils::inputCoordinate() const
{
  return mInputCoordinate;
}

void SnappingUtils::setInputCoordinate( const QPointF& inputCoordinate )
{
  if ( inputCoordinate == mInputCoordinate )
    return;

  mInputCoordinate = inputCoordinate;

  snap();

  emit inputCoordinateChanged();
}

SnappingResult SnappingUtils::snappingResult() const
{
  return mSnappingResult;
}

void SnappingUtils::prepareIndexStarting( int count )
{
  mIndexLayerCount = count;
  emit indexingStarted( count );
}

void SnappingUtils::prepareIndexProgress( int index )
{
  if ( index == mIndexLayerCount )
    emit indexingFinished();
  else
    emit indexingProgress( index );
}

QgsVectorLayer* SnappingUtils::currentLayer() const
{
  return mCurrentLayer;
}

void SnappingUtils::setCurrentLayer( QgsVectorLayer* currentLayer )
{
  if ( currentLayer == mCurrentLayer )
    return;

  mCurrentLayer = currentLayer;
  QgsSnappingUtils::setCurrentLayer( currentLayer );

  emit currentLayerChanged();
}

MapSettings* SnappingUtils::mapSettings() const
{
  return mSettings;
}

void SnappingUtils::setMapSettings( MapSettings* settings )
{
  if ( mSettings == settings )
    return;

  connect( settings, &MapSettings::extentChanged, this, &SnappingUtils::onMapSettingsUpdated );
  connect( settings, &MapSettings::destinationCrsChanged, this, &SnappingUtils::onMapSettingsUpdated );
  connect( settings, &MapSettings::layersChanged, this, &SnappingUtils::onMapSettingsUpdated );

  mSettings = settings;
  emit mapSettingsChanged();
}
