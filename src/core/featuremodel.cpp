/***************************************************************************
                            featuremodel.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "featuremodel.h"
#include "expressioncontextutils.h"
#include "vertexmodel.h"

#include <qgsmessagelog.h>
#include <qgsvectorlayer.h>
#include <QGeoPositionInfoSource>
#include <QDebug>

FeatureModel::FeatureModel( QObject *parent )
  : QAbstractListModel( parent )
{
  connect( this, &FeatureModel::modelReset, this, &FeatureModel::featureChanged );
}

void FeatureModel::setFeature( const QgsFeature& feature )
{
  if ( mFeature == feature )
    return;

  beginResetModel();
  mFeature = feature;
  endResetModel();
  emit featureChanged();
}

void FeatureModel::setCurrentLayer( QgsVectorLayer* layer )
{
  if ( layer == mLayer )
    return;

  mLayer = layer;
  if ( mLayer )
  {
    mFeature = QgsFeature( mLayer->fields() );

    mRememberedAttributes.resize( layer->fields().size() );
    mRememberedAttributes.fill( false );
  }

  emit currentLayerChanged();
}

QgsVectorLayer* FeatureModel::layer() const
{
  return mLayer;
}

VertexModel *FeatureModel::vertexModel()
{
  return mVertexModel;
}

void FeatureModel::setVertexModel( VertexModel *model )
{
  if ( model== mVertexModel )
    return;

  mVertexModel = model;
  emit vertexModelChanged();
}

QgsFeature FeatureModel::feature() const
{
  return mFeature;
}

QHash<int, QByteArray> FeatureModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[AttributeName]  = "AttributeName";
  roles[AttributeValue] = "AttributeValue";
  roles[Field] = "Field";
  roles[RememberAttribute] = "RememberAttribute";

  return roles;
}


int FeatureModel::rowCount( const QModelIndex& parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeature.attributes().count();
}

QVariant FeatureModel::data( const QModelIndex& index, int role ) const
{
  if ( mLayer )
    qWarning() << "Get data " << mLayer->name();

  switch ( role )
  {
    case AttributeName:
      return mLayer->attributeDisplayName( index.row() );
      break;

    case AttributeValue:
      return mFeature.attribute( index.row() );
      break;

    case Field:
      return mLayer->fields().at( index.row() );
      break;

    case RememberAttribute:
      return mRememberedAttributes.at( index.row() );
      break;
  }

  return QVariant();
}

bool FeatureModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( data( index, role ) == value )
    return true;

  switch( role )
  {
    case AttributeValue:
    {
      QVariant val( value );
      QgsField fld = mFeature.fields().at( index.row() );

      if ( !fld.convertCompatible( val ) )
      {
        QgsMessageLog::logMessage( tr( "Value \"%1\" %4 could not be converted to a compatible value for field %2(%3)." ).arg( value.toString(), fld.name(), fld.typeName(), value.isNull() ? "NULL" : "NOT NULL" ) );
        return false;
      }
      bool success = mFeature.setAttribute( index.row(), val );
      if ( success )
        emit dataChanged( index, index, QVector<int>() << role );
      return success;
      break;
    }

    case RememberAttribute:
    {
      mRememberedAttributes[ index.row() ] = value.toBool();
      emit dataChanged( index, index, QVector<int>() << role );
      break;
    }
  }

  return false;
}

bool FeatureModel::save()
{
  if ( !mLayer )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  QgsFeature feat = mFeature;
  if ( !mLayer->updateFeature( feat ) )
    QgsMessageLog::logMessage( tr( "Cannot update feature" ), "QField", Qgis::Warning );
  rv = commit();

  if ( rv )
  {
    QgsFeature feat;
    if ( mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.id() ) ).nextFeature( feat ) )
      setFeature( feat );
    else
      QgsMessageLog::logMessage( tr( "Feature %1 could not be fetched after commit" ).arg( mFeature.id() ), "QField", Qgis::Warning );
  }
  return rv;
}

void FeatureModel::reset()
{
  if ( !mLayer )
    return;

  mLayer->rollBack();
}

bool FeatureModel::suppressFeatureForm() const
{
  if ( !mLayer )
    return false;

  return mLayer->editFormConfig().suppress();
}

void FeatureModel::resetAttributes()
{
  if ( !mLayer )
    return;

  QgsExpressionContext expressionContext = mLayer->createExpressionContext();
  expressionContext << ExpressionContextUtils::positionScope( mPositionSource.get() );
  expressionContext.setFeature( mFeature );

  QgsFields fields = mLayer->fields();

  beginResetModel();
  for ( int i = 0; i < fields.count(); ++i )
  {
    if ( !mRememberedAttributes.at( i ) )
    {
      if ( fields.at( i ).defaultValueDefinition().isValid() )
      {
        QgsExpression exp( fields.at( i ).defaultValueDefinition().expression() );
        exp.prepare( &expressionContext );
        if ( exp.hasParserError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has parser error: %3" ).arg( mLayer->name(), fields.at( i ).name(), exp.parserErrorString() ), QStringLiteral( "QField" ) );

        QVariant value = exp.evaluate( &expressionContext );

        if ( exp.hasEvalError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has evaluation error: %3" ).arg( mLayer->name(), fields.at( i ).name(), exp.evalErrorString() ), QStringLiteral( "QField" ) );


        mFeature.setAttribute( i, value );
      }
      else
      {
        mFeature.setAttribute( i, QVariant() );
      }
    }
  }
  endResetModel();
}

void FeatureModel::applyGeometry()
{
  mFeature.setGeometry( mGeometry->asQgsGeometry() );
}

void FeatureModel::create()
{
  if ( !mLayer )
    return;

  startEditing();
  if ( !mLayer->addFeature( mFeature ) )
  {
    QgsMessageLog::logMessage( tr( "Feature could not be added" ), "QField", Qgis::Critical );
  }
  commit();
}

bool FeatureModel::commit()
{
  if ( !mLayer->commitChanges() )
  {
    QgsMessageLog::logMessage( tr( "Could not save changes. Rolling back." ), "QField", Qgis::Critical );
    mLayer->rollBack();
    return false;
  }
  else
  {
    return true;
  }
}

bool FeatureModel::startEditing()
{
  // Already an edit session active
  if ( mLayer->editBuffer() )
    return true;

  if ( !mLayer->startEditing() )
  {
    QgsMessageLog::logMessage( tr( "Cannot start editing" ), "QField", Qgis::Warning );
    return false;
  }
  else
  {
    return true;
  }
}

QString FeatureModel::positionSourceName() const
{
  return mPositionSource ? mPositionSource->sourceName() : QString();
}

void FeatureModel::setPositionSourceName( const QString& positionSourceName )
{
  if ( mPositionSource && mPositionSource->sourceName() == positionSourceName )
    return;

  mPositionSource.reset( QGeoPositionInfoSource::createSource( positionSourceName, this ) );
  emit positionSourceChanged();
}

void FeatureModel::applyVertexModelToGeometry()
{
  if ( !mVertexModel )
    return;

  mFeature.setGeometry( mVertexModel->geometry() );
}

QVector<bool> FeatureModel::rememberedAttributes() const
{
  return mRememberedAttributes;
}
