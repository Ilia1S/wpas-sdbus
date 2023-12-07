#include <iostream>
#include <sdbus-c++/sdbus-c++.h>

const char* WPAS_DBUS_SERVICE = "fi.w1.wpa_supplicant1";
const char* WPAS_DBUS_INTERFACE = "fi.w1.wpa_supplicant1";
const char* WPAS_DBUS_OPATH = "/fi/w1/wpa_supplicant1";
const char* WPAS_DBUS_INTERFACES_INTERFACE = "fi.w1.wpa_supplicant1.Interface";
const char* WPAS_DBUS_INTERFACES_OPATH = "/fi/w1/wpa_supplicant1/Interfaces";
const char* WPAS_DBUS_BSS_INTERFACE = "fi.w1.wpa_supplicant1.BSS";

void listInterfaces()
{
    std::cout << "Listing interfaces: " << std::endl;
}

void propertiesChanged(std::string)
{
    std::cout << "Properties changed: " << std::endl;
}

void scanDone(std::string)
{
    std::cout << "Scan done." << std::endl;
}

int main(int argc, char* argv[])
{
    // In simple cases, we don't need to create D-Bus connection explicitly for our proxies
    // Create a proxy object
    auto wpas_obj = sdbus::createProxy(WPAS_DBUS_SERVICE, WPAS_DBUS_OPATH);

/*    if (argc != 2)
    {
        listInterfaces();
        return 1;
    } */

    // Subscribe for the signals
    wpas_obj->uponSignal("ScanDone").onInterface(WPAS_DBUS_INTERFACES_INTERFACE).call([](const std::string& str){ scanDone(str); });
    wpas_obj->uponSignal("PropertiesChanged").onInterface(WPAS_DBUS_INTERFACES_INTERFACE).call([](const std::string& str){ propertiesChanged(str); });
    wpas_obj->finishRegistration();

    // Invoke GetInterface method to obtain the D-Bus path to an object related to an interface
    std::string ifname = "wlp3s0"; // to be edited
    sdbus::ObjectPath path;
    wpas_obj->callMethod("GetInterface").onInterface(WPAS_DBUS_INTERFACE).withArguments(ifname).storeResultsTo(path);

//    std::cout << path << std::endl;
/*
    auto if_obj = sdbus::createProxy(WPAS_DBUS_SERVICE, path);

    std::vector<std::string> parcel = {"Type"};
    std::string separator = ":";
    // Invoke the Scan method
    {
        auto method = if_obj->createMethodCall(WPAS_DBUS_INTERFACES_INTERFACE, "Scan");
        method << parcel << separator;
        auto reply = if_obj->callMethod(method);
    } */
    return 0;
}