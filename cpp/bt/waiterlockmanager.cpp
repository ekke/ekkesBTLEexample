#include "waiterlockmanager.hpp"

static const QString ADDIMAT_KEY_SERVICE = "7b83ed4b-c859-47af-b59c-d57461009c81";
static const QString ADDIMAT_KEY_CHARACTERISTIC = "4189e016-6cca-48ab-a510-4d3b4895a788";

static const QString BATTERY_SERVICE = "0x180f";
static const QString BATTERY_LEVEL = "0x2a19";

static const QString NO_KEY = "0000000000000000";

WaiterLockManager::WaiterLockManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false), mBatteryServiceAvailable(false),
    mBatteryServiceConnected(false), mBatteryLevelAvailable(false), mBatteryLevelValue(-1), mKeyServiceAvailable(false), mKeyServiceConnected(false),
    mKeyIdAvailable(false), mFeaturesPrepared(false), mKeyNotificationsActive(false), mHasDevice(false), mSettingsBatteryLevelInfo(70), mSettingsBatteryLevelWarning(30)
{
    mTotterTimer = new QTimer(this);
    mTotterTimer->setInterval(2000);
    mTotterTimer->setSingleShot(true);
    connect(mTotterTimer, &QTimer::timeout, this, &WaiterLockManager::onTottleTimedOut);
}

void WaiterLockManager::init(BluetoothManager *bluetoothManager)
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

void WaiterLockManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("addimat", mWaiterLockSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

int WaiterLockManager::getBatteryLevelValue() const
{
    return mBatteryLevelValue;
}

QString WaiterLockManager::getKeyIdValue() const
{
    return mKeyIdValue;
}

bool WaiterLockManager::getFeaturesPrepared() const
{
    return mFeaturesPrepared;
}

void WaiterLockManager::setFeaturesPrepared(bool isPrepared)
{
    if(mFeaturesPrepared != isPrepared) {
        mFeaturesPrepared = isPrepared;
        emit featuresPreparedChanged();
    }
}

bool WaiterLockManager::getKeyNotificationsActive() const
{
    return mKeyNotificationsActive;
}

void WaiterLockManager::setKeyNotificationsActive(bool isActive)
{
    if(mKeyNotificationsActive != isActive) {
        mKeyNotificationsActive = isActive;
        emit keyNotificationsActiveChanged();
    }
}

bool WaiterLockManager::getHasDevice() const
{
    return mHasDevice;
}

QString WaiterLockManager::getSettingsFavoriteAddress() const
{
    return mSettingsFavoriteAddress;
}

void WaiterLockManager::setSettingsFavoriteAddress(QString address)
{
    if(mSettingsFavoriteAddress != address) {
        mSettingsFavoriteAddress = address;
        mWaiterLockSettingsMap.insert("settingsFavoriteAddress", address);
        updateSettings();
        emit settingsChanged();
    }
}

int WaiterLockManager::getSettingsBatteryLevelInfo() const
{
    return mSettingsBatteryLevelInfo;
}

int WaiterLockManager::getSettingsBatteryLevelWarning() const
{
    return mSettingsBatteryLevelWarning;
}

QVariant WaiterLockManager::getMappings()
{
    return QVariant::fromValue(mMappings);
}

void WaiterLockManager::clearMappings()
{
    qDeleteAll(mMappings);
    mMappings.clear();
    emit mappingsUpdated();
    updateSettings();
}

AddimatMapping *WaiterLockManager::createOrGetMapping(QString addimatKey)
{
    for (int i = 0; i < mMappings.size(); ++i) {
        AddimatMapping* currentAddimatMapping = (AddimatMapping*)mMappings.at(i);
        if(currentAddimatMapping->getAddimatKey() == addimatKey) {
            return currentAddimatMapping;
        }
    }
    // create new
    AddimatMapping* addimatMapping;
    addimatMapping = new AddimatMapping(this);
    addimatMapping->setAddimatKey(addimatKey);
    addimatMapping->setUserName(tr("??"));
    mMappings.append(addimatMapping);
    triggerMappingsDataUpdated();
    return addimatMapping;
}

void WaiterLockManager::triggerMappingsDataUpdated()
{
    emit mappingsUpdated();
    mWaiterLockSettingsMap.insert("mappingsList", mappingsToVariantList());
    updateSettings();
}

bool WaiterLockManager::removeMapping(QString addimatKey)
{
    for (int i = 0; i < mMappings.size(); ++i) {
        AddimatMapping* addimatMapping = (AddimatMapping*)mMappings.at(i);
        if(addimatMapping->getAddimatKey() == addimatKey) {
            mMappings.removeAt(i);
            addimatMapping = nullptr;
            triggerMappingsDataUpdated();
            return true;
        }
    }
    return false;
}

AddimatMapping *WaiterLockManager::findFromMapping(QString addimatKey)
{
    for (int i = 0; i < mMappings.size(); ++i) {
        AddimatMapping* addimatMapping = (AddimatMapping*)mMappings.at(i);
        if(addimatMapping->getAddimatKey() == addimatKey) {
            return addimatMapping;
        }
    }
    return nullptr;
}

void WaiterLockManager::setBatteryValues(int info, int warning)
{
    mSettingsBatteryLevelInfo = info;
    mSettingsBatteryLevelWarning = warning;
    mWaiterLockSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
    mWaiterLockSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
    updateSettings();
}

void WaiterLockManager::setCurrentDevice(MyBluetoothDeviceInfo *myDevice)
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

MyBluetoothDeviceInfo *WaiterLockManager::currentDevice()
{
    return mDeviceInfo;
}

bool WaiterLockManager::isCurrentDeviceConnected()
{
    if(!mDeviceInfo) {
        return false;
    }
    return mDeviceInfo->getDeviceIsConnected();
}

void WaiterLockManager::prepareServices()
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

void WaiterLockManager::updateBatteryLevel()
{
    if(mBatteryLevel) {
        mBatteryService->readCharacteristic(mBatteryLevel);
    }
}

void WaiterLockManager::updateKeyValue()
{
    if(mKeyId) {
        mKeyService->readCharacteristic(mKeyId);
    }
}

void WaiterLockManager::startKeyNotifications()
{
    if(mKeyService && mKeyId) {
        mKeyService->subscribeNotifications(mKeyId);
    }
}

void WaiterLockManager::stopKeyNotifications()
{
    if(mKeyService && mKeyId) {
        mKeyService->unSubscribeNotifications(mKeyId);
    }
}

MyBluetoothCharacteristic *WaiterLockManager::getBatteryLevel()
{
    return mBatteryLevel;
}

void WaiterLockManager::onBatteryCharacteristicsDone()
{
    qDebug() << "onBatteryCharacteristicsDone - get " << BATTERY_LEVEL;
    mBatteryLevel = mBatteryService->getCharacteristicInfo(BATTERY_LEVEL);
    if(mBatteryLevel) {
        connect(mBatteryLevel, &MyBluetoothCharacteristic::currentValueChanged, this, &WaiterLockManager::onBatteryLevelChanged);
        mBatteryLevelAvailable = true;
        checkIfAllPrepared();
        // read current value
        updateBatteryLevel();
    }
}

void WaiterLockManager::onKeyCharacteristicsDone()
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
void WaiterLockManager::onDisconnect()
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

void WaiterLockManager::onBatteryLevelChanged()
{
    QByteArray valueArray = mBatteryLevel->getValue();
    QString hexValue = valueArray.toHex();
    bool ok;
    int intValue = hexValue.toInt(&ok, 16);
    if(ok) {
        if(mBatteryLevelValue != intValue) {
            mBatteryLevelValue = intValue;
            emit batteryLevelValueChanged();
        }
    } else {
        qWarning() << "Battery Level: Value couldn't be converted to int";
    }

}

void WaiterLockManager::onKeyIdChanged()
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

void WaiterLockManager::onTottleTimedOut()
{
    mKeyIdValue = NO_KEY;
    qDebug() << "back from timer - the KEY ID:" << mKeyIdValue;
    emit keyIdValueChanged();
}

QVariantList WaiterLockManager::mappingsToVariantList()
{
    QVariantList theList;
    for (int i = 0; i < mMappings.size(); ++i) {
        AddimatMapping* addimatMapping = (AddimatMapping*) mMappings.at(i);
        theList.append(addimatMapping->toMap());
    }
    qDebug() << "Mappings as VariantList: " << theList.size();
    return theList;
}

void WaiterLockManager::mappingsFromVariantList(QVariantList &theList)
{
    clearMappings();
    for (int i = 0; i < theList.size(); ++i) {
        AddimatMapping* addimatMapping = new AddimatMapping(this);
        addimatMapping->fromMap(theList.at(i).toMap());
        mMappings.append(addimatMapping);
    }
}

void WaiterLockManager::onKeySubscriptionsChanged()
{
    bool isRunning = mKeyId->getNotifyIsRunning();
    if(isRunning != mKeyNotificationsActive) {
        setKeyNotificationsActive(isRunning);
    }
}

void WaiterLockManager::checkIfAllPrepared()
{
    if(mBatteryLevelAvailable && mKeyIdAvailable && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}


