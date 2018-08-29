/***************************************************************************
  picturesource.h - PictureSource

 ---------------------
 begin                : 5.7.2016
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
#ifndef PICTURESOURCE_H
#define PICTURESOURCE_H

#include <QObject>

/**
 * A picture source object will be created when a new picture is requested from the system.
 * It should be subclassed and filled with platform specific code.
 */
class PictureSource : public QObject
{
    Q_OBJECT
  public:
    explicit PictureSource( QObject* parent = nullptr );

    virtual ~PictureSource();

  signals:
    /**
     * Emit this signal when a picture really has been received.
     */
    void pictureReceived( const QString& path );
};

#endif // PICTURESOURCE_H
