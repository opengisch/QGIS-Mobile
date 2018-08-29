/***************************************************************************
  rubberbandmodel.cpp - RubberbandModel

 ---------------------
 begin                : 10.6.2016
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
#include "rubberbandmodel.h"
#include <qgsvectorlayer.h>
#include <qgsproject.h>

RubberbandModel::RubberbandModel( QObject* parent )
  : QObject( parent )
  , mCurrentCoordinateIndex( 0 )
  , mGeometryType( QgsWkbTypes::LineGeometry )
  , mLayer( nullptr )
{
  mPointList.insert( 0, QgsPoint() );
}

int RubberbandModel::vertexCount() const
{
  return mPointList.size();
}

bool RubberbandModel::isEmpty() const
{
  return mPointList.isEmpty();
}

QVector<QgsPoint> RubberbandModel::vertices() const
{
  return mPointList;
}

QVector<QgsPoint> RubberbandModel::flatVertices( bool skipCurrentPoint ) const
{
  QVector<QgsPoint> points;
  Q_FOREACH( const QgsPoint& pt, mPointList )
  {
    points << QgsPoint( pt );
  }
  if (skipCurrentPoint)
      points.remove( mCurrentCoordinateIndex );

  return points;
}

QgsPointSequence RubberbandModel::pointSequence( const QgsCoordinateReferenceSystem& crs, QgsWkbTypes::Type wkbType ) const
{
  QgsPointSequence sequence;

  QgsCoordinateTransform ct( mCrs, crs, QgsProject::instance()->transformContext() );

  Q_FOREACH( const QgsPoint& pt, mPointList )
  {
    QgsPoint p2( ct.transform( pt.x(), pt.y() ) );
    if ( QgsWkbTypes::hasZ( wkbType ) )
      p2.setZ( pt.z() );
    sequence.append( p2 );
  }

  return sequence;
}

QVector<QgsPointXY> RubberbandModel::flatPointSequence( const QgsCoordinateReferenceSystem& crs ) const
{
  QVector<QgsPointXY> sequence;

  QgsCoordinateTransform ct( mCrs, crs, QgsProject::instance()->transformContext() );

  Q_FOREACH( const QgsPoint& pt, mPointList )
  {
    sequence.append( ct.transform( pt.x(), pt.y() ) );
  }

  return sequence;
}

void RubberbandModel::setVertex( int index, QgsPoint coordinate )
{
  if ( mPointList.at( index ) != coordinate )
  {
    mPointList.replace( index, coordinate );
    emit vertexChanged( index );
  }
}

void RubberbandModel::insertVertices( int index, int count )
{
  for ( int i = 0; i < count; ++i )
  {
    mPointList.insert( index, currentCoordinate() );
  }

  emit verticesInserted( index, count );
  emit vertexCountChanged();
}

void RubberbandModel::removeVertices( int index, int count )
{
  if ( mPointList.size() == 1 )
    return;

  mPointList.remove( index, count );
  emit verticesRemoved( index, count );
  emit vertexCountChanged();

  if ( mCurrentCoordinateIndex >= mPointList.size() )
  {
    setCurrentCoordinateIndex( mPointList.size() - 1 );
    emit currentCoordinateChanged();
  }
}

int RubberbandModel::currentCoordinateIndex() const
{
  return mCurrentCoordinateIndex;
}

void RubberbandModel::setCurrentCoordinateIndex( int currentCoordinateIndex )
{
  if ( currentCoordinateIndex == mCurrentCoordinateIndex )
    return;

  mCurrentCoordinateIndex = currentCoordinateIndex;
  emit currentCoordinateIndexChanged();
  emit currentCoordinateChanged();
}

QgsPoint RubberbandModel::currentPoint( const QgsCoordinateReferenceSystem& crs, QgsWkbTypes::Type wkbType ) const
{
  QgsCoordinateTransform ct( mCrs, crs, QgsProject::instance()->transformContext() );

  QgsPoint currentPt = mPointList.at( mCurrentCoordinateIndex );
  double x = currentPt.x();
  double y = currentPt.y();
  double z = QgsWkbTypes::hasZ( currentPt.wkbType() ) ? currentPt.z() : 0;

  ct.transformInPlace( x, y, z );

  QgsPoint resultPt( x, y );
  if ( QgsWkbTypes::hasZ( currentPt.wkbType() ) && QgsWkbTypes::hasZ( wkbType ) )
    resultPt.addZValue( z );

  return resultPt;
}

QgsPoint RubberbandModel::currentCoordinate() const
{
  return mPointList.at( mCurrentCoordinateIndex );
}

void RubberbandModel::setCurrentCoordinate( const QgsPoint& currentCoordinate )
{
  if ( mPointList.at( mCurrentCoordinateIndex ) == currentCoordinate )
    return;

  if (mFrozen)
    return;

  mPointList.replace( mCurrentCoordinateIndex, currentCoordinate );
  emit currentCoordinateChanged();
  emit vertexChanged( mCurrentCoordinateIndex );
}

void RubberbandModel::addVertex()
{
  // Avoid double vertices accidentally
  if ( mPointList.size() > 1 && *( mPointList.end() - 1 ) == *( mPointList.end() - 2 ) )
    return;

  insertVertices( mCurrentCoordinateIndex + 1, 1 );
  setCurrentCoordinateIndex( mCurrentCoordinateIndex + 1 );
}

void RubberbandModel::removeVertex()
{
  setCurrentCoordinateIndex( mCurrentCoordinateIndex - 1 );
  removeVertices( mCurrentCoordinateIndex + 1, 1 );
}

void RubberbandModel::reset()
{
  removeVertices( 0, mPointList.size() - 1 );
  mFrozen = false;
  emit frozenChanged();
}

QgsWkbTypes::GeometryType RubberbandModel::geometryType() const
{
  return mGeometryType;
}

void RubberbandModel::setGeometryType( const QgsWkbTypes::GeometryType& geometryType )
{
  if ( mGeometryType == geometryType )
    return;

  mGeometryType = geometryType;
  emit geometryTypeChanged();
}

QgsCoordinateReferenceSystem RubberbandModel::crs() const
{
  return mCrs;
}

void RubberbandModel::setCrs( const QgsCoordinateReferenceSystem& crs )
{
  if ( crs == mCrs )
    return;

  mCrs = crs;
  emit crsChanged();
}

QgsVectorLayer* RubberbandModel::vectorLayer() const
{
  return mLayer;
}

void RubberbandModel::setVectorLayer( QgsVectorLayer* layer )
{
  if ( layer == mLayer )
    return;

  mLayer = layer;

  if ( mLayer )
    setGeometryType( mLayer->geometryType() );

  emit vectorLayerChanged();
}


bool RubberbandModel::frozen() const
{
  return mFrozen;
}

void RubberbandModel::setFrozen(const bool &frozen )
{
  if ( mFrozen == frozen )
    return;

  mFrozen = frozen;

  emit frozenChanged();
}
