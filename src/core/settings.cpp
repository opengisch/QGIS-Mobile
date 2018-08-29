/***************************************************************************
                            settings.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "settings.h"

#include <QDebug>

Settings::Settings( QObject *parent ) :
  QSettings( parent )
{
}

void Settings::setValue( const QString& key, const QVariant& value )
{
  QSettings::setValue( key, value );
  emit settingChanged( key );
}

QVariant Settings::value( const QString& key, const QVariant& defaultValue )
{
  return QSettings::value( key, defaultValue );
}

bool Settings::valueBool( const QString& key, bool defaultValue )
{
  return QSettings::value( key, defaultValue ).toBool();
}
