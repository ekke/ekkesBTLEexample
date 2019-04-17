#include "feitiancardreadermanager.hpp"

static const QString CARD_READER_SERVICE = "46540001-0002-00c4-0000-465453414645";
static const QString CARD_DATA_CHARACTERISTIC = "46540002-0002-00c4-0000-465453414645";
static const QString CARD_WRITE_CHARACTERISTIC = "46540003-0002-00c4-0000-465453414645";

static const QString CARD_STATE_IN = "0x5001";
static const QString CARD_STATE_OUT = "0x5003";

static const QString SUCCESS_APDU = "0x9000";
static const QString SUCCESS_POWER_OFF = "0x010000";
static const QString FAILED = "0x000000";

static const QString COMMAND_POWER_ON = "0x620000";
static const QString COMMAND_POWER_OFF = "0x630000";
static const QString COMMAND_SELECT_FILE = "0x6f0b00";
static const QString COMMAND_READ_BINARY = "0x6f0700";
static const QString COMMAND_READ_BINARY_NEXT = "0x6f0000";

// select eGK Application
static const QString APDU_SELECT_FILE = "0x00a4040c06d27600000102";
static const QString APDU_READ_BINARY_STATUS_VD = "0x00b08c00000000";
static const QString APDU_READ_BINARY_PERSONAL_DATA = "0x00b08100000000";
// not used yet:
static const QString APDU_READ_BINARY_INSURANCE_DATA = "0x00b08200000000";

static const QString ATR_EGK_G2 = "0x3bd396ff81b1fe451f078081052d";


FeitianCardReaderManager::FeitianCardReaderManager(QObject *parent) : QObject(parent), mDeviceInfo(nullptr), mDeviceIsConnected(false),
    mCardServiceAvailable(false), mCardServiceConnected(false), mCardDataAvailable(false),
    mFeaturesPrepared(false), mCardNotificationsActive(false), mHasDevice(false)
{

}

void FeitianCardReaderManager::init(BluetoothManager *bluetoothManager)
{
    mBluetoothManager = bluetoothManager;
    // get settings
    mFeitianCardReaderSettingsMap = mBluetoothManager->mBluetoothSettingsMap.value("feitiancardreader").toMap();
    if(mFeitianCardReaderSettingsMap.isEmpty()) {
        mFeitianCardReaderSettingsMap.insert("settingsFavoriteAddress", mSettingsFavoriteAddress);
        mBluetoothManager->mBluetoothSettingsMap.insert("feitiancardreader", mFeitianCardReaderSettingsMap);
        mBluetoothManager->cacheSettings();
        qDebug() << "Feitian CardReader Settings created";
    } else {
        mSettingsFavoriteAddress = mFeitianCardReaderSettingsMap.value("settingsFavoriteAddress").toString();
        qDebug() << "Feitian CardReader Settings read:";
    }
}

void FeitianCardReaderManager::updateSettings()
{
    mBluetoothManager->mBluetoothSettingsMap.insert("feitiancardreader", mFeitianCardReaderSettingsMap);
    mBluetoothManager->cacheSettings();
    emit settingsChanged();
}

QString FeitianCardReaderManager::getCardDataValue() const
{
    return mCardDataValue;
}

bool FeitianCardReaderManager::getFeaturesPrepared() const
{
    return mFeaturesPrepared;
}

void FeitianCardReaderManager::setFeaturesPrepared(bool isPrepared)
{
    if(mFeaturesPrepared != isPrepared) {
        mFeaturesPrepared = isPrepared;
        emit featuresPreparedChanged();
    }
}

bool FeitianCardReaderManager::getCardNotificationsActive() const
{
    return mCardNotificationsActive;
}

void FeitianCardReaderManager::setCardNotificationsActive(bool isActive)
{
    if(mCardNotificationsActive != isActive) {
        mCardNotificationsActive = isActive;
        emit cardNotificationsActiveChanged();
    }
}

bool FeitianCardReaderManager::getHasDevice() const
{
    return mHasDevice;
}

QString FeitianCardReaderManager::getSettingsFavoriteAddress() const
{
    return mSettingsFavoriteAddress;
}

void FeitianCardReaderManager::setSettingsFavoriteAddress(QString address)
{
    if(mSettingsFavoriteAddress != address) {
        mSettingsFavoriteAddress = address;
        mFeitianCardReaderSettingsMap.insert("settingsFavoriteAddress", address);
        updateSettings();
        emit settingsChanged();
    }
}

