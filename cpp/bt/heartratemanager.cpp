#include "heartratemanager.hpp"

#include <QtEndian>

static const QString BATTERY_SERVICE = "0x180f";
static const QString BATTERY_LEVEL = "0x2a19";

static const QString HR_SERVICE = "0x180d";
static const QString HR_MEASUREMENT = "0x2a37";
// static const QString BODY_SENSOR = "0x2a38";


HeartRateManager::HeartRateManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false), mBatteryServiceAvailable(false),
    mBatteryServiceConnected(false), mBatteryLevelAvailable(false), mBatteryLevelValue(-1), mMessageServiceAvailable(false), mMessageServiceConnected(false),
    mMessageAvailable(false), mFeaturesPrepared(false), mMessageReadNotificationsActive(false), mBatteryLevelNotificationsActive(false), mHasDevice(false),
    mSettingsBatteryLevelInfo(70), mSettingsBatteryLevelWarning(30)
{
    //
}

void HeartRateManager::init(BluetoothManager *bluetoothManager)
{
    mBluetoothManager = bluetoothManager;
    // get settings
    mHeartRateSettingsMap = mBluetoothManager->mBluetoothSettingsMap.value("heartrate").toMap();
    if(mHeartRateSettingsMap.isEmpty()) {
        mHeartRateSettingsMap.insert("settingsFavoriteAddress", mSettingsFavoriteAddress);
        mHeartRateSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
        mHeartRateSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
        mBluetoothManager->mBluetoothSettingsMap.insert("heartrate", mHeartRateSettingsMap);
        mBluetoothManager->cacheSettings();
        qDebug() << "HeartRate Settings created";
    } else {
        mSettingsFavoriteAddress = mHeartRateSettingsMap.value("settingsFavoriteAddress").toString();
        mSettingsBatteryLevelInfo = mHeartRateSettingsMap.value("settingsBatteryLevelInfo").toInt();
        mSettingsBatteryLevelWarning = mHeartRateSettingsMap.value("settingsBatteryLevelWarning").toInt();
        qDebug() << "HeartRate Settings read";
    }
}

void HeartRateManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("heartrate", mHeartRateSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

int HeartRateManager::getBatteryLevelValue() const
{
    return mBatteryLevelValue;
}

bool HeartRateManager::getFeaturesPrepared() const
{
    return mFeaturesPrepared;
}

void HeartRateManager::setFeaturesPrepared(bool isPrepared)
{
    if(mFeaturesPrepared != isPrepared) {
        mFeaturesPrepared = isPrepared;
        emit featuresPreparedChanged();
    }
}

bool HeartRateManager::getMessageReadNotificationsActive() const
{
    return mMessageReadNotificationsActive;
}

void HeartRateManager::setMessageReadNotificationsActive(bool isActive)
{
    if(mMessageReadNotificationsActive != isActive) {
        mMessageReadNotificationsActive = isActive;
        emit messageReadNotificationsActiveChanged();
    }
}

bool HeartRateManager::getBatteryLevelNotificationsActive() const
{
    return mBatteryLevelNotificationsActive;
}

void HeartRateManager::setBatteryLevelNotificationsActive(bool isActive)
{
    if(mBatteryLevelNotificationsActive != isActive) {
        mBatteryLevelNotificationsActive = isActive;
        emit batteryLevelNotificationsActiveChanged();
    }
}

bool HeartRateManager::getHasDevice() const
{
    return mHasDevice;
}

QString HeartRateManager::getSettingsFavoriteAddress() const
{
    return mSettingsFavoriteAddress;
}

void HeartRateManager::setSettingsFavoriteAddress(QString address)
{
    if(mSettingsFavoriteAddress != address) {
        mSettingsFavoriteAddress = address;
        mHeartRateSettingsMap.insert("settingsFavoriteAddress", address);
        updateSettings();
        emit settingsChanged();
    }
}

int HeartRateManager::getSettingsBatteryLevelInfo() const
{
    return mSettingsBatteryLevelInfo;
}

int HeartRateManager::getSettingsBatteryLevelWarning() const
{
    return mSettingsBatteryLevelWarning;
}

void HeartRateManager::setBatteryValues(int info, int warning)
{
    mSettingsBatteryLevelInfo = info;
    mSettingsBatteryLevelWarning = warning;
    mHeartRateSettingsMap.insert("settingsBatteryLevelInfo", mSettingsBatteryLevelInfo);
    mHeartRateSettingsMap.insert("settingsBatteryLevelWarning", mSettingsBatteryLevelWarning);
    updateSettings();
}

QVariant HeartRateManager::getMessages()
{
    return QVariant::fromValue(mMessages);
}

void HeartRateManager::clearMessages()
{
    qDeleteAll(mMessages);
    mMessages.clear();
    emit messagesUpdated();
}

void HeartRateManager::setCurrentDevice(MyBluetoothDeviceInfo *myDevice)
{
    if(myDevice) {
        bool deviceAddressChanged = false;
        if(mDeviceInfo && mDeviceInfo->getAddress() != myDevice->getAddress()) {
            deviceAddressChanged = true;
        }
        mDeviceInfo = myDevice;
        qDebug() << "HeartRate Manager: current Device " << myDevice->getAddress();
        // remember address
        setSettingsFavoriteAddress(myDevice->getAddress());
        // set expected service uuids
        QStringList sl;
        sl.append(HR_SERVICE);
        sl.append(BATTERY_SERVICE);
        myDevice->setExpectedServiceUuids(sl);
        //
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &HeartRateManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
    } else {
        mDeviceInfo = nullptr;
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setMessageReadNotificationsActive(false);
        setBatteryLevelNotificationsActive(false);
        clearMessages();
        emit messageValueChanged();
        mBatteryLevelValue = -1;
        emit batteryLevelValueChanged();
        if(mHasDevice) {
            mHasDevice = false;
            emit hasDeviceChanged();
        }
        qDebug() << "HeartRate Manager: current Device REMOVED";
    }
}

MyBluetoothDeviceInfo *HeartRateManager::currentDevice()
{
    return mDeviceInfo;
}

bool HeartRateManager::isCurrentDeviceConnected()
{
    if(!mDeviceInfo) {
        return false;
    }
    return mDeviceInfo->getDeviceIsConnected();
}

void HeartRateManager::prepareServices()
{
    qDebug() << "HeartRate: prepareServices and start features";
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

    mMessageServiceAvailable = false;
    mMessageServiceConnected = false;
    mMessageAvailable = false;

    qDebug() << "services #" << myServices.size();
    for (int i = 0; i < myServices.size(); ++i) {
        MyBluetoothServiceInfo* myService = (MyBluetoothServiceInfo*)myServices.at(i);
        if(myService->getUuid() == BATTERY_SERVICE) {
            mBatteryService = myService;
            connect(mBatteryService, &MyBluetoothServiceInfo::characteristicsDone, this, &HeartRateManager::onBatteryCharacteristicsDone);
            mBatteryServiceAvailable = true;
        } else if(myService->getUuid() == HR_SERVICE) {
            mMessageService = myService;
            connect(mMessageService, &MyBluetoothServiceInfo::characteristicsDone, this, &HeartRateManager::onMessageCharacteristicsDone);
            mMessageServiceAvailable = true;
        }
        qDebug() << "SERVICE UUID [" << myService->getUuid() << "]";
    }
    if(mBatteryServiceAvailable) {
        qDebug() << "HeartRate Battery Service found";
        if(mBatteryService->hasCharacteristics()) {
            onBatteryCharacteristicsDone();
        } else {
            mBatteryService->connectToService();
        }
    }
    if(mMessageServiceAvailable) {
        qDebug() << "HeartRate Message Service available";
        if(mMessageService->hasCharacteristics()) {
            onMessageCharacteristicsDone();
        } else {
            mMessageService->connectToService();
        }
    }
}

void HeartRateManager::updateBatteryLevel()
{
    if(mBatteryLevel) {
        mBatteryService->readCharacteristic(mBatteryLevel);
    }
}

void HeartRateManager::updateMessageValue()
{
    if(mMessageService && mMessage) {
        mMessageService->readCharacteristic(mMessage);
    }
}

void HeartRateManager::startMessageReadNotifications()
{
    if(mMessageService && mMessage) {
        mLastHeartRate = 0;
        mMessageService->subscribeNotifications(mMessage);
    }
}

void HeartRateManager::stopMessageReadNotifications()
{
    if(mMessageService && mMessage) {
        mMessageService->unSubscribeNotifications(mMessage);
    }
}

void HeartRateManager::startBatteryLevelNotifications()
{
    if(mBatteryService && mBatteryLevel) {
        // read current value
        updateBatteryLevel();
        mBatteryService->subscribeNotifications(mBatteryLevel);
    }
}

void HeartRateManager::stopBatteryLevelNotifications()
{
    if(mBatteryService && mBatteryLevel) {
        mBatteryService->unSubscribeNotifications(mBatteryLevel);
    }
}

MyBluetoothCharacteristic *HeartRateManager::getBatteryLevel()
{
    return mBatteryLevel;
}

void HeartRateManager::onBatteryCharacteristicsDone()
{
    qDebug() << "onBatteryCharacteristicsDone get " << BATTERY_LEVEL;
    mBatteryLevel = mBatteryService->getCharacteristicInfo(BATTERY_LEVEL);
    if(mBatteryLevel) {
        qDebug() << "battery level characteristic found";
        connect(mBatteryLevel, &MyBluetoothCharacteristic::currentValueChanged, this, &HeartRateManager::onBatteryLevelChanged);
        connect(mBatteryLevel, &MyBluetoothCharacteristic::characteristicChanged, this, &HeartRateManager::onBatterySubscriptionsChanged);
        mBatteryLevelAvailable = true;
        checkIfAllPrepared();
        // read current value
        updateBatteryLevel();
    }
}

void HeartRateManager::onMessageCharacteristicsDone()
{
    qDebug() << "onMessageCharacteristicsDone - get " << HR_MEASUREMENT;
    mMessage = mMessageService->getCharacteristicInfo(HR_MEASUREMENT);
    if(mMessage) {
        qDebug() << "read characteristic found";
        connect(mMessage, &MyBluetoothCharacteristic::currentValueChanged, this, &HeartRateManager::onMessageChanged);
        connect(mMessage, &MyBluetoothCharacteristic::characteristicChanged, this, &HeartRateManager::onMessageSubscriptionsChanged);
        mMessageAvailable = true;
        checkIfAllPrepared();
        // don't read current key - the lock remains the last connected key
    }
}

// SLOT from SIGNAL deviceChanged
void HeartRateManager::onDisconnect()
{
    qDebug() << "HeartRateManager deviceChanged - onDisconnect";
    // we're only interested into unconnect
    if(!mDeviceInfo || !mDeviceInfo->getDeviceIsConnected()) {
        qDebug() << "HeartRateManager onDisconnect";
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setMessageReadNotificationsActive(false);
        setBatteryLevelNotificationsActive(false);
        clearMessages();
    }
}

void HeartRateManager::onBatteryLevelChanged()
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

void HeartRateManager::onMessageChanged()
{
    int heartRateBpm;
    int sensorContact = -1;
    int energyJoule = -1;

    // to understand HowTo get the right Bits and Bytes see:
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.heart_rate_measurement.xml
    // thanks to Martin Woolley and John Murray for all their work on BT LE examples for BlackBerry10 and Qt
    // from Qt Example HeartListener I understood HowTo deal with the data ByteArray

    // get the D A T A
    const quint8 *data = reinterpret_cast<const quint8 *>(mMessage->getCurrentValue().constData());

    // now get the F L A G S (first byte)
    quint8 flags = data[0];

    // H E A R T   R A T E
    // can be 16 bit or 8 bit
    if (flags & 0x1) {
        const quint16 heartRate = qFromLittleEndian<quint16>(data[1]);
        heartRateBpm = heartRate;
    } else {
        const quint8 *heartRate = &data[1];
        heartRateBpm = *heartRate;
    }

    if(heartRateBpm == mLastHeartRate) {
        // ignore - we only display changes
        return;
    }
    mLastHeartRate = heartRateBpm;

    // S E N S O R   C O N T A C T
    // 0,1 not supported 2 suported not detected 3 supported detected
    if(flags & 0x2) {
        // not supported
    } else if(flags & 0x4) {
        // connected
        sensorContact = 1;
    } else {
        // not connected
        sensorContact = 0;
    }

    // E N E R G Y   (Joule)
    if (flags & 0x8) {
        int index = (flags & 0x1) ? 5 : 3;
        const quint16 energy = qFromLittleEndian<quint16>(data[index]);
        energyJoule = energy;
    }

    HeartRateMessages* heartRateMessages;
    heartRateMessages = new HeartRateMessages(this);
    heartRateMessages->setData(heartRateBpm, sensorContact, energyJoule);
    mMessages.insert(0, heartRateMessages);
    qDebug() << "HeartRateMessages: " << mMessages.size();
    emit messagesUpdated();
    emit messageValueChanged();
}

void HeartRateManager::onMessageSubscriptionsChanged()
{
    bool isRunning = mMessage->getNotifyIsRunning();
    if(isRunning != mMessageReadNotificationsActive) {
        setMessageReadNotificationsActive(isRunning);
    }
}

void HeartRateManager::onBatterySubscriptionsChanged()
{
    bool isRunning = mBatteryLevel->getNotifyIsRunning();
    if(isRunning != mBatteryLevelNotificationsActive) {
        setBatteryLevelNotificationsActive(isRunning);
    }
}

void HeartRateManager::checkIfAllPrepared()
{
    if(mBatteryLevelAvailable && mMessageAvailable && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}


