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
#include "audioitem.h"
#include <QSharedData>
#include <QUrl>
#include "client.h"

namespace vk {

class AudioItemData : public QSharedData {
public:
    AudioItemData(Client *client) : client(client),
        id(0), ownerId(0),
        duration(0),
        lyricsId(0),
        albumId(0)
    {}
    AudioItemData(AudioItemData &o) : QSharedData(),
        client(o.client),
        id(o.id), ownerId(o.ownerId),
        artist(o.artist),
        title(o.title),
        duration(o.duration),
        url(o.url),
        lyricsId(o.lyricsId),
        albumId(o.albumId)
    {}
    Client *client;
    int id;
    int ownerId;
    QString artist;
    QString title;
    qreal duration;
    QUrl url;
    int lyricsId;
    int albumId;
};

AudioItem::AudioItem(Client *client) : data(new AudioItemData(client))
{
}

AudioItem::AudioItem(const AudioItem &rhs) : data(rhs.data)
{
}

AudioItem &AudioItem::operator=(const AudioItem &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

AudioItem::~AudioItem()
{
}

bool AudioItem::operator <(const AudioItem &other) const
{
    return id() < other.id();
}

int AudioItem::id() const
{
    return data->id;
}

void AudioItem::setId(int id)
{
    data->id = id;
}

int AudioItem::ownerId() const
{
    return data->ownerId;
}

void AudioItem::setOwnerId(int ownerId)
{
    data->ownerId = ownerId;
}

QString AudioItem::artist() const
{
    return data->artist;
}

void AudioItem::setArtist(const QString &artist)
{
    data->artist = artist;
}

QString AudioItem::title() const
{
    return data->title;
}

void AudioItem::setTitle(const QString &title)
{
    data->title = title;
}

qreal AudioItem::duration() const
{
    return data->duration;
}

void AudioItem::setDuration(qreal duration)
{
    data->duration = duration;
}

QUrl AudioItem::url() const
{
    return data->url;
}

void AudioItem::setUrl(const QUrl &url)
{
    data->url = url;
}

int AudioItem::lyricsId() const
{
    return data->lyricsId;
}

void AudioItem::setLyricsId(int lyricsId)
{
    data->lyricsId = lyricsId;
}

int AudioItem::albumId() const
{
    return data->albumId;
}

void AudioItem::setAlbumId(int albumId)
{
    data->albumId = albumId;
}

} // namespace vk

