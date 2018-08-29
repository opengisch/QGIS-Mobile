/***************************************************************************
  distancearea.cpp - DistanceArea

 ---------------------
 begin                : 23.2.2017
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
#include "distancearea.h"
#include "geometry.h"
#include "rubberband.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

DistanceArea::DistanceArea( QObject* parent )
  : QObject( parent )
  , mRubberbandModel( nullptr )
  , mProject( nullptr )
{
}

void DistanceArea::init()
{
  if ( mProject )
    mDistanceArea.setEllipsoid( mProject->ellipsoid() );
  else
    mDistanceArea.setEllipsoid( GEO_NONE );
  mDistanceArea.setSourceCrs( mCrs, mProject->transformContext() );

  emit lengthUnitsChanged();
  emit areaUnitsChanged();
}

QgsProject* DistanceArea::project() const
{
  return mProject;
}

void DistanceArea::setProject( QgsProject* project )
{
  if ( mProject == project )
    return;

  if ( mProject )
    disconnect( mProject, &QgsProject::readProject, this, &DistanceArea::init );

  mProject = project;

  if ( mProject )
    connect( mProject, &QgsProject::readProject, this, &DistanceArea::init );

  init();

  emit projectChanged();
}

RubberbandModel* DistanceArea::rubberbandModel() const
{
  return mRubberbandModel;
}

void DistanceArea::setRubberbandModel( RubberbandModel* rubberbandModel )
{
  if ( mRubberbandModel == rubberbandModel )
    return;

  if ( mRubberbandModel )
  {
    disconnect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::lengthChanged );
    disconnect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::areaChanged );
    disconnect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::segmentLengthChanged );
    disconnect( mRubberbandModel, &RubberbandModel::vertexCountChanged, this, &DistanceArea::areaValidChanged );
    disconnect( mRubberbandModel, &RubberbandModel::vertexCountChanged, this, &DistanceArea::lengthValidChanged );
  }

  mRubberbandModel = rubberbandModel;

  if ( mRubberbandModel )
  {
    connect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::lengthChanged );
    connect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::areaChanged );
    connect( mRubberbandModel, &RubberbandModel::vertexChanged, this, &DistanceArea::segmentLengthChanged );
    connect( mRubberbandModel, &RubberbandModel::vertexCountChanged, this, &DistanceArea::areaValidChanged );
    connect( mRubberbandModel, &RubberbandModel::vertexCountChanged, this, &DistanceArea::lengthValidChanged );
  }

  emit rubberbandModelChanged();
}

QgsCoordinateReferenceSystem DistanceArea::crs() const
{
  return mCrs;
}

void DistanceArea::setCrs( const QgsCoordinateReferenceSystem& crs )
{
  if ( mCrs == crs )
    return;

  mCrs = crs;
  init();
  emit crsChanged();
}
#if 0
void DistanceArea::setGeometry( Geometry* geometry )
{
  if ( mGeometry == geometry )
    return;

  if ( mGeometry )
    disconnect( mGeometry, &Geometry::rubberbandModelChanged, this, &DistanceArea::rubberbandModelChanged );

  mGeometry = geometry;
  rubberbandModelChanged();

  if ( mGeometry )
    connect( mGeometry, &Geometry::rubberbandModelChanged, this, &DistanceArea::rubberbandModelChanged );

  init();

  emit geometryChanged();
}
#endif

qreal DistanceArea::length() const
{
  if ( mRubberbandModel )
    return mDistanceArea.measureLine( mRubberbandModel->flatPointSequence( mCrs ) );

  return qQNaN();
}

bool DistanceArea::lengthValid() const
{
  if ( !mRubberbandModel )
    return false;

  switch ( mRubberbandModel->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      return false;

    case QgsWkbTypes::LineGeometry:
      FALLTHROUGH;
    case QgsWkbTypes::PolygonGeometry:
      return mRubberbandModel->vertexCount() >= 2;

    default:
      return false;
  }
}

qreal DistanceArea::area() const
{
  if ( mRubberbandModel )
    return mDistanceArea.measurePolygon( mRubberbandModel->flatPointSequence( mCrs ) );

  return qQNaN();
}

bool DistanceArea::areaValid() const
{
  if ( !mRubberbandModel )
    return false;

  switch ( mRubberbandModel->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      return false;

    case QgsWkbTypes::LineGeometry:
      return false;

    case QgsWkbTypes::PolygonGeometry:
      return mRubberbandModel->vertexCount() >= 3;

    default:
      return false;
  }
}

qreal DistanceArea::segmentLength() const
{
  if ( !mRubberbandModel )
    return qQNaN();

  if ( mRubberbandModel->vertexCount() < 2 )
    return qQNaN();

  QVector<QgsPointXY> points = mRubberbandModel->flatPointSequence( mCrs );

  auto pointIt = points.constEnd() - 1;

  QVector<QgsPointXY> flatPoints;

  flatPoints << *pointIt;
  pointIt--;
  flatPoints << *pointIt;

  return mDistanceArea.measureLine( flatPoints );
}

QgsUnitTypes::DistanceUnit DistanceArea::lengthUnits() const
{
  return mDistanceArea.lengthUnits();
}

QgsUnitTypes::AreaUnit DistanceArea::areaUnits() const
{
  return mDistanceArea.areaUnits();
}
