#include "feitiancardreadermanager.hpp"

#include <QDate>
#include <QTime>

#include <QDomDocument>

static const QString YYYY_MM_DD_HH_MM_SS = "yyyy-MM-dd HH:mm:ss";

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
// ATR found for Elektronische Gesundheitskarte Generation 2:

// German public health insurance card (Elektronische Gesundheitskarte eGK), 2nd generation (G2) (HealthCare)
static const QString ATR_EGK_G2_01 = "3bd396ff81b1fe451f078081052d";

// German Elektronische Gesundheitskarte (eGK) (HealthCare) - From TK (HealthCare)
static const QString ATR_EGK_G2_02 = "3bd097ff81b1fe451f072b";

// German public health insurance card („Gesundheitskarte“), 2nd generation (G2), issuer Techniker Krankenkasse (HealthCare)
static const QString ATR_EGK_G2_03 = "3bd096ff81b1fe451f072a";

// New european health insurance card of the German health insurance (G2) (HealthCare)
static const QString ATR_EGK_G2_04 = "3bd096ff81b1fe451f032e";


static const QString PARSE_ATR = "https://smartcard-atr.appspot.com/parse?ATR=";

static const QString THREE_BYTE_FILLER = "000000";
static const QString THREE_BYTE_FILLER_ONE = "000001";
static const QString THREE_BYTE_FILLER_TWO = "000002";
// dont know what this data means
static const QString UNKNOWN_NEXT_BINARY = "001000";

// last 3 Bytes of Metadata if problem with the command
static const QString COMMAND_NOT_EXECUTED_41 = "41fe00";
static const QString COMMAND_NOT_EXECUTED_40 = "40fe00";


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
        emit readATRWrong(tr("Received buffer (%1) too short for a valid ATR.\nWe need 10 Bytes MetaData plus ATR").arg(mCurrentData.length()/2),"");
        resetCommand();
        mRetryCommand = 0;
        return;
    }
    QString responseType = mCurrentData.left(6);
    QString id = mCurrentData.mid(6,8);
    if(id != mCurrentIdHex) {
        qWarning() << "ID seems to be wrong: " << id << " instead of " << mCurrentIdHex;
        // we ignore this in our demo app
    }
    QString unknownFiller = mCurrentData.mid(14,6);
    if(mCurrentData.length() == 20) {
        qWarning() << "response data should be more then 20 Bytes and not " << mCurrentData.length()/2;
        // sometimes there's a timing problem - esp Cards with G2.1
        // so we retry 5 times
        if(mCurrentData.right(6) == COMMAND_NOT_EXECUTED_40 || mCurrentData.right(6) == COMMAND_NOT_EXECUTED_41) {
            mRetryCommand ++;
            if(mRetryCommand <=5) {
                doPowerOn();
                return;
            }
        } else {
            qDebug() << "Process Power ON 3 Bytes at the end: " << mCurrentData.right(6);
        }
        emit readATRWrong(tr("Received buffer (%1) too short for a valid ATR.\nBuffer contains only MetaData (10 Bytes) and no ATR").arg(mCurrentData.length()/2),"");
        resetCommand();
        mRetryCommand = 0;
        return;
    }

    QString payload = mCurrentData.right(mCurrentData.length()-20);

    // now it's safe to reset the command vars
    mRetryCommand = 0;
    resetCommand();

    // go on
    qDebug() << "processing Power On. response type: " << responseType << " ID: " << id << " Filler: " << unknownFiller;
    qDebug() << "Payload (ATR): " << payload;
    if(payload == ATR_EGK_G2_01 || payload == ATR_EGK_G2_02 || payload == ATR_EGK_G2_03 || payload == ATR_EGK_G2_04) {
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
        if(mCurrentData.length() == 20) {
            // sometimes there's a timing problem - esp Cards with G2.1
            // so we retry 5 times
            if(mCurrentData.right(6) == COMMAND_NOT_EXECUTED_40 || mCurrentData.right(6) == COMMAND_NOT_EXECUTED_41) {
                mRetryCommand ++;
                if(mRetryCommand <=5) {
                    doSelectFile();
                    return;
                }
            }
        }
        qDebug() << "Process Select Files 3 Bytes at the end: " << mCurrentData.right(6);
        emit appSelectedFailed(tr("Received buffer not 12 Bytes for a valid Select File response"),"","");
        mRetryCommand = 0;
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
    mRetryCommand = 0;
    resetCommand();

    // go on
    qDebug() << "processing Select File. response type: " << responseType << " ID: " << id << " Filler: " << unknownFiller;
    qDebug() << "Payload: " << payload;
    if(payload == APDU_COMMAND_SUCCESSFULLY_EXECUTED) {
        emit appSelectedSuccess();
        // do the next step
        doReadBinaryStatusVD();
        return;
    }
    emit appSelectedFailed(tr("Cannot select the File on eGK - wrong Response Code"), APDU_RESPONSE_INFO_URL, payload);
}

