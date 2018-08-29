/***************************************************************************
  snappingresult.cpp - SnappingResult

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
#include "snappingresult.h"

SnappingResult::SnappingResult()
  : mType( Invalid )
  , mDist( 0 )
  , mPoint()
  , mLayer( nullptr )
  , mFid( 0 )
  , mVertexIndex( 0 )
{}

SnappingResult::SnappingResult( SnappingResult::Type t, QgsVectorLayer* vl, QgsFeatureId fid, double dist, const QgsPoint& pt, int vertexIndex, QgsPoint* edgePoints )
  : mType( t )
  , mDist( dist )
  , mPoint( pt )
  , mLayer( vl )
  , mFid( fid )
  , mVertexIndex( vertexIndex )
{
  if ( edgePoints )
  {
    mEdgePoints[0] = edgePoints[0];
    mEdgePoints[1] = edgePoints[1];
  }
}

SnappingResult::SnappingResult( const QgsPointLocator::Match& match )
  : mType( matchTypeToSnappingResultType( match.type() ) )
  , mDist( match.distance() )
  , mPoint( match.point() )
  , mLayer( match.layer() )
  , mFid( match.featureId() )
  , mVertexIndex( match.vertexIndex() )
{
}

SnappingResult::Type SnappingResult::type() const
{
  return mType;
}

bool SnappingResult::isValid() const
{
  return mType != Invalid;
}

bool SnappingResult::hasVertex() const
{
  return mType == Vertex;
}

bool SnappingResult::hasEdge() const
{
  return mType == Edge;
}

bool SnappingResult::hasArea() const
{
  return mType == Area;
}

double SnappingResult::distance() const
{
  return mDist;
}

QgsPoint SnappingResult::point() const
{
  return mPoint;
}

int SnappingResult::vertexIndex() const
{
  return mVertexIndex;
}

QgsVectorLayer*SnappingResult::layer() const
{
  return mLayer;
}

QgsFeatureId SnappingResult::featureId() const
{
  return mFid;
}

void SnappingResult::edgePoints( QgsPoint& pt1, QgsPoint& pt2 ) const
{
  pt1 = mEdgePoints[0];
  pt2 = mEdgePoints[1];
}

SnappingResult::Type SnappingResult::matchTypeToSnappingResultType( QgsPointLocator::Type type )
{
  return static_cast<Type>( type );
}