QString FeitianCardReaderManager::getSettingsFavoriteName() const
{
    return mSettingsFavoriteName;
}

void FeitianCardReaderManager::setSettingsFavoriteName(QString name)
{
    if(mSettingsFavoriteName != name) {
        mSettingsFavoriteName = name;
        mFeitianCardReaderSettingsMap.insert("settingsFavoriteName", name);
        updateSettings();
        emit settingsNameChanged();
    }
}

void FeitianCardReaderManager::setCurrentDevice(MyBluetoothDeviceInfo *myDevice)
{
    if(myDevice) {
        bool deviceAddressChanged = false;
        if(mDeviceInfo && mDeviceInfo->getAddress() != myDevice->getAddress()) {
            deviceAddressChanged = true;
        }
        mDeviceInfo = myDevice;
        qDebug() << "Feitian CardReader Manager: current Device " << myDevice->getAddress() << " / " << myDevice->getName();
        // remember address and name
        setSettingsFavoriteAddress(myDevice->getAddress());
        setSettingsFavoriteName(myDevice->getName());
        // set expected service uuids
        QStringList sl;
        sl.append(CARD_READER_SERVICE);
        myDevice->setExpectedServiceUuids(sl);
        //
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &FeitianCardReaderManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
    } else {
        mDeviceInfo = nullptr;
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setCardNotificationsActive(false);
        // mCurrentKey.clear();
        mCardDataValue.clear();
        emit cardDataValueChanged();

        if(mHasDevice) {
            mHasDevice = false;
            emit hasDeviceChanged();
        }
        qDebug() << "Feitian CardReader Manager: current Device REMOVED";
    }
}

MyBluetoothDeviceInfo *FeitianCardReaderManager::currentDevice()
{
    return mDeviceInfo;
}

bool FeitianCardReaderManager::isCurrentDeviceConnected()
{
    if(!mDeviceInfo) {
        return false;
    }
    return mDeviceInfo->getDeviceIsConnected();
}

void FeitianCardReaderManager::prepareServices()
{
    qDebug() << "Feitian CardReader: prepareServices and start features";
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

    mCardServiceAvailable = false;
    mCardServiceConnected = false;
    mCardDataAvailable = false;
    mWriteDataAvailable = false;

    qDebug() << "services #" << myServices.size();
    for (int i = 0; i < myServices.size(); ++i) {
        MyBluetoothServiceInfo* myService = (MyBluetoothServiceInfo*)myServices.at(i);
        if(myService->getUuid() == CARD_READER_SERVICE) {
            qDebug() << "CARD_READER_SERVICE detected";
            mCardService = myService;
            connect(mCardService, &MyBluetoothServiceInfo::characteristicsDone, this, &FeitianCardReaderManager::onCardCharacteristicsDone);
            mCardServiceAvailable = true;
        }
        qDebug() << "SERVICE UUID [" << myService->getUuid() << "]";
    }

    // ATTENTION: on iOS I got some trouble to scan for the characteristics from different services
    // solution: chaining it: as soon as onScanCharacteristicsDone() we do this commented code:
//    if(mDeviceInfoServiceAvailable) {
//        qDebug() << "GeneralScan DeviceInfo Service available";
//        if(mDeviceInfoService->hasCharacteristics()) {
//            qDebug() << "GeneralScan DeviceInfo Service has Characteristics";
//            onDeviceInfoCharacteristicsDone();
//        } else {
//            qDebug() << "GeneralScan DeviceInfo Service connect to service";
//            mDeviceInfoService->connectToService();
//        }
//    }

    if(mCardServiceAvailable) {
        qDebug() << "Feitian CardReader Card Service available";
        if(mCardService->hasCharacteristics()) {
            qDebug() << "Feitian CardReader Card Service has characteristics";
            onCardCharacteristicsDone();
        } else {
            qDebug() << "Feitian Card Service connect to Service";
            mCardService->connectToService();
        }
    }
}

void FeitianCardReaderManager::startCardNotifications()
{
    if(mCardService && mCardData) {
        mCardService->subscribeNotifications(mCardData);
    }
}

void FeitianCardReaderManager::stopCardNotifications()
{
    if(mCardService && mCardData) {
        mCardService->unSubscribeNotifications(mCardData);
    }
}

