/***************************************************************************
                            platformutilities.cpp  -  utilities for qfield

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

#include "platformutilities.h"
#include <QDebug>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

PlatformUtilities::~PlatformUtilities()
{
}

QString PlatformUtilities::configDir() const
{
  return QString();
}

QString PlatformUtilities::shareDir() const
{
  return QString();
}

QString PlatformUtilities::qgsProject() const
{
  return QString();
}

bool PlatformUtilities::createDir( const QString &path, const QString &dirname ) const
{
  QDir parentDir( path );
  return parentDir.mkdir( dirname );
}

bool PlatformUtilities::rmFile( const QString &filename ) const
{
  QFile file ( filename );
  return file.remove( filename );
}

bool PlatformUtilities::renameFile( const QString &filename, const QString &newname ) const
{
  QFile file ( filename );
  return file.rename( newname );
}

PictureSource* PlatformUtilities::getPicture( const QString& prefix )
{
  Q_UNUSED( prefix )
  return nullptr;
}

void PlatformUtilities::open( const QString& data, const QString& type )
{
  QDesktopServices::openUrl( QUrl( data ) );
  Q_UNUSED( type )
}

QString PlatformUtilities::fieldType( const QgsField &field ) const
{
  return QVariant( field.type() ).typeName();
}

