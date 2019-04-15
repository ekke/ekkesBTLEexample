#ifndef FEITIANCARDREADERMANAGER_HPP
#define FEITIANCARDREADERMANAGER_HPP

#include <QObject>

class FeitianCardReaderManager : public QObject
{
    Q_OBJECT
public:
    explicit FeitianCardReaderManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FEITIANCARDREADERMANAGER_HPP