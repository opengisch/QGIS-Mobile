/***************************************************************************
  distancearea.h - DistanceArea

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
#ifndef DISTANCEAREA_H
#define DISTANCEAREA_H

#include <QObject>

#include <qgsdistancearea.h>

class Geometry;
class RubberbandModel;
class QgsProject;

class DistanceArea : public QObject
{
    Q_OBJECT

    Q_PROPERTY( RubberbandModel* rubberbandModel READ rubberbandModel WRITE setRubberbandModel NOTIFY rubberbandModelChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs READ crs WRITE setCrs NOTIFY crsChanged )
    Q_PROPERTY( QgsProject* project READ project WRITE setProject NOTIFY projectChanged )

    Q_PROPERTY( qreal length READ length NOTIFY lengthChanged )
    Q_PROPERTY( bool lengthValid READ lengthValid NOTIFY lengthValidChanged )
    Q_PROPERTY( qreal area READ area NOTIFY areaChanged )
    Q_PROPERTY( bool areaValid READ areaValid NOTIFY areaValidChanged )
    Q_PROPERTY( QgsUnitTypes::DistanceUnit lengthUnits READ lengthUnits NOTIFY lengthUnitsChanged )
    Q_PROPERTY( QgsUnitTypes::AreaUnit areaUnits READ areaUnits NOTIFY areaUnitsChanged )

    /**
     * Contains the length of the last segment
     */
    Q_PROPERTY( qreal segmentLength READ segmentLength NOTIFY segmentLengthChanged )

  public:
    explicit DistanceArea( QObject* parent = nullptr );

    qreal length() const;
    bool lengthValid() const;
    qreal area() const;
    bool areaValid() const;
    qreal segmentLength() const;

    QgsUnitTypes::DistanceUnit lengthUnits() const;
    QgsUnitTypes::AreaUnit areaUnits() const;

    QgsCoordinateReferenceSystem crs() const;
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    RubberbandModel* rubberbandModel() const;
    void setRubberbandModel( RubberbandModel* rubberbandModel );

    QgsProject* project() const;
    void setProject( QgsProject* project );

  signals:
    void rubberbandModelChanged();
    void crsChanged();
    void projectChanged();

    void lengthChanged();
    void lengthValidChanged();
    void areaChanged();
    void areaValidChanged();
    void segmentLengthChanged();
    void lengthUnitsChanged();
    void areaUnitsChanged();

  private slots:
    void init();

  private:
    RubberbandModel* mRubberbandModel;
    QgsCoordinateReferenceSystem mCrs;
    QgsProject* mProject;

    QgsDistanceArea mDistanceArea;
};

#endif // DISTANCEAREA_H
