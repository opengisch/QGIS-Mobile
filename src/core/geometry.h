#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <qgsgeometry.h>
#include <rubberbandmodel.h>

#include <QtPositioning/QGeoCoordinate>

class Geometry : public QObject
{
    Q_OBJECT

    Q_PROPERTY( RubberbandModel* rubberbandModel READ rubberbandModel WRITE setRubberbandModel NOTIFY rubberbandModelChanged )
    Q_PROPERTY( QgsVectorLayer* vectorLayer READ vectorLayer WRITE setVectorLayer NOTIFY vectorLayerChanged )

  public:
    explicit Geometry( QObject* parent = nullptr );

    QgsGeometry asQgsGeometry() const;

    RubberbandModel* rubberbandModel() const;
    void setRubberbandModel( RubberbandModel* rubberbandModel );

    Q_INVOKABLE void applyRubberband();

    QgsVectorLayer* vectorLayer() const;
    void setVectorLayer( QgsVectorLayer* vectorLayer );

  signals:
    void rubberbandModelChanged();
    void vectorLayerChanged();

  private:
    RubberbandModel* mRubberbandModel;
    QgsVectorLayer* mVectorLayer;
};

#endif // GEOMETRY_H
