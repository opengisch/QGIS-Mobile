/***************************************************************************
  snappingresult.h - SnappingResult

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
#ifndef SNAPPINGRESULT_H
#define SNAPPINGRESULT_H

#include <qgspoint.h>
#include <qgsvectorlayer.h>
#include <qgspointlocator.h>

class SnappingResult
{
    Q_GADGET
    Q_PROPERTY( QgsPoint point READ point )
    Q_PROPERTY( bool isValid READ isValid )

  public:
    /**
     * The type of a snap result or the filter type for a snap request.
     */
    enum Type
    {
      Invalid = 0, //!< Invalid
      Vertex  = 1, //!< Snapped to a vertex. Can be a vertex of the geometry or an intersection.
      Edge    = 2, //!< Snapped to an edge
      Area    = 4, //!< Snapped to an area
      All = Vertex | Edge | Area //!< Combination of vertex, edge and area
    };

    Q_DECLARE_FLAGS( Types, Type )

    //! construct invalid match
    SnappingResult();

    SnappingResult( Type t, QgsVectorLayer* vl, QgsFeatureId fid, double dist, const QgsPoint& pt, int vertexIndex = 0, QgsPoint* edgePoints = nullptr );

    SnappingResult( const QgsPointLocator::Match& match );

    Type type() const;

    bool isValid() const;
    bool hasVertex() const;
    bool hasEdge() const;
    bool hasArea() const;

    //! for vertex / edge match
    //! units depending on what class returns it (geom.cache: layer units, map canvas snapper: dest crs units)
    double distance() const;

    //! for vertex / edge match
    //! coords depending on what class returns it (geom.cache: layer coords, map canvas snapper: dest coords)
    QgsPoint point() const;

    //! for vertex / edge match (first vertex of the edge)
    int vertexIndex() const;

    /**
     * The vector layer where the snap occurred.
     * Will be null if the snap happened on an intersection.
     */
    QgsVectorLayer* layer() const;

    /**
     * The id of the feature to which the snapped geometry belongs.
     */
    QgsFeatureId featureId() const;

    //! Only for a valid edge match - obtain endpoints of the edge
    void edgePoints( QgsPoint& pt1, QgsPoint& pt2 ) const;

  private:
    Type matchTypeToSnappingResultType( QgsPointLocator::Type type );

    Type mType;
    double mDist;
    QgsPoint mPoint;
    QgsVectorLayer* mLayer;
    QgsFeatureId mFid;
    int mVertexIndex; // e.g. vertex index
    QgsPoint mEdgePoints[2];
};

#endif // SNAPPINGRESULT_H
