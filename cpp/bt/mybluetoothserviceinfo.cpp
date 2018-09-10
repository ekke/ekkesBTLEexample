#include "mybluetoothserviceinfo.hpp"
#include <QTimer>


MyBluetoothServiceInfo::MyBluetoothServiceInfo(QObject *parent) : QObject(parent)
{
    //
}

void MyBluetoothServiceInfo::setService(QLowEnergyService *service)
{
    mLowEnergyService = service;
    mLowEnergyService->setParent(this);
    // D I S C O V E R
    connect(mLowEnergyService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            this, SLOT(serviceDetailsDiscovered(QLowEnergyService::ServiceState)));
    // R E A D
    connect(mLowEnergyService, &QLowEnergyService::characteristicRead, this, &MyBluetoothServiceInfo::onCharacteristicRead);
    // W R I T E
    connect(mLowEnergyService, &QLowEnergyService::characteristicWritten, this, &MyBluetoothServiceInfo::onCharacteristicWritten);
    // N O T I F I C A T I O N
    connect(mLowEnergyService, &QLowEnergyService::characteristicChanged, this, &MyBluetoothServiceInfo::onCharacteristicChanged);
    // E R R O R
    connect(mLowEnergyService, SIGNAL(error(QLowEnergyService::ServiceError)),
            this, SLOT(onServiceError(QLowEnergyService::ServiceError)));
}

QLowEnergyService *MyBluetoothServiceInfo::service() const
{
    return mLowEnergyService;
}

QString MyBluetoothServiceInfo::getUuid() const
{
    if (!mLowEnergyService) {
        return QString();
    }

    const QBluetoothUuid uuid = mLowEnergyService->serviceUuid();
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success) {
        return QStringLiteral("0x") + QString::number(result32, 16);
    }
    return uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString MyBluetoothServiceInfo::getName() const
{
    if (!mLowEnergyService) {
        return QString();
    }
    if(mLowEnergyService->serviceName() == "Unknown Service") {
        return "Custom Service";
    }

    return mLowEnergyService->serviceName();
}

QString MyBluetoothServiceInfo::getType() const
{
    if (!mLowEnergyService)
        return QString();

    QString result;
    if (mLowEnergyService->type() & QLowEnergyService::PrimaryService)
        result += QStringLiteral("primary");
    else
        result += QStringLiteral("secondary");

    if (mLowEnergyService->type() & QLowEnergyService::IncludedService)
        result += QStringLiteral(" included");

    result.prepend('<').append('>');

    return result;
}

QVariant MyBluetoothServiceInfo::getCharacteristics()
{
    return QVariant::fromValue(mCharacteristics);
}

void MyBluetoothServiceInfo::clearCharacteristics()
{
    for (int i = 0; i < mCharacteristics.size(); ++i) {
        MyBluetoothCharacteristic* characteristic = (MyBluetoothCharacteristic*)mCharacteristics.at(i);
        characteristic->clearDescriptors();
    }

    qDeleteAll(mCharacteristics);
    mCharacteristics.clear();
    emit characteristicsUpdated();
}

bool MyBluetoothServiceInfo::hasCharacteristics()
{
    return mLowEnergyService->state() == QLowEnergyService::ServiceDiscovered;
}

MyBluetoothCharacteristic *MyBluetoothServiceInfo::getCharacteristicInfo(const QString uuid)
{
    for (int i = 0; i < mCharacteristics.size(); ++i) {
        MyBluetoothCharacteristic* myCharacteristic = (MyBluetoothCharacteristic*)mCharacteristics.at(i);
        qDebug() << "serach " << uuid << "contains Characteristic: " << myCharacteristic->getUuid();
        if(myCharacteristic->getUuid() == uuid) {
            qDebug() << "found Chracteristic";
            return myCharacteristic;
        }
    }
    qDebug() << "nothing found - return NULL";
    return nullptr;
}

void MyBluetoothServiceInfo::addAlreadyDiscoveredCharacteristics()
{
    const QList<QLowEnergyCharacteristic> chars = mLowEnergyService->characteristics();
    foreach (const QLowEnergyCharacteristic &ch, chars) {
        MyBluetoothCharacteristic *cInfo = new MyBluetoothCharacteristic();
        cInfo->setCharacteristic(ch);
        mCharacteristics.append(cInfo);
    }

    QTimer::singleShot(0, this, SIGNAL(characteristicsUpdated()));
    QTimer::singleShot(0, this, SIGNAL(characteristicsDone()));
}

