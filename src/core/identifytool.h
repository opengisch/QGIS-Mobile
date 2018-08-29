/***************************************************************************
  identifytool.h - IdentifyTool

 ---------------------
 begin                : 30.8.2016
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
#ifndef IDENTIFYTOOL_H
#define IDENTIFYTOOL_H

#include <QObject>

#include <qgsfeature.h>
#include <qgspoint.h>
#include <qgsmapsettings.h>
#include <qgsrendercontext.h>

class QgsMapLayer;
class MapSettings;
class QgsVectorLayer;
class MultiFeatureListModel;

class IdentifyTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY( MapSettings* mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )
    Q_PROPERTY( double searchRadiusMm READ searchRadiusMm WRITE setSearchRadiusMm NOTIFY searchRadiusMmChanged )
    Q_PROPERTY( MultiFeatureListModel* model READ model WRITE setModel NOTIFY modelChanged )

  public:
    struct IdentifyResult
    {
      IdentifyResult ( QgsMapLayer* layer, const QgsFeature& feature )
        : layer( layer )
        , feature( feature )
      {}

      QgsMapLayer* layer;
      QgsFeature feature;
    };

  public:
    explicit IdentifyTool( QObject *parent = nullptr );

    MapSettings* mapSettings() const;
    void setMapSettings( MapSettings* mapSettings );

    double searchRadiusMm() const;
    void setSearchRadiusMm( double searchRadiusMm );

    MultiFeatureListModel* model() const;
    void setModel( MultiFeatureListModel* model );

  signals:
    void mapSettingsChanged();
    void searchRadiusMmChanged();
    void modelChanged();

  public slots:
    void identify( const QPointF& point ) const;

    QList<IdentifyResult> identifyVectorLayer( QgsVectorLayer* layer, const QgsPointXY& point ) const;

  private:
    MapSettings* mMapSettings;
    MultiFeatureListModel* mModel;

    double searchRadiusMU( const QgsRenderContext& context ) const;
    double searchRadiusMU() const;

    QgsRectangle toLayerCoordinates( QgsMapLayer* layer, const QgsRectangle& rect ) const;

    double mSearchRadiusMm;
};

#endif // IDENTIFYTOOL_H
