#include <drogon/drogon.h>
int main()
{
    // Set HTTP listener address and port
    drogon::app().loadConfigFile("config.json");
    // Load config file
    // drogon::app().loadConfigFile("../config.json");
    // drogon::app().loadConfigFile("../config.yaml");
    // Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
