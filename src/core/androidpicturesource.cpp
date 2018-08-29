/***************************************************************************
  androidpicturesource.cpp - AndroidPictureSource

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
#include "androidpicturesource.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QDir>
#include <QFile>
#include <QDebug>

AndroidPictureSource::AndroidPictureSource( const QString& prefix )
  : PictureSource( nullptr )
  , QAndroidActivityResultReceiver()
  , mPrefix( prefix )
{

}

void AndroidPictureSource::handleActivityResult( int receiverRequestCode, int resultCode, const QAndroidJniObject& data )
{
  jint RESULT_OK = QAndroidJniObject::getStaticField<jint>( "android/app/Activity", "RESULT_OK" );
  if ( receiverRequestCode == 101 && resultCode == RESULT_OK )
  {
    QAndroidJniObject uri = data.callObjectMethod( "getData", "()Landroid/net/Uri;" );
    QAndroidJniObject data = QAndroidJniObject::getStaticObjectField( "android/provider/MediaStore$MediaColumns", "DATA", "Ljava/lang/String;" );
    QAndroidJniEnvironment env;
    jobjectArray projection = ( jobjectArray )env->NewObjectArray( 1, env->FindClass( "java/lang/String" ), NULL );
    jobject dataProjection = env->NewStringUTF( data.toString().toStdString().c_str() );
    env->SetObjectArrayElement( projection, 0, dataProjection );
    QAndroidJniObject contentResolver = QtAndroid::androidActivity().callObjectMethod( "getContentResolver", "()Landroid/content/ContentResolver;" );
    QAndroidJniObject cursor = contentResolver.callObjectMethod( "query", "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;", uri.object<jobject>(), projection, NULL, NULL, NULL );
    jint columnIndex = cursor.callMethod<jint>( "getColumnIndex", "(Ljava/lang/String;)I", data.object<jstring>() );
    cursor.callMethod<jboolean>( "moveToFirst", "()Z" );
    QAndroidJniObject filePath = cursor.callObjectMethod( "getString", "(I)Ljava/lang/String;", columnIndex );

    if ( !QDir::root().mkpath( mPrefix ) )
    {
      QgsApplication::messageLog()->logMessage( tr( "Could not create folder %1" ).arg( mPrefix ), "QField", Qgis::Critical );
      return;
    }

    QDir dir( mPrefix );

    QFileInfo fileInfo( filePath.toString() );
    QString filename( fileInfo.fileName() );

    if ( !QFile( filePath.toString() ).rename( dir.absoluteFilePath( filename ) ) )
    {
      qDebug() << "Couldn't rename file! Trying to copy instead";
      if ( !QFile( filePath.toString() ).copy( dir.absoluteFilePath( filename ) ) )
      {
        QgsApplication::messageLog()->logMessage( tr( "Image %1 could not be copied to project folder %2.", "QField", Qgis::Critical ).arg( filePath.toString(), mPrefix ) );
      }
    }

    emit pictureReceived( filename );
  }
  else
  {
    emit pictureReceived( QString::null );
  }
}
