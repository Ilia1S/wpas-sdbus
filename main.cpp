#include <iomanip>
#include <iostream>
#include <sdbus-c++/sdbus-c++.h>

const std::string WPAS_DBUS_SERVICE = "fi.w1.wpa_supplicant1";
const std::string WPAS_DBUS_INTERFACE = "fi.w1.wpa_supplicant1";
const std::string WPAS_DBUS_OPATH = "/fi/w1/wpa_supplicant1";
const std::string WPAS_DBUS_INTERFACES_INTERFACE = 
                    "fi.w1.wpa_supplicant1.Interface";
const std::string WPAS_DBUS_INTERFACES_OPATH = 
                    "/fi/w1/wpa_supplicant1/Interfaces";
const std::string WPAS_DBUS_BSS_INTERFACE = "fi.w1.wpa_supplicant1.BSS";

std::unique_ptr<sdbus::IProxy> if_obj;

void propertiesChanged(const std::map<std::string, sdbus::Variant>& );
void showBss(const std::string& );
void scanDone(const bool& );
const std::string getCurrentTime();

void scanDone(const bool& success)
{
    std::cout << "Scan done: success=" << success << std::endl;
    std::vector<sdbus::ObjectPath> res;
    res = if_obj->getProperty("BSSs")
        .onInterface(WPAS_DBUS_INTERFACES_INTERFACE);
    std::cout << "Scanned wireless networks:" << std::endl;
    const std::string currentTime = getCurrentTime();
    std::cout << "|" << currentTime << "|" << std::endl;
    for (const std::string& opath : res)
        showBss(opath);
}

void showBss(const std::string& bss)
{
    std::unique_ptr<sdbus::IProxy> net_obj = 
        sdbus::createProxy(WPAS_DBUS_SERVICE, bss);
    std::vector<uint8_t> bssid_orig;
    bssid_orig = net_obj->getProperty("BSSID")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& item : bssid_orig)
    {
        ss << std::setw(2) << static_cast<int>(item);
        ss << ":";
    }
    std::string bssid = ss.str();
    bssid.pop_back();
    std::vector<uint8_t> ssid_orig;
    ssid_orig = net_obj->getProperty("SSID")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    std::string ssid(ssid_orig.begin(), ssid_orig.end());
    uint16_t freq;
    freq = net_obj->getProperty("Frequency")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    int16_t signal;
    signal = net_obj->getProperty("Signal")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    std::vector<uint32_t> rates;
    rates = net_obj->getProperty("Rates")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    uint32_t maxrate = 0;
    if (!rates.empty())
        maxrate = rates[0];

    std::cout << "[ap:'" << bssid << "';ssid:'" << ssid << "';freq:'" << freq
              << "';level:'" << signal << "';max:'" << maxrate << "']"
              << std::endl;
}

const std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t curTime = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::
                        milliseconds>(now.time_since_epoch()) % 1000;
    std::string millisecondsString = std::to_string(milliseconds.count());
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&curTime), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << millisecondsString;
    std::string timeString = oss.str();
    return timeString;
}

int main(int argc, char* argv[])
{
    // Create a D-Bus connection to the system bus and requests name on it
    auto connection = sdbus::createSystemBusConnection();
    // Create a proxy object
    std::unique_ptr<sdbus::IProxy> wpas_obj = 
        sdbus::createProxy(WPAS_DBUS_SERVICE, WPAS_DBUS_OPATH);
    // Check if Ifname is specified, eg. wlp3s0
    if (argc != 2)
    {
        std::cout << "Error. Specify your Ifname" << std::endl;
        return -1;
    }
    std::string ifname = argv[1];
    // Invoke GetInterface method to obtain the D-Bus path
    sdbus::ObjectPath obj_path;
    wpas_obj->callMethod("GetInterface")
        .onInterface(WPAS_DBUS_INTERFACE)
        .withArguments(ifname)
        .storeResultsTo(obj_path);
    std::string path = obj_path.c_str();

    if_obj = sdbus::createProxy(WPAS_DBUS_SERVICE, path);
    // Subscribe for the signals
    if_obj->uponSignal("ScanDone")
        .onInterface(WPAS_DBUS_INTERFACES_INTERFACE)
        .call([](const bool& success){ scanDone(success); });
    if_obj->uponSignal("PropertiesChanged")
        .onInterface(WPAS_DBUS_INTERFACES_INTERFACE)
        .call([](const std::map<std::string, sdbus::Variant>& propDict)
                { propertiesChanged(propDict); });
    if_obj->finishRegistration();
    std::map<std::string, sdbus::Variant> scanDict;
    scanDict["Type"] = sdbus::Variant("active");
    // Invoke Scan method to trigger a scan
    auto method = 
        if_obj->createMethodCall(WPAS_DBUS_INTERFACES_INTERFACE, "Scan");
    method << scanDict;
    if_obj->callMethod(method);

    // Run the loop on the connection
    connection->enterEventLoop();

    return 0;
}