/***************************************************************************
  coordinatetransformer.h - CoordinateTransformer

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
#ifndef COORDINATETRANSFORMER_H
#define COORDINATETRANSFORMER_H

#include <QObject>
#include <QtPositioning/QGeoCoordinate>

#include <qgspoint.h>

/**
 * Helper class for transform of coordinates (QGeoCoordinate) to a different coordinate reference system.
 *
 * It requires connection of transformation context from mapSettings, source position and source CRS to
 * calculate projected position in desired destination CRS.
 *
 * \note QML Type: CoordinateTransformer
 */
class CoordinateTransformer : public QObject
{
    Q_OBJECT

    //! Projected (destination) position (in destination CRS)
    Q_PROPERTY( QgsPoint projectedPosition READ projectedPosition NOTIFY projectedPositionChanged )

    //! Source position
    Q_PROPERTY( QGeoCoordinate sourcePosition READ sourcePosition WRITE setSourcePosition NOTIFY sourcePositionChanged )

    //! Destination CRS
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )

    //! Source CRS, default 4326
    Q_PROPERTY( QgsCoordinateReferenceSystem sourceCrs READ sourceCrs WRITE setSourceCrs NOTIFY sourceCrsChanged )

    //! Transformation context, can be set from MapSettings::transformContext()
    Q_PROPERTY( QgsCoordinateTransformContext transformContext READ transformContext WRITE setTransformContext NOTIFY transformContextChanged )

  public:
    //! Creates new coordinate transformer
    explicit CoordinateTransformer( QObject *parent = nullptr );

    //!\copydoc CoordinateTransformer::projectedPosition
    QgsPoint projectedPosition() const;

    //!\copydoc CoordinateTransformer::sourcePosition
    QGeoCoordinate sourcePosition() const;

    //!\copydoc CoordinateTransformer::sourcePosition
    void setSourcePosition( QGeoCoordinate sourcePosition );

    //!\copydoc CoordinateTransformer::destinationCrs
    QgsCoordinateReferenceSystem destinationCrs() const;

    //!\copydoc CoordinateTransformer::destinationCrs
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    //!\copydoc CoordinateTransformer::sourceCrs
    QgsCoordinateReferenceSystem sourceCrs() const;

    //!\copydoc CoordinateTransformer::sourceCrs
    void setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs );

    //!\copydoc CoordinateTransformer::transformContext
    void setTransformContext( const QgsCoordinateTransformContext &context );

    //!\copydoc CoordinateTransformer::transformContext
    QgsCoordinateTransformContext transformContext() const;

  signals:
    //!\copydoc CoordinateTransformer::projectedPosition
    void projectedPositionChanged();

    //!\copydoc CoordinateTransformer::sourcePosition
    void sourcePositionChanged();

    //!\copydoc CoordinateTransformer::destinationCrs
    void destinationCrsChanged();

    //!\copydoc CoordinateTransformer::sourceCrs
    void sourceCrsChanged();

    //!\copydoc CoordinateTransformer::transformContext
    void transformContextChanged();

  private:
    void updatePosition();

    QgsPoint mProjectedPosition;
    QGeoCoordinate mSourcePosition;
    QgsCoordinateTransform mCoordinateTransform;
};

#endif // COORDINATETRANSFORMER_H
