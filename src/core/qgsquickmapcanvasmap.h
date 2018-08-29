/***************************************************************************
  qgsquickmapcanvasmap.h
  --------------------------------------
  Date                 : 10.12.2014
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
#ifndef QGSQUICKMAPCANVASMAP_H
#define QGSQUICKMAPCANVASMAP_H

#include <QtQuick/QQuickItem>
#include <QFutureSynchronizer>
#include <QTimer>
#include <qgspoint.h>
#include <qgsmapsettings.h>

#include "mapsettings.h"

class QgsMapRendererParallelJob;
class QgsMapRendererCache;
class QgsLabelingResults;
class MapSettings;

/**
 * This class implements a visual Qt Quick Item that does map rendering
 * according to the current map settings. Client code is expected to use
 * MapCanvas item rather than using this class directly.
 *
 * MapCanvasMap instance internally creates MapSettings in
 * constructor. The QgsProject should be attached to the MapSettings.
 * The map settings for other  components should be initialized from
 * MapCanvasMap's mapSettings
 *
 * \note QML Type: MapCanvasMap
 *
 * \sa MapCanvas
 */
class QgsQuickMapCanvasMap : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )
    /**
     * The mapSettings property contains configuration for rendering of the map.
     *
     * It should be used as a primary source of map settings (and project) for
     * all other components in the application.
     *
     * This is a readonly property.
     */
    Q_PROPERTY( MapSettings *mapSettings READ mapSettings )

    /**
     * When freeze property is set to true, the map canvas does not refresh.
     * The value temporary changes during the rendering process.
     */
    Q_PROPERTY( bool freeze READ freeze WRITE setFreeze NOTIFY freezeChanged )

    /**
     * The isRendering property is set to true while a rendering job is pending for this map canvas map.
     * It can be used to show a notification icon about an ongoing rendering job.
     * This is a readonly property.
     */
    Q_PROPERTY( bool isRendering READ isRendering NOTIFY isRenderingChanged )

    /**
     * Interval in milliseconds after which the map canvas will be updated while a rendering job is ongoing.
     * This only has an effect if incrementalRendering is activated.
     * Default is 250 [ms].
     */
    Q_PROPERTY( int mapUpdateInterval READ mapUpdateInterval WRITE setMapUpdateInterval NOTIFY mapUpdateIntervalChanged )

    /**
     * When the incrementalRendering property is set to true, the automatic refresh of map canvas during rendering is allowed.
     */
    Q_PROPERTY( bool incrementalRendering READ incrementalRendering WRITE setIncrementalRendering NOTIFY incrementalRenderingChanged )

  public:
    QgsQuickMapCanvasMap( QQuickItem *parent = nullptr );
    ~QgsQuickMapCanvasMap() = default;

    QgsPoint toMapCoordinates( QPoint canvasCoordinates );

    //! \copydoc MapCanvasMap::mapSettings
    MapSettings *mapSettings() const;

    QgsUnitTypes::DistanceUnit mapUnits() const;
    void setMapUnits( const QgsUnitTypes::DistanceUnit& mapUnits );

    QList<QgsMapLayer*> layerSet() const;
    void setLayerSet( const QList<QgsMapLayer*>& layerSet );

    bool hasCrsTransformEnabled() const;
    void setCrsTransformEnabled( bool hasCrsTransformEnabled );

    QgsCoordinateReferenceSystem destinationCrs() const;
    void setDestinationCrs( const QgsCoordinateReferenceSystem& destinationCrs );

    QgsRectangle extent() const;
    void setExtent( const QgsRectangle& extent );

    QSGNode* updatePaintNode( QSGNode* oldNode, QQuickItem::UpdatePaintNodeData* ) override;

    //! \copydoc MapCanvasMap::freeze
    bool freeze() const;

    //! \copydoc MapCanvasMap::freeze
    void setFreeze( bool freeze );

    //! \copydoc MapCanvasMap::isRendering
    bool isRendering() const;

    //! \copydoc MapCanvasMap::mapUpdateInterval
    int mapUpdateInterval() const;

    //! \copydoc MapCanvasMap::mapUpdateInterval
    void setMapUpdateInterval( int mapUpdateInterval );

    //! \copydoc MapCanvasMap::incrementalRendering
    bool incrementalRendering() const;

    //! \copydoc MapCanvasMap::incrementalRendering
    void setIncrementalRendering( bool incrementalRendering );

  signals:

    /**
     * Signal is emitted when a rendering is starting
     */
    void renderStarting();

    /**
     * Signal is emitted when a canvas is refreshed
     */
    void mapCanvasRefreshed();

    void extentChanged();

    void destinationCrsChanged();

    //! \copydoc MapCanvasMap::freeze
    void freezeChanged();

    //! \copydoc MapCanvasMap::isRendering
    void isRenderingChanged();

    //!\copydoc MapCanvasMap::mapUpdateInterval
    void mapUpdateIntervalChanged();

    //!\copydoc MapCanvasMap::incrementalRendering
    void incrementalRenderingChanged();

  protected:
    void geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry ) override;

  public slots:
    //! Stop map rendering
    void stopRendering();

    /**
     * Set map setting's extent (zoom the map) on the center by given scale
     */
    void zoom( QPointF center, qreal scale );

    /**
     * Set map setting's extent (pan the map) based on the difference of positions
     */
    void pan( QPointF oldPos, QPointF newPos );

    /**
     * Refresh the map canvas.
     * Does nothing when output size of map settings is not set
     */
    void refresh();

  private slots:
    void refreshMap();
    void renderJobUpdated();
    void renderJobFinished();
    void onWindowChanged( QQuickWindow *window );
    void onScreenChanged( QScreen *screen );
    void onExtentChanged();
    void onLayersChanged();

  private:

    /**
     * Should only be called by stopRendering()!
     */
    void destroyJob( QgsMapRendererJob *job );
    QgsMapSettings prepareMapSettings() const;
    void updateTransform();
    void zoomToFullExtent();

    std::unique_ptr<MapSettings> mMapSettings;
    bool mPinching = false;
    QPoint mPinchStartPoint;
    QgsMapRendererParallelJob *mJob = nullptr;
    QgsMapRendererCache *mCache = nullptr;
    QgsLabelingResults *mLabelingResults = nullptr;
    QImage mImage;
    QgsMapSettings mImageMapSettings;
    QTimer mRefreshTimer;
    bool mDirty = false;
    bool mFreeze = false;
    QList<QMetaObject::Connection> mLayerConnections;
    QTimer mMapUpdateTimer;
    int mMapUpdateInterval;
    bool mIncrementalRendering = false;
};

#endif // QGSQUICKMAPCANVASMAP_H
