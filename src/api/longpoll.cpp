/****************************************************************************
**
** VKit - vk.com API Qt bindings
**
** Copyright © 2012 Aleksey Sidorov <gorthauer87@ya.ru>
**
*****************************************************************************
**
** $VKIT_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $VKIT_END_LICENSE$
**
****************************************************************************/
#include "longpoll_p.h"
#include "connection.h"
#include "message.h"
#include "roster.h"
#include "contact.h"
#include <QDateTime>
#include <QDebug>

namespace vk {

static const int chatMessageOffset = 2000000000;

/*!
 * \brief The LongPoll class
 * Api reference: \link http://vk.com/developers.php?oid=-1&p=messages.getLongPollServer
 */

/*!
 * \brief LongPoll::LongPoll
 * \param client
 */
LongPoll::LongPoll(Client *client) :
    QObject(client),
    d_ptr(new LongPollPrivate(this))
{
    Q_D(LongPoll);
    d->client = client;
    setRunning(client->isOnline());

    connect(client, SIGNAL(onlineStateChanged(bool)), SLOT(setRunning(bool)));
}

LongPoll::~LongPoll()
{

}

void LongPoll::setMode(LongPoll::Mode mode)
{
    d_func()->mode = mode;
}

LongPoll::Mode LongPoll::mode() const
{
    return d_func()->mode;
}

int LongPoll::pollInterval() const
{
    return d_func()->pollInterval;
}

void LongPoll::setRunning(bool set)
{
    Q_D(LongPoll);
    if (set != d->isRunning) {
        d->isRunning = set;
        if (set)
            requestServer();
    }
}

void LongPoll::requestServer()
{
    Q_D(LongPoll);
    if (d->isRunning) {
        auto reply = d->client->request("messages.getLongPollServer");
        connect(reply, SIGNAL(resultReady(QVariant)), SLOT(_q_request_server_finished(QVariant)));
    }
}

void LongPoll::requestData(const QByteArray &timeStamp)
{
    Q_D(LongPoll);
    if (d->dataRequestReply) {
        d->dataRequestReply->disconnect(this, SLOT(_q_request_server_finished(QVariant)));
        d->dataRequestReply->deleteLater();
    }
    QUrl url = d->dataUrl;
    url.addQueryItem("ts", timeStamp);
    auto reply = d->client->request(url);
    connect(reply, SIGNAL(resultReady(QVariant)), this, SLOT(_q_on_data_recieved(QVariant)));
    d->dataRequestReply = reply;
}

void LongPollPrivate::_q_request_server_finished(const QVariant &response)
{
    Q_Q(LongPoll);

    QVariantMap data = response.toMap();
    if (data.isEmpty()) {
        QTimer::singleShot(pollInterval, q, SLOT(requestServer()));
        return;
    }

    QString url("http://%1?act=a_check&key=%2&wait=%3&mode=%4");
    dataUrl = url.arg(data.value("server").toString(),
                      data.value("key").toString(),
                      QString::number(waitInterval),
                      QString::number(mode));

    q->requestData(data.value("ts").toByteArray());
}

void LongPollPrivate::_q_on_data_recieved(const QVariant &response)
{
    Q_Q(LongPoll);
    auto data = response.toMap();

    if (data.contains("failed")) {
        q->requestServer();
        return;
    }

    QVariantList updates = data.value("updates").toList();
    for (int i = 0; i < updates.size(); i++) {
        QVariantList update = updates.at(i).toList();
        int updateType = update.value(0, -1).toInt();
        switch (updateType) {
        case LongPoll::MessageDeleted: {
            emit q->messageDeleted(update.value(1).toInt());
            break;
        }
        case LongPoll::MessageAdded: {
            //qDebug() << update;
            Message::Flags flags(update.value(2).toInt());
            Message message(client);
            int cid = update.value(3).toInt();
            //qDebug() << (flags & Message::FlagChat);
            //if (flags & Message::FlagChat)
            //  cid -= chatMessageOffset;
            message.setId(update.value(1).toInt());
            message.setFlags(flags);
            if (flags & Message::FlagOutbox) {
                message.setToId(cid);
                message.setFrom(client->me());
            } else {
                message.setFromId(cid);
                message.setTo(client->me());
            }
            message.setSubject(update.value(5).toString());
            message.setBody(update.value(6).toString());
            message.setDate(QDateTime::currentDateTime());
            emit q->messageAdded(message);
            break;
        }
        case LongPoll::MessageFlagsReplaced: {
            int id = update.value(1).toInt();
            int flags = update.value(2).toInt();
            int userId = update.value(3).toInt();
            emit q->messageFlagsReplaced(id, flags, userId);
            break;
        }
        case LongPoll::MessageFlagsReseted: { //TODO remove copy/paste
            int id = update.value(1).toInt();
            int flags = update.value(2).toInt();
            int userId = update.value(3).toInt();
            emit q->messageFlagsReseted(id, flags, userId);
            break;
        }
        case LongPoll::UserOnline:
        case LongPoll::UserOffline: {
            // WTF? Why VKontakte sends minus as first char of id?
            auto id = qAbs(update.value(1).toInt());
            Buddy::Status status;
            if (updateType == LongPoll::UserOnline)
                status = Buddy::Online;
            else
                status = update.value(2).toInt() == 1 ? Buddy::Away
                                                      : Buddy::Offline;
            emit q->contactStatusChanged(id, status);
            break;
        }
        case LongPoll::GroupChatUpdated: {
            int chat_id = update.value(1).toInt();
            bool self = update.value(1).toInt();
            emit q->groupChatUpdated(chat_id, self);
            break;
        }
        case LongPoll::ChatTyping: {
            int user_id = qAbs(update.value(1).toInt());
            //int flags = update.at(2).toInt();
            emit q->contactTyping(user_id);
            break;
        }
        case LongPoll::GroupChatTyping: {
            int user_id = qAbs(update.value(1).toInt());
            int chat_id = update.at(2).toInt();
            emit q->contactTyping(user_id, chat_id);
            break;
        }
        case LongPoll::UserCall: {
            int user_id = qAbs(update.value(1).toInt());
            int call_id = update.at(2).toInt();
            emit q->contactCall(user_id, call_id);
            break;
        }
        }
    }

    q->requestData(data.value("ts").toByteArray());
}

void LongPoll::setPollInterval(int interval)
{
    Q_D(LongPoll);
    if (d->pollInterval != interval) {
        d->pollInterval = interval;
        emit pollIntervalChanged(interval);
    }
}

} // namespace vk

#include "moc_longpoll.cpp"

