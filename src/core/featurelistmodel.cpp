/***************************************************************************
  featurelistmodel.cpp - FeatureListModel

 ---------------------
 begin                : 1.2.2017
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
#include "featurelistmodel.h"

#include "qgsvectorlayer.h"

FeatureListModel::FeatureListModel()
  : mCurrentLayer( nullptr )
{

}

QModelIndex FeatureListModel::index( int row, int column, const QModelIndex& parent ) const
{
  Q_UNUSED( column )
  Q_UNUSED( parent )

  return createIndex( row, column, 1000 );
}

QModelIndex FeatureListModel::parent( const QModelIndex& child ) const
{
  Q_UNUSED( child )

  // We live in flat structures here
  return QModelIndex();
}

int FeatureListModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return mDisplayStrings.size();
}

int FeatureListModel::columnCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant FeatureListModel::data( const QModelIndex& index, int role ) const
{
  if ( role == Qt::DisplayRole )
    return mDisplayStrings.value( index.row() );
  else if ( role == KeyFieldRole )
    return mKeys.value( index.row() );

  return QVariant();
}

QHash<int, QByteArray> FeatureListModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

  roles[KeyFieldRole] = "keyField";

  return roles;
}

QgsVectorLayer* FeatureListModel::currentLayer() const
{
  return mCurrentLayer;
}

void FeatureListModel::setCurrentLayer( QgsVectorLayer* currentLayer )
{
  if ( mCurrentLayer == currentLayer )
    return;

  if ( mCurrentLayer )
  {
    disconnect( currentLayer, &QgsVectorLayer::featureAdded, this, &FeatureListModel::onFeatureAdded );
    disconnect( currentLayer, &QgsVectorLayer::featureDeleted, this, &FeatureListModel::onFeatureDeleted );
  }

  mCurrentLayer = currentLayer;

  if ( mCurrentLayer )
  {
    connect( currentLayer, &QgsVectorLayer::featureAdded, this, &FeatureListModel::onFeatureAdded );
    connect( currentLayer, &QgsVectorLayer::featureDeleted, this, &FeatureListModel::onFeatureDeleted );
  }

  reloadLayer();

  emit currentLayerChanged();
}

QString FeatureListModel::keyField() const
{
  return mKeyField;
}

void FeatureListModel::setKeyField( const QString& keyField )
{
  if ( mKeyField == keyField )
    return;

  mKeyField = keyField;

  reloadLayer();

  emit keyFieldChanged();
}

int FeatureListModel::findKey( const QVariant& key )
{
  return mKeys.indexOf( key );
}

void FeatureListModel::onFeatureAdded()
{
  reloadLayer();
}

void FeatureListModel::onFeatureDeleted()
{
  reloadLayer();
}

void FeatureListModel::reloadLayer()
{
  mDisplayStrings.clear();
  mKeys.clear();

  QgsFeatureRequest request;
  QgsExpressionContext context = mCurrentLayer->createExpressionContext();
  QgsExpression expression( mCurrentLayer->displayExpression() );
  expression.prepare( &context );

  QSet<QString> referencedColumns = expression.referencedColumns();

  if ( !keyField().isNull() )
    referencedColumns << mKeyField;

  QgsFields fields = mCurrentLayer->fields();

  request.setSubsetOfAttributes( referencedColumns, fields );

  int keyIndex = fields.indexOf( mKeyField );

  QgsFeatureIterator iterator = mCurrentLayer->getFeatures( request );

  QgsFeature feature;

  QStringList displayStrings;
  QVariantList keys;

  while ( iterator.nextFeature( feature ) )
  {
    context.setFeature( feature );
    displayStrings.append( expression.evaluate( &context ).toString() );
    keys.append( feature.attribute( keyIndex ) );
  }

  beginResetModel();
  mDisplayStrings = displayStrings;
  mKeys = keys;
  endResetModel();
}
