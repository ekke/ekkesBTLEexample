#include "addimatmapping.hpp"

AddimatMapping::AddimatMapping(QObject *parent) : QObject(parent)
{

}

void AddimatMapping::setData(QString addimatKey, QString userId, QString userPIN, QString userName)
{
    bool valuesChanged = false;
    if(addimatKey != mAddimatKey) {
        mAddimatKey = addimatKey;
        valuesChanged = true;
    }
    if(userId != mUserId) {
        mUserId = userId;
        valuesChanged = true;
    }
    if(userPIN != mUserPIN) {
        mUserPIN = userPIN;
        valuesChanged = true;
    }
    if(userName != mUserName) {
        mUserName = userName;
        valuesChanged = true;
    }
    if(valuesChanged) {
        emit mappingChanged();
    }
}

QString AddimatMapping::getAddimatKey() const
{
    return mAddimatKey;
}

QString AddimatMapping::getUserId() const
{
    return mUserId;
}

QString AddimatMapping::getUserPIN() const
{
    return mUserPIN;
}

QString AddimatMapping::getUserName() const
{
    return mUserName;
}

void AddimatMapping::setAddimatKey(QString newValue)
{
    if(newValue != mAddimatKey) {
        mAddimatKey = newValue;
        emit mappingChanged();
    }
}

void AddimatMapping::setUserId(QString newValue)
{
    if(newValue != mUserId) {
        mUserId = newValue;
        emit mappingChanged();
    }
}

void AddimatMapping::setUserPIN(QString newValue)
{
    if(newValue != mUserPIN) {
        mUserPIN = newValue;
        emit mappingChanged();
    }
}

void AddimatMapping::setUserName(QString newValue)
{
    if(newValue != mUserName) {
        mUserName = newValue;
        emit mappingChanged();
    }
}

QVariantMap AddimatMapping::toMap()
{
    QVariantMap map;
    map.insert("addimatKey", mAddimatKey);
    map.insert("userId", mUserId);
    map.insert("userPIN", mUserPIN);
    map.insert("userName", mUserName);
    return map;
}

void AddimatMapping::fromMap(QVariantMap map)
{
    mAddimatKey = map.value("addimatKey").toString();
    mUserId = map.value("userId").toString();
    mUserPIN = map.value("userPIN").toString();
    mUserName = map.value("userName").toString();
}
