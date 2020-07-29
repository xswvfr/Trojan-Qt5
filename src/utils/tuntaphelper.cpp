#include "tuntaphelper.h"

#include <QSettings>
#include <QDebug>

TUNTAPHelper::TUNTAPHelper()
{}

QString TUNTAPHelper::GetComponentID()
{
    QSettings adaptersRegistry("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", QSettings::NativeFormat);
    for (QString adapterRegisteryName : adaptersRegistry.childGroups()) {
        if (adapterRegisteryName != "Configuration" && adapterRegisteryName != "Properties") {
            QSettings adapterRegistry("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\" + adapterRegisteryName, QSettings::NativeFormat);

            QString adapterComponentId = adapterRegistry.value("ComponentId").toString();
            if (adapterComponentId == "tap0901" || adapterComponentId == "tap0801") {
                return adapterRegistry.value("NetCfgInstanceId").toString();
            }
        }
     }
     return "";
}
