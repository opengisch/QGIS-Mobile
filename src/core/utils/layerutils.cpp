/***************************************************************************
  layerutils.cpp - LayerUtils

 ---------------------
 begin                : 01.03.2021
 copyright            : (C) 2020 by Mathieu Pellerin
 email                : mathieu@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "layerutils.h"

#include <qgsfillsymbollayer.h>
#include <qgslayoutatlas.h>
#include <qgslayoutmanager.h>
#include <qgslinesymbollayer.h>
#include <qgsmarkersymbollayer.h>
#include <qgsprintlayout.h>
#include <qgsproject.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>
#include <qgssymbollayer.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgswkbtypes.h>

LayerUtils::LayerUtils( QObject *parent )
  : QObject( parent )
{
}

QgsSymbol *LayerUtils::defaultSymbol( QgsVectorLayer *layer )
{
  QgsSymbol *symbol = nullptr;
  QgsSymbolLayerList symbolLayers;
  switch ( layer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsSimpleMarkerSymbolLayer *symbolLayer = new QgsSimpleMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Circle, 2.6, 0.0, DEFAULT_SCALE_METHOD, QColor( 255, 0, 0, 100 ), QColor( 255, 0, 0 ) );
      symbolLayer->setStrokeWidth( 0.6 );
      symbolLayers << symbolLayer;
      symbol = new QgsMarkerSymbol( symbolLayers );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    {
      QgsSimpleLineSymbolLayer *symbolLayer = new QgsSimpleLineSymbolLayer( QColor( 255, 0, 0 ), 0.6 );
      symbolLayers << symbolLayer;
      symbol = new QgsLineSymbol( symbolLayers );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      QgsSimpleFillSymbolLayer *symbolLayer = new QgsSimpleFillSymbolLayer( QColor( 255, 0, 0, 100 ), DEFAULT_SIMPLEFILL_STYLE, QColor( 255, 0, 0 ), DEFAULT_SIMPLEFILL_BORDERSTYLE, 0.6 );
      symbolLayers << symbolLayer;
      symbol = new QgsFillSymbol( symbolLayers );
      break;
    }

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
  return symbol;
}

bool LayerUtils::isAtlasCoverageLayer( QgsVectorLayer *layer )
{
  if ( !layer || !QgsProject::instance()->layoutManager() )
    return false;

  const QList<QgsPrintLayout *> printLayouts = QgsProject::instance()->layoutManager()->printLayouts();
  for ( QgsPrintLayout *printLayout : printLayouts )
  {
    if ( printLayout->atlas() )
    {
      if ( printLayout->atlas()->coverageLayer() == layer )
        return true;
    }
  }

  return false;
}

#if _QGIS_VERSION_INT >= 32100
void LayerUtils::selectFeaturesInLayer( QgsVectorLayer *layer, const QList<int> &fids, Qgis::SelectBehavior behavior )
#else
void LayerUtils::selectFeaturesInLayer( QgsVectorLayer *layer, const QList<int> &fids, QgsVectorLayer::SelectBehavior behavior )
#endif
{
  if ( !layer )
    return;
  QgsFeatureIds qgsFids;
  for ( const int &fid : fids )
    qgsFids << fid;
  layer->selectByIds( qgsFids, behavior );
}

QString LayerUtils::fieldType( const QgsField &field ) const
{
  return QVariant( field.type() ).typeName();
}
