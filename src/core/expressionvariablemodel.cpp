/***************************************************************************
  expressionvariablemodel.cpp - ExpressionVariableModel

 ---------------------
 begin                : 29.9.2016
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

#include "expressionvariablemodel.h"
#include "qgsexpressioncontext.h"
#include <QSettings>
#include <QDebug>

ExpressionVariableModel::ExpressionVariableModel( QObject* parent )
  : QStandardItemModel( parent )
{
  reloadVariables();

  connect( this, &QStandardItemModel::dataChanged, this, &ExpressionVariableModel::onDataChanged );
}

bool ExpressionVariableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  return QStandardItemModel::setData( index, value, role );
}

void ExpressionVariableModel::addCustomVariable( const QString& varName, const QString& varVal )
{
  QStandardItem* nameItem = new QStandardItem( varName );
  nameItem->setData( varName, VariableName );
  nameItem->setData( varVal, VariableValue );
  nameItem->setEditable( true );

  insertRow( rowCount(), QList<QStandardItem*>() << nameItem );
}

void ExpressionVariableModel::removeCustomVariable( int row )
{
  removeRow( row );
}

void ExpressionVariableModel::save()
{
  QVariantMap variables;
  for ( int i = 0; i < rowCount() - 1; ++i )
  {
    if ( item( i )->isEditable() )
    {
      variables.insert( item( i )->data( VariableName ).toString(), item( i )->data( VariableValue ) );
    }
  }

  QgsExpressionContextUtils::setGlobalVariables( variables );
}

void ExpressionVariableModel::reloadVariables()
{
  clear();

  QScopedPointer<QgsExpressionContextScope> scope( QgsExpressionContextUtils::globalScope() );

  QStringList variableNames = scope->variableNames();
  variableNames.sort();

  // First add readonly variables
  for ( const QString& varName : variableNames )
  {
    if ( scope->isReadOnly( varName ) )
    {
      QStandardItem* nameItem = new QStandardItem( varName );
      nameItem->setData( varName, VariableName );
      nameItem->setData( scope->variable( varName ), VariableValue );
      nameItem->setEditable( false );

      insertRow( rowCount(), QList<QStandardItem*>() << nameItem );
    }
  }

  // Then add custom variables
  for ( const QString& varName : variableNames )
  {
    if ( !scope->isReadOnly( varName ) )
    {
      addCustomVariable( varName, scope->variable( varName ).toString() );
    }
  }
  addCustomVariable( QString(), QString() );
}

bool ExpressionVariableModel::isEditable( int row )
{
  QStandardItem* rowItem = item( row );
  if ( rowItem )
    return rowItem->isEditable();
  return false;
}

void ExpressionVariableModel::setName( int row, const QString& name )
{
  QStandardItem* rowItem = item( row );
  if ( rowItem )
    rowItem->setData( name, VariableName );
}

void ExpressionVariableModel::setValue( int row, const QString& value )
{
  QStandardItem* rowItem = item( row );
  if ( rowItem )
    rowItem->setData( value, VariableValue );
}

QHash<int, QByteArray> ExpressionVariableModel::roleNames() const
{
  QHash<int, QByteArray> names = QStandardItemModel::roleNames();
  names[VariableName] = "VariableName";
  names[VariableValue] = "VariableValue";
  return names;
}

void ExpressionVariableModel::onDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles )
{
  Q_UNUSED( bottomRight )
  Q_UNUSED( roles )
  if ( topLeft.row() == rowCount() - 1 )
    addCustomVariable( QString(), QString() );
}
