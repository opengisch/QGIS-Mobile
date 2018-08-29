/***************************************************************************
  legendimageprovider.h - LegendImageProvider

 ---------------------
 begin                : 7.12.2016
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
#ifndef LEGENDIMAGEPROVIDER_H
#define LEGENDIMAGEPROVIDER_H

#include <QQuickImageProvider>

class QgsLayerTreeModel;
class QgsLayerTree;

class LegendImageProvider : public QQuickImageProvider
{
  public:
    LegendImageProvider( QgsLayerTreeModel* layerTreeRoot );

    QPixmap requestPixmap( const QString &id, QSize *size, const QSize& requestedSize );

  private:
    QgsLayerTreeModel* mLayerTreeModel;
    QgsLayerTree* mRootNode;
};

#endif // LEGENDIMAGEPROVIDER_H
