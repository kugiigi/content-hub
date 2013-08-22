/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include "ContentServiceInterface.h"
#include "ContentHandlerInterface.h"
#include "handleradaptor.h"
#include "transfer_p.h"

#include <com/ubuntu/content/hub.h>
#include <com/ubuntu/content/import_export_handler.h>
#include <com/ubuntu/content/peer.h>
#include <com/ubuntu/content/scope.h>
#include <com/ubuntu/content/store.h>
#include <com/ubuntu/content/type.h>

#include <QStandardPaths>

#include <map>

namespace cuc = com::ubuntu::content;

struct cuc::Hub::Private
{
    Private(QObject* parent) : service(
        new com::ubuntu::content::dbus::Service(
            "com.ubuntu.content.dbus.Service",
            "/",
            QDBusConnection::sessionBus(),
            parent))
    {
    }

    com::ubuntu::content::dbus::Service* service;
};

cuc::Hub::Hub(QObject* parent) : QObject(parent), d{new cuc::Hub::Private{this}}
{
}

cuc::Hub::~Hub()
{
}

cuc::Hub* cuc::Hub::Client::instance()
{
    static cuc::Hub* hub = new cuc::Hub(nullptr);
    return hub;
}

void cuc::Hub::register_import_export_handler(cuc::ImportExportHandler* handler)
{
    qDebug() << Q_FUNC_INFO;
    /*
    auto *h =
        new com::ubuntu::content::dbus::Handler(
            "com.ubuntu.content.dbus.Foo",
            "/com/ubuntu/content/transfer/ImportExportHandler",
            QDBusConnection::sessionBus(),
            handler);
    qDebug() << Q_FUNC_INFO << "CONNECTION IS VALID:" << h->isValid();
    */
    auto c = QDBusConnection::sessionBus();
    //auto c = QDBusConnection::connectToPeer(QDBusConnection::sessionBus(), "com.ubuntu.content.dbus.Service");
    auto h = new cuc::detail::Handler(c, handler);
    qDebug() << Q_FUNC_INFO << "baseService:" << c.baseService();

    new HandlerAdaptor(h);
    //auto r = c.registerService("com.ubuntu.content.dbus.Foo");
    //if ( r )
    //    qDebug() << Q_FUNC_INFO << "name success";
    auto o = c.registerObject("/com/ubuntu/content/transfer/ImportExportHandler", h);
    if ( o )
        qDebug() << Q_FUNC_INFO << "object success";
    //auto t = d->service->connection().objectRegisteredAt("/transfers/com_example_pictures/export/1");
    //cuc::Transfer *transfer = t;
    //cuc::Transfer *transfer = static_cast<cuc::Transfer*>(d->service->connection().objectRegisteredAt("/transfers/com_example_pictures/export/1"));
    //if (t == nullptr)
    //    qDebug() << Q_FUNC_INFO << "Invalid transfer";
    //else
    //    qDebug() << Q_FUNC_INFO << "Transfer state:" << transfer->state();
    //auto *foo = d->service->connection().objectRegisteredAt("/transfers/com_example_pictures/export/1");
    //qDebug() << "OBJECT: " << foo->objectName();
    Q_UNUSED(handler);
    Q_UNUSED(h);
    //h->connection().registerObject("/foobar", handler, QDBusConnection::ExportAllContents);
    //h->HandleExport(QDBusObjectPath{"/transfers/com_example_pictures/export/1"});
    //handler->handle_export(transfer);

    qDebug() << Q_FUNC_INFO << "PID: " << c.interface()->servicePid(c.baseService());
    auto x = QDBusConnection::connectToBus(QDBusConnection::SessionBus, c.baseService());
    qDebug() << Q_FUNC_INFO << x.isConnected();
    auto foo = x.objectRegisteredAt("/com/ubuntu/content/transfer/ImportExportHandler");
    if (foo != nullptr)
        qDebug() << Q_FUNC_INFO << "got valid object";
    //x.connect(c.baseService(), "/com/ubuntu/content/transfer/ImportExportHandler", "com.ubuntu.content.dbus.Handler", "foo.bar", this);

    d->service->RegisterImportExportHandler(QString("foo"), QString("com.example.pictures"), QString(c.baseService()), QDBusObjectPath{"/com/ubuntu/content/transfer/ImportExportHandler"});

}

const cuc::Store* cuc::Hub::store_for_scope_and_type(cuc::Scope scope, cuc::Type type)
{
    static const std::map<std::pair<cuc::Scope, cuc::Type>, cuc::Store*> lut =
            {
                {{cuc::system, cuc::Type::Known::pictures()}, new cuc::Store{"/content/Pictures", this}},
                {{cuc::system, cuc::Type::Known::music()}, new cuc::Store{"/content/Music", this}},
                {{cuc::system, cuc::Type::Known::documents()}, new cuc::Store{"/content/Documents", this}},
                {{cuc::user, cuc::Type::Known::pictures()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), this}},
                {{cuc::user, cuc::Type::Known::music()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::MusicLocation), this}},
                {{cuc::user, cuc::Type::Known::documents()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), this}},
                {{cuc::app, cuc::Type::Known::pictures()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Pictures", this}},
                {{cuc::app, cuc::Type::Known::music()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Music", this}},
                {{cuc::app, cuc::Type::Known::documents()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Documents", this}},
            };

    auto it = lut.find(std::make_pair(scope, type));

    if (it == lut.end())
        return nullptr;

    return it->second;
}

cuc::Peer cuc::Hub::default_peer_for_type(cuc::Type t)
{
    auto reply = d->service->DefaultPeerForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return cuc::Peer::unknown();

    return cuc::Peer(reply.value(), this);
}

QVector<cuc::Peer> cuc::Hub::known_peers_for_type(cuc::Type t)
{
    QVector<cuc::Peer> result;

    auto reply = d->service->KnownPeersForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return result;
    
    auto ids = reply.value();

    Q_FOREACH(const QString& id, ids)
    {
        result << cuc::Peer(id, this);
    }

    return result;
}

cuc::Transfer* cuc::Hub::create_import_for_type_from_peer(cuc::Type type, cuc::Peer peer)
{
    auto reply = d->service->CreateImportForTypeFromPeer(type.id(), peer.id());
    reply.waitForFinished();

    if (reply.isError())
        return nullptr;

    return cuc::Transfer::Private::make_transfer(reply.value(), this);
}

void cuc::Hub::quit()
{
    d->service->Quit();
}
