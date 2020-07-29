#ifndef ROUTETABLEHELPER_H
#define ROUTETABLEHELPER_H

#include <QObject>
#include <QThread>

class RouteTableHelper : public QObject
{
    Q_OBJECT

public:
    RouteTableHelper(QString serverAddress);
    ~RouteTableHelper();

    void getDefaultGateWay();
    void getTUNTAPInfo();

    void set();
    void reset();

public slots:
    void setRouteTable();
    void resetRouteTable();

private:
    struct Adapter {
        int index = -1;
        QString gateWay;
    };

    QString serverAddress;
    QString serverIp;

    Adapter adapter;
    Adapter tuntap;

    QThread *thread;
};

#endif // ROUTETABLEHELPER_H