void FeitianCardReaderManager::doReadBinaryStatusVD()
{
    qDebug() << "write to Feitian CardReader ReadBinary StatusVD APDU:";
    // reset commands if something left
    resetCommand();
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_READ_BINARY;
    theCommand.append(mCurrentIdHex);
    theCommand.append(THREE_BYTE_FILLER);
    theCommand.append(APDU_READ_BINARY_STATUS_VD);
    // set running command
    mRunningCommand = COMMAND_READ_BINARY;
    mRunningAPDU = APDU_READ_BINARY_STATUS_VD;
    // Write to CardReader
    mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
}

void FeitianCardReaderManager::processReadBinaryStatusVD(const QString& hexData)
{
    if(hexData.length() == 40) {
        mCurrentData += hexData;
        // wait for more
        return;
    }
    mCurrentData += hexData;
    if(mCurrentData.length() != 74) {
        qWarning() << "response data should be 37 bytes and not " << mCurrentData.length()/2;
        if(mCurrentData.length() == 20) {
            // sometimes there's a timing problem - esp Cards with G2.1
            // so we retry 6 times
            if(mCurrentData.right(6) == COMMAND_NOT_EXECUTED_40 || mCurrentData.right(6) == COMMAND_NOT_EXECUTED_41) {
                mRetryCommand ++;
                if(mRetryCommand <=5) {
                    doReadBinaryStatusVD();
                    return;
                }
            }
        }
        qDebug() << "Process Read Status VD 3 Bytes at the end: " << mCurrentData.right(6);
        emit statusVDFailed(tr("Received buffer not 37 Bytes for a valid Read Binary StatusVD response"),"","");
        resetCommand();
        mRetryCommand = 0;
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
    mRetryCommand = 0;
    resetCommand();

    // go on
    qDebug() << "processing Read Binary StatsVD. response type: " << responseType << " ID: " << id << " Filler: " << unknownFiller;
    qDebug() << "Payload: " << payload;

    if(payload.length() != 54) {
        qWarning() << "read statusVD payload should be 54, but was " << payload.length();
        emit statusVDFailed(tr("Cannot Read StatusVD on eGK - wrong Response Data length."), APDU_RESPONSE_INFO_URL, payload);
        return;
    }
    QString responseStatus = payload.mid(50,4);
    if(responseStatus == APDU_COMMAND_SUCCESSFULLY_EXECUTED) {
        QVariantMap statusVDMap;
        QString status = QByteArray::fromHex(payload.left(2).toLocal8Bit());

        QString year = QByteArray::fromHex(payload.mid(2,8).toLocal8Bit());
        QString month = QByteArray::fromHex(payload.mid(10,4).toLocal8Bit());
        QString day = QByteArray::fromHex(payload.mid(14,4).toLocal8Bit());
        QString hour = QByteArray::fromHex(payload.mid(18,4).toLocal8Bit());
        QString minutes = QByteArray::fromHex(payload.mid(22,4).toLocal8Bit());
        QString seconds = QByteArray::fromHex(payload.mid(26,4).toLocal8Bit());

        int version1 = payload.mid(30,3).toInt();
        int version2 = payload.mid(33,3).toInt();
        int version3 = payload.mid(36,4).toInt();

        QString notUsed = payload.mid(40,10);

        statusVDMap.insert("Status", status.toInt());
        qDebug() << "status: " << status << " value: " << status.toInt();

        qDebug() << "timestamp hex: " << year << "-" << month <<"-" << day << " " << hour << ":" << minutes << ":" << seconds;
        QDateTime timeStamp = QDateTime(QDate(year.toInt(), month.toInt(), day.toInt()), QTime(hour.toInt(), minutes.toInt(), seconds.toInt()));
        statusVDMap.insert("Timestamp", timeStamp.toString(YYYY_MM_DD_HH_MM_SS));
        qDebug() << "Timestamp " << timeStamp.toString(YYYY_MM_DD_HH_MM_SS);

        QString version = QString::number(version1)+"."+QString::number(version2)+"."+QString::number(version3);
        statusVDMap.insert("Version", version);
        qDebug() << "Version: " << version;

        qDebug() << "StatusVDMap: " << statusVDMap;

        emit statusVDSuccess(statusVDMap);
        // do the next step
        doReadBinaryPersonalData();
        return;
    }
    emit statusVDFailed(tr("Cannot select the File on eGK - wrong Response Code"), APDU_RESPONSE_INFO_URL, responseStatus);
}


void FeitianCardReaderManager::doReadBinaryPersonalData()
{
    qDebug() << "write to Feitian CardReader ReadBinary PersonalData APDU:";
    // reset commands if something left
    resetCommand();
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_READ_BINARY;
    theCommand.append(mCurrentIdHex);
    theCommand.append(THREE_BYTE_FILLER);
    theCommand.append(APDU_READ_BINARY_PERSONAL_DATA);
    // set running command
    mRunningCommand = COMMAND_READ_BINARY;
    mRunningAPDU = APDU_READ_BINARY_PERSONAL_DATA;
    //
    mFirstResponseProcessed = false;
    // Write to CardReader
    mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
}

void FeitianCardReaderManager::doReadBinaryNext()
{
    qDebug() << "write next part of data to Feitian CardReader ReadBinary PersonalData APDU:";
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_READ_BINARY_NEXT;
    theCommand.append(mCurrentIdHex);
    theCommand.append(UNKNOWN_NEXT_BINARY);
    // Write to CardReader
    mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
}

void FeitianCardReaderManager::processReadBinaryPersonalData(const QString& hexData)
{
    if(!mFirstResponseProcessed) {
        if(hexData.length() != 40) {
            qWarning() << "Process read PD something went wrong. first part of data always must be 20 Bytes (40), but was " << hexData.length();
            if(hexData.length() == 20) {
                qDebug() << "Process Read PD 3 Bytes at the end: " << mCurrentData.right(6);
                // perhaps retry
            }
            return;
        }
        // calculate the length
        QString expectedBytesHex = hexData.mid(20,4);
        bool ok;
        mExpectedBytes = expectedBytesHex.toInt(&ok,16);
        mFirstResponseProcessed = true;
        // before getting data from first chunk we clear the list
        mCurrentDataChunksList.clear();
        // response Type 3 Bytes
        // ID 4 Bytes
        // filler 3 Bytes (000001)
        // expected length 2 Bytes
        mFirstPrefixBytes = 12;
        // response Type 3 Bytes
        // ID 4 Bytes
        // filler 3 Bytes (000002)
        mNextPrefixBytes = 10;
        // Status per ex 90 00 == success
        mLastPostfixBytes = 2;
        //
        mRemainingBytes = mExpectedBytes + mLastPostfixBytes;
        mReceivedBytes = 0;
        //
        mChunkSizeBytes = 271;
    } // first response

    if(hexData.length() == 40) {
        mCurrentData += hexData;
        // wait for more
        // go on it's the same chunk
        return;
    }
    // concatenate the last row of this chunk
    mCurrentData += hexData;
    // now add to the list
    mCurrentDataChunksList.append(mCurrentData);

    if(mCurrentDataChunksList.size() == 1) {
        // the very first chunk
        mReceivedBytes = mCurrentData.length()/2 - mFirstPrefixBytes;
    } else {
        // we already have more chunks
        mReceivedBytes += (mCurrentData.length()/2 - mNextPrefixBytes);
    }

    QString responseStatus;

    mRemainingBytes = mExpectedBytes + mLastPostfixBytes - mReceivedBytes;
    if(mRemainingBytes > 0) {
        // we need more
        // read next part
        // reset chunk
        mCurrentData.clear();
        // go on
        doReadBinaryNext();
        return;
    }

    // now we have it all
    qDebug() << "PD: received data Bytes " << mReceivedBytes << "including postfix " << mLastPostfixBytes << " expected: "<< mExpectedBytes;

    // create the XML payload
    QString xmlPayload;
    for (int i = 0; i < mCurrentDataChunksList.size(); ++i) {
        QString chunkData = mCurrentDataChunksList.at(i);
        if(i == 0) {
            // the first one
            xmlPayload.append(chunkData.right(chunkData.length()-mFirstPrefixBytes*2));
        } else {
            // the second or more
            xmlPayload.append(chunkData.right(chunkData.length()-mNextPrefixBytes*2));
        }
        if(i == mCurrentDataChunksList.size()-1) {
            // the last one: get the response status and remove from xml
            responseStatus = chunkData.right(4);
            xmlPayload = xmlPayload.left(xmlPayload.length()-4);
        }
    } // loop thru chunks
    if(responseStatus != APDU_COMMAND_SUCCESSFULLY_EXECUTED) {
        emit personalDataFailed(tr("Read Person Data no success."), APDU_RESPONSE_INFO_URL, responseStatus);
        resetCommand();
        return;
    }

    // now it's safe to reset the command vars
    resetCommand();

    QVariantMap pdMap;
    qDebug() << "PD: XML Bytes: " << xmlPayload.length()/2;

    // first 10 Bytes header of gzip
    // First byte : ID1 = 0x1F
    // Secound byte: ID2 = 0x8B
    // Third byte: CM - compression method: 1-7 reserved - 0x08 == DEFLATE
    // bytes 4-10 extra flags, like file name, comments, CRC16, etc.. all are 0
    qDebug() << "GZIP First 10 Bytes. ID1 (0x1F) ID2 (0x8B) Compression (0x08 == DEFLATE)" << xmlPayload.left(20);
    // last 4 Bytes are size (in reverse order)
    // per ex 6a020000 --> 0000026a = 618 bytes

    // qUncompress only works for zlib
    // now trying solution from StackOverflow

    QByteArray uncompressedXML = gUncompress(QByteArray::fromHex(xmlPayload.toLatin1()));
    // qDebug() << "XML ???" << uncompressedXML;

    // now parse uncrompessed XML
    QString xmlErrorMessage;
    QDomDocument xmlDoc;
    xmlDoc.setContent(uncompressedXML);
    QDomElement root=xmlDoc.documentElement();
    // "UC_PersoenlicheVersichertendatenXML"
    qDebug() << "xml root: " << root.tagName();

    QDomNode versicherter = root.firstChild();
    // "Versicherter"
    qDebug() << "xml versicherter: " << versicherter.nodeName();
    if(versicherter.isNull()) {
        xmlErrorMessage = "Node Versicherter not found in XML document";
    } else {
        QDomElement versichertenId = versicherter.firstChildElement("Versicherten_ID");
        if(versichertenId.isNull()) {
            pdMap.insert("Versicherten_ID","");
        } else {
            pdMap.insert("Versicherten_ID", versichertenId.text());
        }

        QDomNodeList versicherterNodes = versicherter.childNodes();
        for (int i = 0; i < versicherterNodes.size(); ++i) {
            QDomNode versicherterNode = versicherterNodes.at(i);
            qDebug() << "Versicherter Nodes: " << versicherterNode.nodeName();
            if(versicherterNode.nodeName() == "Person") {
                qDebug() << "xml person: " << versicherterNode.nodeName();

                QDomNodeList personNodes = versicherterNode.childNodes();
                for (int p = 0; p < personNodes.size(); ++p) {
                    QDomNode personNode = personNodes.at(p);
                    QDomElement personNodeElement = personNode.toElement();
                    if(!personNodeElement.isNull()) {
                        if(personNodeElement.tagName() == "StrassenAdresse") {
                            qDebug() << "PersonNode StrassenAdresse";
                            QDomNodeList streetNodes = personNode.childNodes();
                            for (int s = 0; s < streetNodes.size(); ++s) {
                                QDomNode streetNode = streetNodes.at(s);
                                QDomElement streetNodeElement = streetNode.toElement();
                                if(!streetNodeElement.isNull()) {
                                    if(streetNodeElement.tagName() == "Land") {
                                        qDebug() << "StreetNode Land";
                                        QDomNodeList landNodes = streetNode.childNodes();
                                        for (int l = 0; l < landNodes.size(); ++l) {
                                            QDomNode landNode = landNodes.at(l);
                                            QDomElement landNodeElement = landNode.toElement();
                                            if(!landNodeElement.isNull()) {
                                                // a normal element of Land
                                                qDebug() << "LandNode Elements " << landNodeElement.tagName() << " value: " << landNodeElement.text();
                                                pdMap.insert(landNodeElement.tagName(),landNodeElement.text());
                                            }
                                        }
                                    } else {
                                        // a normal element of Street
                                        qDebug() << "StreetNode Elements " << streetNodeElement.tagName() << " value: " << streetNodeElement.text();
                                        pdMap.insert(streetNodeElement.tagName(),streetNodeElement.text());
                                    }
                                }
                            } // street Nodes

                        } else if(personNodeElement.tagName() == "PostfachAdresse") {
                            qDebug() << "PersonNode PostfachAdresse not implemented";
                        } else {
                            // a normal element of Person
                            qDebug() << "PersonNode Elements " << personNodeElement.tagName() << " value: " << personNodeElement.text();
                            pdMap.insert(personNodeElement.tagName(),personNodeElement.text());
                        }
                    }
                } // Person Nodes

            }// Person
        } // Versicherter Nodes
    } // versicherter

    emit personalDataSuccess(pdMap);
    // do the next step
    doReadBinaryInsuranceData();
}



void FeitianCardReaderManager::doReadBinaryInsuranceData()
{
    qDebug() << "write to Feitian CardReader ReadBinary InsuranceData APDU:";
    // reset commands if something left
    resetCommand();
    // calculate next ID
    calculateNextIdHex();
    // construct the command
    QString theCommand = COMMAND_READ_BINARY;
    theCommand.append(mCurrentIdHex);
    theCommand.append(THREE_BYTE_FILLER);
    theCommand.append(APDU_READ_BINARY_INSURANCE_DATA);
    // set running command
    mRunningCommand = COMMAND_READ_BINARY;
    mRunningAPDU = APDU_READ_BINARY_INSURANCE_DATA;
    //
    mFirstResponseProcessed = false;
    // Write to CardReader
    mCardService->writeCharacteristicAsHex(mWriteData, theCommand, false);
}

void FeitianCardReaderManager::processReadBinaryInsuranceData(const QString& hexData)
{
    if(!mFirstResponseProcessed) {
        if(hexData.length() != 40) {
            qWarning() << "Process read VD something went wrong. first part of data always must be 20 Bytes (40), but was " << hexData.length();
            if(hexData.length() == 20) {
                qDebug() << "Process Read VD 3 Bytes at the end: " << mCurrentData.right(6);
                // perhaps retry
            }
            return;
        }
        // calculate the length
        bool ok;
        QString offset = hexData.mid(20,4);
        mOffsetStartVD = offset.toInt(&ok,16);
        offset = hexData.mid(24,4);
        mOffsetEndVD = offset.toInt(&ok,16);
        offset = hexData.mid(28,4);
        if(offset.toUpper() == "FFFF") {
            mOffsetStartGVD = 0;
            mOffsetEndGVD = 0;
        } else {
            mOffsetStartGVD = offset.toInt(&ok,16);
            offset = hexData.mid(32,4);
            mOffsetEndGVD = offset.toInt(&ok,16);
        }
        // if there are no GVD, then start and end Offset is "ffff"
        if(mOffsetEndGVD > 0) {
            mExpectedBytes = mOffsetEndGVD - 8 + 1;
        } else {
            mExpectedBytes = mOffsetEndVD - 8 + 1;
        }

        mFirstResponseProcessed = true;
        // before getting data from first chunk we clear the list
        mCurrentDataChunksList.clear();
        // response Type 3 Bytes
        // ID 4 Bytes
        // filler 3 (000001)
        // offset length 8 Bytes (start VD 2 Bytes, end VD 2 Bytes, start GVD 2 Bytes, end GVD 2 Bytes)
        mFirstPrefixBytes = 18;
        // response Type 3 Bytes
        // ID 4 Bytes
        // filler 3 Bytes (000002)
        mNextPrefixBytes = 10;
        // Status per ex 90 00 == success
        mLastPostfixBytes = 2;
        //
        mRemainingBytes = mExpectedBytes + mLastPostfixBytes;
        mReceivedBytes = 0;
        //
        mChunkSizeBytes = 271;
    } // first response

    if(hexData.length() == 40) {
        mCurrentData += hexData;
        // wait for more
        // go on it's the same chunk
        return;
    }

    // concatenate the last row of this chunk
    mCurrentData += hexData;
    // now add to the list
    mCurrentDataChunksList.append(mCurrentData);

    if(mCurrentDataChunksList.size() == 1) {
        // the very first chunk
        mReceivedBytes = mCurrentData.length()/2 - mFirstPrefixBytes;
    } else {
        // we already have more chunks
        mReceivedBytes += (mCurrentData.length()/2 - mNextPrefixBytes);
    }

    QString responseStatus;

    mRemainingBytes = mExpectedBytes + mLastPostfixBytes - mReceivedBytes;
    qDebug() << "VD + GVD: received: " << mReceivedBytes << " remaining: " << mRemainingBytes;
    if(mRemainingBytes > 0) {
        // we need more
        // read next part
        // reset chunk
        mCurrentData.clear();
        // go on
        doReadBinaryNext();
        return;
    }

    // now we have it all
    qDebug() << "VD + GVD: received data Bytes " << mReceivedBytes << "including postfix " << mLastPostfixBytes << " expected: "<< mExpectedBytes;
    qDebug() << "VD + GVD offsets: start VD: " << mOffsetStartVD << " end VD: " << mOffsetEndVD << " start GVD: " << mOffsetStartGVD << " end GVD: " << mOffsetEndGVD;

    // create the XML payloads
    QString xmlPayloads;
    for (int i = 0; i < mCurrentDataChunksList.size(); ++i) {
        QString chunkData = mCurrentDataChunksList.at(i);
        if(i == 0) {
            // the first one
            xmlPayloads.append(chunkData.right(chunkData.length()-mFirstPrefixBytes*2));
        } else {
            // the second or more
            xmlPayloads.append(chunkData.right(chunkData.length()-mNextPrefixBytes*2));
        }
        if(i == mCurrentDataChunksList.size()-1) {
            // the last one: get the response status and remove from xml
            responseStatus = chunkData.right(4);
            xmlPayloads = xmlPayloads.left(xmlPayloads.length()-4);
        }
    } // loop thru chunks
    if(responseStatus != APDU_COMMAND_SUCCESSFULLY_EXECUTED) {
        emit insuranceDataFailed(tr("Read Insurance Data no success."), APDU_RESPONSE_INFO_URL, responseStatus);
        resetCommand();
        return;
    }

    // now it's safe to reset the command vars
    resetCommand();

    QVariantMap vdMap;
    QVariantMap gvdMap;
    int xmlPayloadVDLength = 0;
    int xmlPayloadGVDLength = 0;
    QString xmlPayloadVD;
    QString xmlPayloadGVD;
    // Attention there could be some filler / space / unknown between VD and GVD
    xmlPayloadVDLength = (mOffsetEndVD-mOffsetStartVD+1) *2;
    if(mOffsetStartGVD > 0) {
        xmlPayloadGVDLength = (mOffsetEndGVD-mOffsetStartGVD+1)*2;
        qDebug() << "VD: XML Bytes VD: " << xmlPayloadVDLength/2 << " GVD: " << xmlPayloadGVDLength/2;
        xmlPayloadVD = xmlPayloads.left(xmlPayloadVDLength);
        xmlPayloadGVD = xmlPayloads.right(xmlPayloadGVDLength);
    } else {
        xmlPayloadVD = xmlPayloads.left(xmlPayloadVDLength);
        qDebug() << "VD: XML Bytes VD: " << xmlPayloadVDLength/2 << " GVD: EMPTY";
    }

    // first 10 Bytes header of gzip
    // First byte : ID1 = 0x1F
    // Secound byte: ID2 = 0x8B
    // Third byte: CM - compression method: 1-7 reserved - 0x08 == DEFLATE
    // bytes 4-10 extra flags, like file name, comments, CRC16, etc.. all are 0
    qDebug() << "VD: GZIP First 10 Bytes. ID1 (0x1F) ID2 (0x8B) Compression (0x08 == DEFLATE)" << xmlPayloadVD.left(20);
    if(xmlPayloadGVDLength > 0) {
        qDebug() << "GVD: GZIP First 10 Bytes. ID1 (0x1F) ID2 (0x8B) Compression (0x08 == DEFLATE)" << xmlPayloadGVD.left(20);
    }
    // last 4 Bytes are size (in reverse order)
    // per ex 6a020000 --> 0000026a = 618 bytes

    // qUncompress only works for zlib
    // now trying solution from StackOverflow

    QByteArray uncompressedXMLVD = gUncompress(QByteArray::fromHex(xmlPayloadVD.toLatin1()));
    // qDebug() << "VD XML ???" << uncompressedXMLVD;
    QByteArray uncompressedXMLGVD = gUncompress(QByteArray::fromHex(xmlPayloadGVD.toLatin1()));
    // qDebug() << "VD XML ???" << uncompressedXMLGVD;




}


// https://stackoverflow.com/questions/2690328/qt-quncompress-gzip-data
// https://stackoverflow.com/a/7351507
QByteArray FeitianCardReaderManager::gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
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
         mRetryCommand = 0;
         // only to display the data for test or log purpose
         emit cardDataValueChanged();
         // Card is inserted
         emit cardIN();
         return;
     }
     if(hexValue == CARD_STATE_OUT) {
         mRetryCommand = 0;
         // only to display the data for test or log purpose
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
            processReadBinaryStatusVD(hexValue);
            return;
        }
        if(mRunningAPDU == APDU_READ_BINARY_PERSONAL_DATA) {
            processReadBinaryPersonalData(hexValue);
            return;
        }
        if(mRunningAPDU == APDU_READ_BINARY_INSURANCE_DATA) {
            processReadBinaryInsuranceData(hexValue);
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

