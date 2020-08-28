/**
    Copyright:  zirexix 2016-2017

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
    This program should run as root.

    Every time a characterisitc/service is chnaged,
    turn off and on on mobile the BT to clear the cached LE's.

    http://plugable.com/2014/06/23/plugable-usb-bluetooth-adapter-solving-hfphsp-profile-issues-on-linux
    Newer Kernel Versions (3.16 and later)
    wget https://s3.amazonaws.com/plugable/bin/fw-0a5c_21e8.hcd
    sudo mkdir /lib/firmware/brcm
    sudo mv fw-0a5c_21e8.hcd /lib/firmware/brcm/BCM20702A0-0a5c-21e8.hcd

 *              THIS IS A DEMO FOR LIBBUNGET
 *
    This demo adds 1 service 0x123F with 3 characteristis.
        0x3400  control a GPIO pin, we connect a LED, on GPIO 17 '/sys/class/gpio/gpio17/value'
        0x3401
        0x3402


   QT CREATOR APPDEMO
Go to Tools-> Options-> Environment
In the Tab General under **System** Group there is a
Terminal Option.
The default value is set to/usr/bin/xterm -e.
Replace it with /usr/bin/xterm -e sudo or
/usr/bin/gnome-terminal -x sudo.
Press Apply and OK Buttons.
Under Mode Selector click on Projects,
select Run Option. Under Run Group Box
select Run in Terminal


/etc/sudoers using sudo visudo <- /usr/bin/gnome-terminal


*/

// test echo
// #define XECHO_BLENO

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <stropts.h>
#include <libbunget.h>
#include "crypto.h"

using namespace std;

bool __alive = true;

/****************************************************************************************
 * intrerrupt the demo in a orthodox way
*/
int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, 0x541B, &bytesWaiting);
    return bytesWaiting;
}

/****************************************************************************************
 * user procedure to intercept btle events  on****()
*/
class my_proc : public ISrvProc
{
public:
    my_proc();
    Icryptos* get_crypto(){return &_crypt;};
    bool initHciDevice(int devid, const char* name);
    void onServicesDiscovered(std::vector<IHandler*>& els);
    void onReadRequest(IHandler* pc);
    int  onSubscribesNotify(IHandler* pc, bool b);
    void onIndicate(IHandler* pc);
    void onWriteRequest(IHandler* pc);
    void onWriteDescriptor(IHandler* pc, IHandler* pd);
    void onAdvertized(bool onoff);
    void onDeviceStatus(bool onoff);
    void onStatus(const HciDev* connected);
    bool onSpin(IServer* ps, uint16_t notyUuid);

private:
    void        _prepare_gpio17();
    const char* _get_time();
    float       _get_temp();
    const char* _get_temp_s();
    uint8_t     _get_gpio();
    void        _send_value(IHandler* pc);

public:
    char        _some[20];
    bool        _subscribed;
    IHandler*   LedChr;       // RW
    IHandler*   TimeChr;      // NIR
    IHandler*   Temp1Chr;     // NIR
    IHandler*   EchoCht;
private:
    cryptos     _crypt;         // MANDATORY, detached form lib, Use it on your own GNU
};

/****************************************************************************************
*/
#define UID_GPIO    0x3400
#define UID_TIME    0x3401
#define UID_TEMP    0x3403


/****************************************************************************************
 * demo main program
*/
int main(int n, char* v[])
{
    std::cout << LIBBUNGET_VERSION_STRING << "\n";
    if(n==1)
    {
        std::cout << "sudo bunget hcidev#, pass device id as 0,1,2.. as first argument!\n";
        return -1;
    }
    /*
    if(getuid()!=0)
    {
        std::cout << "run under sudo credentials or setcap bunget !\n";
        return -1;
    }
    */


    BtCtx*      ctx = BtCtx::instance();                // BT context
    my_proc     procedure;                              // this procedure
    int dev = ::atoi(v[1]);

    int srdel = 0;
    if(n==3)
        srdel = atoi(v[2]);



    try{
        // leave name empty for 'hostname'
#ifdef XECHO_BLENO
        IServer* BS =  ctx->new_server(&procedure, dev, "echo", srdel, true);
#else
        IServer* BS =  ctx->new_server(&procedure, dev, "bunget", srdel, true, true);
#endif
#if 0   // not tested !!!
        //BS->set_name("advname"); // this is the bt name.
        //99999999-9999-9999-9999-999999999999
        BS->adv_beacon("11111111-1111-1111-1111-111111111111", 1, 10, -10, 0x004C, (const uint8_t*)"todo", 7);
#endif // 0

#ifdef XECHO_BLENO
        IService*   ps = BS->add_service(0xec00,"echo");
        procedure.EchoCht = ps->add_charact(0xec0e,PROPERTY_WRITE|PROPERTY_READ,
                                 0,
                                 FORMAT_RAW, 1); // 1 / 0

#else

        IService*   ps = BS->add_service(0x123F,"bunget");
        procedure.LedChr = ps->add_charact(UID_GPIO,PROPERTY_WRITE|PROPERTY_INDICATE,
                                 0,
                                 FORMAT_RAW, 1); // 1 / 0

        procedure.TimeChr = ps->add_charact(UID_TIME, PROPERTY_READ|PROPERTY_NOTIFY,
                                 0,
                                 FORMAT_RAW, 20); // we send it as string

        procedure.Temp1Chr = ps->add_charact(UID_TEMP, PROPERTY_NOTIFY|PROPERTY_INDICATE,
                                  0,
                                  FORMAT_FLOAT, FORMAT_FLOAT_LEN); // we send it as float
#endif
        BS->advertise(512);
        BS->run();
        BS->stop();
    }
    catch(bunget::hexecption& ex)
    {
        ERROR (ex.report());
    }
    return 0;
}

