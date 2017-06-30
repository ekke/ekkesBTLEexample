#ifndef MYBLUETOOTHDEVICEINFO_H
#define MYBLUETOOTHDEVICEINFO_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <qbluetoothaddress.h>
#include <QBluetoothUuid>
#include <QLowEnergyController>

#include "mybluetoothserviceinfo.hpp"


class MyBluetoothDeviceInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ getName NOTIFY deviceChanged)
    Q_PROPERTY(QString deviceAddress READ getAddress NOTIFY deviceChanged)
    Q_PROPERTY(int deviceRssi READ getRssi NOTIFY deviceChanged)
    Q_PROPERTY(int controllerState READ getControllerState NOTIFY deviceChanged)
    Q_PROPERTY(QString controllerStateDescription READ getControllerStateDescription NOTIFY deviceChanged)
    Q_PROPERTY(bool deviceIsConnected READ getDeviceIsConnected NOTIFY deviceChanged)
    Q_PROPERTY(int deviceType READ getDeviceType WRITE setDeviceType NOTIFY deviceChanged)

    Q_PROPERTY(bool servicesDiscoveryActive READ servicesDiscoveryActive WRITE setServicesDiscoveryActive NOTIFY servicesDiscoveryActiveChanged)
    Q_PROPERTY(bool createServicesActive READ createServicesActive WRITE setCreateServicesActive NOTIFY createServicesActiveChanged)

    Q_PROPERTY(bool retryConnectIsEnabled READ retryConnectIsEnabled WRITE setRetryConnectIsEnabled NOTIFY retryConnectEnabledChanged)
    Q_PROPERTY(int retryCounter READ retryCounter WRITE setRetryCounter NOTIFY retryCounterChanged)
    Q_PROPERTY(bool retryDeviceConnect READ retryDeviceConnect WRITE setRetryDeviceConnect NOTIFY retryDeviceConnectChanged)

    Q_PROPERTY(QVariant servicesList READ getServices NOTIFY servicesUpdated)

public:
    explicit MyBluetoothDeviceInfo(QObject *parent = 0);
    void setController(QLowEnergyController *controller);
    QLowEnergyController *controller() const;
    QString getAddress() const;
    QString getName() const;
    bool isCached() const;
    bool isValid() const;
    QString getDeviceClass() const;
    int getRssi() const;
    int getControllerState() const;
    QString getControllerStateDescription() const;
    int getDeviceType() const;
    void setDeviceType(int newDeviceType);
    bool getDeviceIsConnected() const;

    QString getServiceUUIDs() const;

    QBluetoothDeviceInfo getDevice();
    void setDevice(const QBluetoothDeviceInfo &dev);

    QVariant getServices();
    void clearServices();
    QList<QObject*> getServicesAsList();
    bool servicesDiscoveryActive();
    void setServicesDiscoveryActive(bool discoveryState);
    bool createServicesActive();
    void setCreateServicesActive(bool isActive);
    bool retryDeviceConnect();
    void setRetryDeviceConnect(bool retry);
    bool retryConnectIsEnabled();
    void setRetryConnectIsEnabled(bool isEnabled);
    int retryCounter();
    void setRetryCounter(int counter);

    Q_INVOKABLE
    void connectToDevice();

    Q_INVOKABLE
    void connectToDeviceAgain();

    Q_INVOKABLE
    void disconnectFromDevice();

    // called from Scanner to find ALL services
    Q_INVOKABLE
    void discoverServices();

    void setExpectedServiceUuids(QStringList &serviceUuids);

    // create specific services for known devices
    Q_INVOKABLE
    void createExpectedServices();

    void resetConnectionCounter();

signals:
    void deviceChanged();

    void servicesUpdated();
    void servicesDiscoveryActiveChanged();
    void createServicesActiveChanged();

    void retryConnectEnabledChanged();
    void retryCounterChanged();
    void retryDeviceConnectChanged();

    void connected();
    void disconnected();

    void controllerErrorReceived(QString errorMessage);
    void serviceScanInfo(QString infoMessage);

public slots:
    void onDeviceDisconnected();
    void onDeviceConnected();

    void onServiceDiscovered(const QBluetoothUuid &serviceUuid);
    void onServiceDiscoveryFinished();

    void onControllerError(QLowEnergyController::Error error);
    void onControllerStateChanged(QLowEnergyController::ControllerState);


private slots:

private:
    QBluetoothDeviceInfo mDevice;
    bool mConnected;

    QList<QObject*> mServices;
    bool mServicesDiscoveryActive;
    bool mCreateServicesActive;
    QStringList mExpectedServiceUuids;
    void addExpectedServices();

    QLowEnergyController *mLowEnergyController;

    // getting QLowEnergyController::ConnectionError: we do retry because
    // W QtBluetoothGatt: Unhandled error code on connectionStateChanged: 133
    int mConnectionErrorCounter;
    // should be configured in settings
    int mConnectionErrorMaxRetries;
    bool mRetryDeviceConnect;
    bool mRetryConnectIsEnabled;
    int mDeviceType;

    QString uuidString(const QBluetoothUuid &serviceUuid);
};

#endif // MYBLUETOOTHDEVICEINFO_H
