/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "osx/osxbtservicerecord_p.h"
#include "qbluetoothserver_osx_p.h"
#include "qbluetoothserviceinfo.h"
#include "qbluetoothdeviceinfo.h"
#include "osx/osxbtutility_p.h"
#include "osx/osxbluetooth_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qvariant.h>
#include <QtCore/qglobal.h>
#include <QtCore/qmutex.h>
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>

#include <Foundation/Foundation.h>

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfoPrivate
{
public:

    typedef QBluetoothServiceInfo QSInfo;

    bool registerService(const OSXBluetooth::ObjCStrongReference<NSMutableDictionary> &serviceDict);
    bool isRegistered() const;
    bool unregisterService();

    QBluetoothDeviceInfo deviceInfo;
    QMap<quint16, QVariant> attributes;

    QBluetoothServiceInfo::Sequence protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const;
    QBluetoothServiceInfo::Protocol socketProtocol() const;
    int protocolServiceMultiplexer() const;
    int serverChannel() const;

private:

    bool registered = false;

    typedef OSXBluetooth::ObjCScopedPointer<IOBluetoothSDPServiceRecord> SDPRecord;
    SDPRecord serviceRecord;
    BluetoothSDPServiceRecordHandle serviceRecordHandle = 0;
};

bool QBluetoothServiceInfoPrivate::registerService(const OSXBluetooth::ObjCStrongReference<NSMutableDictionary> &serviceDict)
{
    using namespace OSXBluetooth;

    Q_ASSERT(serviceDict);
    Q_ASSERT(!registered);
    Q_ASSERT_X(!serviceRecord, Q_FUNC_INFO, "not registered, but serviceRecord is not nil");

    SDPRecord newRecord;
    newRecord.reset([[IOBluetoothSDPServiceRecord
                     publishedServiceRecordWithDictionary:serviceDict] retain]);
    if (!newRecord) {
        qCWarning(QT_BT_OSX) << "failed to register a service record";
        return false;
    }

    BluetoothSDPServiceRecordHandle newRecordHandle = 0;
    if ([newRecord getServiceRecordHandle:&newRecordHandle] != kIOReturnSuccess) {
        qCWarning(QT_BT_OSX) << "failed to register a service record";
        [newRecord removeServiceRecord];
        return false;
    }

    const QSInfo::Protocol type = socketProtocol();
    quint16 realPort = 0;
    QBluetoothServerPrivate *server = nullptr;
    bool configured = false;

    if (type == QBluetoothServiceInfo::L2capProtocol) {
        BluetoothL2CAPPSM psm = 0;
        server = QBluetoothServerPrivate::registeredServer(protocolServiceMultiplexer(), type);
        if ([newRecord getL2CAPPSM:&psm] == kIOReturnSuccess) {
            configured = true;
            realPort = psm;
        }
    } else if (type == QBluetoothServiceInfo::RfcommProtocol) {
        BluetoothRFCOMMChannelID channelID = 0;
        server = QBluetoothServerPrivate::registeredServer(serverChannel(), type);
        if ([newRecord getRFCOMMChannelID:&channelID] == kIOReturnSuccess) {
            configured = true;
            realPort = channelID;
        }
    }

    if (!configured) {
        [newRecord removeServiceRecord];
        qCWarning(QT_BT_OSX) << "failed to register a service record";
        return false;
    }

    registered = true;
    serviceRecord.reset(newRecord.take());
    serviceRecordHandle = newRecordHandle;

    if (server)
        server->startListener(realPort);

    return true;
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return registered;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    if (!registered)
        return false;

    Q_ASSERT_X(serviceRecord, Q_FUNC_INFO, "service registered, but serviceRecord is nil");

    [serviceRecord removeServiceRecord];
    serviceRecord.reset(nil);

    const QSInfo::Protocol type = socketProtocol();
    QBluetoothServerPrivate *server = nullptr;

    const QMutexLocker lock(&QBluetoothServerPrivate::channelMapMutex());
    if (type == QSInfo::RfcommProtocol)
        server = QBluetoothServerPrivate::registeredServer(serverChannel(), type);
    else if (type == QSInfo::L2capProtocol)
        server = QBluetoothServerPrivate::registeredServer(protocolServiceMultiplexer(), type);

    if (server)
        server->stopListener();

    registered = false;
    serviceRecordHandle = 0;

    return true;
}

