#pragma ones

#define WIFI_CONNECTION_DELAY 500
#define NTP_ADDR "pool.ntp.org"
#define TIME_CHECKING_DELAY 100
#define TIME_CHECKING_TIMEOUT 10000
#define INET_CHECKING_TIMEOUT 60000

void initWiFi(WebServer* webServer, Context* context);
