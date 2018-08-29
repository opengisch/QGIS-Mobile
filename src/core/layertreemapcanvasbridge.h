/***************************************************************************
  layertreemapcanvasbridge.h - LayerTreeMapCanvasBridge

 ---------------------
 begin                : 26.8.2016
 copyright            : (C) 2016 by Matthias Kuhn, OPENGIS.ch
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef LAYERTREEMAPCANVASBRIDGE_H
#define LAYERTREEMAPCANVASBRIDGE_H

#include <QObject>
#include <QStringList>

#include <qgscoordinatereferencesystem.h>
#include <qgsmapthemecollection.h>

#include "layertreemodel.h"

class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class MapSettings;
class QgsMapCanvasLayer;
class QgsMapLayer;

/**
 * The QgsLayerTreeMapCanvasBridge class takes care of updates of layer set
 * for QgsMapCanvas from a layer tree. The class listens to the updates in the layer tree
 * and updates the list of layers for rendering whenever some layers are added, removed,
 * or their visibility changes.
 *
 * The update of layers is not done immediately - it is postponed, so a series of updates
 * to the layer tree will trigger just one update of canvas layers.
 *
 * Also allows the client to override the default order of layers. This is useful
 * in advanced cases where the grouping in layer tree should be independent from the actual
 * order in the canvas.
 *
 */
class LayerTreeMapCanvasBridge : public QObject
{
    Q_OBJECT
  public:
    //! Constructor: does not take ownership of the layer tree nor canvas
    LayerTreeMapCanvasBridge(  LayerTreeModel* model, MapSettings* mapSettings, QObject* parent = nullptr );

    void clear();

    QgsLayerTree* layerTree() const
    {
      return mModel->layerTree();
    }

    MapSettings* mapSettings() const
    {
      return mMapSettings;
    }

    bool hasCustomLayerOrder() const
    {
      return mHasCustomLayerOrder;
    }
    QStringList customLayerOrder() const
    {
      return mCustomLayerOrder;
    }

    QStringList defaultLayerOrder() const;

    //! if enabled, will automatically set full canvas extent and destination CRS + map units
    //! when first layer(s) are added
    void setAutoSetupOnFirstLayer( bool enabled )
    {
      mAutoSetupOnFirstLayer = enabled;
    }
    bool autoSetupOnFirstLayer() const
    {
      return mAutoSetupOnFirstLayer;
    }

    //! if enabled, will automatically turn on on-the-fly reprojection of layers if a layer
    //! with different source CRS is added
    void setAutoEnableCrsTransform( bool enabled )
    {
      mAutoEnableCrsTransform = enabled;
    }
    bool autoEnableCrsTransform() const
    {
      return mAutoEnableCrsTransform;
    }

  public slots:
    void setHasCustomLayerOrder( bool state );
    void setCustomLayerOrder( const QStringList& order );

    //! force update of canvas layers from the layer tree. Normally this should not be needed to be called.
    void setCanvasLayers();

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

  signals:
    void hasCustomLayerOrderChanged( bool );
    void customLayerOrderChanged( const QStringList& order );

  private:

    void defaultLayerOrder( QgsLayerTreeNode* node, QStringList& order ) const;

    void setCanvasLayers( QgsLayerTreeNode* node, QList<QgsMapLayer*>& layers );

    void deferredSetCanvasLayers();

  private slots:
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();
    void nodeVisibilityChanged();
    void nodeCustomPropertyChanged( QgsLayerTreeNode* node, const QString& key );
    void mapThemeChanged();

  private:
    static bool findRecordForLayer( QgsMapLayer* layer, const QgsMapThemeCollection::MapThemeRecord& rec, QgsMapThemeCollection::MapThemeLayerRecord& layerRec );
    static void applyThemeToLayer( QgsLayerTreeLayer* nodeLayer, const QgsMapThemeCollection::MapThemeRecord& rec );
    static void applyThemeToGroup( QgsLayerTreeGroup* parent, const QgsMapThemeCollection::MapThemeRecord& rec );

    LayerTreeModel* mModel;
    MapSettings* mMapSettings;

    bool mPendingCanvasUpdate;

    bool mHasCustomLayerOrder;
    QStringList mCustomLayerOrder;

    bool mAutoSetupOnFirstLayer;
    bool mAutoEnableCrsTransform;

    bool mHasFirstLayer;
    bool mNoLayersLoaded;

    QgsCoordinateReferenceSystem mFirstCRS;
};

#endif // LAYERTREEMAPCANVASBRIDGE_H
