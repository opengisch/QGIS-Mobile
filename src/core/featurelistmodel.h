/***************************************************************************
  featurelistmodel.h - FeatureListModel

 ---------------------
 begin                : 1.2.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FEATURELISTMODEL_H
#define FEATURELISTMODEL_H

#include <QAbstractItemModel>

class QgsVectorLayer;

/**
 * Provides access to a list of features from a layer.
 * For each feature, the display expression is exposed as DisplayRole
 * and a keyField as KeyFieldRole for a unique identifier.
 */
class FeatureListModel : public QAbstractItemModel
{
    Q_OBJECT

    /**
     * The vector layer to list
     */
    Q_PROPERTY( QgsVectorLayer* currentLayer READ currentLayer WRITE setCurrentLayer NOTIFY currentLayerChanged )
    /**
     * The primary key field
     */
    Q_PROPERTY( QString keyField READ keyField WRITE setKeyField NOTIFY keyFieldChanged )

  public:
    enum FeatureListRoles
    {
      KeyFieldRole = Qt::UserRole + 1
    };

    Q_ENUM( FeatureListRoles )

    FeatureListModel();

    virtual QModelIndex index( int row, int column, const QModelIndex& parent ) const override;
    virtual QModelIndex parent( const QModelIndex& child ) const override;
    virtual int rowCount( const QModelIndex& parent ) const override;
    virtual int columnCount( const QModelIndex& parent ) const override;
    virtual QVariant data( const QModelIndex& index, int role ) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    QgsVectorLayer* currentLayer() const;
    void setCurrentLayer( QgsVectorLayer* currentLayer );

    QString keyField() const;
    void setKeyField( const QString& keyField );

    /**
     * Get the row for a given key value.
     */
    Q_INVOKABLE int findKey( const QVariant& key );
  signals:
    void currentLayerChanged();
    void keyFieldChanged();

  private slots:
    void onFeatureAdded();
    void onFeatureDeleted();

  private:
    void reloadLayer();

    QgsVectorLayer* mCurrentLayer;

    QStringList mDisplayStrings;
    QVariantList mKeys;
    QString mKeyField;
};

#endif // FEATURELISTMODEL_H
