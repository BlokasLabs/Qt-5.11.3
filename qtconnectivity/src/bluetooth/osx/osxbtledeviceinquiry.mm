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

#include "osxbtledeviceinquiry_p.h"
#include "qbluetoothdeviceinfo.h"
#include "osxbtnotifier_p.h"
#include "qbluetoothuuid.h"
#include "osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

QBluetoothUuid qt_uuid(NSUUID *nsUuid)
{
    if (!nsUuid)
        return QBluetoothUuid();

    uuid_t uuidData = {};
    [nsUuid getUUIDBytes:uuidData];
    quint128 qtUuidData = {};
    std::copy(uuidData, uuidData + 16, qtUuidData.data);
    return QBluetoothUuid(qtUuidData);
}

const int timeStepMS = 100;
const int powerOffTimeoutMS = 30000;

struct AdvertisementData {
    // That's what CoreBluetooth has:
    // CBAdvertisementDataLocalNameKey
    // CBAdvertisementDataTxPowerLevelKey
    // CBAdvertisementDataServiceUUIDsKey
    // CBAdvertisementDataServiceDataKey
    // CBAdvertisementDataManufacturerDataKey
    // CBAdvertisementDataOverflowServiceUUIDsKey
    // CBAdvertisementDataIsConnectable
    // CBAdvertisementDataSolicitedServiceUUIDsKey

    // For now, we "parse":
    QString localName;
    QList<QBluetoothUuid> serviceUuids;
    // TODO: other keys probably?
    AdvertisementData(NSDictionary *AdvertisementData);
};

AdvertisementData::AdvertisementData(NSDictionary *advertisementData)
{
    if (!advertisementData)
        return;

    // ... constant CBAdvertisementDataLocalNameKey ...
    // NSString containing the local name of a peripheral.
    NSObject *value = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    if (value && [value isKindOfClass:[NSString class]])
        localName = QString::fromNSString(static_cast<NSString *>(value));

    // ... constant CBAdvertisementDataServiceUUIDsKey ...
    // A list of one or more CBUUID objects, representing CBService UUIDs.

    value = [advertisementData objectForKey:CBAdvertisementDataServiceUUIDsKey];
    if (value && [value isKindOfClass:[NSArray class]]) {
        NSArray *uuids = static_cast<NSArray *>(value);
        for (CBUUID *cbUuid in uuids)
            serviceUuids << qt_uuid(cbUuid);
    }
}

}

QT_END_NAMESPACE

QT_USE_NAMESPACE

@implementation QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry)

-(id)initWithNotifier:(LECBManagerNotifier *)aNotifier
{
    if (self = [super init]) {
        Q_ASSERT(aNotifier);
        notifier = aNotifier;
        internalState = InquiryStarting;
        inquiryTimeoutMS = OSXBluetooth::defaultLEScanTimeoutMS;
    }

    return self;
}

- (void)dealloc
{
    if (manager) {
        [manager setDelegate:nil];
        if (internalState == InquiryActive)
            [manager stopScan];
    }

    if (notifier) {
        notifier->disconnect();
        notifier->deleteLater();
    }

    [super dealloc];
}

- (void)timeout
{
    if (internalState == InquiryActive) {
        [manager stopScan];
        [manager setDelegate:nil];
        internalState = InquiryFinished;
        Q_ASSERT(notifier);
        emit notifier->discoveryFinished();
    } else if (internalState == InquiryStarting) {
        // This is interesting on iOS only, where the system shows an alert
        // asking to enable Bluetooth in the 'Settings' app. If not done yet
        // (after 30 seconds) - we consider this as an error.
        [manager setDelegate:nil];
        internalState = ErrorPoweredOff;
        Q_ASSERT(notifier);
        emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
    }
}

