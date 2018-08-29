/***************************************************************************
  printlayoutlistmodel.cpp
 ---------------------
 begin                : 20.8.2018
 copyright            : (C) 2018 by david
 email                : david at opengisch dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "printlayoutlistmodel.h"
#include <qgsproject.h>

PrintLayoutListModel::PrintLayoutListModel(QObject* parent)
  : QAbstractListModel( parent )
{
}

QHash<int, QByteArray> PrintLayoutListModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[TitleRow]  = "Title";
  roles[IndexRow] = "Index";

  return roles;
}

QgsProject* PrintLayoutListModel::project() const
{
  return mProject;
}

void PrintLayoutListModel::setProject( QgsProject* project )
{
  if( mProject == project )
    return;

  mProject = project;
}

void PrintLayoutListModel::reloadModel()
{
  beginResetModel();
  mPrintLayouts.clear();

  const QList< QgsPrintLayout * > layouts( mProject->layoutManager()->printLayouts() );

  for ( const auto &layout : layouts )
  {
    mPrintLayouts.append( PrintLayout( layout->name() ) );
  }
  endResetModel();

}

int PrintLayoutListModel::rowCount( const QModelIndex& parent ) const
{
  if ( !parent.isValid() )
    return mPrintLayouts.size();
  else
    return 0;
}

QVariant PrintLayoutListModel::data( const QModelIndex& index, int role ) const
{
  if ( index.row() >= mPrintLayouts.size() || index.row() < 0 )
    return QVariant();

  if ( role == TitleRow )
    return mPrintLayouts.at( index.row() ).title;
  else if ( role == IndexRow )
    return  index.row();

  return QVariant();
}
