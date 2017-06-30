#include "nfcreadermanager.hpp"

static const QString NFC_TAG_SERVICE = "0003cdd0-0000-1000-8000-00805f9b0131";
static const QString NFC_TAG_CHARACTERISTIC = "0003cdd1-0000-1000-8000-00805f9b0131";
static const QString NFC_WRITE_CHARACTERISTIC = "0003cdd2-0000-1000-8000-00805f9b0131";

static const QString BATTERY_SERVICE = "0x180f";
static const QString BATTERY_LEVEL = "0x2a19";

static const QString NO_ID = "\r";

NfcReaderManager::NfcReaderManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false), mBatteryServiceAvailable(false),
    mBatteryServiceConnected(false), mBatteryLevelAvailable(false), mBatteryLevelValue(-1), mTagServiceAvailable(false), mTagServiceConnected(false),
    mTagIdAvailable(false), mFeaturesPrepared(false), mTagNotificationsActive(false), mHasDevice(false), mSettingsBatteryLevelInfo(70), mSettingsBatteryLevelWarning(30)
{
}

void NfcReaderManager::init(BluetoothManager *bluetoothManager)
{
    mBluetoothManager = bluetoothManager;
    // get settings
    mNfcReaderSettingsMap = mBluetoothManager->mBluetoothSettingsMap.value("nfc").toMap();
    if(mNfcReaderSettingsMap.isEmpty()) {
        mNfcReaderSettingsMap.insert("settingsFavoriteAddress", mSettingsFavoriteAddress);
        mNfcReaderSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
        mNfcReaderSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
        mNfcReaderSettingsMap.insert("mappingsList", mappingsToVariantList());
        mBluetoothManager->mBluetoothSettingsMap.insert("nfc", mNfcReaderSettingsMap);
        mBluetoothManager->cacheSettings();
        qDebug() << "NFC Reader Settings created | mappings:" << mMappings.size();
    } else {
        mSettingsFavoriteAddress = mNfcReaderSettingsMap.value("settingsFavoriteAddress").toString();
        mSettingsBatteryLevelInfo = mNfcReaderSettingsMap.value("settingsBatteryLevelInfo").toInt();
        mSettingsBatteryLevelWarning = mNfcReaderSettingsMap.value("settingsBatteryLevelWarning").toInt();
        QVariantList vl = mNfcReaderSettingsMap.value("mappingsList").toList();
        mappingsFromVariantList(vl);
        qDebug() << "NFC reader Settings read | mappings:" << mMappings.size();
    }
}

void NfcReaderManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("nfc", mNfcReaderSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

int NfcReaderManager::getBatteryLevelValue() const
{
    return mBatteryLevelValue;
}

QString NfcReaderManager::getTagIdValue() const
{
    return mTagIdValue;
}

bool NfcReaderManager::getFeaturesPrepared() const
{
    return mFeaturesPrepared;
}

void NfcReaderManager::setFeaturesPrepared(bool isPrepared)
{
    if(mFeaturesPrepared != isPrepared) {
        mFeaturesPrepared = isPrepared;
        emit featuresPreparedChanged();
    }
}

bool NfcReaderManager::getTagNotificationsActive() const
{
    return mTagNotificationsActive;
}

void NfcReaderManager::setTagNotificationsActive(bool isActive)
{
    if(mTagNotificationsActive != isActive) {
        mTagNotificationsActive = isActive;
        emit tagNotificationsActiveChanged();
    }
}

bool NfcReaderManager::getHasDevice() const
{
    return mHasDevice;
}

QString NfcReaderManager::getSettingsFavoriteAddress() const
{
    return mSettingsFavoriteAddress;
}

void NfcReaderManager::setSettingsFavoriteAddress(QString address)
{
    if(mSettingsFavoriteAddress != address) {
        mSettingsFavoriteAddress = address;
        mNfcReaderSettingsMap.insert("settingsFavoriteAddress", address);
        updateSettings();
        emit settingsChanged();
    }
}

int NfcReaderManager::getSettingsBatteryLevelInfo() const
{
    return mSettingsBatteryLevelInfo;
}

int NfcReaderManager::getSettingsBatteryLevelWarning() const
{
    return mSettingsBatteryLevelWarning;
}

QVariant NfcReaderManager::getMappings()
{
    return QVariant::fromValue(mMappings);
}

void NfcReaderManager::clearMappings()
{
    qDeleteAll(mMappings);
    mMappings.clear();
    emit mappingsUpdated();
    updateSettings();
}

NfcReaderMapping *NfcReaderManager::createOrGetMapping(QString tagId)
{
    qDebug() << "createOrGetMapping for " << tagId;
    for (int i = 0; i < mMappings.size(); ++i) {
        NfcReaderMapping* currentNfcReaderMapping = (NfcReaderMapping*)mMappings.at(i);
        if(currentNfcReaderMapping->getTagId() == tagId) {
            return currentNfcReaderMapping;
        }
    }
    // create new
    NfcReaderMapping* nfcReaderMapping;
    nfcReaderMapping = new NfcReaderMapping(this);
    nfcReaderMapping->setTagId(tagId);
    nfcReaderMapping->setUserName(tr("??"));
    mMappings.append(nfcReaderMapping);
    triggerMappingsDataUpdated();
    return nfcReaderMapping;
}

void NfcReaderManager::triggerMappingsDataUpdated()
{
    emit mappingsUpdated();
    mNfcReaderSettingsMap.insert("mappingsList", mappingsToVariantList());
    updateSettings();
}

bool NfcReaderManager::removeMapping(QString tagId)
{
    for (int i = 0; i < mMappings.size(); ++i) {
        NfcReaderMapping* nfcReaderMapping = (NfcReaderMapping*)mMappings.at(i);
        if(nfcReaderMapping->getTagId() == tagId) {
            mMappings.removeAt(i);
            nfcReaderMapping = nullptr;
            triggerMappingsDataUpdated();
            return true;
        }
    }
    return false;
}

NfcReaderMapping *NfcReaderManager::findFromMapping(QString tagId)
{
    for (int i = 0; i < mMappings.size(); ++i) {
        NfcReaderMapping* nfcReaderMapping = (NfcReaderMapping*)mMappings.at(i);
        if(nfcReaderMapping->getTagId() == tagId) {
            return nfcReaderMapping;
        }
    }
    return nullptr;
}

void NfcReaderManager::setBatteryValues(int info, int warning)
{
    mSettingsBatteryLevelInfo = info;
    mSettingsBatteryLevelWarning = warning;
    mNfcReaderSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
    mNfcReaderSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
    updateSettings();
}

void NfcReaderManager::setCurrentDevice(MyBluetoothDeviceInfo *myDevice)
{
    if(myDevice) {
        bool deviceAddressChanged = false;
        if(mDeviceInfo && mDeviceInfo->getAddress() != myDevice->getAddress()) {
            deviceAddressChanged = true;
        }
        mDeviceInfo = myDevice;
        qDebug() << "NFC Reader Manager: current Device " << myDevice->getAddress();
        // remember address
        setSettingsFavoriteAddress(myDevice->getAddress());
        // set expected service uuids
        QStringList sl;
        sl.append(NFC_TAG_SERVICE);
        sl.append(BATTERY_SERVICE);
        myDevice->setExpectedServiceUuids(sl);
        //
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &NfcReaderManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
    } else {
        mDeviceInfo = nullptr;
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setTagNotificationsActive(false);
        mCurrentId.clear();
        mTagIdValue.clear();
        emit tagIdValueChanged();
        mBatteryLevelValue = -1;
        emit batteryLevelValueChanged();
        if(mHasDevice) {
            mHasDevice = false;
            emit hasDeviceChanged();
        }
        qDebug() << "NFC Reader Manager: current Device REMOVED";
    }
}

MyBluetoothDeviceInfo *NfcReaderManager::currentDevice()
{
    return mDeviceInfo;
}

bool NfcReaderManager::isCurrentDeviceConnected()
{
    if(!mDeviceInfo) {
        return false;
    }
    return mDeviceInfo->getDeviceIsConnected();
}

void NfcReaderManager::prepareServices()
{
    qDebug() << "NFC Reader: prepareServices and start features";
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

    mTagServiceAvailable = false;
    mTagServiceConnected = false;
    mTagIdAvailable = false;
    mWriteDataAvailable = false;

    qDebug() << "services #" << myServices.size();
    for (int i = 0; i < myServices.size(); ++i) {
        MyBluetoothServiceInfo* myService = (MyBluetoothServiceInfo*)myServices.at(i);
        if(myService->getUuid() == BATTERY_SERVICE) {
            mBatteryService = myService;
            connect(mBatteryService, &MyBluetoothServiceInfo::characteristicsDone, this, &NfcReaderManager::onBatteryCharacteristicsDone);
            mBatteryServiceAvailable = true;
        } else if(myService->getUuid() == NFC_TAG_SERVICE) {
            mTagService = myService;
            connect(mTagService, &MyBluetoothServiceInfo::characteristicsDone, this, &NfcReaderManager::onTagCharacteristicsDone);
            mTagServiceAvailable = true;
        }
        qDebug() << "SERVICE UUID [" << myService->getUuid() << "]";
    }
    if(mBatteryServiceAvailable) {
        qDebug() << "NFC Reader Battery Service found";
        if(mBatteryService->hasCharacteristics()) {
            onBatteryCharacteristicsDone();
        } else {
            mBatteryService->connectToService();
        }
    }
    if(mTagServiceAvailable) {
        qDebug() << "NFC Reader Tag Service available";
        if(mTagService->hasCharacteristics()) {
            onTagCharacteristicsDone();
        } else {
            mTagService->connectToService();
        }
    }
}

void NfcReaderManager::updateBatteryLevel()
{
    if(!mBatteryLevelAvailable) {
        return;
    }
    if(mBatteryLevel) {
        mBatteryService->readCharacteristic(mBatteryLevel);
    }
}

void NfcReaderManager::updateTagValue()
{
    if(mTagId) {
        mTagService->readCharacteristic(mTagId);
    }
}

void NfcReaderManager::startTagNotifications()
{
    if(mTagService && mTagId) {
        mTagService->subscribeNotifications(mTagId);
    }
}

void NfcReaderManager::stopTagNotifications()
{
    if(mTagService && mTagId) {
        mTagService->unSubscribeNotifications(mTagId);
    }
}

MyBluetoothCharacteristic *NfcReaderManager::getBatteryLevel()
{
    return mBatteryLevel;
}

// B0, B1, B2, B3
void NfcReaderManager::doBuzzer(QString buzzerValue)
{
    qDebug() << "write to NFC Reader buzzer command:" << buzzerValue;
    mTagService->writeCharacteristicAsString(mWriteData, buzzerValue, false);
}

// LEDR LEDG LEDB
void NfcReaderManager::doLED(QString ledValue)
{
    qDebug() << "write to NFC Reader LED command:" << ledValue;
    mTagService->writeCharacteristicAsString(mWriteData, ledValue, false);
}

void NfcReaderManager::onBatteryCharacteristicsDone()
{
    qDebug() << "onBatteryCharacteristicsDone - get " << BATTERY_LEVEL;
    mBatteryLevel = mBatteryService->getCharacteristicInfo(BATTERY_LEVEL);
    if(mBatteryLevel) {
        connect(mBatteryLevel, &MyBluetoothCharacteristic::currentValueChanged, this, &NfcReaderManager::onBatteryLevelChanged);
        mBatteryLevelAvailable = true;
        checkIfAllPrepared();
        // read current value
        updateBatteryLevel();
    }
}

void NfcReaderManager::onTagCharacteristicsDone()
{
    qDebug() << "onTagCharacteristicsDone - get " << NFC_TAG_CHARACTERISTIC;
    mTagId = mTagService->getCharacteristicInfo(NFC_TAG_CHARACTERISTIC);
    if(mTagId) {
        connect(mTagId, &MyBluetoothCharacteristic::currentValueChanged, this, &NfcReaderManager::onTagIdChanged);
        connect(mTagId, &MyBluetoothCharacteristic::characteristicChanged, this, &NfcReaderManager::onTagSubscriptionsChanged);
        mTagIdAvailable = true;

        // don't read current key - the lock remains the last connected key
    }
    qDebug() << "onWriteDataCharacteristicsDone - get " << NFC_WRITE_CHARACTERISTIC;
    mWriteData = mTagService->getCharacteristicInfo(NFC_WRITE_CHARACTERISTIC);
    if(mWriteData) {
        qDebug() << "W R I T E   characteristic found";
        // write with no response
        mWriteDataAvailable = true;

    }
    if(mTagId && mWriteData) {
        checkIfAllPrepared();
    }
}

// SLOT from SIGNAL deviceChanged
void NfcReaderManager::onDisconnect()
{
    qDebug() << "NfcReaderManager deviceChanged - onDisconnect";
    // we're only interested into unconnect
    if(!mDeviceInfo || !mDeviceInfo->getDeviceIsConnected()) {
        qDebug() << "NfcReaderManager onDisconnect";
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setTagNotificationsActive(false);
    }
}

void NfcReaderManager::onBatteryLevelChanged()
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

void NfcReaderManager::onTagIdChanged()
{
    QByteArray valueArray = mTagId->getCurrentValue();

    QString stringValue = valueArray;

    if(mTagIdValue.length()> 0 && stringValue == NO_ID) {
        qDebug() << "ignore \r (== empty value)";
        return;
    }

    mTagIdValue = stringValue;
    mTagIdValue.replace("\r","");
    QStringList sl;
    sl = mTagIdValue.split("\n");
    bool found = false;
    for (int i = 0; i < sl.size(); ++i) {
        QString s = sl.at(i);
        if(s.startsWith("UID:")) {
            found = true;
            mTagIdValue = s.right(s.length()-4);
        }
    }
    if(!found) {
        qWarning() << "no UUID: found in stringValue - scan ignored";
        return;
    }

    qDebug() << "another TAG ID. string value: "<< stringValue << " tagId:" << mTagIdValue;
    emit tagIdValueChanged();
}

QVariantList NfcReaderManager::mappingsToVariantList()
{
    QVariantList theList;
    for (int i = 0; i < mMappings.size(); ++i) {
        NfcReaderMapping* nfcReaderMapping = (NfcReaderMapping*) mMappings.at(i);
        theList.append(nfcReaderMapping->toMap());
    }
    qDebug() << "Mappings as VariantList: " << theList.size();
    return theList;
}

void NfcReaderManager::mappingsFromVariantList(QVariantList &theList)
{
    clearMappings();
    for (int i = 0; i < theList.size(); ++i) {
        NfcReaderMapping* nfcReaderMapping = new NfcReaderMapping(this);
        nfcReaderMapping->fromMap(theList.at(i).toMap());
        mMappings.append(nfcReaderMapping);
    }
}

void NfcReaderManager::onTagSubscriptionsChanged()
{
    bool isRunning = mTagId->getNotifyIsRunning();
    if(isRunning != mTagNotificationsActive) {
        setTagNotificationsActive(isRunning);
    }
}

void NfcReaderManager::checkIfAllPrepared()
{
    // TODO mBatteryLevelAvailable &&
    if(mTagIdAvailable && mWriteDataAvailable && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}


