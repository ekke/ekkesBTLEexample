#include "bluetoothmanager.hpp"

#include <QtQml>
#include <QJsonObject>
#include <QFile>

// static const int DEVICE_TYPE_UNKNOWN = 0;
static const int DEVICE_TYPE_HEARTRATE = 1;
// static const int DEVICE_TYPE_WEIGHT = 2;
static const int DEVICE_TYPE_LOCK = 3;
// static const int DEVICE_TYPE_BEACON = 4;
// static const int DEVICE_TYPE_PEGELMETER = 5;
static const int DEVICE_TYPE_NFC = 6;
static const int DEVICE_TYPE_BARCODE = 7;

BluetoothManager::BluetoothManager(QObject *parent) : QObject(parent), mDeviceDiscoveryActive(false)
{
    mDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    // this app waits for SIGNAL finished to get all scanned devices
    // if searching only a specific device you can listen for deviceDiscovered(),
    // check the names or addresses and if found stop() to get cancel() SIGNAL
    connect(mDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothManager::onDeviceDiscovered);
    connect(mDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(onDeviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(mDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BluetoothManager::onDeviceDiscoveryFinished);
    connect(mDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BluetoothManager::onDeviceDiscoveryCanceled);

    // register types
    qmlRegisterType<MyBluetoothDeviceInfo>("org.ekkescorner.btle", 1, 0, "MyBluetoothDeviceInfo");
    qmlRegisterType<MyBluetoothServiceInfo>("org.ekkescorner.btle", 1, 0, "MyBluetoothServiceInfo");
    qmlRegisterType<MyBluetoothCharacteristic>("org.ekkescorner.btle", 1, 0, "MyBluetoothCharacteristic");
    // C U S T O M   T Y P E S
    // after copying BlueToothManager to your own project
    // please comment or remove unused types !
    // custom entity for Addimat WaiterLock
    qmlRegisterType<AddimatMapping>("org.ekkescorner.btle", 1, 0, "AddimatMapping");
    // custom entity for HeartRate
    qmlRegisterType<HeartRateMessages>("org.ekkescorner.btle", 1, 0, "HeartRateMessages");
    // custom entity for NFCReader
    qmlRegisterType<NfcReaderMapping>("org.ekkescorner.btle", 1, 0, "NfcMapping");
}

void BluetoothManager::init()
{
    // Android: HomeLocation works, iOS: not writable
    // Android: AppDataLocation works out of the box, iOS you must create the DIR first !!
    mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
    qDebug() << "Data Root:" << mBluetoothSettingsPath;
    bool ok = checkDirs();
    if(!ok) {
        qFatal("App won't work - cannot access root directory");
    }
    mBluetoothSettingsPath = mDataRoot + "/bt_settings.json";
    qDebug() << "Bluetooth Settings Path:" << mBluetoothSettingsPath;
    // now read cached settings file - if already existing
    readSettings();
    // Settings will be used by all other (Friends) Managers
    // per ex. HeartRateManager, WaiterLockManager
    // each Manager uses some common data (lastUsedAddress, favoriteAddress)
    // and also specific data
    // Settings are only used by BT LE Types known by this app
    // changes to settings are seldom, so they're cached always immediately
}

bool BluetoothManager::checkDirs()
{
    QDir myDir;
    bool exists;
    exists = myDir.exists(mDataRoot);
    if (!exists) {
        bool ok = myDir.mkpath(mDataRoot);
        if(!ok) {
            qWarning() << "Couldn't create mDataRoot " << mDataRoot;
            return false;
        }
        qDebug() << "created directory mDataRoot " << mDataRoot;
    } else {
        qDebug() << "check dirs: all is OK - Data Root exists";
    }
    return true;
}

void BluetoothManager::readSettings()
{
    mBluetoothSettingsMap.clear();
    QJsonDocument jda;
    QFile settingsFile(mBluetoothSettingsPath);
    // check if already something cached
    if (!settingsFile.exists()) {
        qDebug() << "No Bluetooth Settings exists yet";
        return;
    }
    if (!settingsFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file: " << mBluetoothSettingsPath;
        return;
    }
    jda = QJsonDocument::fromJson(settingsFile.readAll());
    settingsFile.close();
    if(!jda.isObject()) {
        qWarning() << "Couldn't create JSON Object from file: " << mBluetoothSettingsPath;
    }
    mBluetoothSettingsMap = jda.toVariant().toMap();
    qDebug() << "Bluetooth Settings Map successfully created";
}

void BluetoothManager::cacheSettings()
{
    // convert Settings Map into JSONDocument and store to app data
    QJsonDocument jda = QJsonDocument::fromVariant(mBluetoothSettingsMap);
    QFile settingsFile(mBluetoothSettingsPath);
    if (!settingsFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file to write " << mBluetoothSettingsPath;
        return;
    }
    qint64 bytesWritten = settingsFile.write(jda.toJson());
    settingsFile.close();
    qDebug() << "Bluetooth Settings Bytes written: " << bytesWritten;
}

// devices will be added to do a clean discovery
// you must call clearDevices() first
void BluetoothManager::startDeviceDiscovery()
{
    // Android iOS device: 25000 is default
    mDiscoveryAgent->setLowEnergyDiscoveryTimeout(15000);

    // new with Qt 5.8: DiscoveryMethod to filter Devices
    // using this also fixes a problem that devices were added twice :)
    mDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    if (mDiscoveryAgent->isActive()) {
        qDebug() << "scanning active ...";
        mDeviceDiscoveryActive = true;
        emit deviceDiscoveryActiveChanged();
    } else {
        qDebug() << "scanning NOT active";
    }
}

void BluetoothManager::stopDeviceDiscovery()
{
    mDiscoveryAgent->stop();
}

MyBluetoothDeviceInfo *BluetoothManager::searchDevice(QString address)
{
    for (int i = 0; i < mDevices.size(); ++i) {
        MyBluetoothDeviceInfo* myDeviceInfo = (MyBluetoothDeviceInfo*)mDevices.at(i);
        if(myDeviceInfo->getAddress() == address) {
            return myDeviceInfo;
        }
    }
    return nullptr;
}

QVariant BluetoothManager::getDevices()
{
    return QVariant::fromValue(mDevices);
}

bool BluetoothManager::deviceDiscoveryActive()
{
    return mDeviceDiscoveryActive;
}

QString BluetoothManager::searchAddress()
{
    return mSearchAddress;
}

void BluetoothManager::setSearchAddress(QString address)
{
    if(mSearchAddress != address) {
        mSearchAddress = address;
        emit searchAddressChanged();
    }
}

void BluetoothManager::onAboutToQuit()
{
    for (int i = 0; i < mDevices.size(); ++i) {
        MyBluetoothDeviceInfo* deviceInfo = (MyBluetoothDeviceInfo*)mDevices.at(i);
        if(deviceInfo->controller()) {
            qDebug() << "disconnect from " << deviceInfo->getDevice().name();
            deviceInfo->controller()->disconnectFromDevice();
        }
    }
}

void BluetoothManager::onDeviceDiscovered(QBluetoothDeviceInfo discoveredDevice)
{
    // check if already discovered
    for (int i = 0; i < mDevices.size(); ++i) {
        MyBluetoothDeviceInfo* mdi = (MyBluetoothDeviceInfo*)mDevices.at(i);
        QBluetoothDeviceInfo bdi = mdi->getDevice();
#if defined (Q_OS_IOS) || defined (Q_OS_MAC)
        // On MacOS and iOS we get no access to device address,
        // only unique UUIDs generated by Core Bluetooth.
        if(bdi.deviceUuid() == discoveredDevice.deviceUuid()) {
            qDebug() << "already discovered: " << mdi->getName() << " " << bdi.deviceUuid();
            return;
        }
#else
        if(bdi.address() == discoveredDevice.address()) {
            qDebug() << "already discovered: " << mdi->getName() << " " << bdi.address();
            return;
        }
#endif
    }

    MyBluetoothDeviceInfo *d = new MyBluetoothDeviceInfo();
    d->setDevice(discoveredDevice);
    // check if device type is known
    bool isKnownDeviceType = false;
    if(discoveredDevice.name().startsWith("Addimat", Qt::CaseInsensitive)) {
        d->setDeviceType(DEVICE_TYPE_LOCK);
        isKnownDeviceType = true;
        // app specific
        // emit addimatAdded(d);
    } else if (discoveredDevice.name() == "NFC_RDR") {
        d->setDeviceType(DEVICE_TYPE_NFC);
        isKnownDeviceType = true;
    }
    if(!isKnownDeviceType) {
        for (int i = 0; i < discoveredDevice.serviceUuids().size(); ++i) {
            QBluetoothUuid uuid = discoveredDevice.serviceUuids().at(i);
            // this also detects the long UUID {0000180d-0000-1000-8000-00805f9b34fb}
            if(uuid == QBluetoothUuid::HeartRate) {
                d->setDeviceType(DEVICE_TYPE_HEARTRATE);
            }
            // detects the long version of fff0: {0000fff0-0000-1000-8000-00805f9b34fb}
            if(uuid.toString() == "{0000fff0-0000-1000-8000-00805f9b34fb}") {
                d->setDeviceType(DEVICE_TYPE_BARCODE);
                // app specific
                // emit barcodeScannerAdded(d);
            }
        }
    }
    //
    mDevices.append(d);
    qDebug() << "Add Device: " << d->getName() << " Address: " << d->getAddress();
    qDebug() << "Class: " << d->getDeviceClass();
    qDebug() << "RSSI: " << d->getRssi();
    qDebug() << "Service U U I D s: " << d->getServiceUUIDs();
    emit devicesUpdated();
    // Check if search was done for specific address
    if(mSearchAddress.length() > 0  && d->getAddress() == mSearchAddress) {
        stopDeviceDiscovery();
    }
}

void BluetoothManager::clearDevices()
{
    // it's a good idea to cleanup before deleting DeviceInfos
    for (int i = 0; i < mDevices.size(); ++i) {
        MyBluetoothDeviceInfo* di = (MyBluetoothDeviceInfo*) mDevices.at(i);
        // this will also reset flags and clear messages lists etc
        di->disconnectFromDevice();
    }
    qDeleteAll(mDevices);
    mDevices.clear();
    emit devicesUpdated();
}

void BluetoothManager::onDeviceDiscoveryFinished()
{
    qDebug() << "discovered devices: " << mDiscoveryAgent->discoveredDevices().size();
    setSearchAddress("");
    if (mDevices.isEmpty()) {
        // TODO show info to UI
        qDebug() << "device scan finished: no Low Energy devices found";
    } else {
        qDebug() << "Scan done - found LE devices: " << mDevices.size();
    }
    mDeviceDiscoveryActive = false;
    emit deviceDiscoveryActiveChanged();
    emit devicesUpdated();
}

void BluetoothManager::onDeviceDiscoveryCanceled()
{
    qDebug() << "device discovery canceled: " << mDiscoveryAgent->discoveredDevices().size();
    setSearchAddress("");
    if (mDevices.isEmpty()) {
        // TODO show info to UI
        qDebug() << "device discovery finished: no Low Energy devices found";
    } else {
        qDebug() << "Device Discovery canceled - found LE devices: " << mDevices.size();
    }
    mDeviceDiscoveryActive = false;
    emit deviceDiscoveryActiveChanged();
    emit devicesUpdated();
}

void BluetoothManager::onDeviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorString;
    errorString = ((QBluetoothDeviceDiscoveryAgent*)sender())->errorString();
    qWarning() << "Error: " << errorString << " | " << error;
    QString errorMessage;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorMessage = tr("The Bluetooth adaptor is powered off, power it on before doing discovery.");
        qDebug() << "deviceScanError: Bluetooth is OFF";
        // iOS lets the user go to settings and switch ON
        // Android user must do this manually
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorMessage = tr("Discovery error: Writing or reading from the device resulted in an error.");
        qDebug() << "deviceScanError: r/w from device error";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorMessage = tr("Discovery error: invalid Bluetooth Adapter");
        qDebug() << "deviceScanError: InvalidBluetoothAdapterError";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        errorMessage = tr("Discovery error: unsupported platform");
        qDebug() << "deviceScanError: UnsupportedPlatformError";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        errorMessage = tr("Discovery error: unsupported Discovery Method"),
                qDebug() << "deviceScanError: UnsupportedDiscoveryMethod";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnknownError:
        errorMessage = tr("Discovery error: unknown error");
        qDebug() << "deviceScanError: UnknownError";
        break;
    default:
        errorMessage = tr("Discovery error: unhandled error. Code: %1").arg(error);
        qWarning() << "deviceScanError: unhandled error: " << error;
        break;
    }

    mDeviceDiscoveryActive = false;
    emit devicesUpdated();
    emit deviceDiscoveryActiveChanged();
    emit deviceDiscoveryErrorReceived(errorMessage);
}