bool QBluetoothServiceInfo::isRegistered() const
{
    return d_ptr->isRegistered();
}

bool QBluetoothServiceInfo::registerService(const QBluetoothAddress &localAdapter)
{
    Q_UNUSED(localAdapter);
    if (isRegistered())
        return false;

    using namespace OSXBluetooth;

    ObjCStrongReference<NSMutableDictionary> serviceDict(iobluetooth_service_dictionary(*this));
    if (!serviceDict) {
        qCWarning(QT_BT_OSX) << "failed to create a service dictionary";
        return false;
    }

    return d_ptr->registerService(serviceDict);
}

bool QBluetoothServiceInfo::unregisterService()
{
    return d_ptr->unregisterService();
}

QBluetoothServiceInfo::QBluetoothServiceInfo()
    : d_ptr(new QBluetoothServiceInfoPrivate)
{
}

QBluetoothServiceInfo::QBluetoothServiceInfo(const QBluetoothServiceInfo &other)
    : d_ptr(other.d_ptr)
{
}

QBluetoothServiceInfo::~QBluetoothServiceInfo()
{
}

bool QBluetoothServiceInfo::isValid() const
{
    return !d_ptr->attributes.isEmpty();
}

bool QBluetoothServiceInfo::isComplete() const
{
    return d_ptr->attributes.contains(ProtocolDescriptorList);
}

QBluetoothDeviceInfo QBluetoothServiceInfo::device() const
{
    return d_ptr->deviceInfo;
}

void QBluetoothServiceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    d_ptr->deviceInfo = device;
}

void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QVariant &value)
{
    d_ptr->attributes[attributeId] = value;
}

QVariant QBluetoothServiceInfo::attribute(quint16 attributeId) const
{
    return d_ptr->attributes.value(attributeId);
}

QList<quint16> QBluetoothServiceInfo::attributes() const
{
    return d_ptr->attributes.keys();
}

bool QBluetoothServiceInfo::contains(quint16 attributeId) const
{
    return d_ptr->attributes.contains(attributeId);
}

void QBluetoothServiceInfo::removeAttribute(quint16 attributeId)
{
    d_ptr->attributes.remove(attributeId);
}

QBluetoothServiceInfo::Protocol QBluetoothServiceInfo::socketProtocol() const
{
    return d_ptr->socketProtocol();
}

int QBluetoothServiceInfo::protocolServiceMultiplexer() const
{
    return d_ptr->protocolServiceMultiplexer();
}

int QBluetoothServiceInfo::serverChannel() const
{
    return d_ptr->serverChannel();
}

QBluetoothServiceInfo::Sequence QBluetoothServiceInfo::protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const
{
    return d_ptr->protocolDescriptor(protocol);
}

QList<QBluetoothUuid> QBluetoothServiceInfo::serviceClassUuids() const
{
    QList<QBluetoothUuid> results;

    const QVariant var = attribute(QBluetoothServiceInfo::ServiceClassIds);
    if (!var.isValid())
        return results;

    const QBluetoothServiceInfo::Sequence seq = var.value<QBluetoothServiceInfo::Sequence>();
    for (int i = 0; i < seq.count(); i++)
        results.append(seq.at(i).value<QBluetoothUuid>());

    return results;
}

QBluetoothServiceInfo &QBluetoothServiceInfo::operator=(const QBluetoothServiceInfo &other)
{
    if (this != &other)
        d_ptr = other.d_ptr;

    return *this;
}

