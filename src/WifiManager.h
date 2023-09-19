
#ifndef wifi_manager_h
#define wifi_manager_h

class WifiManager
{
private:
  
  void configureWifi(char *wifiConfig);
public:
  WifiManager(char *wifiConfig);
  ~WifiManager();

  void update();

};

#endif
