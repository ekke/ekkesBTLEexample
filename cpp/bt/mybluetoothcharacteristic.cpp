#include "mybluetoothcharacteristic.hpp"
#include "qbluetoothuuid.h"
#include <QByteArray>

#include "mybluetoothdescriptor.hpp"


MyBluetoothCharacteristic::MyBluetoothCharacteristic(QObject *parent) : QObject(parent),
    mNotifyIsRunning(false), mCollectData(false)
{
    //
}

void MyBluetoothCharacteristic::setCharacteristic(const QLowEnergyCharacteristic &characteristic)
{
    mCharacteristic = characteristic;
    mNotifyIsRunning = false;
    clearDescriptors();
    for (int i = 0; i < characteristic.descriptors().size(); ++i) {
        MyBluetoothDescriptor* myDescriptor = new MyBluetoothDescriptor(this);
        myDescriptor->setDescriptor(characteristic.descriptors().at(i));
        mDescriptors.append(myDescriptor);
    }

    emit characteristicChanged();
    emit descriptorsUpdated();
}

QString MyBluetoothCharacteristic::getName() const
{
    QString name = mCharacteristic.name();
    if (!name.isEmpty()) {
        return name;
    }

    // find descriptor with CharacteristicUserDescription
    foreach (const QLowEnergyDescriptor &descriptor, mCharacteristic.descriptors()) {
        if (descriptor.type() == QBluetoothUuid::CharacteristicUserDescription) {
            name = descriptor.value();
            break;
        }
    }

    if (name.isEmpty()) {
        name = "Custom Characteristic";
    }

    return name;
}

QString MyBluetoothCharacteristic::getUuid() const
{
    const QBluetoothUuid uuid = mCharacteristic.uuid();
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success) {
        return QStringLiteral("0x") + QString::number(result16, 16);
    }

    quint32 result32 = uuid.toUInt32(&success);
    if (success) {
        return QStringLiteral("0x") + QString::number(result32, 16);
    }

    return uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QByteArray MyBluetoothCharacteristic::getValue() const
{
    return mCharacteristic.value();
}

QString MyBluetoothCharacteristic::getDisplayValue()
{
    QByteArray a = mCharacteristic.value();
    return prepareDisplayValue(a);
}

QByteArray MyBluetoothCharacteristic::getCurrentValue()
{
    return mCurrentValue;
}

void MyBluetoothCharacteristic::setCurrentValue(QByteArray newValue)
{
    if(mNotifyIsRunning && mCollectData) {
        mCurrentValue.append(newValue);
    } else {
        mCurrentValue = newValue;
    }
    emit currentValueChanged();
}

QString MyBluetoothCharacteristic::getDisplayCurrentValue()
{
    // Show raw string first and hex value below
    if (mCurrentValue.isEmpty()) {
        return QString();
    }
    return prepareDisplayValue(mCurrentValue);
}

bool MyBluetoothCharacteristic::getNotifyIsRunning()
{
    return mNotifyIsRunning;
}

void MyBluetoothCharacteristic::setNotifyIsRunning(bool isRunning)
{
    mNotifyIsRunning = isRunning;
    emit characteristicChanged();
}

bool MyBluetoothCharacteristic::getCollectData()
{
    return mCollectData;
}

void MyBluetoothCharacteristic::setCollectData(bool isCollecting)
{
    mCollectData = isCollecting;
    emit characteristicChanged();
}

QString MyBluetoothCharacteristic::prepareDisplayValue(QByteArray &valueArray)
{
    QString result;
    if (valueArray.isEmpty()) {
        result = QStringLiteral("<none>");
        return result;
    }

    result = "String: [";
    result += valueArray;
    result += "]\nByteArray length: ";
    result += QString::number(valueArray.length());
    result += QLatin1Char('\n');
    QString hexValue = valueArray.toHex();
    result += "HEX: [";
    result += hexValue;
    result += ']';
    bool ok;
    int intValue = hexValue.toInt(&ok, 16);
    if(ok) {
        result += QLatin1Char('\n');
        result += "INT: [";
        result += QString::number(intValue);
        result += ']';
    }
    return result;
}


QString MyBluetoothCharacteristic::getHandle() const
{
    return QStringLiteral("0x") + QString::number(mCharacteristic.handle(), 16);
}

QString MyBluetoothCharacteristic::getPermission() const
{
    QString properties;
    int permission = mCharacteristic.properties();
    if (permission & QLowEnergyCharacteristic::Read)
        properties += QStringLiteral("Read ");
    if (permission & QLowEnergyCharacteristic::Write)
        properties += QStringLiteral("Write ");
    if (permission & QLowEnergyCharacteristic::Notify)
        properties += QStringLiteral("Notify ");
    if (permission & QLowEnergyCharacteristic::Indicate)
        properties += QStringLiteral("Indicate ");
    if (permission & QLowEnergyCharacteristic::ExtendedProperty)
        properties += QStringLiteral("ExtendedProperty ");
    if (permission & QLowEnergyCharacteristic::Broadcasting)
        properties += QStringLiteral("Broadcast ");
    if (permission & QLowEnergyCharacteristic::WriteNoResponse)
        properties += QStringLiteral("WriteNoResp ");
    if (permission & QLowEnergyCharacteristic::WriteSigned)
        properties += QStringLiteral("WriteSigned ");
    return properties;
}

bool MyBluetoothCharacteristic::isValid() const
{
    return mCharacteristic.isValid();
}

int MyBluetoothCharacteristic::getDescriptorsSize() const
{
    return mCharacteristic.descriptors().size();
}

QLowEnergyCharacteristic MyBluetoothCharacteristic::getCharacteristic() const
{
    return mCharacteristic;
}

QVariant MyBluetoothCharacteristic::getDescriptors()
{
    return QVariant::fromValue(mDescriptors);
}

void MyBluetoothCharacteristic::clearDescriptors()
{
    qDeleteAll(mDescriptors);
    mDescriptors.clear();
    emit descriptorsUpdated();
}

