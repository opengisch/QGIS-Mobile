/***************************************************************************
  expressionvariablemodel.h - ExpressionVariableModel

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
#ifndef EXPRESSIONVARIABLEMODEL_H
#define EXPRESSIONVARIABLEMODEL_H

#include <QStandardItemModel>

class ExpressionVariableModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    enum Roles
    {
      VariableName = Qt::UserRole,
      VariableValue
    };

    ExpressionVariableModel( QObject* parent = nullptr );

    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    Q_INVOKABLE void addCustomVariable( const QString& varName, const QString& varVal );

    Q_INVOKABLE void removeCustomVariable( int row );

    Q_INVOKABLE void save();

    Q_INVOKABLE void reloadVariables();

    Q_INVOKABLE bool isEditable( int row );

    Q_INVOKABLE void setName( int row, const QString& name );

    Q_INVOKABLE void setValue( int row, const QString& value );

    QHash<int, QByteArray> roleNames() const override;

  private slots:
    void onDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles );
};

#endif // EXPRESSIONVARIABLEMODEL_H
