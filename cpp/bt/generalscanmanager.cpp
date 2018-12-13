#include "generalscanmanager.hpp"

static const QString BARCODE_SCAN_SERVICE = "0000fff0-0000-1000-8000-00805f9b34fb";
static const QString BARCODE_SCAN_SERVICE_SHORT = "0xfff0";
static const QString BARCODE_SCAN_CHARACTERISTIC = "0000fff1-0000-1000-8000-00805f9b34fb";
static const QString BARCODE_SCAN_CHARACTERISTIC_SHORT = "0xfff1";

GeneralScanManager::GeneralScanManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false),
    mScanServiceAvailable(false), mScanServiceConnected(false),
    mBarcodeAvailable(false), mFeaturesPrepared(false), mScanNotificationsActive(false), mHasDevice(false)
{

}

void GeneralScanManager::init(BluetoothManager *bluetoothManager)
{
    mBluetoothManager = bluetoothManager;
    // get settings
    mGeneralScanSettingsMap = mBluetoothManager->mBluetoothSettingsMap.value("generalscan").toMap();
    if(mGeneralScanSettingsMap.isEmpty()) {
        mGeneralScanSettingsMap.insert("settingsFavoriteAddress", mSettingsFavoriteAddress);
        mBluetoothManager->mBluetoothSettingsMap.insert("generalscan", mGeneralScanSettingsMap);
        mBluetoothManager->cacheSettings();
        qDebug() << "GeneralScan Settings created";
    } else {
        mSettingsFavoriteAddress = mGeneralScanSettingsMap.value("settingsFavoriteAddress").toString();
        qDebug() << "GeneralScan Settings read:";
    }
}

void GeneralScanManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("generalscan", mGeneralScanSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

QString GeneralScanManager::getBarcodeValue() const
{
    return mBarcodeValue;
}

bool GeneralScanManager::getFeaturesPrepared() const
{
    return mFeaturesPrepared;
}

void GeneralScanManager::setFeaturesPrepared(bool isPrepared)
{
    if(mFeaturesPrepared != isPrepared) {
        mFeaturesPrepared = isPrepared;
        emit featuresPreparedChanged();
    }
}

bool GeneralScanManager::getScanNotificationsActive() const
{
    return mScanNotificationsActive;
}

void GeneralScanManager::setScanNotificationsActive(bool isActive)
{
    if(mScanNotificationsActive != isActive) {
        mScanNotificationsActive = isActive;
        emit scanNotificationsActiveChanged();
    }
}

bool GeneralScanManager::getHasDevice() const
{
    return mHasDevice;
}

QString GeneralScanManager::getSettingsFavoriteAddress() const
{
    return mSettingsFavoriteAddress;
}

void GeneralScanManager::setSettingsFavoriteAddress(QString address)
{
    if(mSettingsFavoriteAddress != address) {
        mSettingsFavoriteAddress = address;
        mGeneralScanSettingsMap.insert("settingsFavoriteAddress", address);
        updateSettings();
        emit settingsChanged();
    }
}

void GeneralScanManager::setCurrentDevice(MyBluetoothDeviceInfo *myDevice)
{
    if(myDevice) {
        bool deviceAddressChanged = false;
        if(mDeviceInfo && mDeviceInfo->getAddress() != myDevice->getAddress()) {
            deviceAddressChanged = true;
        }
        mDeviceInfo = myDevice;
        qDebug() << "GeneralScan Manager: current Device " << myDevice->getAddress();
        // remember address
        setSettingsFavoriteAddress(myDevice->getAddress());
        // set expected service uuids
        QStringList sl;
        sl.append(BARCODE_SCAN_SERVICE);
        sl.append(BARCODE_SCAN_SERVICE_SHORT);
        myDevice->setExpectedServiceUuids(sl);
        //
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &GeneralScanManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
    } else {
        mDeviceInfo = nullptr;
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setScanNotificationsActive(false);
        // mCurrentKey.clear();
        mBarcodeValue.clear();
        emit barcodeValueChanged();
        if(mHasDevice) {
            mHasDevice = false;
            emit hasDeviceChanged();
        }
        qDebug() << "GeneralScan Manager: current Device REMOVED";
    }
}

MyBluetoothDeviceInfo *GeneralScanManager::currentDevice()
{
    return mDeviceInfo;
}

bool GeneralScanManager::isCurrentDeviceConnected()
{
    if(!mDeviceInfo) {
        return false;
    }
    return mDeviceInfo->getDeviceIsConnected();
}

void GeneralScanManager::prepareServices()
{
    qDebug() << "GeneralScan: prepareServices and start features";
    if(!mDeviceInfo) {
        qWarning() << "no Device Info";
        setFeaturesPrepared(false);
        return;
    }
    if(!mDeviceInfo->controller()) {
        qWarning() << "no QLowEnergyController";
        setFeaturesPrepared(false);
        return;
    }
    if(!mDeviceInfo->getDeviceIsConnected()){
        qDebug() << "device not connected";
        setFeaturesPrepared(false);
        return;
    }
    mDeviceIsConnected = true;
    // services can be created by:
    // * scanner: discovering all services
    // * creating expected services for known devices
    QList<QObject*> myServices = mDeviceInfo->getServicesAsList();
    if(myServices.size() == 0 &&  mDeviceInfo->controller()->state() != QLowEnergyController::DiscoveredState) {
        qDebug() << "device wrong state " << mDeviceInfo->controller()->state();
        setFeaturesPrepared(false);
        return;
    }

    mScanServiceAvailable = false;
    mScanServiceConnected = false;
    mBarcodeAvailable = false;

    qDebug() << "services #" << myServices.size();
    for (int i = 0; i < myServices.size(); ++i) {
        MyBluetoothServiceInfo* myService = (MyBluetoothServiceInfo*)myServices.at(i);
        if(myService->getUuid() == BARCODE_SCAN_SERVICE || myService->getUuid() == BARCODE_SCAN_SERVICE_SHORT) {
            qDebug() << "BARCODE_SCAN_SERVICE detected";
            mScanService = myService;
            connect(mScanService, &MyBluetoothServiceInfo::characteristicsDone, this, &GeneralScanManager::onScanCharacteristicsDone);
            mScanServiceAvailable = true;
        }
        qDebug() << "SERVICE UUID [" << myService->getUuid() << "]";
    }
    if(mScanServiceAvailable) {
        qDebug() << "GeneralScan Barcode Service available";
        if(mScanService->hasCharacteristics()) {
            onScanCharacteristicsDone();
        } else {
            mScanService->connectToService();
        }
    }
}

void GeneralScanManager::updateScanValue()
{
    if(mBarcode) {
        mScanService->readCharacteristic(mBarcode);
    }
}

void GeneralScanManager::startScanNotifications()
{
    if(mScanService && mBarcode) {
        mScanService->subscribeNotifications(mBarcode);
    }
}

void GeneralScanManager::stopScanNotifications()
{
    if(mScanService && mBarcode) {
        mScanService->unSubscribeNotifications(mBarcode);
    }
}

void GeneralScanManager::onScanCharacteristicsDone()
{
    qDebug() << "onScanCharacteristicsDone - get " << BARCODE_SCAN_CHARACTERISTIC;
    mBarcode = mScanService->getCharacteristicInfo(BARCODE_SCAN_CHARACTERISTIC);
    if(!mBarcode) {
        qDebug() << "onScanCharacteristicsDone - get " << BARCODE_SCAN_CHARACTERISTIC_SHORT;
        mBarcode = mScanService->getCharacteristicInfo(BARCODE_SCAN_CHARACTERISTIC_SHORT);
    }
    if(mBarcode) {
        connect(mBarcode, &MyBluetoothCharacteristic::currentValueChanged, this, &GeneralScanManager::onBarcodeChanged);
        connect(mBarcode, &MyBluetoothCharacteristic::characteristicChanged, this, &GeneralScanManager::onScanSubscriptionsChanged);
        mBarcodeAvailable = true;
        checkIfAllPrepared();
        // TODO ??????  don't read current key - the lock remains the last connected key
    } else {
        qWarning() << "cannot create mBarcode from " << BARCODE_SCAN_CHARACTERISTIC << "or " << BARCODE_SCAN_CHARACTERISTIC_SHORT;
    }
}

// SLOT from SIGNAL deviceChanged
void GeneralScanManager::onDisconnect()
{
    qDebug() << "GeneralScanManager deviceChanged - onDisconnect";
    // we're only interested into unconnect
    if(!mDeviceInfo || !mDeviceInfo->getDeviceIsConnected()) {
        qDebug() << "GeneralScanManager onDisconnect";
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setScanNotificationsActive(false);
    }
}

void GeneralScanManager::onBarcodeChanged()
{
    QByteArray valueArray = mBarcode->getCurrentValue();
    QString hexValue = valueArray.toHex();
//    if(mBarcodeValue == hexValue) {
//        qDebug() << "same key while tottle timer is running";
//        return;
//    }
//    if(mBarcodeValue.length()> 0 && hexValue == NO_KEY) {
//        mCurrentKey = mKeyIdValue;
//        mTotterTimer->start();
//        return;
//    }
    mBarcodeValue = hexValue;
    qDebug() << "it is a BARCODE:" << mBarcodeValue;
    emit barcodeValueChanged();
}

void GeneralScanManager::onScanSubscriptionsChanged()
{
    bool isRunning = mBarcode->getNotifyIsRunning();
    if(isRunning != mScanNotificationsActive) {
        setScanNotificationsActive(isRunning);
    }
}

void GeneralScanManager::checkIfAllPrepared()
{
    if(mBarcodeAvailable && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}
