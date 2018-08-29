/***************************************************************************
  layertreemodel.h - LayerTree

 ---------------------
 begin                : 6.12.2016
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
#ifndef LAYERTREEMODEL_H
#define LAYERTREEMODEL_H

#include <QSortFilterProxyModel>

class QgsLayerTree;
class QgsLayerTreeModel;
class QgsProject;

class LayerTreeModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY( QString mapTheme READ mapTheme WRITE setMapTheme NOTIFY mapThemeChanged )

  public:
    enum Roles
    {
      VectorLayer = Qt::UserRole + 1,
      LegendImage,
      Type,
      Visible
    };
    Q_ENUMS( Roles )

    explicit LayerTreeModel( QgsLayerTree* layerTree, QgsProject* project, QObject* parent = nullptr );

    Q_INVOKABLE QVariant data( const QModelIndex& index, int role ) const override;

    Q_INVOKABLE bool setData( const QModelIndex& index, const QVariant &value, int role ) override;

    QHash<int, QByteArray> roleNames() const override;

    QgsLayerTreeModel* layerTreeModel() const;

    QgsLayerTree* layerTree() const;

    QString mapTheme() const;
    void setMapTheme( const QString& mapTheme );

    QgsProject* project() const;

  signals:
    void mapThemeChanged();

  private:
    QgsLayerTreeModel* mLayerTreeModel;
    QString mMapTheme;
    QgsProject* mProject;
};

#endif // LAYERTREEMODEL_H
