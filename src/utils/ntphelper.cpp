#include "ntphelper.h"
#include "logger.h"

#include <QMessageBox>

NTPHelper::NTPHelper(QObject *parent) : QObject(parent)
{}

void NTPHelper::checkTime()
{
    m_client = new NtpClient(this);
    connect(m_client, SIGNAL(replyReceived(QHostAddress,quint16,NtpReply)), this, SLOT(onReplyReceived(QHostAddress,quint16,NtpReply)));
    QHostAddress address = QHostInfo::fromName("pool.ntp.org").addresses().first();
    Logger::debug(QString("[NTP] Sending ntp request to %1").arg(address.toString()));
    m_client->sendRequest(address, 123);
}

void NTPHelper::onReplyReceived(QHostAddress host, quint16 port, NtpReply reply)
{
    int offsetms = reply.localClockOffset();
    int offset = offsetms / 1000;
    Logger::debug(QString("[NTP] Received ntp response, time offset is %1ms").arg(QString::number(offsetms)));
    if (qAbs(offset) >= 90)
        QMessageBox::warning(NULL, tr("Inaccurate Time Detected"), tr("Your time offset is over 90s.\nV2Ray will not work in this situtation."));
    else if (qAbs(offset) >= 15)
        QMessageBox::warning(NULL, tr("Inaccurate Time Detected"), tr("Your time offset is over 15s.\nAlthough V2Ray will work, we still suggest you adjust your system clock."));
}
