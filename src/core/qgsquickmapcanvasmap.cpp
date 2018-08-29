/***************************************************************************
  qgsquickmapcanvasmap.cpp
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

#include "qgsquickmapcanvasmap.h"

#include <qgsmaprendererparalleljob.h>
#include <qgsvectorlayer.h>
#include <qgsmessagelog.h>
#include <QQuickWindow>
#include <QScreen>
#include <qgspallabeling.h>
#include <QSGSimpleTextureNode>
#include <QtConcurrent>

QgsQuickMapCanvasMap::QgsQuickMapCanvasMap(  QQuickItem *parent )
  : QQuickItem( parent )
  , mMapSettings( new MapSettings() )
{
  connect( this, &QQuickItem::windowChanged, this, &QgsQuickMapCanvasMap::onWindowChanged );
  connect( &mRefreshTimer, &QTimer::timeout, this, &QgsQuickMapCanvasMap::refreshMap );
  connect( &mMapUpdateTimer, &QTimer::timeout, this, &QgsQuickMapCanvasMap::renderJobUpdated );

  connect( mMapSettings.get(), &MapSettings::extentChanged, this, &QgsQuickMapCanvasMap::onExtentChanged );
  connect( mMapSettings.get(), &MapSettings::layersChanged, this, &QgsQuickMapCanvasMap::onLayersChanged );

  connect( this, &QgsQuickMapCanvasMap::renderStarting, this, &QgsQuickMapCanvasMap::isRenderingChanged );
  connect( this, &QgsQuickMapCanvasMap::mapCanvasRefreshed, this, &QgsQuickMapCanvasMap::isRenderingChanged );

  mMapUpdateTimer.setSingleShot( false );
  mMapUpdateTimer.setInterval( 250 );
  mRefreshTimer.setSingleShot( true );
  setTransformOrigin( QQuickItem::TopLeft );
  setFlags( QQuickItem::ItemHasContents );
}

MapSettings *QgsQuickMapCanvasMap::mapSettings() const
{
  return mMapSettings.get();
}

void QgsQuickMapCanvasMap::zoom( QPointF center, qreal scale )
{
  QgsRectangle extent = mMapSettings->extent();
  QgsPoint oldCenter( extent.center() );
  QgsPoint mousePos( mMapSettings->screenToCoordinate( center ) );
  QgsPointXY newCenter( mousePos.x() + ( ( oldCenter.x() - mousePos.x() ) * scale ),
                        mousePos.y() + ( ( oldCenter.y() - mousePos.y() ) * scale ) );

  // same as zoomWithCenter (no coordinate transformations are needed)
  extent.scale( scale, &newCenter );
  mMapSettings->setExtent( extent );
}

void QgsQuickMapCanvasMap::pan( QPointF oldPos, QPointF newPos )
{
  QgsPoint start = mMapSettings->screenToCoordinate( oldPos.toPoint() );
  QgsPoint end = mMapSettings->screenToCoordinate( newPos.toPoint() );

  double dx = end.x() - start.x();
  double dy = end.y() - start.y();

  // modify the extent
  QgsRectangle extent = mMapSettings->extent();

  extent.setXMinimum( extent.xMinimum() + dx );
  extent.setXMaximum( extent.xMaximum() + dx );
  extent.setYMaximum( extent.yMaximum() + dy );
  extent.setYMinimum( extent.yMinimum() + dy );

  mMapSettings->setExtent( extent );
}

void QgsQuickMapCanvasMap::refreshMap()
{
  stopRendering(); // if any...

  QgsMapSettings mapSettings = mMapSettings->mapSettings();

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                    << QgsExpressionContextUtils::mapSettingsScope( mapSettings );

  mapSettings.setExpressionContext( expressionContext );

  // create the renderer job
  Q_ASSERT( !mJob );
  mJob = new QgsMapRendererParallelJob( mapSettings );

  if ( mIncrementalRendering )
    mMapUpdateTimer.start();

  connect( mJob, &QgsMapRendererJob::renderingLayersFinished, this, &QgsQuickMapCanvasMap::renderJobUpdated );
  connect( mJob, &QgsMapRendererJob::finished, this, &QgsQuickMapCanvasMap::renderJobFinished );
  mJob->setCache( mCache );

  mJob->start();

  emit renderStarting();
}

void QgsQuickMapCanvasMap::renderJobUpdated()
{
  mImage = mJob->renderedImage();
  mImageMapSettings = mJob->mapSettings();
  mDirty = true;
  // Temporarily freeze the canvas, we only need to reset the geometry but not trigger a repaint
  bool freeze = mFreeze;
  mFreeze = true;
  updateTransform();
  mFreeze = freeze;

  update();
  emit mapCanvasRefreshed();
}

void QgsQuickMapCanvasMap::renderJobFinished()
{
  const QgsMapRendererJob::Errors errors = mJob->errors();
  for ( const QgsMapRendererJob::Error &error : errors )
  {
    QgsMessageLog::logMessage( QStringLiteral( "%1 :: %2" ).arg( error.layerID, error.message ), tr( "Rendering" ) );
  }

  // take labeling results before emitting renderComplete, so labeling map tools
  // connected to signal work with correct results
  delete mLabelingResults;
  mLabelingResults = mJob->takeLabelingResults();

  mImage = mJob->renderedImage();
  mImageMapSettings = mJob->mapSettings();

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;
  mDirty = true;
  mMapUpdateTimer.stop();

  // Temporarily freeze the canvas, we only need to reset the geometry but not trigger a repaint
  bool freeze = mFreeze;
  mFreeze = true;
  updateTransform();
  mFreeze = freeze;

  update();
  emit mapCanvasRefreshed();
}

void QgsQuickMapCanvasMap::onWindowChanged( QQuickWindow* window )
{
  disconnect( this, SLOT( onScreenChanged( QScreen* ) ) );
  if ( window )
  {
    connect( window, &QQuickWindow::screenChanged, this, &QgsQuickMapCanvasMap::onScreenChanged );
    onScreenChanged( window->screen() );
  }
}

void QgsQuickMapCanvasMap::onScreenChanged( QScreen *screen )
{
  if ( screen )
    mMapSettings->setOutputDpi( screen->physicalDotsPerInch() );
}

void QgsQuickMapCanvasMap::onExtentChanged()
{
  updateTransform();

  // And trigger a new rendering job
  refresh();
}

void QgsQuickMapCanvasMap::updateTransform()
{
  QgsMapSettings currentMapSettings = mMapSettings->mapSettings();
  QgsMapToPixel mtp = currentMapSettings.mapToPixel();

  QgsPointXY pixelPt = mtp.transform( mImageMapSettings.visibleExtent().xMinimum(), mImageMapSettings.visibleExtent().yMaximum() );
  setScale( mImageMapSettings.scale() / currentMapSettings.scale() );

  setX( pixelPt.x() );
  setY( pixelPt.y() );
}

int QgsQuickMapCanvasMap::mapUpdateInterval() const
{
  return mMapUpdateTimer.interval();
}

void QgsQuickMapCanvasMap::setMapUpdateInterval( int mapUpdateInterval )
{
  if ( mMapUpdateInterval == mapUpdateInterval )
    return;

  mMapUpdateTimer.setInterval( mapUpdateInterval );

  emit mapUpdateIntervalChanged();
}

bool QgsQuickMapCanvasMap::incrementalRendering() const
{
  return mIncrementalRendering;
}

void QgsQuickMapCanvasMap::setIncrementalRendering( bool incrementalRendering )
{
  if ( incrementalRendering == mIncrementalRendering )
    return;

  mIncrementalRendering = incrementalRendering;
  emit incrementalRenderingChanged();
}

bool QgsQuickMapCanvasMap::freeze() const
{
  return mFreeze;
}

void QgsQuickMapCanvasMap::setFreeze( bool freeze )
{
  if ( freeze == mFreeze )
    return;

  mFreeze = freeze;

  if ( !mFreeze )
    refresh();

  emit freezeChanged();
}

bool QgsQuickMapCanvasMap::isRendering() const
{
  return mJob;
}

QgsRectangle QgsQuickMapCanvasMap::extent() const
{
  return mMapSettings->extent();
}

void QgsQuickMapCanvasMap::setExtent( const QgsRectangle& extent )
{
  mMapSettings->setExtent( extent );
}

QSGNode* QgsQuickMapCanvasMap::updatePaintNode( QSGNode* oldNode, QQuickItem::UpdatePaintNodeData* )
{
  if ( mDirty )
  {
    delete oldNode;
    oldNode = nullptr;
    mDirty = false;
  }

  QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>( oldNode );
  if ( !node )
  {
    node = new QSGSimpleTextureNode();
    QSGTexture* texture = window()->createTextureFromImage( mImage );
    node->setTexture( texture );
    node->setOwnsTexture( true );
  }

  QRectF rect( boundingRect() );

  // Check for resizes that change the w/h ratio
  if ( !rect.isEmpty() && !mImage.size().isEmpty() && rect.width() / rect.height() != mImage.width() / mImage.height() )
  {
    if ( rect.height() == mImage.height() )
    {
      rect.setHeight( rect.width() / mImage.width() * mImage.height() );
    }
    else
    {
      rect.setWidth( rect.height() / mImage.height() * mImage.width() );
    }
  }

  node->setRect( rect );

  return node;
}

QgsCoordinateReferenceSystem QgsQuickMapCanvasMap::destinationCrs() const
{
  return mMapSettings->destinationCrs();
}

void QgsQuickMapCanvasMap::setDestinationCrs( const QgsCoordinateReferenceSystem& destinationCrs )
{
  mMapSettings->setDestinationCrs( destinationCrs );
}

void QgsQuickMapCanvasMap::geometryChanged( const QRectF& newGeometry, const QRectF& oldGeometry )
{
  Q_UNUSED( oldGeometry )
  // The Qt documentation advices to call the base method here.
  // However, this introduces instabilities and heavy performance impacts on Android.
  // It seems on desktop disabling it prevents us from downsizing the window...
  // Be careful when re-enabling it.
  // QQuickItem::geometryChanged( newGeometry, oldGeometry );

  mMapSettings->setOutputSize( newGeometry.size().toSize() );
  refresh();
}

void QgsQuickMapCanvasMap::onLayersChanged()
{
  if ( mMapSettings->extent().isEmpty() )
    zoomToFullExtent();

  Q_FOREACH( const QMetaObject::Connection& conn, mLayerConnections )
  {
    disconnect( conn );
  }
  mLayerConnections.clear();

  const QList<QgsMapLayer *> layers = mMapSettings->layers();
  for ( QgsMapLayer *layer : layers )
  {
    mLayerConnections << connect( layer, &QgsMapLayer::repaintRequested, this, &QgsQuickMapCanvasMap::refresh );
  }

  refresh();
}

void QgsQuickMapCanvasMap::destroyJob( QgsMapRendererJob *job )
{
  job->cancel();
  job->deleteLater();
}

void QgsQuickMapCanvasMap::stopRendering()
{
  if ( mJob )
  {
    disconnect( mJob, &QgsMapRendererJob::renderingLayersFinished, this, &QgsQuickMapCanvasMap::renderJobUpdated );
    disconnect( mJob, &QgsMapRendererJob::finished, this, &QgsQuickMapCanvasMap::renderJobFinished );

    mJob->cancelWithoutBlocking();
    mJob = nullptr;
  }
}

void QgsQuickMapCanvasMap::zoomToFullExtent()
{
  QgsRectangle extent;
  const QList<QgsMapLayer *> layers = mMapSettings->layers();
  for ( QgsMapLayer *layer : layers )
  {
    extent.combineExtentWith( layer->extent() );
  }
  mMapSettings->setExtent( extent );

  refresh();
}

void QgsQuickMapCanvasMap::refresh()
{
  if ( mMapSettings->outputSize().isNull() )
    return;  // the map image size has not been set yet

  if ( !mFreeze )
    mRefreshTimer.start( 1 );
}
