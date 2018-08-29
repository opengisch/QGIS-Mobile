/***************************************************************************
  modelhelper.cpp - ModelHelper

 ---------------------
 begin                : 24.2.2016
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
#include "modelhelper.h"

ModelHelper::ModelHelper( QObject* parent )
  : QObject( parent )
  , mModel( nullptr )
{

}

QModelIndex ModelHelper::index( int row, int column )
{
  if ( !mModel )
    return QModelIndex();

  return mModel->index( row, column );
}

int ModelHelper::role( QString roleName )
{
  if ( !mModel )
    return -1;

  return mModel->roleNames().key( roleName.toLatin1() );
}

QVariant ModelHelper::data( int row, int column, QString roleName )
{
  if ( !mModel )
    return QVariant();

  return mModel->data( mModel->index( row, column ), mModel->roleNames().key( roleName.toLatin1() ) );
}

void ModelHelper::setModel( QAbstractItemModel* model )
{
  if ( mModel != model )
  {
    mModel = model;
    emit modelChanged();
  }
}

QAbstractItemModel*ModelHelper::model() const
{
  return mModel;
}
