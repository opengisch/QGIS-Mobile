/***************************************************************************
  legendimageprovider.cpp - LegendImageProvider

 ---------------------
 begin                : 7.12.2016
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
#include "legendimageprovider.h"

#include <qgsproject.h>
#include <qgslayertree.h>
#include <qgslayertreelayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemodellegendnode.h>

LegendImageProvider::LegendImageProvider( QgsLayerTreeModel* layerTreeModel )
  : QQuickImageProvider( Pixmap )
  , mLayerTreeModel( layerTreeModel )
  , mRootNode( layerTreeModel->rootGroup() )
{

}

QPixmap LegendImageProvider::requestPixmap( const QString& id, QSize* size, const QSize& requestedSize )
{
  Q_UNUSED( size )
  QStringList idParts = id.split( '/' );

  if ( idParts.value( 0 ) == QStringLiteral( "legend" ) )
  {
    QgsLayerTreeLayer* layerNode = mRootNode->findLayer( idParts.value( 1 ) );
    QModelIndex layerIndex = mLayerTreeModel->node2index( layerNode );
    int legendCount = mLayerTreeModel->rowCount( layerIndex );
    for ( int i = 0; i < legendCount; ++i )
    {
      QModelIndex legendIndex = mLayerTreeModel->index( i, 0, layerIndex );
      if ( idParts.value( 2 ) == mLayerTreeModel->data( legendIndex ) )
        return mLayerTreeModel->data( legendIndex, Qt::DecorationRole ).value<QPixmap>();
    }
  }
  if ( idParts.value( 0 ) == QStringLiteral( "layer" ) )
  {
    QgsLayerTreeLayer* layerNode = mRootNode->findLayer( idParts.value( 1 ) );

    if ( layerNode )
    {
      QgsLayerTreeModelLegendNode* legendNode = mLayerTreeModel->legendNodeEmbeddedInParent( layerNode );
      if ( legendNode )
        return legendNode->data( Qt::DecorationRole ).value<QPixmap>();
    }
  }

  return QPixmap( requestedSize );
}
