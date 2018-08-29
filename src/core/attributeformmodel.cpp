/***************************************************************************
  attributeformmodel.cpp - attributeformmodel

 ---------------------
 begin                : 22.9.2016
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

#include "attributeformmodel.h"
#include "attributeformmodelbase.h"

AttributeFormModel::AttributeFormModel( QObject* parent )
  : QSortFilterProxyModel( parent )
  , mSourceModel( new AttributeFormModelBase( this ) )
{
  setSourceModel( mSourceModel );
  connect( mSourceModel, &AttributeFormModelBase::hasTabsChanged, this, &AttributeFormModel::hasTabsChanged );
  connect( mSourceModel, &AttributeFormModelBase::featureModelChanged, this, &AttributeFormModel::featureModelChanged );
  connect( mSourceModel, &AttributeFormModelBase::featureChanged, this, &AttributeFormModel::featureChanged );
  connect( mSourceModel, &AttributeFormModelBase::constraintsValidChanged, this, &AttributeFormModel::constraintsValidChanged );
}

bool AttributeFormModel::hasTabs() const
{
  return mSourceModel->hasTabs();
}

void AttributeFormModel::setHasTabs( bool hasTabs )
{
  mSourceModel->setHasTabs( hasTabs );
}

FeatureModel* AttributeFormModel::featureModel() const
{
  return mSourceModel->featureModel();
}

void AttributeFormModel::setFeatureModel( FeatureModel* featureModel )
{
  mSourceModel->setFeatureModel( featureModel );
}

bool AttributeFormModel::constraintsValid() const
{
  return mSourceModel->constraintsValid();
}

void AttributeFormModel::save()
{
  mSourceModel->save();
}

void AttributeFormModel::create()
{
  mSourceModel->create();
}

QVariant AttributeFormModel::attribute( const QString& name )
{
  return mSourceModel->attribute( name );
}

bool AttributeFormModel::filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const
{
  return mSourceModel->data( mSourceModel->index( source_row, 0, source_parent ), CurrentlyVisible ).toBool();
}