void MyBluetoothServiceInfo::connectToService()
{
    if (!mLowEnergyService) {
        qWarning() << "connectToService - MyBluetoothServiceInfo has no QLowEnergyService";
        return;
    }
    clearCharacteristics();
    // already discovered ?
    if (mLowEnergyService->state() == QLowEnergyService::ServiceDiscovered) {
        qDebug() << "Service details discovery already done";
        addAlreadyDiscoveredCharacteristics();
        return;
    }
    // discovery required ?
    if (mLowEnergyService->state() == QLowEnergyService::DiscoveryRequired) {
        qDebug() << "Service details discovery required";
        mLowEnergyService->discoverDetails();
        return;
    }
    if (mLowEnergyService->state() == QLowEnergyService::DiscoveringServices) {
        qDebug() << "Service is discovering details " << service()->state();
        emit serviceDiscovering();
        return;
    }
    // service invalid !!
    qDebug() << "Service is invalid: " << service()->state();
    emit serviceInvalid();
}

void MyBluetoothServiceInfo::readCharacteristic(MyBluetoothCharacteristic *myCharacteristic)
{
    if(!myCharacteristic) {
        qWarning() << "readCharacteristic MyBluetoothCharacteristic is NULL";
        // emit something
        return;
    }
    mLowEnergyService->readCharacteristic(myCharacteristic->getCharacteristic());

}

void MyBluetoothServiceInfo::writeCharacteristicAsString(MyBluetoothCharacteristic *myCharacteristic, QString valueString, bool withResponse)
{
    if(!myCharacteristic) {
        qWarning() << "writeCharacteristicAsString MyBluetoothCharacteristic is NULL";
        // emit something
        return;
    }

    QLowEnergyCharacteristic characteristic = myCharacteristic->getCharacteristic();
    if(!characteristic.isValid()) {
        qWarning() << "Write Characteristic not valid";
        // emit something
        return;
    }
    QByteArray valueArray = valueString.toLatin1();
    int permission = characteristic.properties();
    if(withResponse) {
        if((!permission) & QLowEnergyCharacteristic::Write) {
            qWarning() << "Write String Characteristic: no Response possible";
            // emit something
            return;
        }
        qDebug() << "write with response: [" << valueArray << "]";
        mLowEnergyService->writeCharacteristic(characteristic, valueArray, QLowEnergyService::WriteWithResponse);
    } else {
        if((!permission) & QLowEnergyCharacteristic::WriteNoResponse) {
            qWarning() << "Write Characteristic: not possible without response";
            // emit something
            return;
        }
        qDebug() << "write String with no response: [" << valueArray << "]";
        mLowEnergyService->writeCharacteristic(characteristic, valueArray, QLowEnergyService::WriteWithoutResponse);
    }
}

void MyBluetoothServiceInfo::writeCharacteristicAsHex(MyBluetoothCharacteristic *myCharacteristic, QString hexString, bool withResponse)
{
    if(!myCharacteristic) {
        qWarning() << "writeCharacteristicAsHex MyBluetoothCharacteristic is NULL";
        // emit something
        return;
    }

    QLowEnergyCharacteristic characteristic = myCharacteristic->getCharacteristic();
    if(!characteristic.isValid()) {
        qWarning() << "Write Characteristic not valid";
        // emit something
        return;
    }
    // http://stackoverflow.com/questions/26619420/construct-a-qbytearray-from-a-hex-value-entered-as-a-qstring
    // hexString is without '0x'
    QString myHex;
    if(hexString.startsWith("0x")) {
        myHex = hexString.mid(2);
    } else {
        myHex = hexString;
    }
    QByteArray valueArray = QByteArray::fromHex(myHex.toLatin1());
    qDebug() << "hexString " << hexString << " | myHex:" << myHex << " as HEX ByteArray: " << valueArray.toHex();
    int permission = characteristic.properties();
    if(withResponse) {
        if((!permission) & QLowEnergyCharacteristic::Write) {
            qWarning() << "Write Characteristic: no Response possible";
            // emit something
            return;
        }
        qDebug() << "write HEX with response: [" << valueArray << "]";
        mLowEnergyService->writeCharacteristic(characteristic, valueArray, QLowEnergyService::WriteWithResponse);
    } else {
        if((!permission) & QLowEnergyCharacteristic::WriteNoResponse) {
            qWarning() << "Write Characteristic: not possible without response";
            // emit something
            return;
        }
        qDebug() << "write HEX with no response: [" << valueArray << "]";
        mLowEnergyService->writeCharacteristic(characteristic, valueArray, QLowEnergyService::WriteWithoutResponse);
    }
}

void MyBluetoothServiceInfo::subscribeNotifications(MyBluetoothCharacteristic *myCharacteristic)
{
    if(!myCharacteristic) {
        qWarning() << "subscribeNotifications MyBluetoothCharacteristic is NULL";
        // emit something
        return;
    }

    QLowEnergyDescriptor myDescriptor = myCharacteristic->getCharacteristic().descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if(!myDescriptor.isValid()) {
        qWarning() << "subscribeNotifications Descriptor not valid";
        // emit something
        return;
    }
    mLowEnergyService->writeDescriptor(myDescriptor, QByteArray::fromHex("0100"));
    myCharacteristic->setNotifyIsRunning(true);
    qDebug() << "subscribeNotifications " << myCharacteristic->getName();
}

