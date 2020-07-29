#include "trojanthread.h"
#include "Trojan-Qt5-Core.h"

TrojanThread::TrojanThread(QString filePath) : filePath(filePath)
{}

TrojanThread::~TrojanThread()
{
    stop();
}

void TrojanThread::run()
{
    startTrojanGo(filePath.toUtf8().data());
}

void TrojanThread::stop()
{
    stopTrojanGo();
}
