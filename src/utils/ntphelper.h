#ifndef NTPHELPER_H
#define NTPHELPER_H

#include <QObject>
#include <QHostAddress>
#include <QHostInfo>

#include "qntp/QNtp.h"
#include "qntp/NtpClient.h"
#include "qntp/NtpReply.h"

class NTPHelper : public QObject
{
    Q_OBJECT
public:
    explicit NTPHelper(QObject *parent = 0);
    void checkTime();

signals:

private slots:
    void onReplyReceived(QHostAddress host, quint16 port, NtpReply reply);

private:
    NtpClient * m_client;
};

#endif // NTPHELPER_H
