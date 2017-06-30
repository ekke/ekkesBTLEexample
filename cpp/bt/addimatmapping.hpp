#ifndef ADDIMATMAPPING_HPP
#define ADDIMATMAPPING_HPP

#include <QObject>
#include <QVariantMap>

class AddimatMapping : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString addimatKey READ getAddimatKey WRITE setAddimatKey NOTIFY mappingChanged)
    Q_PROPERTY(QString userId READ getUserId WRITE setUserId NOTIFY mappingChanged)
    Q_PROPERTY(QString userPIN READ getUserPIN WRITE setUserPIN NOTIFY mappingChanged)
    Q_PROPERTY(QString userName READ getUserName WRITE setUserName NOTIFY mappingChanged)

public:
    explicit AddimatMapping(QObject *parent = 0);

    Q_INVOKABLE
    void setData(QString addimatKey, QString userId, QString userPIN, QString userName);

    QString getAddimatKey() const;
    QString getUserId() const;
    QString getUserPIN() const;
    QString getUserName() const;
    void setAddimatKey(QString newValue);
    void setUserId(QString newValue);
    void setUserPIN(QString newValue);
    void setUserName(QString newValue);

    QVariantMap toMap();
    void fromMap(QVariantMap map);

signals:
    void mappingChanged();

public slots:

private:
    QString mAddimatKey;
    QString mUserId;
    QString mUserPIN;
    QString mUserName;
};

#endif // ADDIMATMAPPING_HPP
