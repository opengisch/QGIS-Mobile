/***************************************************************************
              maptoscreen.h
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

#ifndef MAPTOSCREEN_H
#define MAPTOSCREEN_H

#include <QObject>
#include <QPointF>
#include <qgspoint.h>

#include "mapsettings.h"

/**
 * @brief The MapToScreen class transform a map point to screen coordinates.
 * The map point CRS is the one from the map settings.
 * Screen point will be automatically updated on map extent changes.
 */
class MapToScreen : public QObject
{
    Q_OBJECT

    //! Map settings is used to define the map canvas CRS and detect any extent change
    Q_PROPERTY( MapSettings* mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )
    //! the point in map coordinates
    Q_PROPERTY( QgsPoint mapPoint READ mapPoint WRITE setMapPoint NOTIFY mapPointChanged )
    //! the point in screen coordinates (read-only)
    Q_PROPERTY( QPointF screenPoint READ screenPoint NOTIFY screenPointChanged )

  public:
    explicit MapToScreen( QObject *parent = nullptr );

    //! \copydoc mapSettings
    void setMapSettings( MapSettings *mapSettings );
    //! \copydoc mapSettings
    MapSettings *mapSettings();

    //! \copydoc mapPoint
    void setMapPoint( const QgsPoint &point );
    //! \copydoc mapPoint
    QgsPoint mapPoint();

    //! \copydoc screenPoint
    QPointF screenPoint();


  signals:
    //! \copydoc mapSettings
    void mapSettingsChanged();
    //! \copydoc mapPoint
    void mapPointChanged();
    //! \copydoc screenPoint
    void screenPointChanged();

  private slots:
    void transformPoint();

  private:
    MapSettings *mMapSettings = nullptr;
    QgsPoint mMapPoint = QgsPoint();
    QPointF mScreenPoint = QPointF();
};

#endif // MAPTOSCREEN_H
