#include "mybluetoothdeviceinfo.hpp"

MyBluetoothDeviceInfo::MyBluetoothDeviceInfo(QObject *parent) : QObject(parent), mConnected(false), mServicesDiscoveryActive(false),
    mCreateServicesActive(false), mLowEnergyController(0), mConnectionErrorCounter(0), mConnectionErrorMaxRetries(5), mRetryDeviceConnect(false),
    mRetryConnectIsEnabled(false), mDeviceType(0)
{
    //
}

void MyBluetoothDeviceInfo::setController(QLowEnergyController *controller)
{
    mLowEnergyController = controller;
    mLowEnergyController->setParent(this);
}

QLowEnergyController *MyBluetoothDeviceInfo::controller() const
{
    return mLowEnergyController;
}

QString MyBluetoothDeviceInfo::getAddress() const
{
#if defined (Q_OS_IOS) || defined (Q_OS_MAC)
    // On MacOS and iOS we get no access to device address,
    // only unique UUIDs generated by Core Bluetooth.
    qDebug() << "getting address from deviceUuid()";
    return mDevice.deviceUuid().toString();
#else
    return mDevice.address().toString();
#endif
}

QString MyBluetoothDeviceInfo::getName() const
{
    if(mDevice.name().isEmpty()) {
        return "n/a";
    }
    return mDevice.name();
}

bool MyBluetoothDeviceInfo::isCached() const
{
    return mDevice.isCached();
}

bool MyBluetoothDeviceInfo::isValid() const
{
    return mDevice.isValid();
}

// TODO add minorDeviceClass
// http://doc.qt.io/qt-5/qbluetoothdeviceinfo.html#minorDeviceClass
QString MyBluetoothDeviceInfo::getDeviceClass() const
{
    switch (mDevice.majorDeviceClass()) {
    case QBluetoothDeviceInfo::MiscellaneousDevice:
        return tr("MiscellaneousDevice");
    case QBluetoothDeviceInfo::ComputerDevice:
        return tr("ComputerDevice");
    case QBluetoothDeviceInfo::PhoneDevice:
        return tr("PhoneDevice");
    case QBluetoothDeviceInfo::LANAccessDevice:
        return tr("LANAccessDevice");
    case QBluetoothDeviceInfo::AudioVideoDevice:
        return tr("AudioVideoDevice");
    case QBluetoothDeviceInfo::PeripheralDevice:
        return tr("PeripheralDevice");
    case QBluetoothDeviceInfo::WearableDevice:
        return tr("WearableDevice");
    case QBluetoothDeviceInfo::ImagingDevice:
        return tr("ImagingDevice");
    case QBluetoothDeviceInfo::ToyDevice:
        return tr("ToyDevice");
    case QBluetoothDeviceInfo::HealthDevice:
        return tr("HealthDevice");
    default:
        return tr("UncategorizedDevice");
    }
}

int MyBluetoothDeviceInfo::getRssi() const
{
    return mDevice.rssi();
}

int MyBluetoothDeviceInfo::getControllerState() const
{
    return mLowEnergyController->state();
}

QString MyBluetoothDeviceInfo::getControllerStateDescription() const
{
    switch (mLowEnergyController->state()) {
    case QLowEnergyController::UnconnectedState:
        return tr("unconnected");
    case QLowEnergyController::ConnectingState:
        return tr("connecting ...");
    case QLowEnergyController::ConnectedState:
        return tr("connected");
    case QLowEnergyController::DiscoveringState:
        return tr("discovering services ...");
    case QLowEnergyController::DiscoveredState:
        return tr("discovered all services");
    case QLowEnergyController::ClosingState:
        return tr("closing ...");
    case QLowEnergyController::AdvertisingState:
        return tr("advertising ...");
    default:
        return tr("unknown state");
    }
}


int MyBluetoothDeviceInfo::getDeviceType() const
{
    return mDeviceType;
}

void MyBluetoothDeviceInfo::setDeviceType(int newDeviceType)
{
    mDeviceType = newDeviceType;
    emit deviceChanged();
}

bool MyBluetoothDeviceInfo::getDeviceIsConnected() const
{
    return mConnected;
}

QString MyBluetoothDeviceInfo::getServiceUUIDs() const
{
    QStringList info;
    QList<QBluetoothUuid> uuidList = mDevice.serviceUuids();
    for (int i = 0; i < uuidList.size(); ++i) {
        QBluetoothUuid btUuid = uuidList.at(i);
        info.append(btUuid.toString());
    }
    return info.join(" | ");
}