/****************************************************************************************
*/
my_proc::my_proc()
{
    _subscribed=false;
    Temp1Chr = 0;
    _prepare_gpio17();
}

/****************************************************************************************
 * add your console hciconfig preambul to setup hci before BTLE is starting
*/
bool my_proc::initHciDevice(int devid, const char* devn)
{
#if 0
    char name[128];
    // system("service bluetoothd stop");
    // system("service bluetooth stop");
    // system("sudo systemctl stop bluetooth");
    // system("rfkill unblock bluetooth");
    ::sprintf(name,"hciconfig hci%d down", devid);
    system(name);
    ::sleep(2);
    ::sprintf(name,"hciconfig hci%d up", devid);
    system(name);
/*
    ::sprintf(name,"hciconfig hci%d sspmode 0", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d nosecmgr", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d noencrypt", devid);
    system(name);
*/
    ::sprintf(name,"hciconfig hci%d noauth", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d noleadv", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d noscan", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d name  %s", devid, devn);
    system(name);
/*
    ::sprintf(name,"hciconfig hci%d piscan", devid);
    system(name);
    ::sprintf(name,"hciconfig hci%d leadv", devid);
    system(name);
*/

    printf("%s", "done dirty work\n");
#endif
    return true;
}

/****************************************************************************************
*/
bool my_proc::onSpin(IServer* ps, uint16_t notyUuid)
{
    if(_kbhit()){
        if(getchar()=='q')
        return false;
    }


/**
    Control notyUuid frequency from BS->advertise(512<-this value in ms);
    where 512 is the interval in milliseconds when the
    notyUuid flag is passed in for the notification you may send it over.

    If the interval in milliseconds for notification to send
    data, is too small would flood the hci on some dongles, and
    puts dongles in a weird state which requires unpluging and replugging the device.

    for some onboard btle device would rewuire a  full power off

    Tweak the timing from advertise(#) call also as much
    the hci socket to be responsive and the client
    as Android/Ios to be able to eat that data.
    On some Android(s) this got as much as 120 times/ second
    giving a minimum time of 8 ms, but some data were observed to be lost on Modile devices.

    Though on some dongles like noname chinese got in unknown states
    with even 100ms interval.


   In order to have a maximum troughput use read/indicate property and drive the
   read from the mobile device as much the mobile device can suck the data trough it's
   notification / completion event shit. That would go something like.

   read-indicator->  wait callback to complette, push data to main app thread  and issue next read.
   I acheved with this a max 1.4 Kb/second on android and 2k/sec on iOS with a standard HCI 20 bytes payload.


*/
    if(_subscribed)
    {
#ifndef XECHO_BLENO
        if(notyUuid==TimeChr->get_handle())
            _send_value(TimeChr);
        else if(notyUuid==Temp1Chr->get_handle())
            _send_value(Temp1Chr);
#else
//        if(notyUuid==EchoCht->get_handle())
//            _send_value(EchoCht);
#endif
    }
    return true;
}


/****************************************************************************************
*/
void my_proc::onServicesDiscovered(std::vector<IHandler*>& els)
{
    TRACE("my_proc event: onServicesDiscovered");
}

/****************************************************************************************
*/
/// remote reads pc characteristics
void my_proc::onReadRequest(IHandler* pc)
{
    TRACE("my_proc event:  onReadRequest:" <<  std::hex<< pc->get_16uid() << std::dec);
    _send_value(pc);
}

/****************************************************************************************
*/
int my_proc::onSubscribesNotify(IHandler* pc, bool b)
{
    TRACE("my_proc event: onSubscribesNotify:" << std::hex<< pc->get_16uid() << "="<<(int)b<< std::dec);
    _subscribed = b;
    return 0 ;
}

/****************************************************************************************
*/
void my_proc::onIndicate(IHandler* pc)
{
    TRACE("my_proc event:  onIndicate:" <<  std::hex<< pc->get_16uid() << std::dec);
    _send_value(pc);
}

/****************************************************************************************
*/
void my_proc::onWriteRequest(IHandler* pc)
{
    TRACE("my_proc event:  onWriteRequest:" <<  std::hex<< pc->get_16uid() << std::dec);
    std::string     ret;
    const uint8_t*  value = pc->get_value();
    char            by[4];
    int             i=0;

    for(;i<pc->get_length();i++)
    {
        ::sprintf(by,"%02X:",value[i]);
        ret.append(by);
    }
    TRACE("Remote data:" << ret);
    if(pc->get_16uid() == UID_GPIO)
    {
        if(::access("/sys/class/gpio/gpio17/value",0)==0)
        {
            if(value[0]==0)
                system("echo 0 > /sys/class/gpio/gpio17/value");
            else
                system("echo 1 > /sys/class/gpio/gpio17/value");
        }
    }
}

/****************************************************************************************
*/
//descriptor chnaged of the charact
void my_proc::onWriteDescriptor(IHandler* pc, IHandler* pd)
{
    TRACE("my_proc event:  onWriteDescriptor:" << int(*((int*)(pd->get_value()))));
}

/****************************************************************************************
*/
void my_proc::onAdvertized(bool onoff)
{
    TRACE("my_proc event:  onAdvertized:" << onoff);
}

/****************************************************************************************
*/
void my_proc::onDeviceStatus(bool onoff)
{
    TRACE("my_proc event:  onDeviceStatus:" << onoff);
    if(onoff==false)
    {
        _subscribed = false;
    }
}

/****************************************************************************************
*/
void my_proc::onStatus(const HciDev* device)
{
    if(device == 0)
    {
        _subscribed = false;
        TRACE("my_proc event: disconnected");
    }
    else
    {
        TRACE("accepted connection: " << device->_mac <<","<< device->_name);
    }
}

/****************************************************************************************
*/
void my_proc::_prepare_gpio17()
{
    if(::access("/sys/class/gpio/export/",0)==0)
    {
        system ("chmod 777 /sys/class/gpio/export");
        system ("echo 17 > /sys/class/gpio/export");
        system ("sync");
        if(::access("/sys/class/gpio/gpio17/",0)==0)
            system ("chmod 777 /sys/class/gpio/gpio17/*");
        system ("sync");
    }
}

/****************************************************************************************
*/
const char*  my_proc::_get_time()
{
    time_t secs = time(0);
    struct tm *local = localtime(&secs);
    sprintf(_some, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
    return _some;
}

/****************************************************************************************
*/
float my_proc::_get_temp()
{
    float ftamp=0.0;
#ifdef ARM_CC
    if(::access("/opt/vc/bin/vcgencmd",0)==0)
    {
        ::system("/opt/vc/bin/vcgencmd measure_temp > /tmp/bunget");
        std::ifstream ifs("/tmp/bunget");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        temp = temp.substr(5);
        ftamp =::atof(temp.c_str());
    }
#else //fake it
    std::string temp = "temp=32.5";
    temp = temp.substr(5);
    ftamp =::atof(temp.c_str());
    ftamp += rand()%15;
#endif
    return ftamp;
}

/****************************************************************************************
*/
const char* my_proc::_get_temp_s()
{
#ifdef ARM_CC
    if(::access("/opt/vc/bin/vcgencmd",0)==0)
    {
        ::system("/opt/vc/bin/vcgencmd measure_temp > /tmp/bunget");
        std::ifstream ifs("/tmp/bunget");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        ::strcpy(_some,temp.c_str());
    }
#else //fake it
    static int num = 10;
    ++num;
    ::sprintf(_some,"temp = %d ºC", num);
#endif
    return _some;
}

/****************************************************************************************
*/
uint8_t my_proc::_get_gpio()
{
    if(::access("/sys/class/gpio/gpio17/value",0)==0)
    {
        std::ifstream ifs("/sys/class/gpio/gpio17/value");
        std::string temp( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()));
        return uint8_t(::atoi(temp.c_str()));
    }

    return 0;
}

/****************************************************************************************
*/
void my_proc::_send_value(IHandler* pc)
{
    uint16_t uid = pc->get_16uid();
    switch(uid)
    {
        case  UID_GPIO:
            {
                uint8_t gp = _get_gpio();
                // pc->put_value((uint8_t*)&gp,1);
                GattRw(pc).write(gp);
            }
            break;
        case  UID_TIME:
            {
                const char* t = _get_time();
                pc->put_value((uint8_t*)t,::strlen(t));
            }
            break;
        case  UID_TEMP:
            {
                //float ft = _get_temp();
                //pc->put_value((uint8_t*)&ft,sizeof(float));
                const char* fts = _get_temp_s();
                pc->put_value((uint8_t*)fts,::strlen(fts));
            }
            break;
        case  0xec0e:
            {
                //float ft = _get_temp();
                //pc->put_value((uint8_t*)&ft,sizeof(float));
                //const char* fts = _get_temp_s();
                static int K=0;

                char rands[32];
                ::sprintf(rands,"%d", K++);
                pc->put_value((uint8_t*)rands,::strlen(rands));
            }
        break;
        default:
            break;
    }
}


