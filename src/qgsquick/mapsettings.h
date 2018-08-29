/***************************************************************************
  mapsettings.h
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include <QObject>

#include <qgsrectangle.h>
#include <qgsmapthemecollection.h>
#include <qgsmapsettings.h>
#include <qgsproject.h>
#include <qgspoint.h>

/**
 * The MapSettings class encapsulates QgsMapSettings class to offer
 * settings of configuration of map rendering via QML properties.
 *
 * \note QML Type: MapSettings
 */
class MapSettings : public QObject
{
    Q_OBJECT

    /**
     * Geographical coordinates of the rectangle that should be rendered.
     * The actual visible extent used for rendering could be slightly different
     * since the given extent may be expanded in order to fit the aspect ratio
     * of output size. Use MapSettings::visibleExtent to get the resulting extent.
     *
     * Automatically loaded from project on QgsProject::readProject
     */
    Q_PROPERTY( QgsRectangle extent READ extent WRITE setExtent NOTIFY extentChanged )
    //! \copydoc QgsMapSettings::visibleExtent()
    Q_PROPERTY( QgsRectangle visibleExtent READ visibleExtent NOTIFY visibleExtentChanged )
    //! \copydoc QgsMapSettings::mapUnitsPerPixel()
    Q_PROPERTY( double mapUnitsPerPixel READ mapUnitsPerPixel NOTIFY mapUnitsPerPixelChanged )

    /**
     * The rotation of the resulting map image, in degrees clockwise.
     * Map canvas rotation support is not implemented, 0 is used
     */
    Q_PROPERTY( double rotation READ rotation WRITE setRotation NOTIFY rotationChanged )

    /**
     * The size of the resulting map image
     *
     * Automatically loaded from project on QgsProject::readProject
     */
    Q_PROPERTY( QSize outputSize READ outputSize WRITE setOutputSize NOTIFY outputSizeChanged )

    /**
     * Output DPI used for conversion between real world units (e.g. mm) and pixels
     *
     * Automatically loaded from project on QgsProject::readProject
     */
    Q_PROPERTY( double outputDpi READ outputDpi WRITE setOutputDpi NOTIFY outputDpiChanged )

    /**
      * CRS of destination coordinate reference system.
      *
      * Automatically loaded from project on QgsProject::readProject
      */
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )

    /**
      * The string of the QgsUnitTypes::DistanceUnits CRS of destination coordinate reference system.
      */
    Q_PROPERTY( QString abbreviatedStringOfDistanceUnit READ abbreviatedStringOfDistanceUnit NOTIFY abbreviatedStringOfDistanceUnitChanged )

    /**
     * Set list of layers for map rendering. The layers must be registered in QgsProject.
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     *
     * \note Any non-spatial layers will be automatically stripped from the list (since they cannot be rendered!).
     *
     * Not loaded automatically from the associated project
     */
    Q_PROPERTY( QList<QgsMapLayer *> layers READ layers WRITE setLayers NOTIFY layersChanged )

  public:
    //! Create new map settings
    MapSettings( QObject *parent = nullptr );
    ~MapSettings() = default;

    //! Clone map settings
    QgsMapSettings mapSettings() const;

    //! \copydoc QgsMapSettings::extent()
    QgsRectangle extent() const;

    //! \copydoc QgsMapSettings::setExtent()
    void setExtent( const QgsRectangle &extent );

    //! Move current map extent to have center point defined by \a center
    Q_INVOKABLE void setCenter( const QgsPoint &center );

    //! \copydoc QgsMapSettings::mapUnitsPerPixel()
    double mapUnitsPerPixel() const;

    //! \copydoc QgsMapSettings::visibleExtent()
    QgsRectangle visibleExtent() const;

    //! \copydoc QgsMapSettings::transformContext()
    Q_INVOKABLE QgsCoordinateTransformContext transformContext() const;

    /**
     * Convert a map coordinate to screen pixel coordinates
     *
     * \param point A coordinate in map coordinates
     *
     * \return A coordinate in pixel / screen space
     */
    Q_INVOKABLE QPointF coordinateToScreen( const QgsPoint &point ) const;


    /**
     * Convert a screen coordinate to a map coordinate
     *
     * \param point A coordinate in pixel / screen coordinates
     *
     * \return A coordinate in map coordinates
     */
    Q_INVOKABLE QgsPoint screenToCoordinate( const QPointF &point ) const;

    //! \copydoc QgsMapSettings::setTransformContext()
    void setTransformContext( const QgsCoordinateTransformContext &context );

    //! \copydoc MapSettings::rotation
    double rotation() const;

    //! \copydoc MapSettings::rotation
    void setRotation( double rotation );

    //! \copydoc QgsMapSettings::outputSize()
    QSize outputSize() const;

    //! \copydoc QgsMapSettings::setOutputSize()
    void setOutputSize( const QSize &outputSize );

    //! \copydoc QgsMapSettings::outputDpi()
    double outputDpi() const;

    //! \copydoc QgsMapSettings::setOutputDpi()
    void setOutputDpi( double outputDpi );

    //! \copydoc QgsMapSettings::destinationCrs()
    QgsCoordinateReferenceSystem destinationCrs() const;

    //! \copydoc QgsMapSettings::setDestinationCrs()
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    //! \copydoc QgsMapSettings::layers()
    QList<QgsMapLayer *> layers() const;

    //! \copydoc QgsMapSettings::setLayers()
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * The string of the QgsUnitTypes::DistanceUnits CRS of destination coordinate reference system.
     */
    QString abbreviatedStringOfDistanceUnit() const;

  signals:
    //! \copydoc MapSettings::extent
    void extentChanged();

    //! \copydoc MapSettings::destinationCrs
    void destinationCrsChanged();

    //! \copydoc MapSettings::mapUnitsPerPixel
    void mapUnitsPerPixelChanged();

    //! \copydoc MapSettings::rotation
    void rotationChanged();

    //! \copydoc MapSettings::visibleExtent
    void visibleExtentChanged();

    //! \copydoc MapSettings::outputSize
    void outputSizeChanged();

    //! \copydoc MapSettings::outputDpi
    void outputDpiChanged();

    //! \copydoc MapSettings::layers
    void layersChanged();

    //! for recalculating the abbreviatedString when destinationCrs changed
    void abbreviatedStringOfDistanceUnitChanged();

  private slots:

    /**
     * Read map canvas settings stored in a QGIS project file
     *
     * \param doc parsed DOM of a QgsProject
     */
    void onReadProject( const QDomDocument &doc );

  private:
    QgsMapSettings mMapSettings;

};

#endif // MAPSETTINGS_H
