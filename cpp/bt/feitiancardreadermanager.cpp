#include "feitiancardreadermanager.hpp"

static const QString CARD_READER_SERVICE = "46540001-0002-00c4-0000-465453414645";
static const QString CARD_DATA_CHARACTERISTIC = "46540003-0002-00c4-0000-465453414645";
static const QString CARD_WRITE_CHARACTERISTIC = "46540002-0002-00c4-0000-465453414645";

static const QString CARD_STATE_IN = "5003";
static const QString CARD_STATE_OUT = "5002";

// https://www.eftlab.com/index.php/site-map/knowledge-base/118-apdu-response-list
// https://de.wikipedia.org/wiki/Application_Protocol_Data_Unit
static const QString APDU_COMMAND_SUCCESSFULLY_EXECUTED = "9000";


static const QString SUCCESS_POWER_OFF = "010000";
static const QString FAILED = "000000";

static const QString COMMAND_POWER_ON = "620000";
static const QString COMMAND_POWER_OFF = "630000";
static const QString COMMAND_SELECT_FILE = "6f0b00";
static const QString COMMAND_READ_BINARY = "6f0700";
static const QString COMMAND_READ_BINARY_NEXT = "6f0000";

// select eGK Application - there's only a single application
static const QString APDU_SELECT_FILE = "00a4040c06d27600000102";
// there are three Binary files: Status VD, Personal Data, Insurance Data
static const QString APDU_READ_BINARY_STATUS_VD = "00b08c00000000";
static const QString APDU_READ_BINARY_PERSONAL_DATA = "00b08100000000";
// not used yet:
static const QString APDU_READ_BINARY_INSURANCE_DATA = "00b08200000000";
// https://www.eftlab.com/index.php/site-map/knowledge-base/118-apdu-response-list
// https://de.wikipedia.org/wiki/Application_Protocol_Data_Unit#Statusw%C3%B6rter
static const QString APDU_RESPONSE_INFO_URL = "https://de.wikipedia.org/wiki/Application_Protocol_Data_Unit#Statusw%C3%B6rter";

// https://de.wikipedia.org/wiki/Answer_to_Reset
// List of ATR http://ludovic.rousseau.free.fr/softwares/pcsc-tools/smartcard_list.txt
// online check ATR: https://smartcard-atr.appspot.com/parse?ATR=xxxxx
// Elektronische Gesundheitskarte Generation 2:
static const QString ATR_EGK_G2 = "3bd396ff81b1fe451f078081052d";
static const QString PARSE_ATR = "https://smartcard-atr.appspot.com/parse?ATR=";

static const QString THREE_BYTE_FILLER = "000000";


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
    mCurrentIdHex = "00000000";
    mRunningCommand.clear();
    mRunningAPDU.clear();
    mCurrentData.clear();
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
        qDebug() << "AAAA";
        QStringList sl;
        sl.append(CARD_READER_SERVICE);
        myDevice->setExpectedServiceUuids(sl);
        //
        qDebug() << "BBBB";
        mDeviceIsConnected = mDeviceInfo->getDeviceIsConnected();
        connect(mDeviceInfo, &MyBluetoothDeviceInfo::deviceChanged, this, &FeitianCardReaderManager::onDisconnect);
        if(!mHasDevice || deviceAddressChanged) {
            mHasDevice = true;
            emit hasDeviceChanged();
        }
        qDebug() << "CCCC";
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

void FeitianCardReaderManager::resetCommand()
{
    mRunningCommand.clear();
    mRunningAPDU.clear();
    mCurrentData.clear();
}

void FeitianCardReaderManager::doPowerOn()
{
    qDebug() << "write to Feitian CardReader PowerOn command:";
    // reset commands if something left
    resetCommand();
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_POWER_ON;
    theCommand.append(mCurrentIdHex);
    theCommand.append(THREE_BYTE_FILLER);
    // set running command
    mRunningCommand = COMMAND_POWER_ON;
    // Write to CardReader
    mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
}

void FeitianCardReaderManager::processPowerOn(const QString& hexData)
{
    if(hexData.length() == 40) {
        mCurrentData += hexData;
        // wait for more
        return;
    }
    mCurrentData += hexData;
    if(mCurrentData.length() < 20) {
        qWarning() << "response data should be 20 or more bytes and not " << mCurrentData.length()/2;
        emit readATRWrong(tr("Received buffer too short for a valid ATR"),"");
        resetCommand();
        return;
    }
    QString responseType = mCurrentData.left(6);
    QString id = mCurrentData.mid(6,8);
    if(id != mCurrentIdHex) {
        qWarning() << "ID seems to be wrong: " << id << " instead of " << mCurrentIdHex;
        // we ignore this in our demo app
    }
    QString unknownFiller = mCurrentData.mid(14,6);
    QString payload = mCurrentData.right(mCurrentData.length()-20);

    // now it's safe to reset the command vars
    resetCommand();

    // go on
    qDebug() << "processing Power On. response type: " << responseType << " ID: " << id << " Filler: " << unknownFiller;
    qDebug() << "Payload: " << payload;
    if(payload == ATR_EGK_G2) {
        emit readATRSuccess();
        // no do the next step
        doSelectFile();
        return;
    }
    QString parseUrl = PARSE_ATR;
    parseUrl.append(payload);
    qDebug() << "wrong ATR. Parse URL:" << parseUrl;
    emit readATRWrong(tr("Wrong Card - only eGK 2 is implemented yet"),parseUrl);
}

// do we need this ???
void FeitianCardReaderManager::doPowerOff()
{
    qDebug() << "write to Feitian CardReader PowerOff command:";
    mCardService->writeCharacteristicAsHex(mWriteData, "", false);
}

