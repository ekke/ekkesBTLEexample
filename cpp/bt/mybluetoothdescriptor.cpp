#include "mybluetoothdescriptor.hpp"

MyBluetoothDescriptor::MyBluetoothDescriptor(QObject *parent) : QObject(parent)
{
    //
}

void MyBluetoothDescriptor::setDescriptor(const QLowEnergyDescriptor &descriptor)
{
    mDescriptor = descriptor;
    emit descriptorChanged();
}

QString MyBluetoothDescriptor::getName() const
{
    QString name = mDescriptor.name();
    if (!name.isEmpty()) {
        return name;
    }
    return "n/a";
}

QString MyBluetoothDescriptor::getUuid() const
{
    const QBluetoothUuid uuid = mDescriptor.uuid();
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

QString MyBluetoothDescriptor::getValue() const
{
    // Show raw string first and hex value below
    QByteArray a = mDescriptor.value();
    QString result;
    if (a.isEmpty()) {
        result = QStringLiteral("<none>");
        return result;
    }

    result = "String: [";
    result += a;
    result += "]\nByteArray length: ";
    result += QString::number(a.length());
    result += QLatin1Char('\n');
    QString hexValue = a.toHex();
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

QString MyBluetoothDescriptor::getHandle() const
{
    return QStringLiteral("0x") + QString::number(mDescriptor.handle(), 16);
}

bool MyBluetoothDescriptor::isValid() const
{
    return mDescriptor.isValid();
}

QLowEnergyDescriptor MyBluetoothDescriptor::getCharacteristic() const
{
    return mDescriptor;
}
