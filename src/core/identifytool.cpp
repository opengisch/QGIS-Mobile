/***************************************************************************
  identifytool.cpp - IdentifyTool

 ---------------------
 begin                : 30.8.2016
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
#include "identifytool.h"

#include "mapsettings.h"
#include "multifeaturelistmodel.h"

#include <qgsvectorlayer.h>
#include <qgsproject.h>
#include <qgsrenderer.h>

IdentifyTool::IdentifyTool( QObject *parent )
  : QObject( parent )
  , mMapSettings( nullptr )
  , mSearchRadiusMm( 8 )
{

}

MapSettings* IdentifyTool::mapSettings() const
{
  return mMapSettings;
}

void IdentifyTool::setMapSettings( MapSettings* mapSettings )
{
  if ( mapSettings == mMapSettings )
    return;

  mMapSettings = mapSettings;
  emit mapSettingsChanged();
}

void IdentifyTool::identify( const QPointF& point ) const
{
  if ( !mModel || !mMapSettings )
  {
    qWarning() << "Unable to use IdentifyTool without mapSettings or model property set.";
    return;
  }

  mModel->clear();

  QgsPointXY mapPoint = mMapSettings->mapSettings().mapToPixel().toMapCoordinates( point.toPoint() );

  QStringList noIdentifyLayerIdList = QgsProject::instance()->nonIdentifiableLayers();

  Q_FOREACH( QgsMapLayer* layer, mMapSettings->mapSettings().layers() )
  {
    if ( noIdentifyLayerIdList.contains( layer->id() ) )
      continue;

    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );
    if ( vl )
    {
      QList<IdentifyResult> results = identifyVectorLayer( vl, mapPoint );

      mModel->appendFeatures( results );
    }
  }
}

QList<IdentifyTool::IdentifyResult> IdentifyTool::identifyVectorLayer ( QgsVectorLayer* layer, const QgsPointXY& point ) const
{
  QList<IdentifyResult> results;

  if ( !layer || !layer->isSpatial() )
    return results;

  if ( !layer->isInScaleRange( mMapSettings->mapSettings().scale() ) )
    return results;

  QgsFeatureList featureList;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    // create the search rectangle
    double searchRadius = searchRadiusMU();

    QgsRectangle r;
    r.setXMinimum( point.x() - searchRadius );
    r.setXMaximum( point.x() + searchRadius );
    r.setYMinimum( point.y() - searchRadius );
    r.setYMaximum( point.y() + searchRadius );

    r = toLayerCoordinates( layer, r );

    QgsFeatureRequest req;
    req.setFilterRect( r );
    req.setLimit( QSettings().value( "/QField/identify/limit" , 100 ).toInt() );
    req.setFlags( QgsFeatureRequest::ExactIntersect );

    QgsFeatureIterator fit = layer->getFeatures( req );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
      featureList << QgsFeature( f );
  }
  catch ( QgsCsException & cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
  }

  bool filter = false;

  QgsRenderContext context( QgsRenderContext::fromMapSettings( mMapSettings->mapSettings() ) );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( layer );
  QgsFeatureRenderer* renderer = layer->renderer();
  if ( renderer && renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
  {
    // setup scale for scale dependent visibility (rule based)
    renderer->startRender( context, layer->fields() );
    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;
  }

  Q_FOREACH( const QgsFeature& feature, featureList )
  {
    context.expressionContext().setFeature( feature );

    if ( filter && !renderer->willRenderFeature( const_cast<QgsFeature&>( feature ), context ) )
      continue;

    results.append( IdentifyResult( layer, feature ) );
  }

  if ( renderer && renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
  {
    renderer->stopRender( context );
  }

  return results;
}

MultiFeatureListModel* IdentifyTool::model() const
{
  return mModel;
}

void IdentifyTool::setModel( MultiFeatureListModel* model )
{
  if ( model == mModel )
    return;

  mModel = model;
  emit modelChanged();
}

double IdentifyTool::searchRadiusMU( const QgsRenderContext& context ) const
{
  return mSearchRadiusMm * context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
}

double IdentifyTool::searchRadiusMU() const
{
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapSettings->mapSettings() );
  return searchRadiusMU( context );
}

QgsRectangle IdentifyTool::toLayerCoordinates( QgsMapLayer* layer, const QgsRectangle& rect ) const
{
  return mMapSettings->mapSettings().mapToLayerCoordinates( layer, rect );
}

double IdentifyTool::searchRadiusMm() const
{
  return mSearchRadiusMm;
}

void IdentifyTool::setSearchRadiusMm( double searchRadiusMm )
{
  if ( mSearchRadiusMm == searchRadiusMm )
    return;

  mSearchRadiusMm = searchRadiusMm;
  emit searchRadiusMmChanged();
}
