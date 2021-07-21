/***************************************************************************
                        test_attributeformmodel.h
                        --------------------
  begin                : Jul 2021
  copyright            : (C) 2021 by Mathieu Pellerin
  email                : mathieu@opengis.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "attributeformmodel.h"
#include "featuremodel.h"

#include "qfield_testbase.h"

#include <QtTest>


class TestAttributeFormModel : public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase()
    {
      mLayer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=EPSG:3857&field=fid:integer&field=str:string&field=str2:string" ), QStringLiteral( "Input Layer" ), QStringLiteral( "memory" ) );
      QVERIFY( mLayer->isValid() );

      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << 1 << QStringLiteral( "string_a1" ) << QStringLiteral( "string_b1" ) );
      qDebug() << f1.attributes();
      QgsFeature f2;
      f2.setAttributes( QgsAttributes() << 2 << QStringLiteral( "string_a2" ) << QStringLiteral( "string_b2" ) );
      QgsFeature f3;
      f3.setAttributes( QgsAttributes() << 1 << QStringLiteral( "string_a3" ) << QStringLiteral( "string_b3" ) );

      mLayer->startEditing();
      mLayer->addFeature( f1 );
      mLayer->addFeature( f2 );
      mLayer->addFeature( f3 );
      mLayer->commitChanges();
      QCOMPARE( mLayer->featureCount(), 3 );

      mLayer->setDefaultValueDefinition( 2, QgsDefaultValue( QStringLiteral( " coalesce(\"str\",'') || '__'" ), true ) );

      mAttributeFormModel = std::make_unique< AttributeFormModel >();
      mFeatureModel = std::make_unique< FeatureModel >();
      mAttributeFormModel->setFeatureModel( mFeatureModel.get() );
      mFeatureModel->setCurrentLayer( mLayer.get() );
    }

    void testAttributes()
    {
      mFeatureModel->setFeature( mLayer->getFeature( 1 ) );
      QCOMPARE( mAttributeFormModel->attribute( QStringLiteral( "str" ) ), QStringLiteral( "string_a1" ) );
      QCOMPARE( mAttributeFormModel->data( mAttributeFormModel->index( 1, 0 ), AttributeFormModel::AttributeValue ), QStringLiteral( "string_a1" ) );
    }

    void testFeatureDefaultValue()
    {
      mFeatureModel->resetFeature();
      mFeatureModel->resetAttributes();

      mAttributeFormModel->setData( mAttributeFormModel->index( 1, 0 ), QString( "new_feature" ), AttributeFormModel::AttributeValue );
      // test default value changed on update with new feature
      QCOMPARE( mAttributeFormModel->attribute( QStringLiteral( "str2" ) ), QStringLiteral( "new_feature__" ) );

      // create a feature through the attribute form model
      QVERIFY( mAttributeFormModel->create() );

      QgsFeatureId fid = mFeatureModel->feature().id();
      QVERIFY( fid > 0 );

      mAttributeFormModel->setData( mAttributeFormModel->index( 1, 0 ), QString( "edit_feature" ), AttributeFormModel::AttributeValue );
      // test default value changed on update with existing feature being edited
      QCOMPARE( mAttributeFormModel->attribute( QStringLiteral( "str2" ) ), QStringLiteral( "edit_feature__" ) );

      QVERIFY( mAttributeFormModel->save() );

      QgsFeature feature = mLayer->getFeature( fid );
      QCOMPARE( feature.attributes().at( 2 ), QStringLiteral( "edit_feature__" ) );
    }

  private:

    std::unique_ptr<QgsVectorLayer> mLayer;
    std::unique_ptr<FeatureModel> mFeatureModel;
    std::unique_ptr<AttributeFormModel> mAttributeFormModel;

};

QFIELDTEST_MAIN( TestAttributeFormModel )
#include "test_attributeformmodel.moc"
