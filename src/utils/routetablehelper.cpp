#include "routetablehelper.h"
#include "tuntaphelper.h"
#include "logger.h"

#include <stdlib.h>
#include <QProcess>
#include <QHostInfo>
#if defined (Q_OS_WIN)
#include "WinSock2.h"
#include "Trojan-Qt5-Route.h"
#endif

RouteTableHelper::RouteTableHelper(QString serverAddress) : serverAddress(serverAddress)
{
    getDefaultGateWay();
    getTUNTAPInfo();
    thread = new QThread(this);
    this->moveToThread(thread);
    connect(thread, SIGNAL(started()), this, SLOT(setRouteTable()));
    connect(thread, SIGNAL(finished()), this, SLOT(resetRouteTable()));
}

RouteTableHelper::~RouteTableHelper()
{}

void RouteTableHelper::getDefaultGateWay()
{
    QProcess *task = new QProcess;
    QStringList param;
    QString gateWay;
#if defined (Q_OS_WIN)
    MIB_IPFORWARDROW BestRoute;
    DWORD dwRet = GetBestRoute(QHostAddress("114.114.114.114").toIPv4Address(), 0, &BestRoute);
    if (dwRet == NO_ERROR) {
        adapter.index = BestRoute.dwForwardIfIndex;
        gateWay = QHostAddress(htonl(BestRoute.dwForwardNextHop)).toString();
        Logger::debug(QString("[Advance Mode] Adapter Index: %1").arg(adapter.index));
        Logger::debug(QString("[Advance Mode] GateWay Address: %1").arg(gateWay));
    } else {
        gateWay = "";
        Logger::error("[Advance Mode] GetBestRoute Failed");
    }
#elif defined (Q_OS_MAC)
    param << "-c" << "route get default | grep gateway | awk '{print $2}'";
    task->start("bash", param);
    task->waitForFinished();
    gateWay = task->readAllStandardOutput();
    gateWay = gateWay.remove("\n");
#elif defined (Q_OS_LINUX)
    param << "-c" << "route -n | awk '{print $2}' | awk 'NR == 3 {print}'";
    task->start("bash", param);
    task->waitForFinished();
    gateWay = task->readAllStandardOutput();
    gateWay = gateWay.remove("\n");
#endif
    // let's process domain & ip address
    QRegExp pattern("^([a-zA-Z0-9-]+.)+([a-zA-Z])+$");
    if (pattern.exactMatch(gateWay)) {
        QList<QHostAddress> gatewayAddress = QHostInfo::fromName(gateWay).addresses();
        if (!gatewayAddress.isEmpty())
            gateWay = gatewayAddress.first().toString();
    }
    adapter.gateWay = gateWay;
}

void RouteTableHelper::getTUNTAPInfo()
{
#if defined (Q_OS_WIN)
    TUNTAPHelper *tth = new TUNTAPHelper();
    QString ComponentID = tth->GetComponentID();
    BOOL IS_TAP_FIND = FALSE;

    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);

    if (nRel == ERROR_BUFFER_OVERFLOW) {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }

    if (nRel == ERROR_SUCCESS) {
        while (pIpAdapterInfo) {
            if (pIpAdapterInfo->AdapterName == ComponentID && !IS_TAP_FIND) {
                tuntap.index = pIpAdapterInfo->Index;
                IS_TAP_FIND = TRUE;

                Logger::debug(QString("[Advance Mode] find TUN/TAP Adapter: %1").arg(pIpAdapterInfo->AdapterName));
            }
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }

    if (pIpAdapterInfo) {
        delete pIpAdapterInfo;
    }
#endif
}

void RouteTableHelper::set()
{
    thread->start();
}

void RouteTableHelper::reset()
{
    thread->exit();
}

void RouteTableHelper::setRouteTable()
{
    // let's process domain & ip address
    QRegExp pattern("^([a-zA-Z0-9-]+.)+([a-zA-Z])+$");
    if (pattern.exactMatch(serverAddress)) {
        QList<QHostAddress> ipAddress = QHostInfo::fromName(serverAddress).addresses();
        if (!ipAddress.isEmpty())
            serverIp = ipAddress.first().toString();
        else
            return;
    }
    else
        serverIp = serverAddress;
#if defined (Q_OS_WIN)
    // Bypass Server IP
    CreateRoute(serverIp.toUtf8().data(), 32, adapter.gateWay.toUtf8().data(), adapter.index);
    // Bypass LAN IP
    CreateRoute("10.0.0.0", 8, adapter.gateWay.toUtf8().data(), adapter.index);
    CreateRoute("172.16.0.0", 12, adapter.gateWay.toUtf8().data(), adapter.index);
    CreateRoute("192.168.0.0", 16, adapter.gateWay.toUtf8().data(), adapter.index);
    // Create Default Route
    CreateRoute("0.0.0.0", 0, "10.0.0.1", tuntap.index, 10);
#elif defined (Q_OS_MAC)
    QProcess::execute("route delete default");
    QThread::msleep(200); // wait for tun2socks to be up
    QProcess::execute("route add default 240.0.0.1");
    QProcess::execute(QString("route add default %1 -ifscope en0").arg(adapter.gateWay));
    QProcess::execute(QString("route add 10.0.0.0/8 %1").arg(adapter.gateWay));
    QProcess::execute(QString("route add 172.16.0.0/12 %1").arg(adapter.gateWay));
    QProcess::execute(QString("route add 192.168.0.0/16 %1").arg(adapter.gateWay));
    QProcess::execute(QString("route add %1/32 %2").arg(serverIp).arg(adapter.gateWay));
#elif defined (Q_OS_LINUX)
    QProcess::execute("ip route del default");
    QProcess::execute("ip route add default via 240.0.0.1");
    QProcess::execute(QString("ip route add %1/32 via %2").arg(serverIp).arg(adapter.gateWay));
#endif

}

void RouteTableHelper::resetRouteTable()
{
#if defined (Q_OS_WIN)
    // Delete Default Route
    DeleteRoute("0.0.0.0", 10, "10.0.0.1", tuntap.index, 10);
    // Delete Bypass LAN IP
    DeleteRoute("10.0.0.0", 8, adapter.gateWay.toUtf8().data(), adapter.index);
    DeleteRoute("172.16.0.0", 12, adapter.gateWay.toUtf8().data(), adapter.index);
    DeleteRoute("192.168.0.0", 16, adapter.gateWay.toUtf8().data(), adapter.index);
    // Delete Bypass Server IP
    DeleteRoute(serverIp.toUtf8().data(), 32, adapter.gateWay.toUtf8().data(), adapter.index);
#elif defined (Q_OS_MAC)
    QProcess::execute("route delete default");
    QProcess::execute(QString("route add default %1").arg(adapter.gateWay).toUtf8().data());
    QProcess::execute("route delete 10.0.0.0/8");
    QProcess::execute("route delete 172.16.0.0/12");
    QProcess::execute("route delete 192.168.0.0/16");
    QProcess::execute(QString("route delete %1/32").arg(serverIp));
#elif defined (Q_OS_LINUX)
    QProcess::execute("ip route delete default");
    QProcess::execute(QString("ip route add default via %1").arg(adapter.gateWay).toUtf8().data());
    QProcess::execute(QString("ip route delete %1/32").arg(serverIp));
#endif
}
