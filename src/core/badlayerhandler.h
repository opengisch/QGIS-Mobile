/***************************************************************************
  badlayerhandler.h - BadLayerHandler

 ---------------------
 begin                : 3.10.2016
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
#ifndef BADLAYERHANDLER_H
#define BADLAYERHANDLER_H

#include <QStandardItemModel>
#include <qgsprojectbadlayerhandler.h>

class QgsProject;

class BadLayerHandler : public QStandardItemModel, public QgsProjectBadLayerHandler
{
    Q_OBJECT

    Q_PROPERTY( QgsProject* project READ project WRITE setProject NOTIFY projectChanged )

  public:
    enum Roles
    {
      DataSourceRole = Qt::UserRole,
      LayerNameRole
    };

    BadLayerHandler( QObject* parent = nullptr );

    QHash<int, QByteArray> roleNames() const override;

    QgsProject* project() const;
    void setProject( QgsProject* project );

    void handleBadLayers( const QList<QDomNode>& layers ) override;

  signals:
    void projectChanged();
    void badLayersFound();

  private:
    QString layerName( const QDomNode& layerNode ) const;

    QgsProject* mProject;
};

#endif // BADLAYERHANDLER_H
