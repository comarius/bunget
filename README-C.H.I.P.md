
# BTLE On C.H.I.P Board as GATT Server using Bunget library 


I got my CHIP Board few hours ago.  0.180-0.300A Amps, Has 2 wifi, 1 BT4.
  - 0.18A idling there (with the desktop, LightDM is running), with wlan0 and BT on. 
  - 0.26A BT + wlan0 + wlan1 (as HOST AP ) (though hostapd does not starts as service but in shell it does) 
  - 0.340A picks here and there during apt-get install and make 

The BT bluez, bluetooth, and  python daemons related to BT should be killed. 
I dont have a display, therefore I killed lighdm service as well. 


```
  101  hciconfig
  102  sudo apt-get install git
  105  sudo apt-get install cmake
  106  sudo apt-get install libcrypto++-dev
  110  git clone https://github.com/comarius/bunget
  111  cd bunget/
  113  cd src/
  116  cd lib
  117  mv libbunget.a libbunget86.a
  118  mv ./R-PI-MINIBAN/libbunget.a ./
  119  cd ..
  120  cmake .
  121  make
  123  sudo apt-get install build-essential
  127  cmake .
  128  make
  129  cd bin
  130  hciconfig
  133  sudo service bluetooth status
  134  sudo update-rc.d bluetooth disable
  135  sudo service bluetooth stop
  136  ps ax | grep blue
  137  sudo service lightdm stop # stop UI
  138  ps ax | grep blue # stop any process using BT
  139  sudo kill -9 12581
  140  sudo kill -9 12575
  141  ps ax | grep blue
  142  sudo ./bunget 0
  ```
  
  
  ![bunget on Acer phone with C H I P](http://209.177.145.195/share/chip.jpg "bunget-lib")
  
  
  
