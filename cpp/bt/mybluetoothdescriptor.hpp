#ifndef MYBLUETOOTHDESCRIPTOR_HPP
#define MYBLUETOOTHDESCRIPTOR_HPP

#include <QObject>
#include <QLowEnergyDescriptor>

class MyBluetoothDescriptor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString descriptorName READ getName NOTIFY descriptorChanged)
    Q_PROPERTY(QString descriptorUuid READ getUuid NOTIFY descriptorChanged)
    Q_PROPERTY(QString descriptorValue READ getValue NOTIFY descriptorChanged)
    Q_PROPERTY(QString descriptorHandle READ getHandle NOTIFY descriptorChanged)
    Q_PROPERTY(bool descriptorIsValid READ isValid NOTIFY descriptorChanged)

public:
    explicit MyBluetoothDescriptor(QObject *parent = 0);

    void setDescriptor(const QLowEnergyDescriptor &descriptor);
    QString getName() const;
    QString getUuid() const;
    QString getValue() const;
    QString getHandle() const;
    bool isValid() const;
    QLowEnergyDescriptor getCharacteristic() const;

signals:
    void descriptorChanged();

public slots:

private:
    QLowEnergyDescriptor mDescriptor;
};

#endif // MYBLUETOOTHDESCRIPTOR_HPP
