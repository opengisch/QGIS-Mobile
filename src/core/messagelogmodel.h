/***************************************************************************
  messagelogmodel.h - MessageLogModel

 ---------------------
 begin                : 13.7.2016
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
#ifndef MESSAGELOGMODEL_H
#define MESSAGELOGMODEL_H

#include <QAbstractListModel>
#include <qgsmessagelog.h>

/**
 * This model will connect to the message log and publish any
 * messages received from there.
 */
class MessageLogModel : public QAbstractListModel
{
    Q_OBJECT

    struct LogMessage
    {
      LogMessage()
      {}

      LogMessage( const QString& tag, const QString& message, Qgis::MessageLevel level )
      {
        this->tag = tag;
        this->message = message;
        this->level = level;
      }

      QString tag;
      QString message;
      Qgis::MessageLevel level;
    };

    enum Roles
    {
      MessageRole = Qt::UserRole,
      MessageTagRole,
      MessageLevelRole
    };

  public:
    MessageLogModel( QObject* parent = nullptr );

    QHash<int, QByteArray> roleNames() const override;

    int rowCount( const QModelIndex& parent ) const override;
    QVariant data( const QModelIndex& index, int role ) const override;

  private slots:
    void onMessageReceived( const QString& message, const QString& tag, Qgis::MessageLevel level );

  private:
    QgsMessageLog* mMessageLog;
    QVector<LogMessage> mMessages;
};

#endif // MESSAGELOGMODEL_H
