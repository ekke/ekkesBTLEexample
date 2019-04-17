#ifndef MYBLUETOOTHSERVICEINFO_H
#define MYBLUETOOTHSERVICEINFO_H

#include <QObject>
#include <QtBluetooth/QLowEnergyService>

#include "mybluetoothcharacteristic.hpp"

class MyBluetoothServiceInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serviceName READ getName NOTIFY serviceChanged)
    Q_PROPERTY(QString serviceUuid READ getUuid NOTIFY serviceChanged)
    Q_PROPERTY(QString serviceType READ getType NOTIFY serviceChanged)

    Q_PROPERTY(QVariant characteristicList READ getCharacteristics NOTIFY characteristicsUpdated)

public:
    explicit MyBluetoothServiceInfo(QObject *parent = 0);
    void setService(QLowEnergyService *service);
    QLowEnergyService *service() const;
    QString getUuid() const;
    QString getName() const;
    QString getType() const;

    QVariant getCharacteristics();
    void clearCharacteristics();
    bool hasCharacteristics();
    MyBluetoothCharacteristic* getCharacteristicInfo(const QString uuid);

    Q_INVOKABLE
    void connectToService();

    Q_INVOKABLE
    void readCharacteristic(MyBluetoothCharacteristic* myCharacteristic);

    Q_INVOKABLE
    void writeCharacteristicAsString(MyBluetoothCharacteristic* myCharacteristic, QString valueString, bool withResponse);

    Q_INVOKABLE
    void writeCharacteristicAsHex(MyBluetoothCharacteristic* myCharacteristic, QString hexString, bool withResponse);

    Q_INVOKABLE
    void subscribeNotifications(MyBluetoothCharacteristic* myCharacteristic);
    Q_INVOKABLE
    void unSubscribeNotifications(MyBluetoothCharacteristic* myCharacteristic);

signals:
    void serviceChanged();
    void characteristicsUpdated();
    void characteristicsDone();
    void serviceInvalid();
    void serviceDiscovering();

    void characteristicReadError();
    void characteristicWriteError();

    void descriptorReadError();
    void descriptorWriteError();

    void operationError();
    void serviceError(QString errorMessage);

    // temp solution !!!
    void cardData(const QByteArray cardDataArray);

public slots:
    // QLowEnergyService related
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

private slots:
    // R E A D
    void onCharacteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    // W R I T E
    void onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    // N O T I F I C A T I O N
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    // E R R O R
    void onServiceError(const QLowEnergyService::ServiceError &newError);

private:
    QLowEnergyService *mLowEnergyService;

    QList<QObject*> mCharacteristics;

    void addAlreadyDiscoveredCharacteristics();
};

#endif // MYBLUETOOTHSERVICEINFO_H
