#include "generalscanmanager.hpp"

static const QString BARCODE_SCAN_SERVICE = "fff0";
static const QString BARCODE_SCAN_CHARACTERISTIC = "fff1";

GeneralScanManager::GeneralScanManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false),
    mScanServiceAvailable(false), mScanServiceConnected(false),
    mBarcodeAvailable(false), mFeaturesPrepared(false), mScanNotificationsActive(false), mHasDevice(false)
{

}

void GeneralScanManager::init(BluetoothManager *bluetoothManager)
{
    mBluetoothManager = bluetoothManager;
    // get settings
    mWaiterLockSettingsMap = mBluetoothManager->mBluetoothSettingsMap.value("addimat").toMap();
    if(mWaiterLockSettingsMap.isEmpty()) {
        mWaiterLockSettingsMap.insert("settingsFavoriteAddress", mSettingsFavoriteAddress);
        mWaiterLockSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
        mWaiterLockSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
        mWaiterLockSettingsMap.insert("mappingsList", mappingsToVariantList());
        mBluetoothManager->mBluetoothSettingsMap.insert("addimat", mWaiterLockSettingsMap);
        mBluetoothManager->cacheSettings();
        qDebug() << "WaiterLock Settings created | mappings:" << mMappings.size();
    } else {
        mSettingsFavoriteAddress = mWaiterLockSettingsMap.value("settingsFavoriteAddress").toString();
        mSettingsBatteryLevelInfo = mWaiterLockSettingsMap.value("settingsBatteryLevelInfo").toInt();
        mSettingsBatteryLevelWarning = mWaiterLockSettingsMap.value("settingsBatteryLevelWarning").toInt();
        QVariantList vl = mWaiterLockSettingsMap.value("mappingsList").toList();
        mappingsFromVariantList(vl);
        qDebug() << "WaiterLock Settings read | mappings:" << mMappings.size();
    }
}

void GeneralScanManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("addimat", mWaiterLockSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

QString GeneralScanManager::getKeyIdValue() const
{
    return mKeyIdValue;
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

bool GeneralScanManager::getKeyNotificationsActive() const
{
    return mKeyNotificationsActive;
}

void GeneralScanManager::setKeyNotificationsActive(bool isActive)
{
    if(mKeyNotificationsActive != isActive) {
        mKeyNotificationsActive = isActive;
        emit keyNotificationsActiveChanged();
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
        mWaiterLockSettingsMap.insert("settingsFavoriteAddress", address);
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
        qDebug() << "WaiterLock Manager: current Device " << myDevice->getAddress();
        // remember address
        setSettingsFavoriteAddress(myDevice->getAddress());
        // set expected service uuids
        QStringList sl;
        sl.append(ADDIMAT_KEY_SERVICE);
        sl.append(BATTERY_SERVICE);
        myDevice->setExpectedServiceUuids(sl);
        //
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &WaiterLockManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
    } else {
        mDeviceInfo = nullptr;
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setKeyNotificationsActive(false);
        mCurrentKey.clear();
        mKeyIdValue.clear();
        emit keyIdValueChanged();
        mBatteryLevelValue = -1;
        emit batteryLevelValueChanged();
        if(mHasDevice) {
            mHasDevice = false;
            emit hasDeviceChanged();
        }
        qDebug() << "WaiterLock Manager: current Device REMOVED";
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
    qDebug() << "Addimat: prepareServices and start features";
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
    mBatteryServiceAvailable = false;
    mBatteryServiceConnected = false;
    mBatteryLevelAvailable = false;

    mKeyServiceAvailable = false;
    mKeyServiceConnected = false;
    mKeyIdAvailable = false;

    qDebug() << "services #" << myServices.size();
    for (int i = 0; i < myServices.size(); ++i) {
        MyBluetoothServiceInfo* myService = (MyBluetoothServiceInfo*)myServices.at(i);
        if(myService->getUuid() == BATTERY_SERVICE) {
            mBatteryService = myService;
            connect(mBatteryService, &MyBluetoothServiceInfo::characteristicsDone, this, &WaiterLockManager::onBatteryCharacteristicsDone);
            mBatteryServiceAvailable = true;
        } else if(myService->getUuid() == ADDIMAT_KEY_SERVICE) {
            mKeyService = myService;
            connect(mKeyService, &MyBluetoothServiceInfo::characteristicsDone, this, &WaiterLockManager::onKeyCharacteristicsDone);
            mKeyServiceAvailable = true;
        }
        qDebug() << "SERVICE UUID [" << myService->getUuid() << "]";
    }
    if(mBatteryServiceAvailable) {
        qDebug() << "Addimat Battery Service found";
        if(mBatteryService->hasCharacteristics()) {
            onBatteryCharacteristicsDone();
        } else {
            mBatteryService->connectToService();
        }
    }
    if(mKeyServiceAvailable) {
        qDebug() << "Addimat Key Service available";
        if(mKeyService->hasCharacteristics()) {
            onKeyCharacteristicsDone();
        } else {
            mKeyService->connectToService();
        }
    }
}

void GeneralScanManager::updateKeyValue()
{
    if(mKeyId) {
        mKeyService->readCharacteristic(mKeyId);
    }
}

void GeneralScanManager::startKeyNotifications()
{
    if(mKeyService && mKeyId) {
        mKeyService->subscribeNotifications(mKeyId);
    }
}

void GeneralScanManager::stopKeyNotifications()
{
    if(mKeyService && mKeyId) {
        mKeyService->unSubscribeNotifications(mKeyId);
    }
}

void GeneralScanManager::onKeyCharacteristicsDone()
{
    qDebug() << "onKeyCharacteristicsDone - get " << ADDIMAT_KEY_CHARACTERISTIC;
    mKeyId = mKeyService->getCharacteristicInfo(ADDIMAT_KEY_CHARACTERISTIC);
    if(mKeyId) {
        connect(mKeyId, &MyBluetoothCharacteristic::currentValueChanged, this, &WaiterLockManager::onKeyIdChanged);
        connect(mKeyId, &MyBluetoothCharacteristic::characteristicChanged, this, &WaiterLockManager::onKeySubscriptionsChanged);
        mKeyIdAvailable = true;
        checkIfAllPrepared();
        // don't read current key - the lock remains the last connected key
    }

}

// SLOT from SIGNAL deviceChanged
void GeneralScanManager::onDisconnect()
{
    qDebug() << "WaiterLockManager deviceChanged - onDisconnect";
    // we're only interested into unconnect
    if(!mDeviceInfo || !mDeviceInfo->getDeviceIsConnected()) {
        qDebug() << "WaiterLockManager onDisconnect";
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setKeyNotificationsActive(false);
    }
}

void GeneralScanManager::onKeyIdChanged()
{
    mTotterTimer->stop();
    QByteArray valueArray = mKeyId->getCurrentValue();
    QString hexValue = valueArray.toHex();
    if(mKeyIdValue == hexValue) {
        qDebug() << "same key while tottle timer is running";
        return;
    }
    if(mKeyIdValue.length()> 0 && hexValue == NO_KEY) {
        mCurrentKey = mKeyIdValue;
        mTotterTimer->start();
        return;
    }
    mKeyIdValue = hexValue;
    qDebug() << "another KEY ID:" << mKeyIdValue;
    emit keyIdValueChanged();
}

void GeneralScanManager::onKeySubscriptionsChanged()
{
    bool isRunning = mKeyId->getNotifyIsRunning();
    if(isRunning != mKeyNotificationsActive) {
        setKeyNotificationsActive(isRunning);
    }
}

void GeneralScanManager::checkIfAllPrepared()
{
    if(mBatteryLevelAvailable && mKeyIdAvailable && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}
