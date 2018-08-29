/***************************************************************************
  modelhelper.h - ModelHelper

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
#ifndef MODELHELPER_H
#define MODELHELPER_H

#include <QObject>
#include <QAbstractItemModel>

class ModelHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged )
  public:
    explicit ModelHelper( QObject* parent = nullptr );

    Q_INVOKABLE QModelIndex index( int row, int column );

    Q_INVOKABLE int role( QString roleName );

    Q_INVOKABLE QVariant data( int row, int column, QString roleName );

    void setModel( QAbstractItemModel* model );

    QAbstractItemModel* model() const;

  signals:
    void modelChanged();

  private:
    QAbstractItemModel* mModel;
};

#endif // MODELHELPER_H
