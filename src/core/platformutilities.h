/***************************************************************************
                            platformutilities.h  -  utilities for qfield

                              -------------------
              begin                : Wed Dec 04 10:48:28 CET 2015
              copyright            : (C) 2015 by Marco Bernasocchi
              email                : marco@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLATFORMUTILITIES_H
#define PLATFORMUTILITIES_H

#include <QObject>
#include <qgsfield.h>
#include "picturesource.h"

class PlatformUtilities : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString configDir READ configDir )
    Q_PROPERTY( QString shareDir READ shareDir )

  public:
    virtual ~PlatformUtilities();

    virtual QString configDir() const;
    virtual QString shareDir() const;
    virtual QString qgsProject() const;
    Q_INVOKABLE bool createDir( const QString &path, const QString &dirname ) const;
    Q_INVOKABLE bool rmFile( const QString &filename ) const;
    Q_INVOKABLE bool renameFile( const QString &filename, const QString &newname ) const;

    /**
     * Get a picture and copy it to the requested prefix
     * @param prefix The folder where the picture should be put
     * @return The name of the picture or null
     */
    Q_INVOKABLE virtual PictureSource* getPicture( const QString& prefix );

    Q_INVOKABLE virtual void open( const QString& data, const QString& type );

    /**
     * Returns the QVariant typeName of a field.
     * This is a stable identifier (compared to the provider field name).
     */
    Q_INVOKABLE QString fieldType( const QgsField& field ) const;
};
#endif // PLATFORMUTILITIES_H
