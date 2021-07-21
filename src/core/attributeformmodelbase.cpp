/***************************************************************************
  attributeformmodelbase.cpp - AttributeFormModelBase

 ---------------------
 begin                : 16.8.2016
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

#include "attributeformmodel.h"
#include "attributeformmodelbase.h"

#include <QRegularExpression>
#include <qgsattributeeditorcontainer.h>
#include <qgsattributeeditorelement.h>
#include <qgsattributeeditorfield.h>
#include <qgsattributeeditorhtmlelement.h>
#include <qgsattributeeditorqmlelement.h>
#include <qgsattributeeditorrelation.h>
#include <qgsdatetimefieldformatter.h>
#include <qgseditorwidgetsetup.h>
#include <qgsmapthemecollection.h>
#include <qgsproject.h>
#include <qgsrelationmanager.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerutils.h>


AttributeFormModelBase::AttributeFormModelBase( QObject *parent )
  : QStandardItemModel( 0, 1, parent )
{
  connect( QgsProject::instance(), &QgsProject::mapThemeCollectionChanged, this, &AttributeFormModelBase::onMapThemeCollectionChanged );
  if ( QgsProject::instance()->mapThemeCollection() )
    onMapThemeCollectionChanged();
}

AttributeFormModelBase::~AttributeFormModelBase()
{
  delete mTemporaryContainer;
}

void AttributeFormModelBase::onMapThemeCollectionChanged()
{
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeChanged, this, [ = ] { resetModel(); applyFeatureModel(); } );
}

QHash<int, QByteArray> AttributeFormModelBase::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

  roles[AttributeFormModel::ElementType] = "Type";
  roles[AttributeFormModel::Name] = "Name";
  roles[AttributeFormModel::AttributeValue] = "AttributeValue";
  roles[AttributeFormModel::AttributeEditable] = "AttributeEditable";
  roles[AttributeFormModel::EditorWidget] = "EditorWidget";
  roles[AttributeFormModel::EditorWidgetConfig] = "EditorWidgetConfig";
  roles[AttributeFormModel::RelationEditorWidget] = "RelationEditorWidget";
  roles[AttributeFormModel::RelationEditorWidgetConfig] = "RelationEditorWidgetConfig";
  roles[AttributeFormModel::RememberValue] = "RememberValue";
  roles[AttributeFormModel::Field] = "Field";
  roles[AttributeFormModel::RelationId] = "RelationId";
  roles[AttributeFormModel::NmRelationId] = "NmRelationId";
  roles[AttributeFormModel::Group] = "Group";
  roles[AttributeFormModel::ConstraintHardValid] = "ConstraintHardValid";
  roles[AttributeFormModel::ConstraintSoftValid] = "ConstraintSoftValid";
  roles[AttributeFormModel::ConstraintDescription] = "ConstraintDescription";
  roles[AttributeFormModel::AttributeAllowEdit] = "AttributeAllowEdit";
  roles[AttributeFormModel::EditorWidgetCode] = "EditorWidgetCode";
  roles[AttributeFormModel::TabIndex] = "TabIndex";
  roles[AttributeFormModel::Color] = "Color";

  return roles;
}

bool AttributeFormModelBase::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !qgsVariantEqual( data( index, role ), value ) )
  {
    switch ( role )
    {
      case AttributeFormModel::AttributeAllowEdit:
      {
        QStandardItem *item = itemFromIndex( index );
        int fieldIndex = item->data( AttributeFormModel::FieldIndex ).toInt();
        mFeatureModel->setData( mFeatureModel->index( fieldIndex ), value, FeatureModel::AttributeAllowEdit );
        item->setData( value, AttributeFormModel::AttributeAllowEdit );
        updateVisibilityAndConstraints( fieldIndex );
        break;
      }

      case AttributeFormModel::RememberValue:
      {
        QStandardItem *item = itemFromIndex( index );
        int fieldIndex = item->data( AttributeFormModel::FieldIndex ).toInt();
        mFeatureModel->setData( mFeatureModel->index( fieldIndex ), value, FeatureModel::RememberAttribute );
        item->setData( value, AttributeFormModel::RememberValue );
        break;
      }

      case AttributeFormModel::AttributeValue:
      {
        QStandardItem *item = itemFromIndex( index );
        int fieldIndex = item->data( AttributeFormModel::FieldIndex ).toInt();
        bool changed = mFeatureModel->setData( mFeatureModel->index( fieldIndex ), value, FeatureModel::AttributeValue );
        if ( changed )
        {
          item->setData( value, AttributeFormModel::AttributeValue );
          emit dataChanged( index, index, QVector<int>() << role );
        }
        updateDefaultValues( fieldIndex );
        updateVisibilityAndConstraints( fieldIndex );
        return changed;
      }
    }
  }
  return false;
}

FeatureModel *AttributeFormModelBase::featureModel() const
{
  return mFeatureModel;
}

void AttributeFormModelBase::setFeatureModel( FeatureModel *featureModel )
{
  if ( mFeatureModel == featureModel )
    return;

  if ( mFeatureModel )
  {
    disconnect( mFeatureModel, &FeatureModel::currentLayerChanged, this, &AttributeFormModelBase::resetModel );
    disconnect( mFeatureModel, &FeatureModel::modelReset, this, &AttributeFormModelBase::applyFeatureModel );
    disconnect( mFeatureModel, &FeatureModel::featureUpdated, this, &AttributeFormModelBase::applyFeatureModel );
  }

  mFeatureModel = featureModel;

  connect( mFeatureModel, &FeatureModel::currentLayerChanged, this, &AttributeFormModelBase::resetModel );
  connect( mFeatureModel, &FeatureModel::modelReset, this, &AttributeFormModelBase::applyFeatureModel );
  connect( mFeatureModel, &FeatureModel::featureUpdated, this, &AttributeFormModelBase::applyFeatureModel );

  emit featureModelChanged();
}

void AttributeFormModelBase::resetModel()
{
  clear();

  mVisibilityExpressions.clear();
  mConstraints.clear();

  if ( !mFeatureModel )
    return;

  mLayer = mFeatureModel->layer();

  if ( mLayer )
  {
    QgsAttributeEditorContainer *root;
    delete mTemporaryContainer;
    mTemporaryContainer = nullptr;

    if ( mLayer->editFormConfig().layout() == QgsEditFormConfig::TabLayout )
    {
      root = mLayer->editFormConfig().invisibleRootContainer();
    }
    else
    {
      root = generateRootContainer();
      mTemporaryContainer = root;
    }

    setHasTabs( !root->children().isEmpty() && QgsAttributeEditorElement::AeTypeContainer == root->children().first()->type() );

    invisibleRootItem()->setColumnCount( 1 );
    if ( mHasTabs )
    {
      const QList<QgsAttributeEditorElement *> children { root->children() };
      int currentTab = 0;
      for ( QgsAttributeEditorElement *element : children )
      {
        if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
        {
          QgsAttributeEditorContainer *container = static_cast<QgsAttributeEditorContainer *>( element );

          QStandardItem *item = new QStandardItem();
          item->setData( element->name(), AttributeFormModel::Name );
          item->setData( "container", AttributeFormModel::ElementType );
          item->setData( true, AttributeFormModel::CurrentlyVisible );
          item->setData( true, AttributeFormModel::ConstraintHardValid );
          item->setData( true, AttributeFormModel::ConstraintSoftValid );
          invisibleRootItem()->appendRow( item );

          if ( container->visibilityExpression().enabled() )
          {
            mVisibilityExpressions.append( qMakePair( container->visibilityExpression().data(), QVector<QStandardItem *>() << item ) );
          }

          QVector<QStandardItem *> dummy;
          flatten( container, item, QString(), dummy, currentTab, container->backgroundColor() );
          currentTab++;
        }
      }
    }
    else
    {
      QVector<QStandardItem *> dummy;
      flatten( invisibleRootContainer(), invisibleRootItem(), QString(), dummy );
    }

    mExpressionContext = mLayer->createExpressionContext();
  }
}

void AttributeFormModelBase::applyFeatureModel()
{
  for ( int i = 0; i < invisibleRootItem()->rowCount(); ++i )
  {
    updateAttributeValue( invisibleRootItem()->child( i ) );
  }
  updateVisibilityAndConstraints();
}

QgsAttributeEditorContainer *AttributeFormModelBase::generateRootContainer() const
{
  QgsAttributeEditorContainer *root = new QgsAttributeEditorContainer( QString(), nullptr );
  //get fields
  QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    if ( fields.at( i ).editorWidgetSetup().type() != QStringLiteral( "Hidden" ) )
    {
      QgsAttributeEditorField *field = new QgsAttributeEditorField( fields.at( i ).name(), i, root );
      root->addChildElement( field );
    }
  }
  //get relations
  const QList<QgsRelation> referencingRelations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );
  for ( const QgsRelation &referencingRelation : referencingRelations )
  {
    QgsAttributeEditorRelation *relation = new QgsAttributeEditorRelation( referencingRelation, root );
    root->addChildElement( relation );
  }
  return root;
}

QgsAttributeEditorContainer *AttributeFormModelBase::invisibleRootContainer() const
{
  return mTemporaryContainer ? mTemporaryContainer : mLayer->editFormConfig().invisibleRootContainer();
}

void AttributeFormModelBase::updateAttributeValue( QStandardItem *item )
{
  if ( item->data( AttributeFormModel::ElementType ) == QStringLiteral( "field" ) )
  {
    int fieldIndex = item->data( AttributeFormModel::FieldIndex ).toInt();
    QVariant attributeValue = mFeatureModel->data( mFeatureModel->index( fieldIndex ), FeatureModel::AttributeValue );
    item->setData( attributeValue.isNull() ? QVariant() : attributeValue, AttributeFormModel::AttributeValue );
    item->setData( mFeatureModel->data( mFeatureModel->index( fieldIndex ), FeatureModel::AttributeAllowEdit ), AttributeFormModel::AttributeAllowEdit );
    //set item visibility to false in case it's a linked attribute
    item->setData( !mFeatureModel->data( mFeatureModel->index( fieldIndex ), FeatureModel::LinkedAttribute ).toBool(), AttributeFormModel::CurrentlyVisible );
  }
  else if ( item->data( AttributeFormModel::ElementType ) == QStringLiteral( "qml" ) || item->data( AttributeFormModel::ElementType ) == QStringLiteral( "html" ) )
  {
    QString code = mEditorWidgetCodes[item];

    QRegularExpression re( "expression\\.evaluate\\(\\s*\\\"(.*?[^\\\\])\\\"\\s*\\)" );
    QRegularExpressionMatch match = re.match( code );
    while ( match.hasMatch() )
    {
      QString expression = match.captured( 1 );
      expression = expression.replace( QStringLiteral( "\\\"" ), QStringLiteral( "\"" ) );

      QgsExpressionContext expressionContext = mLayer->createExpressionContext();
      expressionContext.setFeature( mFeatureModel->feature() );

      QgsExpression exp = QgsExpression( expression );
      exp.prepare( &expressionContext );
      QVariant result = exp.evaluate( &expressionContext );

      QString resultString;
      switch ( static_cast<QMetaType::Type>( result.type() ) )
      {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Double:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
          resultString = result.toString();
          break;
        case QMetaType::Bool:
          resultString = result.toBool() ? QStringLiteral( "true" ) : QStringLiteral( "false" );
          break;
        default:
          resultString = QStringLiteral( "'%1'" ).arg( result.toString() );
          break;
      }
      code = code.mid( 0, match.capturedStart( 0 ) ) + resultString + code.mid( match.capturedEnd( 0 ) );
      match = re.match( code );
    }
    item->setData( code, AttributeFormModel::EditorWidgetCode );
  }
  else
  {
    for ( int i = 0; i < item->rowCount(); ++i )
    {
      updateAttributeValue( item->child( i ) );
    }
  }
}

void AttributeFormModelBase::flatten( QgsAttributeEditorContainer *container, QStandardItem *parent, const QString &parentVisibilityExpressions, QVector<QStandardItem *> &items, int currentTabIndex, const QColor &color )
{
  const QList<QgsAttributeEditorElement *> children { container->children() };
  for ( QgsAttributeEditorElement *element : children )
  {
    switch ( element->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer:
      {
        QString visibilityExpression = parentVisibilityExpressions;
        QgsAttributeEditorContainer *innerContainer = static_cast<QgsAttributeEditorContainer *>( element );
        if ( innerContainer->visibilityExpression().enabled() )
        {
          if ( visibilityExpression.isNull() )
            visibilityExpression = innerContainer->visibilityExpression().data().expression();
          else
            visibilityExpression += " AND " + innerContainer->visibilityExpression().data().expression();
        }

        QVector<QStandardItem *> newItems;
        flatten( innerContainer, parent, visibilityExpression, newItems, 0, innerContainer->backgroundColor() );
        if ( !visibilityExpression.isEmpty() )
          mVisibilityExpressions.append( qMakePair( QgsExpression( visibilityExpression ), newItems ) );
        break;
      }

      case QgsAttributeEditorElement::AeTypeField:
      {
        QgsAttributeEditorField *editorField = static_cast<QgsAttributeEditorField *>( element );

        // editorField->idx() is not working on joined fields
        const QgsFields fields = mLayer->fields();
        int fieldIndex = fields.lookupField( editorField->name() );

        if ( fieldIndex < 0 || fieldIndex >= mLayer->fields().size() )
          continue;

        QgsField field = mLayer->fields().at( fieldIndex );

        QStandardItem *item = new QStandardItem();

        item->setData( currentTabIndex, AttributeFormModel::TabIndex );
        item->setData( mLayer->attributeDisplayName( fieldIndex ), AttributeFormModel::Name );
        item->setData( !mLayer->editFormConfig().readOnly( fieldIndex ) &&
                       !( field.defaultValueDefinition().isValid() && field.defaultValueDefinition().applyOnUpdate() ), AttributeFormModel::AttributeEditable );
        const QgsEditorWidgetSetup setup = findBest( fieldIndex );
        item->setData( setup.type(), AttributeFormModel::EditorWidget );
        item->setData( setup.config(), AttributeFormModel::EditorWidgetConfig );
        item->setData( mFeatureModel->rememberedAttributes().at( fieldIndex ) ? Qt::Checked : Qt::Unchecked, AttributeFormModel::RememberValue );
        item->setData( QgsField( field ), AttributeFormModel::Field );
        item->setData( "field", AttributeFormModel::ElementType );
        item->setData( fieldIndex, AttributeFormModel::FieldIndex );
        item->setData( container->isGroupBox() ? container->name() : QString(), AttributeFormModel::Group );
        item->setData( true, AttributeFormModel::CurrentlyVisible );
        item->setData( true, AttributeFormModel::ConstraintHardValid );
        item->setData( true, AttributeFormModel::ConstraintSoftValid );
        item->setData( mFeatureModel->data( mFeatureModel->index( fieldIndex ), FeatureModel::AttributeAllowEdit ), AttributeFormModel::AttributeAllowEdit );
        if ( color.isValid() )
          item->setData( color, AttributeFormModel::Color );

        // create constraint description
        QStringList descriptions;
        if ( field.constraints().constraints() & QgsFieldConstraints::ConstraintExpression )
        {
          descriptions << ( !field.constraints().constraintDescription().isEmpty()
                            ? field.constraints().constraintDescription()
                            : tr( "Expression constraint" ) );
        }
        if ( field.constraints().constraints() & QgsFieldConstraints::ConstraintNotNull )
        {
          descriptions << tr( "Not NULL" );
        }
        if ( field.constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
        {
          descriptions << tr( "Unique" );
        }

        item->setData( descriptions.join( ", " ), AttributeFormModel::ConstraintDescription );

        updateAttributeValue( item );

        mConstraints.insert( item, field.constraints() );

        items.append( item );

        parent->appendRow( item );
        break;
      }

      case QgsAttributeEditorElement::AeTypeRelation:
      {
        QgsAttributeEditorRelation *editorRelation = static_cast<QgsAttributeEditorRelation *>( element );
        QgsRelation relation = editorRelation->relation();

        QStandardItem *item = new QStandardItem();

        item->setData( currentTabIndex, AttributeFormModel::TabIndex );
        item->setData( relation.name(), AttributeFormModel::Name );
        item->setData( true, AttributeFormModel::AttributeEditable );
        item->setData( true, AttributeFormModel::CurrentlyVisible );
        item->setData( "relation", AttributeFormModel::ElementType );
        item->setData( "RelationEditor", AttributeFormModel::EditorWidget );
        item->setData( editorRelation->relationWidgetTypeId(), AttributeFormModel::RelationEditorWidget );
        item->setData( editorRelation->relationEditorConfiguration(), AttributeFormModel::RelationEditorWidgetConfig );
        item->setData( relation.id(), AttributeFormModel::RelationId );
        item->setData( mLayer->editFormConfig().widgetConfig( relation.id() )[QStringLiteral( "nm-rel" )].toString(), AttributeFormModel::NmRelationId );
        item->setData( container->isGroupBox() ? container->name() : QString(), AttributeFormModel::Group );
        item->setData( true, AttributeFormModel::CurrentlyVisible );
        item->setData( true, AttributeFormModel::ConstraintHardValid );
        item->setData( true, AttributeFormModel::ConstraintSoftValid );
        item->setData( true, AttributeFormModel::AttributeAllowEdit );

        items.append( item );

        parent->appendRow( item );
        break;
      }

      case QgsAttributeEditorElement::AeTypeQmlElement:
      {
        QgsAttributeEditorQmlElement *qmlElement = static_cast<QgsAttributeEditorQmlElement *>( element );
        QStandardItem *item = new QStandardItem();

        item->setData( currentTabIndex, AttributeFormModel::TabIndex );
        item->setData( "qml", AttributeFormModel::ElementType );
        item->setData( qmlElement->name(), AttributeFormModel::Name );
        item->setData( true, AttributeFormModel::CurrentlyVisible );
        item->setData( false, AttributeFormModel::AttributeEditable );
        item->setData( false, AttributeFormModel::AttributeAllowEdit );
        item->setData( container->isGroupBox() ? container->name() : QString(), AttributeFormModel::Group );

        mEditorWidgetCodes.insert( item, qmlElement->qmlCode() );

        updateAttributeValue( item );

        items.append( item );
        parent->appendRow( item );
        break;
      }

      case QgsAttributeEditorElement::AeTypeHtmlElement:
      {
        QgsAttributeEditorHtmlElement *htmlElement = static_cast<QgsAttributeEditorHtmlElement *>( element );
        QStandardItem *item = new QStandardItem();

        item->setData( currentTabIndex, AttributeFormModel::TabIndex );
        item->setData( "html", AttributeFormModel::ElementType );
        item->setData( htmlElement->name(), AttributeFormModel::Name );
        item->setData( true, AttributeFormModel::CurrentlyVisible );
        item->setData( false, AttributeFormModel::AttributeEditable );
        item->setData( false, AttributeFormModel::AttributeAllowEdit );
        item->setData( container->isGroupBox() ? container->name() : QString(), AttributeFormModel::Group );

        mEditorWidgetCodes.insert( item, htmlElement->htmlCode() );

        updateAttributeValue( item );

        items.append( item );
        parent->appendRow( item );
        break;
      }

      case QgsAttributeEditorElement::AeTypeInvalid:
        // todo
        break;
    }
  }
}

void AttributeFormModelBase::updateDefaultValues( int fieldIndex )
{
  const QgsFields fields = mFeatureModel->feature().fields();
  const QString fieldName = fields.at( fieldIndex ).name();

  mExpressionContext.setFields( fields );
  mExpressionContext.setFeature( mFeatureModel->feature() );

  QMap<QStandardItem *, QgsFieldConstraints>::ConstIterator constraintIterator( mConstraints.constBegin() );
  for ( ; constraintIterator != mConstraints.constEnd(); ++constraintIterator )
  {
    QStandardItem *item = constraintIterator.key();
    int fidx = item->data( AttributeFormModel::FieldIndex ).toInt();
    if ( fidx == fieldIndex || !fields.at( fidx ).defaultValueDefinition().isValid() || !fields.at( fidx ).defaultValueDefinition().applyOnUpdate() )
      continue;

    QgsExpression exp( fields.at( fidx ).defaultValueDefinition().expression() );
    exp.prepare( &mExpressionContext );

    // avoid cost of value update if expression doesn't contain the field which triggered the default values update
    if ( !exp.referencedColumns().contains( fieldName ) )
      continue;

    QVariant defaultValue = exp.evaluate( &mExpressionContext );
    bool changed = mFeatureModel->setData( mFeatureModel->index( fidx ), defaultValue, FeatureModel::AttributeValue );
    if ( changed )
    {
      item->setData( defaultValue, AttributeFormModel::AttributeValue );
    }
  }
}

void AttributeFormModelBase::updateVisibilityAndConstraints( int fieldIndex )
{
  QgsFields fields = mFeatureModel->feature().fields();
  mExpressionContext.setFields( fields );
  mExpressionContext.setFeature( mFeatureModel->feature() );

  for ( const VisibilityExpression &it : std::as_const( mVisibilityExpressions ) )
  {
    if ( fieldIndex == -1 || it.first.referencedAttributeIndexes( fields ).contains( fieldIndex ) )
    {
      QgsExpression exp = it.first;
      exp.prepare( &mExpressionContext );

      bool visible = exp.evaluate( &mExpressionContext ).toInt();
      for ( QStandardItem *item : std::as_const( it.second ) )
      {
        if ( item->data( AttributeFormModel::CurrentlyVisible ).toBool() != visible )
        {
          item->setData( visible, AttributeFormModel::CurrentlyVisible );
        }
      }
    }
  }

  // reset contrainsts status of containers
  if ( mHasTabs )
  {
    QStandardItem *root = invisibleRootItem();
    for ( int i = 0; i < root->rowCount(); i++ )
    {
      QStandardItem *item = root->child( i, 0 );
      item->setData( true, AttributeFormModel::ConstraintHardValid );
      item->setData( true, AttributeFormModel::ConstraintSoftValid );
    }
  }

  bool allConstraintsHardValid = true;
  bool allConstraintsSoftValid = true;
  QMap<QStandardItem *, QgsFieldConstraints>::ConstIterator constraintIterator( mConstraints.constBegin() );
  for ( ; constraintIterator != mConstraints.constEnd(); ++constraintIterator )
  {
    QStandardItem *item = constraintIterator.key();
    const bool isVisible = item->data( AttributeFormModel::CurrentlyVisible ).toBool();
    int fidx = item->data( AttributeFormModel::FieldIndex ).toInt();
    if ( isVisible && mFeatureModel->data( mFeatureModel->index( fidx ), FeatureModel::AttributeAllowEdit ) == true )
    {
      QStringList errors;

      QgsFeature feature = mFeatureModel->feature();
      QString defaultValueClause = mLayer->dataProvider()->defaultValueClause( fidx );
      QString attrValue = feature.attribute( fidx ).toString();

      // Providers will check for a literal "defaultValueClause" to autogenerate PKs.
      // For example, the gpkg provider will generate a fid if it is set to "Autogenerate".
      // On QField, if the user leaves the field empty, we will assume he wants to autogenerate it.
      // This makes sure, the NOT NULL constraint is skipped in this case.
      if ( attrValue.isEmpty() && !defaultValueClause.isEmpty() )
      {
        feature.setAttribute( fidx, defaultValueClause );
      }

      bool hardConstraintSatisfied = QgsVectorLayerUtils::validateAttribute( mLayer, feature, fidx, errors, QgsFieldConstraints::ConstraintStrengthHard );
      if ( hardConstraintSatisfied != item->data( AttributeFormModel::ConstraintHardValid ).toBool() )
      {
        item->setData( hardConstraintSatisfied, AttributeFormModel::ConstraintHardValid );
      }
      if ( !item->data( AttributeFormModel::ConstraintHardValid ).toBool() )
      {
        allConstraintsHardValid = false;
        if ( mHasTabs && item->parent() )
          item->parent()->setData( false, AttributeFormModel::ConstraintHardValid );
      }

      QStringList softErrors;
      bool softConstraintSatisfied = QgsVectorLayerUtils::validateAttribute( mLayer, mFeatureModel->feature(), fidx, softErrors, QgsFieldConstraints::ConstraintStrengthSoft );
      if ( softConstraintSatisfied != item->data( AttributeFormModel::ConstraintSoftValid ).toBool() )
      {
        item->setData( softConstraintSatisfied, AttributeFormModel::ConstraintSoftValid );
      }
      if ( !item->data( AttributeFormModel::ConstraintSoftValid ).toBool() )
      {
        allConstraintsSoftValid = false;
        if ( mHasTabs && item->parent() )
          item->parent()->setData( false, AttributeFormModel::ConstraintSoftValid );
      }
    }
    else
    {
      item->setData( true, AttributeFormModel::ConstraintHardValid );
      item->setData( true, AttributeFormModel::ConstraintSoftValid );
    }
  }

  setConstraintsHardValid( allConstraintsHardValid );
  setConstraintsSoftValid( allConstraintsSoftValid );
}

bool AttributeFormModelBase::constraintsHardValid() const
{
  return mConstraintsHardValid;
}

bool AttributeFormModelBase::constraintsSoftValid() const
{
  return mConstraintsSoftValid;
}

QVariant AttributeFormModelBase::attribute( const QString &name )
{
  if ( !mLayer )
    return QVariant();

  int idx = mLayer->fields().indexOf( name );
  return mFeatureModel->data( mFeatureModel->index( idx ), FeatureModel::AttributeValue );
}

void AttributeFormModelBase::setConstraintsHardValid( bool constraintsHardValid )
{
  if ( constraintsHardValid == mConstraintsHardValid )
    return;

  mConstraintsHardValid = constraintsHardValid;
  emit constraintsHardValidChanged();
}

void AttributeFormModelBase::setConstraintsSoftValid( bool constraintsSoftValid )
{
  if ( constraintsSoftValid == mConstraintsSoftValid )
    return;

  mConstraintsSoftValid = constraintsSoftValid;
  emit constraintsSoftValidChanged();
}

QgsEditorWidgetSetup AttributeFormModelBase::findBest( const int fieldIndex )
{
  QgsFields fields = mLayer->fields();

  //make the default one
  QgsEditorWidgetSetup setup = QgsEditorWidgetSetup( QStringLiteral( "TextEdit" ), QVariantMap() );

  if ( fieldIndex >= 0 && fieldIndex < fields.count() )
  {
    //when field has a configured setup, take it
    setup = mLayer->editorWidgetSetup( fieldIndex );
    if ( !setup.isNull() )
      return setup;

    //when it's a provider field with default value clause, take Textedit
    if ( fields.fieldOrigin( fieldIndex ) == QgsFields::OriginProvider )
    {
      int providerOrigin = fields.fieldOriginIndex( fieldIndex );
      if ( !mLayer->dataProvider()->defaultValueClause( providerOrigin ).isEmpty() )
        return setup;
    }

    //find the best one
    const QgsField field = fields.at( fieldIndex );
    //on a boolean type take "CheckBox"
    if ( field.type() == QVariant::Bool )
      setup = QgsEditorWidgetSetup( QStringLiteral( "CheckBox" ), QVariantMap() );
    //on a date or time type take "DateTime"
    if ( field.isDateOrTime() )
    {
      QVariantMap config;
      config.insert( QStringLiteral( "field_format" ), QgsDateTimeFieldFormatter::defaultFormat( field.type() ) );
      config.insert( QStringLiteral( "display_format" ), QgsDateTimeFieldFormatter::defaultFormat( field.type() ) );
      config.insert( QStringLiteral( "calendar_popup" ), true );
      config.insert( QStringLiteral( "allow_null" ), true );
      setup = QgsEditorWidgetSetup( QStringLiteral( "DateTime" ), config );
    }
    //on numeric types take "Range"
    if ( field.type() == QVariant::Int || field.type() == QVariant::Double || field.isNumeric() )
      setup = QgsEditorWidgetSetup( QStringLiteral( "Range" ), QVariantMap() );
    //if it's a foreign key configured in a relation take "RelationReference"
    if ( !mLayer->referencingRelations( fieldIndex ).isEmpty() )
      setup = QgsEditorWidgetSetup( QStringLiteral( "RelationReference" ), QVariantMap() );
  }

  return setup;
}

bool AttributeFormModelBase::hasTabs() const
{
  return mHasTabs;
}

void AttributeFormModelBase::setHasTabs( bool hasTabs )
{
  if ( hasTabs == mHasTabs )
    return;

  mHasTabs = hasTabs;
  emit hasTabsChanged();
}

bool AttributeFormModelBase::save()
{
  return mFeatureModel->save();
}

bool AttributeFormModelBase::create()
{
  return mFeatureModel->create();
}

bool AttributeFormModelBase::deleteFeature()
{
  return mFeatureModel->deleteFeature();
}