void FeitianCardReaderManager::doSelectFile()
{
    qDebug() << "write to Feitian CardReader SelectFile APDU:";
    // reset commands if something left
    resetCommand();
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_SELECT_FILE;
    theCommand.append(mCurrentIdHex);
    theCommand.append(THREE_BYTE_FILLER);
    theCommand.append(APDU_SELECT_FILE);
    // set running command
    mRunningCommand = COMMAND_SELECT_FILE;
    // Write to CardReader
    // split command
    if(theCommand.length() > 40) {
        mCardService->writeCharacteristicAsHex(mWriteData, theCommand.left(40), false);
        mCardService->writeCharacteristicAsHex(mWriteData, theCommand.right(theCommand.length()-40), false);
    } else {
        qDebug() << "uuups - command should be length 42 (21 Bytes)";
        mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
    }
}

void FeitianCardReaderManager::processSelectFiles(const QString& hexData)
{
    if(hexData.length() == 40) {
        mCurrentData += hexData;
        // wait for more
        qWarning() << "uuups - data should be only length of 24 (12 Bytes) and not 40 or more";
        return;
    }
    mCurrentData += hexData;
    if(mCurrentData.length() != 24) {
        qWarning() << "response data should be 12 bytes and not " << mCurrentData.length()/2;
        emit appSelectedFailed(tr("Received buffer not 12 Bytes for a valid Select File response"),"","");
        resetCommand();
        return;
    }
    QString responseType = mCurrentData.left(6);
    QString id = mCurrentData.mid(6,8);
    if(id != mCurrentIdHex) {
        qWarning() << "ID seems to be wrong: " << id << " instead of " << mCurrentIdHex;
        // we ignore this in our demo app
    }
    QString unknownFiller = mCurrentData.mid(14,6);
    QString payload = mCurrentData.right(mCurrentData.length()-20);

    // now it's safe to reset the command vars
    resetCommand();

    // go on
    qDebug() << "processing Power On. response type: " << responseType << " ID: " << id << " Filler: " << unknownFiller;
    qDebug() << "Payload: " << payload;
    if(payload == APDU_COMMAND_SUCCESSFULLY_EXECUTED) {
        emit appSelectedSuccess();
        return;
    }
    emit appSelectedFailed(tr("Cannot select the File on eGK - wrong Response Code"), APDU_RESPONSE_INFO_URL, payload);
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
    // reset all values from command processing
    resetCommand();
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
    qDebug() << "onCardDataChanged()";
    QByteArray cardDataArray = mCardData->getCurrentValue();
     QString hexValue = cardDataArray.toHex();
     mCardDataValue = hexValue;
     // at first check card status
     if(hexValue == CARD_STATE_IN) {
         // only to display the data for test purpose
         emit cardDataValueChanged();
         // Card is inserted
         emit cardIN();
         return;
     }
     if(hexValue == CARD_STATE_OUT) {
         // only to display the data for test purpose
         emit cardDataValueChanged();
         // Card is removed
         emit cardOUT();
         return;
     }
    qDebug() << "ByteArray of " << cardDataArray.length() << " value: " << cardDataArray << " HEXVALUE length " << hexValue.length() << " value: " << hexValue;
    // only to display the data for test purpose
    emit cardDataValueChanged();
    // check if there's data
    if(hexValue.length() == 0) {
        qDebug() << "getting no data. nothing to do";
        return;
    }
    // now check if there's a running command
    if(mRunningCommand.length() == 0) {
        qWarning() << "getting Data from Card, but no Running Command.";
        return;
    }
    if(mRunningCommand == COMMAND_POWER_ON) {
        processPowerOn(hexValue);
        return;
    }
    if(mRunningCommand == COMMAND_POWER_OFF) {
        // do we need this ???
        return;
    }
    if(mRunningCommand == COMMAND_SELECT_FILE) {
        processSelectFiles(hexValue);
        return;
    }
    if(mRunningCommand == COMMAND_READ_BINARY || mRunningCommand == COMMAND_READ_BINARY_NEXT) {
        if(mRunningAPDU == APDU_READ_BINARY_STATUS_VD) {

            return;
        }
        if(mRunningAPDU == APDU_READ_BINARY_PERSONAL_DATA) {

            return;
        }
        if(mRunningAPDU == APDU_READ_BINARY_INSURANCE_DATA) {
            qWarning() << "reading Insurance Data not implemented yet";
            return;
        }
        return;
    }
    qWarning() << "uuups: nor handled situation. getting data back from card, but dont know what kind of data";
}

void FeitianCardReaderManager::onCardSubscriptionsChanged()
{
    qDebug() << "onCardSubscriptionsChanged()";
    bool isRunning = mCardData->getNotifyIsRunning();
    if(isRunning != mCardNotificationsActive) {
        setCardNotificationsActive(isRunning);
    }
    // if there's no subscription running we clear all command processing vars
    resetCommand();
}

void FeitianCardReaderManager::checkIfAllPrepared()
{
    if(mCardDataAvailable && mWriteDataAvailable  && mDeviceIsConnected) {
        setFeaturesPrepared(true);
    }
}

void FeitianCardReaderManager::calculateNextIdHex()
{
    bool ok = false;
    uint nextId = mCurrentIdHex.toUInt(&ok,16);
    nextId++;
    QString nextIdHex;
    nextIdHex.setNum(nextId, 16);
    mCurrentIdHex = mCurrentIdHex.left(mCurrentIdHex.length()-nextIdHex.length());
    mCurrentIdHex.append(nextIdHex);
}

