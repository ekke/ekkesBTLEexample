#ifndef NFCREADERMAPPING_HPP
#define NFCREADERMAPPING_HPP

#include <QObject>
#include <QVariantMap>

class NfcReaderMapping : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString tagId READ getTagId WRITE setTagId NOTIFY mappingChanged)
    Q_PROPERTY(QString userName READ getUserName WRITE setUserName NOTIFY mappingChanged)

public:
    explicit NfcReaderMapping(QObject *parent = nullptr);

    Q_INVOKABLE
    void setData(QString tagId, QString userName);

    QString getTagId() const;
    QString getUserName() const;
    void setTagId(QString newValue);
    void setUserName(QString newValue);

    QVariantMap toMap();
    void fromMap(QVariantMap map);

signals:
    void mappingChanged();

public slots:

private:
    QString mTagId;
    QString mUserName;


};

#endif // NFCREADERMAPPING_HPP
