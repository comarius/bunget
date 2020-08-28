
#### Build lib bunget and demo application

### Prerequisites 
* Linux
  * sudo apt-get install uuid-dev
  * sudo apt-get install cmake
  * sudo apt-get install build-essential
  * sudo apt-get install libcrypto++-dev
  
 
### x86


```javascript

cd libbunget
cmake .
make
cd ..
cmake .
make
sudo ./bin/bunget HCI_DEVICCE_NUMBER
// on Android mobile
// install ArduiUiPush or BTLE explorer
// connect
// you can see the time in clear text.
// temeprature as float
// LED as R / W for RPI
```


### Raspbery PI
```javascript
// copy all on R-PI
// install libcrypto++-dev
// installl bluetooth utils or tools. Needs hciconfig and hcitool in root's path
// copy libbunget/R-PI/* to /libbunget/*
// copy R-PI/* to ./*
// repeat as above
```


#### Fresh Linux Mint Installation Log
```javascript

/// 1. clone repo
user@hph git clone https://github.com/comarius/bunget
user@hph cd bunget/src/libbunget

/// 2. goto libbunget and make
user@hph ~/bunget/src/libbunget $ cmake .
-- The C compiler identification is GNU 4.8.4
-- The CXX compiler identification is GNU 4.8.4
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/marius/bunget/src/libbunget

user@hph ~/bunget/src/libbunget $ make 
Scanning dependencies of target bunget
[  6%] Building CXX object CMakeFiles/bunget.dir/ascon.cpp.o
[ 13%] Building CXX object CMakeFiles/bunget.dir/bt_incinpl.cpp.o
[ 20%] Building CXX object CMakeFiles/bunget.dir/bt_socket.cpp.o
[ 26%] Building CXX object CMakeFiles/bunget.dir/bu_gap.cpp.o
[ 33%] Building CXX object CMakeFiles/bunget.dir/bu_hci.cpp.o
[ 40%] Building CXX object CMakeFiles/bunget.dir/bybuff.cpp.o
[ 46%] Building CXX object CMakeFiles/bunget.dir/cryptos.cpp.o
[ 53%] Building CXX object CMakeFiles/bunget.dir/gattdefs.cpp.o
[ 60%] Building CXX object CMakeFiles/bunget.dir/hci_socket.cpp.o
[ 66%] Building CXX object CMakeFiles/bunget.dir/l2cap_socket.cpp.o
[ 73%] Building CXX object CMakeFiles/bunget.dir/libbungetpriv.cpp.o
[ 80%] Building CXX object CMakeFiles/bunget.dir/rfcomm_socket.cpp.o
[ 86%] Building CXX object CMakeFiles/bunget.dir/sco_socket.cpp.o
[ 93%] Building CXX object CMakeFiles/bunget.dir/secmanp.cpp.o
[100%] Building CXX object CMakeFiles/bunget.dir/uguid.cpp.o
Linking CXX static library /home/marius/bunget/src/lib/libbunget.a
[100%] Built target bunget

/// 3. goto bunget and make
user@hph ~/bunget/src/libbunget $ cd ..
user@hph ~/bunget/src $ cmake .
-- The C compiler identification is GNU 4.8.4
-- The CXX compiler identification is GNU 4.8.4
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
CMake Warning (dev) at CMakeLists.txt:33 (link_directories):
  This command specifies the relative path

    ./lib

  as a link directory.

  Policy CMP0015 is not set: link_directories() treats paths relative to the
  source dir.  Run "cmake --help-policy CMP0015" for policy details.  Use the
  cmake_policy command to set the policy and suppress this warning.
This warning is for project developers.  Use -Wno-dev to suppress it.

-- Configuring done
-- Generating done
-- Build files have been written to: /home/marius/bunget/src
marius@hph ~/bunget/src $ make 
Scanning dependencies of target bunget
[100%] Building CXX object CMakeFiles/bunget.dir/main.cpp.o
/home/user/bunget/src/main.cpp: In member function ‘const char* my_proc::_get_temp_s()’:
/home/user/bunget/src/main.cpp:351:10: warning: unused variable ‘ftamp’ [-Wunused-variable]
    float ftamp=0.0;
          ^
Linking CXX executable bin/bunget
[100%] Built target bunget

/// 2. find out hco device. My BT4 dongle is 5C:F3:70:6B:72:D6 which is hci1 -> 1
user@hph ~/bunget/src $ hciconfig
hci1:	Type: BR/EDR  Bus: USB
	BD Address: 5C:F3:70:6B:72:D6  ACL MTU: 1021:8  SCO MTU: 64:1
	UP RUNNING PSCAN 
	RX bytes:612 acl:0 sco:0 events:37 errors:0
	TX bytes:942 acl:0 sco:0 commands:37 errors:0

hci0:	Type: BR/EDR  Bus: USB
	BD Address: E0:2A:82:2F:D4:08  ACL MTU: 1021:8  SCO MTU: 64:1
	UP RUNNING PSCAN 
	RX bytes:1050 acl:0 sco:0 events:52 errors:0
	TX bytes:1423 acl:0 sco:0 commands:52 errors:0

/// start bunget on hci 1
user@hph ~/bunget/src $ sudo ./bin/bunget 1
sh: echo: I/O error
chmod: cannot access ‘/sys/class/gpio/gpio17/*’: No such file or directory
bluetoothd: unrecognized service
bluetooth stop/waiting
/// can connect and see time
/// press q to exit
^Cuser@hph ~/bunget/src $ 
```
