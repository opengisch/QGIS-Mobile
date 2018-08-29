/***************************************************************************
              featurelistmodelhighlight.h
               ----------------------------------------------------
              date                 : 9.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FEATURELISTMODELHIGHLIGHT_H
#define FEATURELISTMODELHIGHLIGHT_H

#include <QtQuick/QQuickItem>

#include "multifeaturelistmodel.h"
#include "featurelistmodelselection.h"

#include "mapsettings.h"

/**
 * Creates map highlights for a series of geometries provided by a FeatureListModel.
 * The highlights are compatible with the QtQuick scene graph.
 */
class FeatureListModelHighlight : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY( MultiFeatureListModel* model MEMBER mModel NOTIFY modelChanged )
    Q_PROPERTY( FeatureListModelSelection* selection READ selection WRITE setSelection NOTIFY selectionChanged )
    Q_PROPERTY( QColor color MEMBER mColor NOTIFY colorChanged )
    Q_PROPERTY( QColor selectionColor MEMBER mSelectionColor NOTIFY selectionColorChanged )
    Q_PROPERTY( unsigned int width MEMBER mWidth NOTIFY widthChanged )
    Q_PROPERTY( MapSettings* mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

  public:
    explicit FeatureListModelHighlight( QQuickItem *parent = nullptr );

    void setSelection( FeatureListModelSelection* selection );
    FeatureListModelSelection* selection() const;

  signals:
    void modelChanged();
    void selectionChanged();
    void colorChanged();
    void mapCanvasChanged();
    void selectionColorChanged();
    void widthChanged();
    void mapSettingsChanged();

  private slots:
    void onDataChanged();
    void onModelDataChanged();
    void onSelectionChanged();

  private:
    virtual QSGNode* updatePaintNode( QSGNode *n, UpdatePaintNodeData * ) override;

    QColor mColor;
    QColor mSelectionColor;
    MultiFeatureListModel* mModel;
    FeatureListModelSelection* mSelection;
    bool mDirty;
    unsigned int mWidth;
    MapSettings* mMapSettings;
};

#endif // FEATURELISTMODELHIGHLIGHT_H
