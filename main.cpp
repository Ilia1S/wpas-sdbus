#include <iomanip>
#include <iostream>
#include <unistd.h>
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

void propertiesChanged(const std::map<std::string, sdbus::Variant>& ); // PropertiesChanged signal handler
void showBss(const std::string& ); // show BSSs after scanning
void scanDone(const bool& ); // ScanDone signal handler
const std::string getCurrentTime(); // get the current time
const std::string getPath(std::unique_ptr<sdbus::IProxy>& wpas_obj,
                               const std::string& ifname); // obtain the D-Bus path

void propertiesChanged(const std::map<std::string, sdbus::Variant>& propDict)
{
    static std::string prevState = if_obj->getProperty("State")
        .onInterface(WPAS_DBUS_INTERFACES_INTERFACE);
    for (char& c : prevState)
            c = std::toupper(c);

    if (propDict.count("State") > 0)
    {
        
        const std::string currentTime = getCurrentTime();
        std::string newState = propDict.at("State").get<std::string>();
        for (char& c : newState)
            c = std::toupper(c);

        std::cout << "|" << currentTime << "| [WLANSTATE] ::" << prevState 
                  << " -> " << newState << std::endl;
        prevState = newState;
    }
}

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
    std::vector<uint8_t> bssidOrig;
    bssidOrig = net_obj->getProperty("BSSID")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& item : bssidOrig)
    {
        ss << std::setw(2) << static_cast<int>(item);
        ss << ":";
    }
    std::string bssid = ss.str();
    bssid.pop_back();
    std::vector<uint8_t> ssidOrig;
    ssidOrig = net_obj->getProperty("SSID")
        .onInterface(WPAS_DBUS_BSS_INTERFACE);
    std::string ssid(ssidOrig.begin(), ssidOrig.end());
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

const std::string getPath(std::unique_ptr<sdbus::IProxy>& wpas_obj,
                               const std::string& ifname)
{
    std::string path;
    try
    {
        sdbus::ObjectPath objPath;
        wpas_obj->callMethod("GetInterface")
            .onInterface(WPAS_DBUS_INTERFACE)
            .withArguments(ifname)
            .storeResultsTo(objPath);
        path = objPath.c_str();
    }
    catch (const sdbus::Error& exc)
    {
        std::string errorMessageU = exc.what();
        std::cout << errorMessageU << std::endl;
        if (errorMessageU.find\
        ("[fi.w1.wpa_supplicant1.InterfaceUnknown]") == std::string::npos)
            throw exc;
        try
        {
            {
                sdbus::ObjectPath objPath;
                std::map<std::string, sdbus::Variant> crIntDict;
                crIntDict["Ifname"] = sdbus::Variant(ifname);
                crIntDict["Driver"] = sdbus::Variant("test");
                wpas_obj->callMethod("CreateInterface")
                    .onInterface(WPAS_DBUS_INTERFACE)
                    .withArguments(crIntDict)
                    .storeResultsTo(objPath);
                    path = objPath.c_str();
                sleep(1);
            }
        }
        catch (const sdbus::Error& exc)
        {
            std::string errorMessageE = exc.what();
            if (errorMessageE.find\
            ("[fi.w1.wpa_supplicant1.InterfaceExists]") == std::string::npos)
                throw exc;
        }
    }
    return path;
}

int main(int argc, char* argv[])
{
    // Create a D-Bus connection to the system bus and requests name on it
    auto connection = sdbus::createSystemBusConnection();
    // Create a proxy object for WPAS_DBUS_SERVICE and WPAS_DBUS_OPATH
    std::unique_ptr<sdbus::IProxy> wpas_obj;
    wpas_obj = sdbus::createProxy(WPAS_DBUS_SERVICE, WPAS_DBUS_OPATH);
    // Check if Ifname is specified, eg. wlp3s0
    if (argc != 2)
    {
        std::cout << "Error. Specify your Ifname" << std::endl;
        return 1;
    }
    std::string ifname = argv[1];
    // Invoke getPath method to obtain the D-Bus path
    // and see if wpa_supplicant already knows about this interface
    const std::string path = getPath(wpas_obj, ifname);
    // Create a proxy object for WPAS_DBUS_SERVICE and path
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
    if_obj->callMethod("Scan")
        .onInterface(WPAS_DBUS_INTERFACES_INTERFACE)
        .withArguments(scanDict)
        .storeResultsTo();
    // Run the loop on the connection
    connection->enterEventLoop();

    return 0;
}
