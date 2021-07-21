/***************************************************************************
  attributeformmodel.h - attributeformmodel

 ---------------------
 begin                : 22.9.2016
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
#ifndef ATTRIBUTEFORMMODEL_H
#define ATTRIBUTEFORMMODEL_H

#include "attributeformmodelbase.h"

#include <QSortFilterProxyModel>

class FeatureModel;

class AttributeFormModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY( FeatureModel *featureModel READ featureModel WRITE setFeatureModel NOTIFY featureModelChanged )
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )
    Q_PROPERTY( bool constraintsHardValid READ constraintsHardValid NOTIFY constraintsHardValidChanged )
    Q_PROPERTY( bool constraintsSoftValid READ constraintsSoftValid NOTIFY constraintsSoftValidChanged )

  public:
    enum FeatureRoles
    {
      ElementType = Qt::UserRole + 1,
      Name,
      AttributeValue,
      AttributeEditable,
      EditorWidget,
      EditorWidgetConfig,
      RelationEditorWidget,
      RelationEditorWidgetConfig,
      RememberValue,
      Field,
      RelationId,
      NmRelationId,
      FieldIndex,
      Group,
      AttributeEditorElement,
      CurrentlyVisible,
      ConstraintHardValid,
      ConstraintSoftValid,
      ConstraintDescription,
      AttributeAllowEdit,
      EditorWidgetCode, //<! Returns a QML or HTML code string used by the relevant widgets
      TabIndex,
      Color,
    };

    Q_ENUM( FeatureRoles )

    explicit AttributeFormModel( QObject *parent = nullptr );

    bool hasTabs() const;
    void setHasTabs( bool hasTabs );

    FeatureModel *featureModel() const;
    void setFeatureModel( FeatureModel *featureModel );

    bool constraintsHardValid() const;
    bool constraintsSoftValid() const;

    /**
     * Save the current (already existing) feature
     */
    Q_INVOKABLE bool save();

    /**
     * Create the current (not existing yet) feature
     */
    Q_INVOKABLE bool create();

    /**
     * Delete the current feature
     */
    Q_INVOKABLE bool deleteFeature();

    /**
     * Get the attribute of the current feature by name
     * \param name the name of the attribute
     * \return value of the attribute
     */
    Q_INVOKABLE QVariant attribute( const QString &name );

  signals:
    void featureModelChanged();
    void hasTabsChanged();
    void featureChanged();
    void constraintsHardValidChanged();
    void constraintsSoftValidChanged();

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    AttributeFormModelBase *mSourceModel = nullptr;
};

#endif // ATTRIBUTEFORMMODEL_H
