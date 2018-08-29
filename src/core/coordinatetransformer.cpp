/***************************************************************************
  coordinatetransformer.cpp - CoordinateTransformer

 ---------------------
 begin                : 1.6.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "coordinatetransformer.h"

#include <QtDebug>

CoordinateTransformer::CoordinateTransformer( QObject *parent )
  : QObject( parent )
{
  mCoordinateTransform.setSourceCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
}

QgsPoint CoordinateTransformer::projectedPosition() const
{
  return mProjectedPosition;
}

QGeoCoordinate CoordinateTransformer::sourcePosition() const
{
  return mSourcePosition;
}

void CoordinateTransformer::setSourcePosition( QGeoCoordinate sourcePosition )
{
#if 0
  float r = static_cast <float> ( rand() ) / static_cast <float> ( RAND_MAX ) / 100;
  float rd = static_cast <float> ( rand() ) / static_cast <float> ( RAND_MAX ) / 100;
  float rz = static_cast <float> ( rand() ) / static_cast <float> ( RAND_MAX ) * 100;

  sourcePosition.setLatitude( 46.9483 + r );
  sourcePosition.setLongitude( 7.44225 + rd );
  sourcePosition.setAltitude( 400 + rz );
#endif

  if ( mSourcePosition == sourcePosition )
    return;

  mSourcePosition = sourcePosition;

  emit sourcePositionChanged();
  updatePosition();
}

QgsCoordinateReferenceSystem CoordinateTransformer::destinationCrs() const
{
  return mCoordinateTransform.destinationCrs();
}

void CoordinateTransformer::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( destinationCrs == mCoordinateTransform.destinationCrs() )
    return;

  mCoordinateTransform.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
  updatePosition();
}

QgsCoordinateReferenceSystem CoordinateTransformer::sourceCrs() const
{
  return mCoordinateTransform.sourceCrs();
}

void CoordinateTransformer::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  if ( sourceCrs == mCoordinateTransform.sourceCrs() )
    return;

  mCoordinateTransform.setSourceCrs( sourceCrs );

  emit sourceCrsChanged();
  updatePosition();
}

void CoordinateTransformer::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mCoordinateTransform.setContext( context );
  emit transformContextChanged();
}

QgsCoordinateTransformContext CoordinateTransformer::transformContext() const
{
  return mCoordinateTransform.context();
}

void CoordinateTransformer::updatePosition()
{
  double x = mSourcePosition.longitude();
  double y = mSourcePosition.latitude();
  double z = mSourcePosition.altitude();

  // If Z is NaN, coordinate transformation (proj4) will
  // also set X and Y to NaN. But we also want to get projected
  // coords if we do not have any Z coordinate.
  if ( qIsNaN( z ) )
  {
    z = 0;
  }

  mCoordinateTransform.transformInPlace( x, y, z );

  mProjectedPosition = QgsPoint( x, y );
  mProjectedPosition.addZValue( mSourcePosition.altitude() );

  emit projectedPositionChanged();
}