QBluetoothDeviceInfo MyBluetoothDeviceInfo::getDevice()
{
    return mDevice;
}

void MyBluetoothDeviceInfo::setDevice(const QBluetoothDeviceInfo &dev)
{
    mDevice = dev;
    mDeviceType = 0;
    mLowEnergyController = new QLowEnergyController(mDevice);
    // CONNECTED ?
    connect(mLowEnergyController, SIGNAL(connected()),
            this, SLOT(onDeviceConnected()));
    connect(mLowEnergyController, SIGNAL(disconnected()),
            this, SLOT(onDeviceDisconnected()));
    // DISCOVERING SERVICES
    connect(mLowEnergyController, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(onServiceDiscovered(QBluetoothUuid)));
    connect(mLowEnergyController, SIGNAL(discoveryFinished()),
            this, SLOT(onServiceDiscoveryFinished()));
    // CONTROLLER STATE if you want more detailed info about state changes
    connect(mLowEnergyController, SIGNAL(stateChanged(QLowEnergyController::ControllerState)),
            this, SLOT(onControllerStateChanged(QLowEnergyController::ControllerState)));
    // CONTROLLER ERROR
    connect(mLowEnergyController, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(onControllerError(QLowEnergyController::Error)));

    emit deviceChanged();
}

QVariant MyBluetoothDeviceInfo::getServices()
{
    return QVariant::fromValue(mServices);
}

void MyBluetoothDeviceInfo::onServiceDiscovered(const QBluetoothUuid &serviceUuid)
{
    // check if we're only looking for expected services
    if(createServicesActive()) {
        if(!mExpectedServiceUuids.contains(uuidString(serviceUuid))) {
            qDebug() << "Service ignored: " << serviceUuid; // .toString()
            return;
        } else {
            qDebug() << "Service accepted: " << serviceUuid;
        }
    }
    QLowEnergyService *service = mLowEnergyController->createServiceObject(serviceUuid);
    if (!service) {
        qWarning() << "Cannot create QLowEnergyService for uuid " << serviceUuid;
        return;
    }
    MyBluetoothServiceInfo *serv = new MyBluetoothServiceInfo();
    serv->setService(service);
    mServices.append(serv);
    qDebug() << "Service Name: " << serv->getName() << " Type: " << serv->getType();
    qDebug() << "Service UUID: " << serv->getUuid();
    emit servicesUpdated();
}

