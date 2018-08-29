/***************************************************************************
                            appinterface.h
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

#ifndef APPINTERFACE_H
#define APPINTERFACE_H

#include <QObject>
#include <QPointF>

#include <QStandardItemModel>

class QgisMobileapp;

class AppInterface : public QObject
{
    Q_OBJECT

  public:
    AppInterface( QgisMobileapp* app );
    AppInterface()
    {
      // You shouldn't get here, this constructor only exists that we can register it as a QML type
      Q_ASSERT( false );
    }

    Q_INVOKABLE void loadProject( const QString& path );
    Q_INVOKABLE void print( int layoutIndex );

  public slots:
    void openFeatureForm();

  signals:
    void openFeatureFormRequested();

    void loadProjectStarted( const QString& path );

    void loadProjectEnded();

  private:
    QgisMobileapp* mApp;
};

#endif // APPINTERFACE_H