static void dumpAttributeVariant(const QVariant &var, const QString indent)
{
    switch (int(var.type())) {
    case QMetaType::Void:
        qDebug("%sEmpty", indent.toLocal8Bit().constData());
        break;
    case QMetaType::UChar:
        qDebug("%suchar %u", indent.toLocal8Bit().constData(), var.toUInt());
        break;
    case QMetaType::UShort:
        qDebug("%sushort %u", indent.toLocal8Bit().constData(), var.toUInt());
    case QMetaType::UInt:
        qDebug("%suint %u", indent.toLocal8Bit().constData(), var.toUInt());
        break;
    case QMetaType::Char:
        qDebug("%schar %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::Short:
        qDebug("%sshort %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::Int:
        qDebug("%sint %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::QString:
        qDebug("%sstring %s", indent.toLocal8Bit().constData(), var.toString().toLocal8Bit().constData());
        break;
    case QMetaType::Bool:
        qDebug("%sbool %d", indent.toLocal8Bit().constData(), var.toBool());
        break;
    case QMetaType::QUrl:
        qDebug("%surl %s", indent.toLocal8Bit().constData(), var.toUrl().toString().toLocal8Bit().constData());
        break;
    case QVariant::UserType:
        if (var.userType() == qMetaTypeId<QBluetoothUuid>()) {
            QBluetoothUuid uuid = var.value<QBluetoothUuid>();
            switch (uuid.minimumSize()) {
            case 0:
                qDebug("%suuid NULL", indent.toLocal8Bit().constData());
                break;
            case 2:
                qDebug("%suuid %04x", indent.toLocal8Bit().constData(), uuid.toUInt16());
                break;
            case 4:
                qDebug("%suuid %08x", indent.toLocal8Bit().constData(), uuid.toUInt32());
                break;
            case 16:
                qDebug("%suuid %s", indent.toLocal8Bit().constData(), QByteArray(reinterpret_cast<const char *>(uuid.toUInt128().data), 16).toHex().constData());
                break;
            default:
                qDebug("%suuid ???", indent.toLocal8Bit().constData());
                ;
            }
        } else if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
            qDebug("%sSequence", indent.toLocal8Bit().constData());
            const QBluetoothServiceInfo::Sequence *sequence = static_cast<const QBluetoothServiceInfo::Sequence *>(var.data());
            foreach (const QVariant &v, *sequence)
                dumpAttributeVariant(v, indent + QLatin1Char('\t'));
        } else if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
            qDebug("%sAlternative", indent.toLocal8Bit().constData());
            const QBluetoothServiceInfo::Alternative *alternative = static_cast<const QBluetoothServiceInfo::Alternative *>(var.data());
            foreach (const QVariant &v, *alternative)
                dumpAttributeVariant(v, indent + QLatin1Char('\t'));
        }
        break;
    default:
        qDebug("%sunknown variant type %d", indent.toLocal8Bit().constData(), var.userType());
    }
}

QDebug operator << (QDebug dbg, const QBluetoothServiceInfo &info)
{
    foreach (quint16 id, info.attributes()) {
        dumpAttributeVariant(info.attribute(id), QString::fromLatin1("(%1)\t").arg(id));
    }
    return dbg;
}

QBluetoothServiceInfo::Sequence QBluetoothServiceInfoPrivate::protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const
{
    if (!attributes.contains(QBluetoothServiceInfo::ProtocolDescriptorList))
        return QBluetoothServiceInfo::Sequence();

    foreach (const QVariant &v, attributes.value(QBluetoothServiceInfo::ProtocolDescriptorList).value<QBluetoothServiceInfo::Sequence>()) {
        QBluetoothServiceInfo::Sequence parameters = v.value<QBluetoothServiceInfo::Sequence>();
        if (parameters.empty())
            continue;
        if (parameters.at(0).userType() == qMetaTypeId<QBluetoothUuid>()) {
            if (parameters.at(0).value<QBluetoothUuid>() == protocol)
                return parameters;
        }
    }

    return QBluetoothServiceInfo::Sequence();
}

QBluetoothServiceInfo::Protocol QBluetoothServiceInfoPrivate::socketProtocol() const
{
    QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::Rfcomm);
    if (!parameters.isEmpty())
        return QBluetoothServiceInfo::RfcommProtocol;

    parameters = protocolDescriptor(QBluetoothUuid::L2cap);
    if (!parameters.isEmpty())
        return QBluetoothServiceInfo::L2capProtocol;

    return QBluetoothServiceInfo::UnknownProtocol;
}


int QBluetoothServiceInfoPrivate::protocolServiceMultiplexer() const
{
    const QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::L2cap);
    if (parameters.isEmpty())
        return -1;
    else if (parameters.count() == 1)
        return 0;

    return parameters.at(1).toUInt();
}


int QBluetoothServiceInfoPrivate::serverChannel() const
{
    const QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::Rfcomm);
    if (parameters.isEmpty())
        return -1;
    else if (parameters.count() == 1)
        return 0;

    return parameters.at(1).toUInt();
}

QT_END_NAMESPACE