QString MyBluetoothDeviceInfo::uuidString(const QBluetoothUuid & serviceUuid)
{
    bool success = false;
    quint16 result16 = serviceUuid.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = serviceUuid.toUInt32(&success);
    if (success) {
        return QStringLiteral("0x") + QString::number(result32, 16);
    }
    return serviceUuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

void MyBluetoothDeviceInfo::onServiceDiscoveryFinished()
{
    // setUpdate("Back\n(Service scan done!)");
    // force UI in case we didn't find anything
    if (mServices.isEmpty()) {
        emit servicesUpdated();
    }

    // reset maxRetriesError
    resetConnectionCounter();
    if(servicesDiscoveryActive()) {
        setServicesDiscoveryActive(false);
        qDebug() << "Services discovered #" << mServices.size();
    }
    if(createServicesActive()) {
        setCreateServicesActive(false);
        qDebug() << "expected services created: #" << mServices.size();
    }
}

void MyBluetoothDeviceInfo::onDeviceDisconnected()
{
    qDebug() << "Disconnected from device " << mDevice.name() << " | " << mDevice.address();
    mConnected = false;
    emit disconnected();
    emit deviceChanged();
}

void MyBluetoothDeviceInfo::onDeviceConnected()
{
    qDebug() << "connected to device";
    mConnected = true;
    setRetryConnectIsEnabled(false);
    // clear current data
    clearServices();
    //
    emit connected();
    emit deviceChanged();
    if(servicesDiscoveryActive()) {
        qDebug() << "now discovering services";
        mLowEnergyController->discoverServices();
        return;
    }
    if(createServicesActive()) {
        qDebug() << "now adding expected services";
        addExpectedServices();
    }
}

// Getting Connection Error also for:
// W QtBluetoothGatt: Unhandled error code on connectionStateChanged: 133
// sometimes error code on connectionStateChanged: 34 or 8
// see bug https://bugreports.qt.io/browse/QTBUG-31674
void MyBluetoothDeviceInfo::onControllerError(QLowEnergyController::Error error)
{
    QString errorString;
    errorString = mLowEnergyController->errorString();
    qWarning() << "QLowEnergyController Error: " << errorString << " | " << error;
    QString errorMessage;

    //case 8:
    // will be mapped to ConnectionError see Bug 31674
    // happens for me if switching device off
    //case 34:
    // happens for me if switching device off and on again before disconnect event happens
    int myError = error;
    if(myError == 8 || myError == 34) {
        error = QLowEnergyController::ConnectionError;
    }

    switch (error) {
    case QLowEnergyController::UnknownRemoteDeviceError:
        errorMessage = tr("The Bluetooth Device is unknown - perhaps scan Devices again.");
        qDebug() << "LE Controller: Bluetooth is OFF";
        // iOS lets the user go to settings and switch ON
        // Android user must do this manually
        break;
    case QLowEnergyController::NetworkError:
        errorMessage = tr("LE Controller Error: Writing or reading from the device resulted in an error.");
        qDebug() << "LE Controller Error:: r/w from device error";
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        errorMessage = tr("LE Controller Error: invalid Bluetooth Adapter");
        qDebug() << "LE Controller Error: InvalidBluetoothAdapterError";
        break;
    case QLowEnergyController::ConnectionError:
        errorMessage = tr("LE Controller Error: cannot connect to Device. Please try switch on/off, check Battery");
        if(retryConnectIsEnabled()) {
            // auto-reconnect only if getting this error from connect to device
            // can also be reported from other operations
            mConnectionErrorCounter++;
            emit retryCounterChanged();
            qDebug() << "LE Controller Error: ConnectionError - counter: " << mConnectionErrorCounter << " max: " << mConnectionErrorMaxRetries;
            if(mConnectionErrorCounter <= mConnectionErrorMaxRetries) {
                // must be watched from UI to restart connecting
                setRetryDeviceConnect(true);
                return;
            }
        }
        break;
    case QLowEnergyController::AdvertisingError:
        errorMessage = tr("LE Controller Error: start Advertising failed"),
                qDebug() << "LE Controller Error: AdvertisingError";
        break;
    case QLowEnergyController::UnknownError:
        errorMessage = tr("LE Controller Error: unknown error");
        qDebug() << "LE Controller Error: UnknownError";
        break;
    default:
        errorMessage = tr("LE Controller Error: unhandled error. Code: %1").arg(error);
        qWarning() << "LE Controller Error: unhandled error: " << error;
        break;
    }
    setServicesDiscoveryActive(false);
    setCreateServicesActive(false);
    emit controllerErrorReceived(errorMessage);
}

void MyBluetoothDeviceInfo::onControllerStateChanged(QLowEnergyController::ControllerState)
{
    // let the UI know about state changes
    emit deviceChanged();
}

void MyBluetoothDeviceInfo::clearServices()
{
    for (int i = 0; i < mServices.size(); ++i) {
        MyBluetoothServiceInfo* service = (MyBluetoothServiceInfo*)mServices.at(i);
        service->clearCharacteristics();
    }

    qDeleteAll(mServices);
    mServices.clear();
    emit servicesUpdated();
}

QList<QObject *> MyBluetoothDeviceInfo::getServicesAsList()
{
    return mServices;
}

bool MyBluetoothDeviceInfo::servicesDiscoveryActive()
{
    return mServicesDiscoveryActive;
}

void MyBluetoothDeviceInfo::setServicesDiscoveryActive(bool discoveryState)
{
    mServicesDiscoveryActive = discoveryState;
    emit servicesDiscoveryActiveChanged();
}

bool MyBluetoothDeviceInfo::createServicesActive()
{
    return mCreateServicesActive;
}

void MyBluetoothDeviceInfo::setCreateServicesActive(bool isActive)
{
    mCreateServicesActive = isActive;
    emit createServicesActiveChanged();
}

bool MyBluetoothDeviceInfo::retryDeviceConnect()
{
    return mRetryDeviceConnect;
}

void MyBluetoothDeviceInfo::setRetryDeviceConnect(bool retry)
{
    mRetryDeviceConnect = retry;
    emit retryDeviceConnectChanged();
}

bool MyBluetoothDeviceInfo::retryConnectIsEnabled()
{
    return mRetryConnectIsEnabled;
}

void MyBluetoothDeviceInfo::setRetryConnectIsEnabled(bool isEnabled)
{
    mRetryConnectIsEnabled = isEnabled;
    emit retryConnectIsEnabled();
}

int MyBluetoothDeviceInfo::retryCounter()
{
    return mConnectionErrorCounter;
}

void MyBluetoothDeviceInfo::setRetryCounter(int counter)
{
    mConnectionErrorCounter = counter;
    emit retryCounterChanged();
}

void MyBluetoothDeviceInfo::connectToDevice()
{
    resetConnectionCounter();
    setRetryConnectIsEnabled(true);
    mLowEnergyController->connectToDevice();
}
void MyBluetoothDeviceInfo::connectToDeviceAgain()
{
    mLowEnergyController->connectToDevice();
}

void MyBluetoothDeviceInfo::disconnectFromDevice()
{
    mLowEnergyController->disconnectFromDevice();
}

void MyBluetoothDeviceInfo::resetConnectionCounter()
{
    mConnectionErrorCounter = 0;
    emit retryCounterChanged();
    setRetryDeviceConnect(false);
}


// Note: Some platforms internally cache the service list of a device which was discovered in the past.
// This can be problematic if the remote device changed its list of services or their inclusion tree.
// If this behavior is a problem, the best workaround is to temporarily turn Bluetooth off.
// his causes a reset of the cache data.
// Currently Android exhibits such a cache behavior.
void MyBluetoothDeviceInfo::discoverServices()
{
    if (!isValid()) {
        qWarning() << "Not a valid device: " << getAddress();
        return;
    }

    if (!mLowEnergyController) {
        qWarning() << "Device contains no Controller: " << getAddress();
        return;
    }

    if(mLowEnergyController->state() == QLowEnergyController::DiscoveredState) {
        qDebug() << "Already discovered - do nothing";
        return;
    }
    qDebug() << "scanning Services for " << getAddress();

    clearServices();

    // randomaddress not handled
    mLowEnergyController->setRemoteAddressType(QLowEnergyController::PublicAddress);
    // discover services
    setServicesDiscoveryActive(true);
    setRetryCounter(0);
    // if ConnectionError because of GATT error 133, wo do retry discover
    if(mLowEnergyController->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << "LE Controller: now connect to device ...";
        connectToDevice();
    } else if(mLowEnergyController->state() == QLowEnergyController::ConnectedState){
        qDebug() << "LE Controller: allready connected - now discover services ...";
        mLowEnergyController->discoverServices();
    } else {
        qDebug() << "LE Controller: wrong state: " << mLowEnergyController->state();
        // TODO INFO UI
        // try disconnect
        disconnectFromDevice();
        setServicesDiscoveryActive(false);
    }

}

void MyBluetoothDeviceInfo::setExpectedServiceUuids(QStringList &serviceUuids)
{
    mExpectedServiceUuids = serviceUuids;
}

// while discoverServices() asks the device for ALL services
// this method creates specific services for known devices
// usually you have checked if these services are available using discoverServices()
void MyBluetoothDeviceInfo::createExpectedServices()
{
    if (!isValid()) {
        qWarning() << "Not a valid device: " << getAddress();
        return;
    }

    if (!mLowEnergyController) {
        qWarning() << "Device contains no Controller: " << getAddress();
        return;
    }

    if(mLowEnergyController->state() == QLowEnergyController::DiscoveredState) {
        qDebug() << "Already discovered - do nothing";
        return;
    }
    if(mExpectedServiceUuids.size() == 0) {
        qDebug() << "List of expected servcice UUIDs is empty - do nothing";
        return;
    }
    qDebug() << "create Expected Services for " << getAddress();

    setCreateServicesActive(true);
    // randomaddress not handled
    mLowEnergyController->setRemoteAddressType(QLowEnergyController::PublicAddress);
    setRetryCounter(0);
    if(mLowEnergyController->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << "LE Controller: now connect to device ...";
        connectToDevice();
        return;
    }

    addExpectedServices();
}

// we are connected to the device
// and only want to add expected services
void MyBluetoothDeviceInfo::addExpectedServices()
{
    clearServices();

    if(mLowEnergyController->state() == QLowEnergyController::ConnectedState){
        qDebug() << "LE Controller: allready connected - now discover services ...";
        mLowEnergyController->discoverServices();
    } else {
        qDebug() << "LE Controller: wrong state: " << mLowEnergyController->state();
        // TODO INFO UI
        // try disconnect
        disconnectFromDevice();
        setServicesDiscoveryActive(false);
    }
}
