/***************************************************************************
                            featuremodel.h
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FEATUREMODEL_H
#define FEATUREMODEL_H

#include <QAbstractListModel>
#include <QGeoPositionInfoSource>
#include <memory>
#include "geometry.h"

class VertexModel;

class FeatureModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( QgsFeature feature READ feature WRITE setFeature NOTIFY featureChanged )
    //! the vertex model is used to highlight vertices on the map
    Q_PROPERTY( VertexModel* vertexModel READ vertexModel WRITE setVertexModel NOTIFY vertexModelChanged )
    Q_PROPERTY( Geometry* geometry MEMBER mGeometry NOTIFY geometryChanged )
    Q_PROPERTY( QgsVectorLayer* currentLayer READ layer WRITE setCurrentLayer NOTIFY currentLayerChanged )
    Q_PROPERTY( QString positionSourceName READ positionSourceName WRITE setPositionSourceName NOTIFY positionSourceChanged )
    Q_ENUMS( FeatureRoles )

  public:
    enum FeatureRoles
    {
      AttributeName = Qt::UserRole + 1,
      AttributeValue,
      Field,
      RememberAttribute
    };

    explicit FeatureModel( QObject *parent = nullptr );
    explicit FeatureModel( const QgsFeature& feat, QObject *parent = nullptr );

    void setFeature( const QgsFeature& feature );

    /**
     * Return the feature wrapped in a QVariant for passing it around in QML
     */
    QgsFeature feature() const;

    void setCurrentLayer( QgsVectorLayer* layer );
    QgsVectorLayer* layer() const;

    //! \copydoc vertexModel
    VertexModel *vertexModel();
    //! \copydoc vertexModel
    void setVertexModel( VertexModel *model );

    QHash<int, QByteArray> roleNames() const override;
    int rowCount( const QModelIndex& parent ) const override;
    QVariant data( const QModelIndex& index, int role ) const override;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    /**
     * Will commit the edit buffer of this layer.
     * May change in the future to only commit the changes buffered in this model.
     *
     * @return Success of the operation
     */
    Q_INVOKABLE bool save();

    /**
     * Will reset the feature to the original values and dismiss any buffered edits.
     */
    Q_INVOKABLE void reset();
    Q_INVOKABLE void create();

    Q_INVOKABLE bool suppressFeatureForm() const;

    Q_INVOKABLE void resetAttributes();

    QVector<bool> rememberedAttributes() const;

    /**
     * The name of the position source to use.
     * A QGeoPositionInfoSource is created internally based on this name.
     */
    QString positionSourceName() const;

    /**
     * The name of the position source to use.
     * A QGeoPositionInfoSource is created internally based on this name.
     */
    void setPositionSourceName( const QString &positionSourceName );

    //! Apply the vertex model to the feature geometry.
    //! \note This shall be used if the feature model is used with the vertex model rather than the geometry and rubberband model
    Q_INVOKABLE void applyVertexModelToGeometry();

  public slots:
    void applyGeometry();

  signals:
    void featureChanged();
    void vertexModelChanged();
    void geometryChanged();
    void currentLayerChanged();
    void positionSourceChanged();

    void warning( const QString& text );

  private:
    bool commit();
    bool startEditing();

    QgsVectorLayer* mLayer;
    QgsFeature mFeature;
    VertexModel *mVertexModel = nullptr;
    Geometry* mGeometry;
    QVector<bool> mRememberedAttributes;
    std::unique_ptr<QGeoPositionInfoSource> mPositionSource;
    QString mTempName;
};

#endif // FEATUREMODEL_H
