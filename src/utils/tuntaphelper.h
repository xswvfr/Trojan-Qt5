#ifndef TUNTAPHELPER_H
#define TUNTAPHELPER_H

#include <QObject>

class TUNTAPHelper : public QObject
{
    Q_OBJECT
public:
    TUNTAPHelper();
    QString GetComponentID();

};

#endif // TUNTAPHELPER_H
