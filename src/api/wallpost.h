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
#ifndef WALLPOST_H
#define WALLPOST_H

#include <QSharedDataPointer>
#include <QVariant>
#include "vk_global.h"
#include "attachment.h"

namespace vk {

class WallPostData;
class Client;
class Contact;

class VK_SHARED_EXPORT WallPost
{
public:  
    WallPost(Client *client = 0);
    WallPost(const WallPost &);
    WallPost &operator=(const WallPost &);
    ~WallPost();

    Client *client() const;
    void setId(int id);
    int id() const;
    void setBody(const QString &body);
    QString body() const;
    void setFromId(int id);
    int fromId() const;
    void setToId(int id);
    int toId() const;
    void setDate(const QDateTime &date);
    QDateTime date() const;
    Attachment::Hash attachments() const;
    Attachment::List attachments(Attachment::Type type) const;
    void setAttachments(const Attachment::List &attachmentList);
    QVariantMap likes() const;
    void setLikes(const QVariantMap &likes);
    QVariantMap reposts() const;
    void setReposts(const QVariantMap &reposts);

    Contact *from();
    Contact *to();

    static WallPost fromData(const QVariant data, Client *client);

    QVariant property(const QString &name, const QVariant &def = QVariant()) const;
    template<typename T>
    T property(const char *name, const T &def) const
    { return qVariantValue<T>(property(name, qVariantFromValue<T>(def))); }

    void setProperty(const QString &name, const QVariant &value);
    QStringList dynamicPropertyNames() const;
protected:
    WallPost(QVariantMap data, Client *client);
private:
    QSharedDataPointer<WallPostData> d;
};
typedef QList<WallPost> WallPostList;

} //namespace vk

#endif // WALLPOST_H

