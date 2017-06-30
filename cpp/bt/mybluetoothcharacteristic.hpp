#ifndef MYBLUETOOTHCHARACTERISTIC_HPP
#define MYBLUETOOTHCHARACTERISTIC_HPP

#include <QObject>
#include <QtBluetooth/QLowEnergyCharacteristic>

class MyBluetoothCharacteristic : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString characteristicName READ getName NOTIFY characteristicChanged)
    Q_PROPERTY(QString characteristicUuid READ getUuid NOTIFY characteristicChanged)
    Q_PROPERTY(QByteArray characteristicValue READ getValue NOTIFY characteristicChanged)
    Q_PROPERTY(QString displayValue READ getDisplayValue NOTIFY characteristicChanged)
    Q_PROPERTY(QString characteristicHandle READ getHandle NOTIFY characteristicChanged)
    Q_PROPERTY(QString characteristicPermission READ getPermission NOTIFY characteristicChanged)
    Q_PROPERTY(bool characteristicIsValid READ isValid NOTIFY characteristicChanged)
    Q_PROPERTY(int characteristicDescriptorsSize READ getDescriptorsSize NOTIFY characteristicChanged)
    Q_PROPERTY(QByteArray currentValue READ getCurrentValue WRITE setCurrentValue NOTIFY currentValueChanged)
    Q_PROPERTY(QString displayCurrentValue READ getDisplayCurrentValue NOTIFY currentValueChanged)

    Q_PROPERTY(bool notifyIsRunning READ getNotifyIsRunning WRITE setNotifyIsRunning NOTIFY characteristicChanged)
    Q_PROPERTY(bool collectData READ getCollectData WRITE setCollectData NOTIFY characteristicChanged)

    Q_PROPERTY(QVariant descriptorList READ getDescriptors NOTIFY descriptorsUpdated)

public:
    explicit MyBluetoothCharacteristic(QObject *parent = 0);

    void setCharacteristic(const QLowEnergyCharacteristic &characteristic);
    QString getName() const;
    QString getUuid() const;
    QByteArray getValue() const;
    QString getDisplayValue();
    QByteArray getCurrentValue();
    void setCurrentValue(QByteArray newValue);
    QString getDisplayCurrentValue();
    bool getNotifyIsRunning();
    void setNotifyIsRunning(bool isRunning);
    bool getCollectData();
    void setCollectData(bool isCollecting);
    QString getHandle() const;
    QString getPermission() const;
    bool isValid() const;
    int getDescriptorsSize() const;
    QLowEnergyCharacteristic getCharacteristic() const;

    QVariant getDescriptors();
    void clearDescriptors();

signals:
    void characteristicChanged();
    void descriptorsUpdated();
    void currentValueChanged();

public slots:

private:
    QLowEnergyCharacteristic mCharacteristic;
    QByteArray mCurrentValue;
    bool mNotifyIsRunning;
    bool mCollectData;
    QString prepareDisplayValue(QByteArray &valueArray);

    QList<QObject*> mDescriptors;
};

#endif // MYBLUETOOTHCHARACTERISTIC_HPP