void MyBluetoothServiceInfo::unSubscribeNotifications(MyBluetoothCharacteristic *myCharacteristic)
{
    if(!myCharacteristic) {
        qWarning() << "unSubscribeNotifications MyBluetoothCharacteristic is NULL";
        // emit something
        return;
    }

    QLowEnergyDescriptor myDescriptor = myCharacteristic->getCharacteristic().descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if(!myDescriptor.isValid()) {
        qWarning() << "unSubscribeNotifications Descriptor not valid";
        // emit something
        return;
    }
    mLowEnergyService->writeDescriptor(myDescriptor, QByteArray::fromHex("0000"));
    myCharacteristic->setCurrentValue(QByteArray());
    myCharacteristic->setNotifyIsRunning(false);
    qDebug() << "unSubscribeNotifications " << myCharacteristic->getName();
}

void MyBluetoothServiceInfo::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    if (newState != QLowEnergyService::ServiceDiscovered) {
        // do not hang in "Scanning for characteristics" mode forever
        // in case the service discovery failed
        // We have to queue the signal up to give UI time to even enter
        // the above mode
        if (newState != QLowEnergyService::DiscoveringServices) {
            QMetaObject::invokeMethod(this, "characteristicsUpdated",
                                      Qt::QueuedConnection);
            QMetaObject::invokeMethod(this, "characteristicsDone",
                                      Qt::QueuedConnection);
        }
        return;
    }

    const QList<QLowEnergyCharacteristic> chars = mLowEnergyService->characteristics();
    foreach (const QLowEnergyCharacteristic &ch, chars) {
        MyBluetoothCharacteristic *cInfo = new MyBluetoothCharacteristic(this);
        cInfo->setCharacteristic(ch);
        mCharacteristics.append(cInfo);
    }
    emit characteristicsUpdated();
    emit characteristicsDone();
}

void MyBluetoothServiceInfo::onCharacteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << characteristic.name() << " READ value " << value;
    for (int i = 0; i < mCharacteristics.size(); ++i) {
        MyBluetoothCharacteristic* myCharacteristic = (MyBluetoothCharacteristic*)mCharacteristics.at(i);
        if(myCharacteristic->getCharacteristic() == characteristic) {
            qDebug() << "(READ VALUE) my characteristic found ";
            myCharacteristic->setCurrentValue(value);
        }
    }
}

// we only get this for QLowEnergyCharacteristic::Write
// not for QLowEnergyCharacteristic::WriteNoResponse (also no ERROR reported)
void MyBluetoothServiceInfo::onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << characteristic.name() << " WRITTEN value " << value;
}

void MyBluetoothServiceInfo::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << characteristic.name() << " NOTIFIED value change " << value;
    for (int i = 0; i < mCharacteristics.size(); ++i) {
        MyBluetoothCharacteristic* myCharacteristic = (MyBluetoothCharacteristic*)mCharacteristics.at(i);
        if(myCharacteristic->getCharacteristic() == characteristic) {
            // qDebug() << "(NOTIFIED UPDATED VALUE) my characteristic found ";
            myCharacteristic->setCurrentValue(value);
        }
    }
}

void MyBluetoothServiceInfo::onServiceError(const QLowEnergyService::ServiceError &newError)
{
    QString errorMessage;
    switch (newError) {
    case QLowEnergyService::OperationError:
        qDebug() << "onServiceError: OperationError";
        emit operationError();
        return;
    case QLowEnergyService::CharacteristicReadError:
        qDebug() << "onServiceError: CharacteristicReadError";
        emit characteristicReadError();
        return;
    case QLowEnergyService::CharacteristicWriteError:
        qDebug() << "onServiceError: CharacteristicWriteError";
        emit characteristicWriteError();
        return;
    case QLowEnergyService::DescriptorReadError:
        qDebug() << "onServiceError: DescriptorReadError";
        emit descriptorReadError();
        return;
    case QLowEnergyService::DescriptorWriteError:
        qDebug() << "onServiceError: DescriptorWriteError";
        emit descriptorWriteError();
        return;
    case QLowEnergyService::UnknownError:
        qDebug() << "onServiceError: UnknownError";
        errorMessage = tr("Unknown Error from QLowEnergyService");
        break;
    default:
        qWarning() << "onServiceError: unhandled QLowEnergyService error: " << newError;
        errorMessage = tr("Unhandled Error %1 from QLowEnergyService").arg(newError);
        break;
    }
    emit serviceError(errorMessage);
}
