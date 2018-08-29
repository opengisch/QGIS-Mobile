/***************************************************************************
  layertreemapcanvasbridge.cpp - LayerTreeMapCanvasBridge

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

#include "layertreemapcanvasbridge.h"
#include "qgsquickmapcanvasmap.h"
#include "mapsettings.h"

#include <qgslayertreegroup.h>
#include <qgslayertree.h>
#include <qgsmaplayer.h>
#include <qgslayertreeutils.h>
#include <qgsmaplayerstylemanager.h>
#include <qgslayertreemodel.h>

LayerTreeMapCanvasBridge::LayerTreeMapCanvasBridge( LayerTreeModel* model, MapSettings* mapSettings, QObject* parent )
  : QObject( parent )
  , mModel( model )
  , mMapSettings( mapSettings )
  , mPendingCanvasUpdate( false )
  , mHasCustomLayerOrder( false )
  , mAutoSetupOnFirstLayer( false )
  , mAutoEnableCrsTransform( true )
  , mNoLayersLoaded( !model->layerTree()->findLayers().isEmpty() )
{
  connect( model->layerTree(), &QgsLayerTreeGroup::addedChildren, this, &LayerTreeMapCanvasBridge::nodeAddedChildren );
  connect( model->layerTree(), &QgsLayerTreeGroup::customPropertyChanged, this, &LayerTreeMapCanvasBridge::nodeCustomPropertyChanged );
  connect( model->layerTree(), &QgsLayerTreeGroup::removedChildren, this, &LayerTreeMapCanvasBridge::nodeRemovedChildren );
  connect( model->layerTree(), &QgsLayerTreeGroup::visibilityChanged, this, &LayerTreeMapCanvasBridge::nodeVisibilityChanged );
  connect( model, &LayerTreeModel::mapThemeChanged, this, &LayerTreeMapCanvasBridge::mapThemeChanged );

  setCanvasLayers();
}

void LayerTreeMapCanvasBridge::clear()
{
  setHasCustomLayerOrder( false );
  setCustomLayerOrder( defaultLayerOrder() );
}

QStringList LayerTreeMapCanvasBridge::defaultLayerOrder() const
{
  QStringList order;
  defaultLayerOrder( mModel->layerTree(), order );
  return order;
}

void LayerTreeMapCanvasBridge::defaultLayerOrder( QgsLayerTreeNode* node, QStringList& order ) const
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    order << nodeLayer->layerId();
  }

  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
    defaultLayerOrder( child, order );
}


void LayerTreeMapCanvasBridge::setHasCustomLayerOrder( bool state )
{
  if ( mHasCustomLayerOrder == state )
    return;

  mHasCustomLayerOrder = state;
  emit hasCustomLayerOrderChanged( mHasCustomLayerOrder );

  deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::setCustomLayerOrder( const QStringList& order )
{
  if ( mCustomLayerOrder == order )
    return;

  // verify that the new order is correct
  QStringList defOrder( defaultLayerOrder() );
  QStringList newOrder( order );
  QStringList sortedNewOrder( order );
  qSort( defOrder );
  qSort( sortedNewOrder );

  if ( defOrder.size() < sortedNewOrder.size() )
  {
    // might contain bad layers, but also duplicates
    QSet<QString> ids( defOrder.toSet() );

    for ( int i = 0; i < sortedNewOrder.size(); i++ )
    {
      if ( !ids.contains( sortedNewOrder[i] ) )
      {
        newOrder.removeAll( sortedNewOrder[i] );
        sortedNewOrder.removeAt( i-- );
      }
    }
  }

  if ( defOrder != sortedNewOrder )
    return; // must be permutation of the default order

  mCustomLayerOrder = newOrder;
  emit customLayerOrderChanged( mCustomLayerOrder );

  if ( mHasCustomLayerOrder )
    deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::setCanvasLayers()
{
  QList<QgsMapLayer*> layers;

  if ( mHasCustomLayerOrder )
  {
    Q_FOREACH ( const QString& layerId, mCustomLayerOrder )
    {
      QgsLayerTreeLayer* nodeLayer = mModel->layerTree()->findLayer( layerId );
      if ( nodeLayer )
      {
        if ( nodeLayer->isVisible() )
          layers << nodeLayer->layer();
      }
    }
  }
  else
    setCanvasLayers( mModel->layerTree(), layers );

  QList<QgsLayerTreeLayer*> layerNodes = mModel->layerTree()->findLayers();
  bool firstLayers = mAutoSetupOnFirstLayer && mNoLayersLoaded && !layerNodes.empty();

  if ( firstLayers )
  {
    Q_FOREACH ( QgsLayerTreeLayer* layerNode, layerNodes )
    {
      if ( layerNode->layer() && layerNode->isVisible() && layerNode->layer()->isSpatial() )
      {
        mMapSettings->setDestinationCrs( layerNode->layer()->crs() );
        break;
      }
    }
  }

  mMapSettings->setLayers( layers );

  if ( !mFirstCRS.isValid() )
  {
    // find out what is the first used CRS in case we may need to turn on OTF projections later
    Q_FOREACH ( QgsLayerTreeLayer* layerNode, layerNodes )
    {
      if ( layerNode->layer() && layerNode->layer()->crs().isValid() )
      {
        mFirstCRS = layerNode->layer()->crs();
        break;
      }
    }
  }

  if ( mFirstCRS.isValid() && firstLayers )
  {
    mMapSettings->setDestinationCrs( mFirstCRS );
    QgsProject::instance()->setCrs( mFirstCRS );
  }

  mNoLayersLoaded = layerNodes.isEmpty();
  if ( mNoLayersLoaded )
    mFirstCRS = QgsCoordinateReferenceSystem();

  mPendingCanvasUpdate = false;
}

void LayerTreeMapCanvasBridge::readProject( const QDomDocument& doc )
{
  mFirstCRS = QgsCoordinateReferenceSystem(); // invalidate on project load

  QDomElement elem = doc.documentElement().firstChildElement( "layer-tree-canvas" );
  if ( elem.isNull() )
  {
    bool oldEnabled;
    QStringList oldOrder;
    if ( QgsLayerTreeUtils::readOldLegendLayerOrder( doc.documentElement().firstChildElement( "legend" ), oldEnabled, oldOrder ) )
    {
      setHasCustomLayerOrder( oldEnabled );
      setCustomLayerOrder( oldOrder );
    }
    return;
  }

  QDomElement customOrderElem = elem.firstChildElement( "custom-order" );
  if ( !customOrderElem.isNull() )
  {
    QStringList order;
    QDomElement itemElem = customOrderElem.firstChildElement( "item" );
    while ( !itemElem.isNull() )
    {
      order.append( itemElem.text() );
      itemElem = itemElem.nextSiblingElement( "item" );
    }

    setHasCustomLayerOrder( customOrderElem.attribute( "enabled", QString() ).toInt() );
    setCustomLayerOrder( order );
  }
}

void LayerTreeMapCanvasBridge::writeProject( QDomDocument& doc )
{
  QDomElement elem = doc.createElement( "layer-tree-canvas" );
  QDomElement customOrderElem = doc.createElement( "custom-order" );
  customOrderElem.setAttribute( "enabled", mHasCustomLayerOrder ? 1 : 0 );

  Q_FOREACH ( const QString& layerId, mCustomLayerOrder )
  {
    QDomElement itemElem = doc.createElement( "item" );
    itemElem.appendChild( doc.createTextNode( layerId ) );
    customOrderElem.appendChild( itemElem );
  }
  elem.appendChild( customOrderElem );

  doc.documentElement().appendChild( elem );
}

void LayerTreeMapCanvasBridge::setCanvasLayers( QgsLayerTreeNode* node, QList<QgsMapLayer*>& layers )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    if ( nodeLayer->isVisible() )
    {
      layers << nodeLayer->layer();
    }
  }

  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
    setCanvasLayers( child, layers );
}

void LayerTreeMapCanvasBridge::deferredSetCanvasLayers()
{
  if ( mPendingCanvasUpdate )
    return;

  mPendingCanvasUpdate = true;
  QMetaObject::invokeMethod( this, "setCanvasLayers", Qt::QueuedConnection );
}

void LayerTreeMapCanvasBridge::nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  // collect layer IDs that have been added in order to put them into custom layer order
  QStringList layerIds;
  QList<QgsLayerTreeNode*> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode* child = children.at( i );
    if ( QgsLayerTree::isLayer( child ) )
    {
      layerIds << QgsLayerTree::toLayer( child )->layerId();
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      Q_FOREACH ( QgsLayerTreeLayer* nodeL, QgsLayerTree::toGroup( child )->findLayers() )
        layerIds << nodeL->layerId();
    }
  }

  Q_FOREACH ( const QString& layerId, layerIds )
  {
    if ( !mCustomLayerOrder.contains( layerId ) )
      mCustomLayerOrder.append( layerId );
  }

  emit customLayerOrderChanged( mCustomLayerOrder );

  deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::nodeRemovedChildren()
{
  // no need to disconnect from removed nodes as they are deleted

  // check whether the layers are still there, if not, remove them from the layer order!
  QList<int> toRemove;
  for ( int i = 0; i < mCustomLayerOrder.count(); ++i )
  {
    QgsLayerTreeLayer* node = mModel->layerTree()->findLayer( mCustomLayerOrder[i] );
    if ( !node )
      toRemove << i;
  }
  for ( int i = toRemove.count() - 1; i >= 0; --i )
    mCustomLayerOrder.removeAt( toRemove[i] );
  emit customLayerOrderChanged( mCustomLayerOrder );

  deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::nodeVisibilityChanged()
{
  deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::nodeCustomPropertyChanged( QgsLayerTreeNode* node, const QString& key )
{
  Q_UNUSED( node );
  if ( key == "overview" )
    deferredSetCanvasLayers();
}

void LayerTreeMapCanvasBridge::mapThemeChanged()
{
  QgsProject::instance()->mapThemeCollection()->applyTheme( mModel->mapTheme(), mModel->layerTree(), mModel->layerTreeModel() );
}


bool LayerTreeMapCanvasBridge::findRecordForLayer( QgsMapLayer* layer, const QgsMapThemeCollection::MapThemeRecord& rec, QgsMapThemeCollection::MapThemeLayerRecord& layerRec )
{
  Q_FOREACH ( const QgsMapThemeCollection::MapThemeLayerRecord& lr, rec.layerRecords() )
  {
    if ( lr.layer() == layer )
    {
      layerRec = lr;
      return true;
    }
  }
  return false;
}

void LayerTreeMapCanvasBridge::applyThemeToLayer( QgsLayerTreeLayer* nodeLayer, const QgsMapThemeCollection::MapThemeRecord& rec )
{
  QgsMapThemeCollection::MapThemeLayerRecord layerRec;
  bool isVisible = findRecordForLayer( nodeLayer->layer(), rec, layerRec );

  nodeLayer->setItemVisibilityChecked( isVisible );

  if ( !isVisible )
    return;

  if ( layerRec.usingCurrentStyle )
  {
    // apply desired style first
    nodeLayer->layer()->styleManager()->setCurrentStyle( layerRec.currentStyle );
  }
#if 0
  if ( layerRec.usingLegendItems )
  {
    // some nodes are not checked
    Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer, true ) )
    {
      QString ruleKey = legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
      Qt::CheckState shouldHaveState = layerRec.checkedLegendItems.contains( ruleKey ) ? Qt::Checked : Qt::Unchecked;
      if ( ( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
           legendNode->data( Qt::CheckStateRole ).toInt() != shouldHaveState )
        legendNode->setData( shouldHaveState, Qt::CheckStateRole );
    }
  }
  else
  {
    // all nodes should be checked
    Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, model->layerLegendNodes( nodeLayer, true ) )
    {
      if ( ( legendNode->flags() & Qt::ItemIsUserCheckable ) &&
           legendNode->data( Qt::CheckStateRole ).toInt() != Qt::Checked )
        legendNode->setData( Qt::Checked, Qt::CheckStateRole );
    }
  }
#endif
}


void LayerTreeMapCanvasBridge::applyThemeToGroup( QgsLayerTreeGroup* parent, const QgsMapThemeCollection::MapThemeRecord& rec )
{
  Q_FOREACH ( QgsLayerTreeNode* node, parent->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      applyThemeToGroup( QgsLayerTree::toGroup( node ), rec );
    else if ( QgsLayerTree::isLayer( node ) )
      applyThemeToLayer( QgsLayerTree::toLayer( node ), rec );
  }
}
