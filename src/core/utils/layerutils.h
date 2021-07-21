/***************************************************************************
  layerutils.h - LayerUtils

 ---------------------
 begin                : 01.03.2021
 copyright            : (C) 2020 by Mathieu Pellerin
 email                : mathieu@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LAYERUTILS_H
#define LAYERUTILS_H

#include <QObject>

#include <qgis.h>
#include <qgsvectorlayer.h>

class QgsVectorLayer;
class QgsSymbol;

class LayerUtils : public QObject
{
    Q_OBJECT

  public:
    explicit LayerUtils( QObject *parent = nullptr );

    /**
    * Returns the default symbol for a given layer.
    * \param layer the vector layer used to create the default symbol
    */
    static QgsSymbol *defaultSymbol( QgsVectorLayer *layer );

    /**
    * Returns TRUE if the vector layer is used as an atlas coverage layer in
    * any of the print layouts of the currently opened project.
    * \param layer the vector layer to check against print layouts
    */
    static Q_INVOKABLE bool isAtlasCoverageLayer( QgsVectorLayer *layer );

    /**
     * Selects features in a layer
     * This method is required since QML cannot perform the conversion of a feature ID to a QgsFeatureId
     * \param layer the vector layer
     * \param fids the list of feature IDs
     * \param behavior the selection behavior
     */
#if _QGIS_VERSION_INT >= 32100
    static Q_INVOKABLE void selectFeaturesInLayer( QgsVectorLayer *layer, const QList<int> &fids, Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection );
#else
    static Q_INVOKABLE void selectFeaturesInLayer( QgsVectorLayer *layer, const QList<int> &fids, QgsVectorLayer::SelectBehavior behavior = QgsVectorLayer::SetSelection );
#endif

    /**
     * Returns the QVariant typeName of a \a field.
     * This is a stable identifier (compared to the provider field name).
     */
    Q_INVOKABLE QString fieldType( const QgsField &field ) const;
};

#endif // LAYERUTILS_H
