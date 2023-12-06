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

void propertiesChanged(sdbus::Signal& properties)
{
    std::cout << "Properties changed: " << std::endl;
}

void scanDone(sdbus::Signal& success)
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
    wpas_obj->registerSignalHandler(WPAS_DBUS_INTERFACES_INTERFACE, "ScanDone", &scanDone);
//    wpas_obj->registerSignalHandler(WPAS_DBUS_INTERFACES_INTERFACE, "PropertiesChanged", &propertiesChanged);
    wpas_obj->finishRegistration();
    std::string ifname = "wlp3s0"; // to be edited
//    std::string path;
//    wpas_obj->callMethod("GetInterface").onInterface(WPAS_DBUS_INTERFACE).withArguments(ifname).storeResultsTo(path);
    auto method = wpas_obj->createMethodCall(WPAS_DBUS_INTERFACE, "GetInterface");
    method << ifname;
    auto path = wpas_obj->callMethod(method);

    std::cout << path << std::endl;
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