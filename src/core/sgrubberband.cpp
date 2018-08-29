#include "sgrubberband.h"
extern "C" {
#include "tessellate.h"
}

SGRubberband::SGRubberband( const QVector<QgsPoint>& points, QgsWkbTypes::GeometryType type, const QColor& color, qreal width )
  : QSGNode()
{
  mMaterial.setColor( color );

  if ( points.isEmpty() )
    return;

  switch ( type )
  {
    case QgsWkbTypes::PointGeometry:
      // TODO: Implement
      break;

    case QgsWkbTypes::LineGeometry:
    {
      appendChildNode( createLineGeometry( points, width ) );
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      appendChildNode( createLineGeometry( points, width ) );
      appendChildNode( createPolygonGeometry( points ) );
      break;
    }

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
}

QSGGeometryNode* SGRubberband::createLineGeometry( const QVector<QgsPoint>& points, qreal width )
{
  QSGGeometryNode* node = new QSGGeometryNode;
  QSGGeometry* sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), points.count() );
  QSGGeometry::Point2D* vertices = sgGeom->vertexDataAsPoint2D();

  int i = 0;
  Q_FOREACH( const QgsPoint& pt, points )
  {
    vertices[i++].set( pt.x(), pt.y() );
  }

  sgGeom->setLineWidth( width );
  sgGeom->setDrawingMode( GL_LINE_STRIP );
  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}

QSGGeometryNode*SGRubberband::createPolygonGeometry( const QVector<QgsPoint>& points )
{
  double* coordinates_out;
  int* tris_out;
  int nverts, ntris;

  double* vertices_in = ( double* )malloc( points.size() * 2 * sizeof( double ) );
  const double* contours_array[] = { vertices_in, vertices_in + points.size() * 2 };
  int i = 0;

  Q_FOREACH( const QgsPoint& pt, points )
  {
    vertices_in[i++] = pt.x();
    vertices_in[i++] = pt.y();
  }

  tessellate( &coordinates_out, &nverts,
              &tris_out, &ntris,
              contours_array, contours_array + 2 );

  QSGGeometryNode* node = new QSGGeometryNode;
  QSGGeometry* sgGeom = new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), ntris * 3 );

  QSGGeometry::Point2D* vertices = sgGeom->vertexDataAsPoint2D();

  for ( int j = 0; j < ntris*3; j++ )
  {
    vertices[j].x = coordinates_out[tris_out[j]*2];
    vertices[j].y = coordinates_out[tris_out[j]*2+1];
  }

  free( vertices_in );
  free( coordinates_out );
  free( tris_out );

  sgGeom->setDrawingMode( GL_TRIANGLES );
  node->setGeometry( sgGeom );
  node->setMaterial( &mMaterial );
  node->setFlag( QSGNode::OwnsGeometry );
  node->setFlag( QSGNode::OwnedByParent );
  return node;
}
