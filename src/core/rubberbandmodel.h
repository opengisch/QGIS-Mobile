/***************************************************************************
  rubberbandmodel.h - RubberbandModel

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
#ifndef RUBBERBANDMODEL_H
#define RUBBERBANDMODEL_H

#include <QVector>
#include <QObject>
#include <QPointF>
#include <qgis.h>
#include <qgspoint.h>
#include <qgsabstractgeometry.h>

class QgsVectorLayer;

/**
 * This model manages a list of vertices.
 *
 * It can be used as a linestring or as a ring in a polygon.
 */

class RubberbandModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QgsPoint currentCoordinate READ currentCoordinate WRITE setCurrentCoordinate NOTIFY currentCoordinateChanged )
    Q_PROPERTY( int currentCoordinateIndex READ currentCoordinateIndex WRITE setCurrentCoordinateIndex NOTIFY currentCoordinateIndexChanged )
    Q_PROPERTY( QgsWkbTypes::GeometryType geometryType READ geometryType NOTIFY geometryTypeChanged )
    Q_PROPERTY( QgsVectorLayer* vectorLayer READ vectorLayer WRITE setVectorLayer NOTIFY vectorLayerChanged )
    Q_PROPERTY( int vertexCount READ vertexCount NOTIFY vertexCountChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs READ crs WRITE setCrs NOTIFY crsChanged )
    //! freeze the rubberband so it doesn't get modified while panning map
    Q_PROPERTY( bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged )

  public:
    explicit RubberbandModel( QObject *parent = nullptr );

    int vertexCount() const;

    bool isEmpty() const;

    QVector<QgsPoint> vertices() const;

    QVector<QgsPoint> flatVertices( bool skipCurrentPoint = false ) const;

    /**
     * The target CRS into which points should be reprojected.
     * To retrieve unprojected points pass an invalid QgsCoordinateReferenceSystem object.
     *
     * By default coordinates will be returned unprojected.
     */
    QgsPointSequence pointSequence( const QgsCoordinateReferenceSystem& crs = QgsCoordinateReferenceSystem(), QgsWkbTypes::Type wkbType = QgsWkbTypes::PointZ ) const;

    QVector<QgsPointXY> flatPointSequence( const QgsCoordinateReferenceSystem& crs = QgsCoordinateReferenceSystem() ) const;

    void setVertex( int index, QgsPoint coordinate );

    void insertVertices( int index, int count );

    void removeVertices( int index, int count );

    int currentCoordinateIndex() const;
    void setCurrentCoordinateIndex( int currentCoordinateIndex );

    QgsPoint currentPoint( const QgsCoordinateReferenceSystem& crs = QgsCoordinateReferenceSystem(), QgsWkbTypes::Type wkbType = QgsWkbTypes::PointZ ) const;

    QgsPoint currentCoordinate() const;
    void setCurrentCoordinate( const QgsPoint& currentCoordinate );

    Q_INVOKABLE void addVertex();
    Q_INVOKABLE void removeVertex();

    Q_INVOKABLE void reset();

    QgsWkbTypes::GeometryType geometryType() const;

    QgsVectorLayer* vectorLayer() const;
    void setVectorLayer( QgsVectorLayer* vectorLayer );

    QgsCoordinateReferenceSystem crs() const;
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    //! \copydoc frozen
    bool frozen() const;
    //! \copydoc frozen
    void setFrozen( const bool &frozen );


  signals:
    void vertexChanged( int index );
    void verticesInserted( int index, int count );
    void verticesRemoved( int index, int count );
    void currentCoordinateIndexChanged();
    void currentCoordinateChanged();
    void geometryTypeChanged();
    void vectorLayerChanged();
    void vertexCountChanged();
    void crsChanged();
    //! \copydoc frozen
    void frozenChanged();


  private:
    void setGeometryType( const QgsWkbTypes::GeometryType& geometryType );

    QVector<QgsPoint> mPointList;
    int mCurrentCoordinateIndex;
    QgsWkbTypes::GeometryType mGeometryType;
    QgsVectorLayer* mLayer;
    QgsCoordinateReferenceSystem mCrs;
    bool mFrozen = false;
};

#endif // RUBBERBANDMODEL_H
