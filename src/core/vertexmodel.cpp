/***************************************************************************
  vertexmodel.h - VertexModel

 ---------------------
 begin                : 18.08.2018
 copyright            : (C) 2018 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include <QStandardItem>

#include <qgsgeometry.h>
#include <qgswkbtypes.h>
#include <qgslinestring.h>
#include <qgspolygon.h>

#include "vertexmodel.h"
#include "mapsettings.h"


VertexModel::VertexModel( QObject *parent )
  : QStandardItemModel( parent )
{
  connect( this, &VertexModel::editingModeChanged, this, &VertexModel::updateCanRemoveVertex );
  connect( this, &VertexModel::vertexCountChanged, this, &VertexModel::updateCanRemoveVertex );
  connect( this, &VertexModel::vertexCountChanged, this, &VertexModel::updateCanPreviousNextVertex );
}

void VertexModel::setMapSettings( MapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  mMapSettings = mapSettings;

  emit mapSettingsChanged();
}

MapSettings *VertexModel::mapSettings()
{
  return mMapSettings;
}

void VertexModel::setGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs )
{
  clear();
  mOriginalGeoemtry = geometry;
  mCurrentIndex = -1;
  mGeometryType = geometry.type();
  QgsGeometry geom = QgsGeometry( geometry );

  if ( mMapSettings )
  {
    mTransform = QgsCoordinateTransform( crs, mMapSettings->destinationCrs(), mMapSettings->transformContext() );
    if ( mTransform.isValid() )
      geom.transform( mTransform );
  }

  mIsMulti = QgsWkbTypes::isMultiType( geometry.wkbType() );

  setColumnCount( 1 );
  setRowCount( 0 );

  const QgsAbstractGeometry *abstractGeom = geom.constGet();
  if ( !abstractGeom )
    return;

  QgsVertexId vertexId;
  QgsPoint pt;
  int r = 0;
  while ( abstractGeom->nextVertex( vertexId, pt ) )
  {
    if ( vertexId.part > 1 || vertexId.ring > 1 )
      break;

    // skip first vertex on polygon, as it's duplicate of the last one
    if ( geometry.type() == QgsWkbTypes::PolygonGeometry && vertexId.vertex == 0 )
      continue;

    QStandardItem *item = new QStandardItem();
    item->setData( QVariant::fromValue<QgsPoint>( pt ), PointRole );
    item->setData( r == mCurrentIndex, CurrentVertexRole );
    appendRow( QList<QStandardItem *>() << item );
    r++;
  }

  setDirty( false );

  emit vertexCountChanged();

  // for points, enable the editing mode directly
  if ( mGeometryType == QgsWkbTypes::PointGeometry )
    setCurrentVertex( 0 );

  updateCanAddVertex();
}

QgsGeometry VertexModel::geometry() const
{
  if ( mIsMulti )
  {
    // TODO: handle multi, for now return original to avoid any data destruction
    return mOriginalGeoemtry;
  }

  QVector<QgsPoint> vertices = flatVertices();
  QgsGeometry geometry;

  switch ( mGeometryType )
  {
    case QgsWkbTypes::PointGeometry:
    {
      geometry.set( new QgsPoint( vertices.first() ) );
      break;
    }
    case QgsWkbTypes::LineGeometry:
    {
      geometry = QgsGeometry::fromPolyline( vertices );
      break;
    }
    case QgsWkbTypes::PolygonGeometry:
    {
      std::unique_ptr<QgsLineString> ls( qgis::make_unique<QgsLineString>( vertices ) );
      std::unique_ptr<QgsPolygon> polygon( qgis::make_unique<QgsPolygon>() );
      polygon->setExteriorRing( ls.release() );
      geometry.set( polygon.release() );
      break;
    }
    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      break;
  }

  if ( mTransform.isValid() )
    geometry.transform( mTransform, QgsCoordinateTransform::ReverseTransform );

  return geometry;
}

void VertexModel::clear()
{
  setEditingMode( NoEditing );
  QStandardItemModel::clear();
  updateCanRemoveVertex();
  updateCanAddVertex();
  emit vertexCountChanged();
  setDirty( false );
}

void VertexModel::previous()
{
  if ( !mCanPreviousVertex )
    return;
  setCurrentVertex( mCurrentIndex == -1 ? rowCount() - 1 : mCurrentIndex - 1 );
}

void VertexModel::next()
{
  if ( !mCanNextVertex )
    return;

  setCurrentVertex( mCurrentIndex == -1 ? 0 : mCurrentIndex + 1 );
}

void VertexModel::removeCurrentVertex()
{
  if ( !mCanRemoveVertex )
    return;

  removeRow( mCurrentIndex );

  setDirty( true );

  emit vertexCountChanged();

  setCurrentVertex( mCurrentIndex, true );
}

VertexModel::EditingMode VertexModel::editingMode() const
{
  return mMode;
}

QgsPoint VertexModel::currentPoint() const
{
  QStandardItem *it = item( mCurrentIndex );
  if ( it )
    return it->data( PointRole ).value<QgsPoint>();
  else
    return QgsPoint();
}

void VertexModel::setCurrentPoint( const QgsPoint &point )
{
  if ( ( mMode == EditVertex || mMode == AddVertex ) && rowCount() > 0 )
  {
    QStandardItem *it = item( mCurrentIndex );
    if ( it )
    {
      bool pointHasChanged = it->data( PointRole ).value<QgsPoint>() != point;
      if ( pointHasChanged )
      {
        setDirty( true );
      }
      it->setData( QVariant::fromValue<QgsPoint>( point ), PointRole );
      emit currentPointChanged();
      if ( mMode == AddVertex && pointHasChanged )
      {
        it->setData( false, SegmentVertexRole );
        setEditingMode( EditVertex );
      }
    }
  }
}

void VertexModel::setCurrentVertex( int newVertex, bool forceUpdate )
{
  if ( newVertex < 0 )
    newVertex = mGeometryType == QgsWkbTypes::PolygonGeometry ? rowCount() - 1 : 0;

  if ( newVertex >= rowCount() )
    newVertex = mGeometryType == QgsWkbTypes::PolygonGeometry ? 0 : rowCount() - 1;


  if ( rowCount() == 0 )
  {
    setEditingMode( NoEditing );
    newVertex = -1;
  }

  if ( !forceUpdate && mCurrentIndex == newVertex )
    return;

  int oldVertex = mCurrentIndex;
  mCurrentIndex = newVertex;

  if ( mMode == AddVertex && oldVertex >= 0 )
  {
    bool goingForward = newVertex > oldVertex;
    bool isExtending = ( goingForward && newVertex == rowCount() - 1 ) || ( !goingForward && newVertex == 0 );
    int leftVertex, rightVertex;
    if ( goingForward )
    {
      leftVertex = oldVertex - ( isExtending ? 1 : 0 );
      rightVertex = newVertex - ( isExtending ? 1 : 0 );
    }
    else
    {
      leftVertex = newVertex - 1 + ( isExtending ? 1 : 0 );
      rightVertex = oldVertex - 1 + ( isExtending ? 1 : 0 );
    }
    QList<QStandardItem *> row = takeRow( oldVertex );
    Q_ASSERT( row.count() == 1 && row.at( 0 ) );
    QgsPoint point = segmentCentroid( leftVertex, rightVertex, isExtending ).point;
    row[0]->setData( QVariant::fromValue<QgsPoint>( point ), PointRole );
    insertRow( mCurrentIndex, row );
    emit currentPointChanged();
  }
  else
  {
    for ( int r = 0; r < rowCount(); r++ )
    {
      QStandardItem *it = item( r );
      it->setData( r == mCurrentIndex, CurrentVertexRole );

      if ( r == mCurrentIndex )
      {
        // following 2 lines must be in this order
        if ( mMode == NoEditing )
          setEditingMode( EditVertex );
        emit currentPointChanged();
      }
    }
  }

  updateCanPreviousNextVertex();
}

VertexModel::Centroid VertexModel::segmentCentroid( int leftIndex, int rightIndex, bool isExtending )
{
  Centroid centroid;

  QList<int> indexes = QList<int>() << leftIndex << rightIndex;
  qSort( indexes.begin(), indexes.end() );

  if ( indexes[1] - indexes[0] > 1 )
    indexes[0] = indexes[1] - 1;

  if ( indexes[0] < 0 )
  {
    if ( mGeometryType == QgsWkbTypes::LineGeometry )
    {
      indexes = QList<int>() << 0 << 1;
    }
    else
    {
      indexes = QList<int>() << rowCount() - 1 << 0;
    }
  }
  if ( indexes[1] > rowCount() - 1 )
  {
    if ( mGeometryType == QgsWkbTypes::LineGeometry )
    {
      indexes = QList<int>() << rowCount() - 2 << rowCount() - 1;
    }
    else
    {
      indexes = QList<int>() << 0 << rowCount() - 1;
    }
  }

  QVector<QgsPoint> points = QVector<QgsPoint>();
  for ( const int row : indexes )
  {
    QStandardItem *it = item( row );
    if ( it )
      points << it->data( PointRole ).value<QgsPoint>();
  }
  Q_ASSERT( points.count() == 2 );

  QgsLineString ls = QgsLineString( points );
  centroid.point = ls.centroid();
  centroid.index = indexes[1];

  if ( isExtending )
  {
    int idx = indexes[0] == 0 ? 0 : 1;
    centroid.point = points[idx] - ( centroid.point - points[idx] ) / 2;
  }

  return centroid;
}

int VertexModel::vertexCount() const
{
  return rowCount();
}

bool VertexModel::dirty() const
{
  return mDirty;
}

bool VertexModel::canRemoveVertex()
{
  return mCanRemoveVertex;
}

bool VertexModel::canAddVertex()
{
  return mCanAddVertex;
}

bool VertexModel::canPreviousVertex()
{
  return mCanPreviousVertex;
}

bool VertexModel::canNextVertex()
{
  return mCanNextVertex;
}

QgsWkbTypes::GeometryType VertexModel::geometryType() const
{
  return mGeometryType;
}

QVector<QgsPoint> VertexModel::flatVertices() const
{
  QVector<QgsPoint> vertices = QVector<QgsPoint>();
  for ( int r = 0; r < rowCount(); r++ )
  {
    QStandardItem *it = item( r );
    if ( it->data( SegmentVertexRole ).toBool() )
      continue;
    vertices << it->data( PointRole ).value<QgsPoint>();
  }
  // re-append
  if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
  {
    vertices << vertices.constFirst();
  }
  return vertices;
}

QHash<int, QByteArray> VertexModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PointRole] = "Point";
  roles[CurrentVertexRole] = "CurrentVertex";
  roles[SegmentVertexRole] = "SegmentVertex";
  return roles;
}

void VertexModel::setDirty( bool dirty )
{
  if ( mDirty == dirty )
    return;

  mDirty = dirty;
  emit dirtyChanged();
}

void VertexModel::updateCanRemoveVertex()
{
  bool canRemoveVertex = false;

  if ( mMode == EditVertex )
  {

    switch ( mGeometryType )
    {
      case QgsWkbTypes::PointGeometry:
        canRemoveVertex = false;
        break;
      case QgsWkbTypes::LineGeometry:
        canRemoveVertex =  rowCount() > 2;
        break;
      case QgsWkbTypes::PolygonGeometry:
        canRemoveVertex =  rowCount() > 3;
        break;
      case QgsWkbTypes::NullGeometry:
      case QgsWkbTypes::UnknownGeometry:
        canRemoveVertex =  false;
        break;
    }
  }

  if ( canRemoveVertex == mCanRemoveVertex )
    return;

  mCanRemoveVertex = canRemoveVertex;

  emit canRemoveVertexChanged();
}

void VertexModel::updateCanAddVertex()
{
  bool canAddVertex = vertexCount() > 0 && mGeometryType != QgsWkbTypes::PointGeometry;

  if ( canAddVertex == mCanAddVertex )
    return;

  mCanAddVertex = canAddVertex;
  emit canAddVertexChanged();
}

void VertexModel::updateCanPreviousNextVertex()
{
  bool canPrevious = false;
  bool canNext = false;

  switch ( mGeometryType )
  {
    case QgsWkbTypes::PointGeometry:
      // todo for multi
      break;
    case QgsWkbTypes::LineGeometry:
      canPrevious = mCurrentIndex > 0 || mMode == NoEditing;
      canNext = mCurrentIndex < rowCount() - 1 || mMode == NoEditing;
      break;
    case QgsWkbTypes::PolygonGeometry:
      canPrevious = true;
      canNext = true;
      break;
    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }

  if ( canPrevious != mCanPreviousVertex )
  {
    mCanPreviousVertex = canPrevious;
    emit canPreviousVertexChanged();
  }
  if ( canNext != mCanNextVertex )
  {
    mCanNextVertex = canNext;
    emit canNextVertexChanged();
  }
}

void VertexModel::setEditingMode( VertexModel::EditingMode mode )
{
  if ( !rowCount() )
    mode = NoEditing;

  if ( mGeometryType == QgsWkbTypes::PointGeometry && mode == AddVertex )
    mode = NoEditing;

  if ( mMode == mode )
    return;

  if ( mode == AddVertex )
  {
    switch ( mGeometryType )
    {
      case QgsWkbTypes::PointGeometry:
      {
        // should not happen for now
        break;
      }
      case QgsWkbTypes::LineGeometry:
      case QgsWkbTypes::PolygonGeometry:
      {
        Centroid centroid = segmentCentroid( mCurrentIndex, mCurrentIndex + 1, false );

        QStandardItem *item = new QStandardItem();
        item->setData( QVariant::fromValue<QgsPoint>( centroid.point ), PointRole );
        item->setData( true, SegmentVertexRole );
        insertRow( centroid.index, QList<QStandardItem *>() << item );
        emit vertexCountChanged();
        setCurrentVertex( centroid.index, true );
        emit currentPointChanged();
        break;
      }
      case QgsWkbTypes::NullGeometry:
      case QgsWkbTypes::UnknownGeometry:
        break;
    }
  }

  else if ( mMode == AddVertex )
  {
    // old mode was AddVertex, remove node to be added
    for ( int r = rowCount() - 1; r > 0; r-- )
    {
      QStandardItem *it = item( r );
      if ( it->data( SegmentVertexRole ).toBool() )
      {
        removeRow( r );
        emit vertexCountChanged();
        emit currentPointChanged();
      }
    }
  }

  mMode = mode;

  emit editingModeChanged();
}

