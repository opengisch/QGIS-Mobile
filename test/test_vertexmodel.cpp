

#include <QtTest>


#include <qgslinestring.h>
#include <qgsgeometry.h>
#include <qgspoint.h>
#include <qgspointxy.h>

#include "vertexmodel.h"


class TestVertexModel: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase()
    {
      mModel = new VertexModel();

      QgsLineString *lineString = new QgsLineString( QVector<QgsPoint>() << QgsPoint( 0, 1 ) << QgsPoint( 2, 3 ) << QgsPoint( 4, 3 ) );
      mLineGeometry = QgsGeometry( lineString );

      mPolygonGeometry = QgsGeometry::fromPolygonXY( QVector<QVector<QgsPointXY>>()
                         << ( QVector<QgsPointXY>()
                              << QgsPointXY( 0, 0 )
                              << QgsPointXY( 2, 0 )
                              << QgsPointXY( 2, 2 )
                              << QgsPointXY( 0, 2 )
                              << QgsPointXY( 0, 0 ) ) ) ;
    }

    void canRemoveVertexTest()
    {
      // line
      mModel->setGeometry( mLineGeometry, QgsCoordinateReferenceSystem() );
      QVERIFY( !mModel->canRemoveVertex() );
      mModel->setEditingMode( VertexModel::EditVertex );
      QVERIFY( mModel->canRemoveVertex() );

      mModel->setEditingMode( VertexModel::NoEditing );
      QCOMPARE( mModel->editingMode(), VertexModel::NoEditing );
      mModel->setCurrentVertex( 0 );
      QCOMPARE( mModel->editingMode(), VertexModel::EditVertex );

      QCOMPARE( mModel->vertexCount(), 3 );
      mModel->removeCurrentVertex();
      QCOMPARE( mModel->vertexCount(), 2 );
      QVERIFY( !mModel->canRemoveVertex() );

      // polygon
      mModel->setGeometry( mPolygonGeometry, QgsCoordinateReferenceSystem() );
      QCOMPARE( mModel->editingMode(), VertexModel::NoEditing );
      QVERIFY( !mModel->canRemoveVertex() );
      mModel->setCurrentVertex( 0 );
      mModel->removeCurrentVertex();
      QCOMPARE( mModel->vertexCount(), 3 );
      QVERIFY( !mModel->canRemoveVertex() );
    }

    void addVertexTest()
    {
      mModel->setGeometry( mPolygonGeometry, QgsCoordinateReferenceSystem() );
      QCOMPARE( mModel->vertexCount(), 4 );
      mModel->setEditingMode( VertexModel::AddVertex );
      QCOMPARE( mModel->vertexCount(), 5 );

      mModel->setGeometry( mLineGeometry, QgsCoordinateReferenceSystem() );
      mModel->setEditingMode( VertexModel::AddVertex );
      QCOMPARE( mModel->mCurrentIndex, 1 );
      QVERIFY( mModel->canPreviousVertex() );
      mModel->previous();
      QVERIFY( !mModel->canPreviousVertex() );
      QCOMPARE( mModel->mCurrentIndex, 0 );
      mModel->next();
      QCOMPARE( mModel->mCurrentIndex, 1 );

      mModel->setGeometry( mLineGeometry, QgsCoordinateReferenceSystem() );
      mModel->setEditingMode( VertexModel::AddVertex );
      QCOMPARE( mModel->mCurrentIndex, 1 );
      QCOMPARE( mModel->currentPoint(), QgsPoint( 1, 2 ) );
      mModel->next();
      QCOMPARE( mModel->mCurrentIndex, 2 );
      QCOMPARE( mModel->currentPoint(), QgsPoint( 3, 3 ) );
      mModel->next();
      QCOMPARE( mModel->mCurrentIndex, 3 );
      QCOMPARE( mModel->currentPoint().x(), 4.5 );
      QCOMPARE( mModel->currentPoint().y(), 3.0 );
      QCOMPARE( mModel->currentPoint(), QgsPoint( 4.5, 3 ) );
      QVERIFY( !mModel->canNextVertex() );
      mModel->next();
      QCOMPARE( mModel->mCurrentIndex, 3 );
      mModel->previous();
      QCOMPARE( mModel->mCurrentIndex, 2 );
      QCOMPARE( mModel->currentPoint(), QgsPoint( 3, 3 ) );
      mModel->previous();
      QCOMPARE( mModel->mCurrentIndex, 1 );
      QCOMPARE( mModel->currentPoint().x(), 1.0 );
    }

    void transformTest()
    {
      // TODO
    }

    void returnGeometryTest()
    {
      // TODO, also test when adding vertex mode
    }

    void xxxTest()
    {
      // TODO
    }

    void yyyTest()
    {
      // TODO
    }

    void zzzTest()
    {
      // TODO
    }

    void cleanupTestCase()
    {
      delete mModel;
    }

  private:
    VertexModel *mModel;
    QgsGeometry mLineGeometry;
    QgsGeometry mPolygonGeometry;
};

QTEST_MAIN( TestVertexModel )
#include "test_vertexmodel.moc"