- (void)startWithTimeout:(int)timeout
{
    dispatch_queue_t leQueue(OSXBluetooth::qt_LE_queue());
    Q_ASSERT(leQueue);
    inquiryTimeoutMS = timeout;
    manager.reset([[CBCentralManager alloc] initWithDelegate:self queue:leQueue]);
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

    if (central != manager)
        return;

    if (internalState != InquiryActive && internalState != InquiryStarting)
        return;

    Q_ASSERT(notifier);

    using namespace OSXBluetooth;

    const auto state = central.state;
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    if (state == CBManagerStatePoweredOn) {
#else
    if (state == CBCentralManagerStatePoweredOn) {
#endif
        if (internalState == InquiryStarting) {
            internalState = InquiryActive;

            if (inquiryTimeoutMS > 0) {
                [elapsedTimer cancelTimer];
                elapsedTimer.reset([[GCDTimerObjC alloc] initWithDelegate:self]);
                [elapsedTimer startWithTimeout:inquiryTimeoutMS step:timeStepMS];
            }

            [manager scanForPeripheralsWithServices:nil options:nil];
        } // Else we ignore.
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    } else if (state == CBManagerStateUnsupported || state == CBManagerStateUnauthorized) {
#else
    } else if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
#endif
        if (internalState == InquiryActive) {
            [manager stopScan];
            // Not sure how this is possible at all,
            // probably, can never happen.
            internalState = ErrorPoweredOff;
            emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        } else {
            internalState = ErrorLENotSupported;
            emit notifier->LEnotSupported();
        }

        [manager setDelegate:nil];
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    } else if (state == CBManagerStatePoweredOff) {
#else
    } else if (state == CBCentralManagerStatePoweredOff) {
#endif
        if (internalState == InquiryStarting) {
#ifndef Q_OS_OSX
            // On iOS a user can see at this point an alert asking to
            // enable Bluetooth in the "Settings" app. If a user does so,
            // we'll receive 'PoweredOn' state update later.
            // No change in internalState. Wait for 30 seconds.
            [elapsedTimer cancelTimer];
            elapsedTimer.reset([[GCDTimerObjC alloc] initWithDelegate:self]);
            [elapsedTimer startWithTimeout:powerOffTimeoutMS step:300];
            return;
#else
            Q_UNUSED(powerOffTimeoutMS)
#endif
            internalState = ErrorPoweredOff;
            emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        } else {
            [manager stopScan];
            emit notifier->CBManagerError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        }

        [manager setDelegate:nil];
    } else {
        // The following two states we ignore (from Apple's docs):
        //"
        // -CBCentralManagerStateUnknown
        // The current state of the central manager is unknown;
        // an update is imminent.
        //
        // -CBCentralManagerStateResetting
        // The connection with the system service was momentarily
        // lost; an update is imminent. "
        // Wait for this imminent update.
    }

#pragma clang diagnostic pop
}

- (void)stop
{
    if (internalState == InquiryActive)
        [manager stopScan];

    [elapsedTimer cancelTimer];

    [manager setDelegate:nil];
    internalState = InquiryCancelled;

    notifier->disconnect();
    notifier->deleteLater();
    notifier = nullptr;
}

- (void)centralManager:(CBCentralManager *)central
        didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
        RSSI:(NSNumber *)RSSI
{
    using namespace OSXBluetooth;

    if (central != manager)
        return;

    if (internalState != InquiryActive)
        return;

    if (!notifier)
        return;

    QBluetoothUuid deviceUuid;

    if (!peripheral.identifier) {
        qCWarning(QT_BT_OSX) << "peripheral without NSUUID";
        return;
    }

    deviceUuid = OSXBluetooth::qt_uuid(peripheral.identifier);

    if (deviceUuid.isNull()) {
        qCWarning(QT_BT_OSX) << "no way to address peripheral, QBluetoothUuid is null";
        return;
    }

    const AdvertisementData qtAdvData(advertisementData);
    QString name(qtAdvData.localName);
    if (!name.size() && peripheral.name)
        name = QString::fromNSString(peripheral.name);

    // TODO: fix 'classOfDevice' (0 for now).
    QBluetoothDeviceInfo newDeviceInfo(deviceUuid, name, 0);
    if (RSSI)
        newDeviceInfo.setRssi([RSSI shortValue]);

    if (qtAdvData.serviceUuids.size()) {
        newDeviceInfo.setServiceUuids(qtAdvData.serviceUuids,
                                      QBluetoothDeviceInfo::DataIncomplete);
    }

    // CoreBluetooth scans only for LE devices.
    newDeviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    emit notifier->deviceDiscovered(newDeviceInfo);
}

@end
