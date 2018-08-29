/***************************************************************************
                            featurelistmodelselection.h
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

#ifndef FEATURELISTMODELSELECTION_H
#define FEATURELISTMODELSELECTION_H

#include <QObject>
#include <QItemSelectionModel>

#include "multifeaturelistmodel.h"

class FeatureListModelSelection : public QObject
{
    Q_OBJECT
    Q_PROPERTY( MultiFeatureListModel* model READ model WRITE setModel NOTIFY modelChanged )
    Q_PROPERTY( int selection READ selection WRITE setSelection NOTIFY selectionChanged )
    Q_PROPERTY( QgsVectorLayer* selectedLayer READ selectedLayer NOTIFY selectionChanged )
    Q_PROPERTY( QgsFeature selectedFeature READ selectedFeature NOTIFY selectionChanged )

  public:
    explicit FeatureListModelSelection( QObject *parent = nullptr );

    int selection();
    void setSelection( int selection );

    MultiFeatureListModel* model() const;
    void setModel( MultiFeatureListModel* model );

    QgsVectorLayer* selectedLayer() const;
    const QgsFeature selectedFeature() const;

  signals:
    void modelChanged();
    void selectionChanged();

  private:
    MultiFeatureListModel* mModel;
    QItemSelectionModel* mSelection;
};

#endif // FEATURELISTMODELSELECTION_H
