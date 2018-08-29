/***************************************************************************
                            featurelistmodelselection.cpp
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

#include <QDebug>

#include <qgsvectorlayer.h>
#include "featurelistmodelselection.h"

FeatureListModelSelection::FeatureListModelSelection( QObject *parent )
  : QObject( parent )
  , mSelection( 0 )
{
}

int FeatureListModelSelection::selection()
{
  if ( mSelection  && mSelection->selectedIndexes().count() )
  {
    return mSelection->selectedIndexes().first().row();
  }
  return -1;
}

void FeatureListModelSelection::setSelection( int selection )
{
  if ( mSelection )
  {
    if ( !mSelection->selectedIndexes().count() || mSelection->selectedIndexes().first().row() != selection )
    {
      mSelection->select( mModel->index( selection, 0 ), QItemSelectionModel::ClearAndSelect );
      emit selectionChanged();
    }
  }
}

MultiFeatureListModel* FeatureListModelSelection::model() const
{
  return mModel;
}

void FeatureListModelSelection::setModel( MultiFeatureListModel* model )
{
  if ( mModel != model )
  {
    delete mSelection;
    mSelection = new QItemSelectionModel( model );
    mModel = model;
    emit modelChanged();
  }
}

QgsVectorLayer*FeatureListModelSelection::selectedLayer() const
{
  if ( mSelection->selectedIndexes().count() )
  {
    return mModel->data( mSelection->selectedIndexes().first(), MultiFeatureListModel::LayerRole ).value<QgsVectorLayer*>();
  }
  return nullptr;
}

const QgsFeature FeatureListModelSelection::selectedFeature() const
{
  if ( mSelection->selectedIndexes().count() )
  {
    return mModel->data( mSelection->selectedIndexes().first(), MultiFeatureListModel::FeatureRole ).value<QgsFeature>();
  }
  return QgsFeature();
}
