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
#include "client_p.h"
#include "message.h"
#include "contact.h"
#include "groupmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>

namespace vk {

Client::Client(QObject *parent) :
    QObject(parent),
    d_ptr(new ClientPrivate(this))
{
}

Client::Client(const QString &login, const QString &password, QObject *parent) :
    QObject(parent),
    d_ptr(new ClientPrivate(this))
{
    Q_D(Client);
    d->login = login;
    d->password = password;
}

Client::~Client()
{
}

QString Client::password() const
{
    return d_func()->password;
}

void Client::setPassword(const QString &password)
{
    d_func()->password = password;
    emit passwordChanged(password);
}

QString Client::login() const
{
    return d_func()->login;
}

void Client::setLogin(const QString &login)
{
    d_func()->login = login;
    emit loginChanged(login);
}

Client::State Client::connectionState() const
{
    Q_D(const Client);
    if (d->connection.isNull())
        return StateInvalid;
    return d->connection.data()->connectionState();
}

bool Client::isOnline() const
{
    if (auto c = connection())
        return c->connectionState() == Client::StateOnline;
    else
        return false;
}

QString Client::activity() const
{
    return d_func()->activity;
}

Connection *Client::connection() const
{
    return d_func()->connection.data();
}

Connection *Client::connection()
{
    Q_D(Client);
    if (d->connection.isNull())
        qWarning("Unknown method of connection. Use oauth webkit connection");
    //  setConnection(new DirectConnection(this));
    return d_func()->connection.data();
}

void Client::setConnection(Connection *connection)
{
    Q_D(Client);
    if (d->connection) {
        d->connection.data()->deleteLater();
    }

    d->connection = connection;
    connect(connection, SIGNAL(connectionStateChanged(vk::Client::State)),
            this, SLOT(_q_connection_state_changed(vk::Client::State)));
    connect(connection, SIGNAL(error(vk::Client::Error)), this, SIGNAL(error(vk::Client::Error)));
}

Roster *Client::roster() const
{
    return d_func()->roster.data();
}

Roster *Client::roster()
{
    Q_D(Client);
    if (d->roster.isNull()) {
        d->roster = new Roster(this);
    }
    return d->roster.data();
}

LongPoll *Client::longPoll() const
{
    return d_func()->longPoll.data();
}

LongPoll *Client::longPoll()
{
    Q_D(Client);
    if (d->longPoll.isNull()) {
        d->longPoll = new LongPoll(this);

        connect(this, SIGNAL(onlineStateChanged(bool)),
                d->longPoll.data(), SLOT(setRunning(bool)));

        emit longPollChanged(d->longPoll.data());
    }
    return d->longPoll.data();
}

GroupManager *Client::groupManager() const
{
    return d_func()->groupManager.data();
}

GroupManager *Client::groupManager()
{
    Q_D(Client);
    if (!d->groupManager)
        d->groupManager = new GroupManager(this);
    return d->groupManager.data();
}

Reply *Client::request(const QUrl &url)
{
    QNetworkRequest request(url);
    auto networkReply = connection()->get(request);
    auto reply = new Reply(networkReply);
    d_func()->processReply(reply);
    return reply;
}

Reply *Client::request(const QString &method, const QVariantMap &args)
{
    auto reply = new Reply(connection()->request(method, args));
    d_func()->processReply(reply);
    return reply;
}

Contact *Client::me() const
{
    if (auto r = roster())
        return r->owner();
    return 0;
}

Contact *Client::contact(int id) const
{
    Contact *contact = 0;
    if (id >= 0) {
        if (roster())
            contact = roster()->buddy(id);
        if (!contact && groupManager())
            contact = groupManager()->group(id);
    } else if (groupManager())
        contact = groupManager()->group(-id);
    return contact;
}

Reply *Client::sendMessage(const Message &message)
{
    //TODO add delayed send
    if (!isOnline())
        return 0;

    //checks
    Q_ASSERT(message.to());

    QVariantMap args;
    //TODO add chat messages support and contact check
    args.insert("uid", message.to()->id());
    args.insert("message", message.body());
    args.insert("title", message.subject());
    return request("messages.send", args);
}

Reply *Client::addLike(int ownerId, int postId, bool retweet, const QString &message)
{
    QVariantMap args;
    args.insert("owner_id", ownerId);
    args.insert("post_id", postId);
    args.insert("repost", (int)retweet);
    args.insert("message", message);
    return request("wall.addLike", args);
}

Reply *Client::deleteLike(int ownerId, int postId)
{
    QVariantMap args;
    args.insert("owner_id", ownerId);
    args.insert("post_id", postId);
    auto reply = request("wall.deleteLike", args);
    return reply;
}

void Client::connectToHost()
{
    Q_D(Client);
    //TODO add warnings
    connection()->connectToHost(d->login, d->password);
}

void Client::connectToHost(const QString &login, const QString &password)
{
    setLogin(login);
    setPassword(password);
    connectToHost();
}

void Client::disconnectFromHost()
{
    connection()->disconnectFromHost();
}


void Client::setActivity(const QString &activity)
{
    Q_D(Client);
    if (d->activity != activity) {
        auto reply = setStatus(activity);
        connect(reply, SIGNAL(resultReady(const QVariant &)), SLOT(_q_activity_update_finished(const QVariant &)));
    }
}

bool Client::isInvisible() const
{
    return d_func()->isInvisible;
}

void Client::setInvisible(bool set)
{
    Q_D(Client);
    if (d->isInvisible != set) {
        d->isInvisible = set;
        if (isOnline())
            d->setOnlineUpdaterRunning(!set);
        emit invisibleChanged(set);
    }
}

Reply *Client::setStatus(const QString &text, int aid)
{
    QVariantMap args;
    args.insert("text", text);
    if (aid)
        args.insert("audio", QString("%1_%2").arg(me()->id()).arg(aid));
    return request("status.set", args);
}

void ClientPrivate::_q_activity_update_finished(const QVariant &response)
{
    Q_Q(Client);
    auto reply = sender_cast<Reply*>(q->sender());
    if (response.toInt() == 1) {
        activity = reply->networkReply()->url().queryItemValue("text");
        emit q->activityChanged(activity);
    }
}

void ClientPrivate::_q_update_online()
{
    Q_Q(Client);
    q->request("account.setOnline");
}

void ClientPrivate::processReply(Reply *reply)
{
    Q_Q(Client);
    q->connect(reply, SIGNAL(resultReady(const QVariant &)), q, SLOT(_q_reply_finished(const QVariant &)));
    q->connect(reply, SIGNAL(error(int)), q, SLOT(_q_error_received(int)));
    emit q->replyCreated(reply);
}

void ClientPrivate::setOnlineUpdaterRunning(bool set)
{
    if (set) {
        onlineUpdater.start();
        _q_update_online();
    } else
        onlineUpdater.stop();
}

void ClientPrivate::_q_connection_state_changed(Client::State state)
{
    Q_Q(Client);
    switch (state) {
    case Client::StateOffline:
        emit q->onlineStateChanged(false);
        setOnlineUpdaterRunning(false);
        break;
    case Client::StateOnline:
        emit q->onlineStateChanged(true);
        if (!roster.isNull()) {
            roster.data()->setUid(connection.data()->uid());
            emit q->meChanged(roster.data()->owner());
        }
        if (!isInvisible)
            setOnlineUpdaterRunning(true);
        break;
    default:
        break;
    }
    emit q->connectionStateChanged(state);
}

void ClientPrivate::_q_error_received(int code)
{
    Q_Q(Client);
    auto reply = sender_cast<Reply*>(q->sender());
    qDebug() << "Error received :" << code << reply->networkReply()->url();
    reply->deleteLater();
    auto error = static_cast<Client::Error>(code);
    emit q->error(error);
    if (error == Client::ErrorAuthorizationFailed) {
        connection.data()->disconnectFromHost();
        connection.data()->clear();
    }
}

void ClientPrivate::_q_reply_finished(const QVariant &)
{
    auto reply = sender_cast<Reply*>(q_func()->sender());
    reply->deleteLater();
}

void ClientPrivate::_q_network_manager_error(int)
{

}

} // namespace vk

#include "moc_client.cpp"

