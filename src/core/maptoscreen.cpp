/***************************************************************************
              maptoscreen.cpp
               ----------------------------------------------------
              date                 : 22.08.2018
              copyright            : (C) 2018 by Denis Rouzaud
              email                : denis (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "maptoscreen.h"
#include "qgspoint.h"

MapToScreen::MapToScreen( QObject *parent ) :
  QObject( parent )
{
}

void MapToScreen::setMapSettings( MapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  if ( mMapSettings )
  {
    disconnect( mMapSettings, &MapSettings::extentChanged, this, &MapToScreen::transformPoint );
  }

  mMapSettings = mapSettings;

  connect( mMapSettings, &MapSettings::extentChanged, this, &MapToScreen::transformPoint );

  transformPoint();

  emit mapSettingsChanged();
}

MapSettings *MapToScreen::mapSettings()
{
  return mMapSettings;
}

void MapToScreen::setMapPoint( const QgsPoint &point )
{
  if( mMapPoint == point )
    return;

  mMapPoint = point;
  emit mapPointChanged();
  transformPoint();
}

QgsPoint MapToScreen::mapPoint()
{
  return  mMapPoint;
}

QPointF MapToScreen::screenPoint()
{
  return mScreenPoint;
}

void MapToScreen::transformPoint()
{
  if ( !mMapSettings )
  {
    mScreenPoint = QPointF();
  }
  else
  {
    mScreenPoint = mMapSettings->coordinateToScreen( mMapPoint );
  }
  emit screenPointChanged();
}