void FeitianCardReaderManager::resetFoundDevices()
{
    mFoundDevices.clear();
}

void FeitianCardReaderManager::addToFoundDevices(MyBluetoothDeviceInfo *deviceInfo)
{
    mFoundDevices.append(deviceInfo);
    emit foundDevicesCounter(mFoundDevices.size());
}

QList<QObject *> FeitianCardReaderManager::foundDevices()
{
    return mFoundDevices;
}

void FeitianCardReaderManager::doPowerOn()
{
    qDebug() << "write to Feitian CardReader PowerOn command:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doPowerOff()
{
    qDebug() << "write to Feitian CardReader PowerOff command:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doSelectFile()
{
    qDebug() << "write to Feitian CardReader SelectFile APDU:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doReadBinaryStatusVD()
{
    qDebug() << "write to Feitian CardReader ReadBinary StatusVD APDU:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doReadBinaryPersonalData()
{
    qDebug() << "write to Feitian CardReader ReadBinary PersonalData APDU:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doReadBinaryInsuranceData()
{
    qDebug() << "write to Feitian CardReader ReadBinary InsuranceData APDU:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::onCardCharacteristicsDone()
{
    qDebug() << "process onCardCharacteristicsDone - get " << CARD_DATA_CHARACTERISTIC;
    mCardData = mCardService->getCharacteristicInfo(CARD_DATA_CHARACTERISTIC);
    if(mCardData) {
        qDebug() << "CardReader N O T I F Y   characteristic found";
        connect(mCardData, &MyBluetoothCharacteristic::currentValueChanged, this, &FeitianCardReaderManager::onCardDataChanged);
        connect(mCardData, &MyBluetoothCharacteristic::characteristicChanged, this, &FeitianCardReaderManager::onCardSubscriptionsChanged);
        mCardDataAvailable = true;
    }
    qDebug() << "onWriteDataCharacteristicsDone - get " << CARD_WRITE_CHARACTERISTIC;
    mWriteData = mCardService->getCharacteristicInfo(CARD_WRITE_CHARACTERISTIC);
    if(mWriteData) {
        qDebug() << "W R I T E   characteristic found";
        // write with no response
        mWriteDataAvailable = true;

    }
    if(mCardData && mWriteData) {
        checkIfAllPrepared();
    }
}

// SLOT from SIGNAL deviceChanged
void FeitianCardReaderManager::onDisconnect()
{
    qDebug() << "Feitian CardReader Manager deviceChanged - onDisconnect";
    // we're only interested into unconnect
    if(!mDeviceInfo || !mDeviceInfo->getDeviceIsConnected()) {
        qDebug() << "Feitian CardReader Manager onDisconnect";
        mDeviceIsConnected = false;
        setFeaturesPrepared(false);
        setCardNotificationsActive(false);
    }
}

void FeitianCardReaderManager::onCardDataChanged()
{
     QByteArray cardDataArray = mCardData->getCurrentValue();
     QString hexValue = cardDataArray.toHex();
     if(hexValue == CARD_STATE_IN) {
         emit cardIN();
         return;
     }
     if(hexValue == CARD_STATE_OUT) {
         emit cardOUT();
     }
     qDebug() << "ByteArray of " << cardDataArray.length() << " value: " << cardDataArray << " HEXVALUE length " << hexValue.length() << " value: " << hexValue;

//    if(mBarcodeValue == hexValue) {
//        qDebug() << "same key while tottle timer is running";
//        return;
//    }
//    if(mBarcodeValue.length()> 0 && hexValue == NO_KEY) {
//        mCurrentKey = mKeyIdValue;
//        mTotterTimer->start();
//        return;
//    }
    // mBarcodeValue = hexValue;
    mCardDataValue = hexValue; // QString::fromUtf8(mCardData->getCurrentValue());
    qDebug() << "we got changed Card Data:" << mCardDataValue;
    emit cardDataValueChanged();
}

void FeitianCardReaderManager::onCardSubscriptionsChanged()
{
    bool isRunning = mCardData->getNotifyIsRunning();
    if(isRunning != mCardNotificationsActive) {
        setCardNotificationsActive(isRunning);
    }
}

void FeitianCardReaderManager::checkIfAllPrepared()
{
    if(mCardDataAvailable && mWriteDataAvailable  && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}

