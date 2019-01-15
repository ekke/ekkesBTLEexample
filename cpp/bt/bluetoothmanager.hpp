#ifndef BLUETOOTHMANAGER_HPP
#define BLUETOOTHMANAGER_HPP

#include <QObject>

#include <QLowEnergyController>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include<QtBluetooth/QBluetoothServiceInfo>
#include <QtBluetooth/QBluetoothDeviceInfo>

#include "mybluetoothdeviceinfo.hpp"
#include "addimatmapping.hpp"
#include "heartratemessages.hpp"
#include "nfcreadermapping.hpp"

// C U S T O M   T Y P E S
// after copying BlueToothManager to your own project
// please comment or remove unused includes and unused friend classes below !

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant devicesList READ getDevices NOTIFY devicesUpdated)
    Q_PROPERTY(bool deviceDiscoveryActive READ deviceDiscoveryActive NOTIFY deviceDiscoveryActiveChanged)
    Q_PROPERTY(QString searchAddress READ searchAddress WRITE setSearchAddress NOTIFY searchAddressChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    friend class HeartRateManager;
    friend class WaiterLockManager;
    friend class NfcReaderManager;
    friend class GeneralScanManager;

    void init();

    Q_INVOKABLE
    void startDeviceDiscovery();

    Q_INVOKABLE
    void stopDeviceDiscovery();

    Q_INVOKABLE
    void clearDevices();

    Q_INVOKABLE
    MyBluetoothDeviceInfo* searchDevice(QString address);

    QVariant getDevices();
    bool deviceDiscoveryActive();
    QString searchAddress();
    void setSearchAddress(QString address);

    // CLEAN UP
    void onAboutToQuit();

signals:
    void devicesUpdated();
    void deviceDiscoveryActiveChanged();
    void searchAddressChanged();

    void deviceDiscoveryErrorReceived(QString errorMessage);

    // app specific !!
    // per ex BarcodeScanner, Addimat, MobilePrinter, ...
    // void barcodeScannerAdded(MyBluetoothDeviceInfo* deviceInfo);
    // void addimatAdded(MyBluetoothDeviceInfo* deviceInfo);

public slots:

private slots:
    // QBluetoothDeviceDiscoveryAgent related
    void onDeviceDiscoveryFinished();
    void onDeviceDiscoveryCanceled();
    void onDeviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error);
    void onDeviceDiscovered(QBluetoothDeviceInfo discoveredDevice);

private:

    QList<QObject*> mDevices;

    bool mDeviceDiscoveryActive;
    QString mSearchAddress;

    QBluetoothDeviceDiscoveryAgent* mDiscoveryAgent;

    QString mDataRoot;
    QString mBluetoothSettingsPath;
    bool checkDirs();
    QVariantMap mBluetoothSettingsMap;
    void readSettings();
    void cacheSettings();

};

#endif // BLUETOOTHMANAGER_HPP
