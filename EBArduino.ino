#include <ModbusRtu.h>
#include <SoftwareSerial.h>
#include <EDPComm.h>

// Global Variables
uint16_t au16data[16]; // data array with 16 elements each with 16 bits (2 bytes) for modbus network sharing
uint8_t u8state;       //control state variable
unsigned long u32wait; //delay variable < delay() may not work well with Modbus >
int aux_sig_send;      //sigfox send control variable

Modbus master(0);                   //use 0 for master and [1,247] for slaves
modbus_t telegram;                  //initalize structure for query slave
SoftwareSerial mySerial(10, 11);    //Create a SoftwareSerial on pins 10&11
EDPComm edpComm(&mySerial); //start EDPComm lib

void setup()
{
  delay(1000);
  Serial.begin(9600); //use the hardware serial if you want to connect to your computer via usb cable, etc.
  
  master.begin(&mySerial, 9600); // The first parameter is the address of your SoftwareSerial. Do not forget the "&"! 9600 means baud-rate at 9600
  master.setTimeOut(2000);           // if there is no answer in 2000 ms, roll over
  u32wait = millis() + 2000;         //delay 2000 ms
  u8state = 0; //State control variable
}

void SendEBRequest(uint8_t u8id, uint8_t u8fct, uint16_t u16RegAdd, uint16_t u16CoilsNo, uint16_t *au16reg)
{
  /*
    u8id : slave address, in this case =1
    u8fct : function code (check documentation)
    u16RegAdd : start address in slave (check documentation)
    u16CoilsNo : number of elements (coils or registers) to read 
    au16reg : pointer to a memory array in the Arduino
  */
  telegram = {u8id, u8fct, u16RegAdd, u16CoilsNo, au16reg};
  Serial.println("**Sending Request to EB**");
  master.query(telegram); // send query (only once)
}

void loop()
{
  switch (u8state)
  {
  case 0:
    // wait state
    if (millis() > u32wait)
      u8state++;
    break;
  case 1:
    /* For FCT_READLOADPROFILE the u16RegAdd is 769 (00000011 00000001 in binary) 
         That means values 0x0003 and 0x0001" in hexadecimal:
         0x0003 for Measumerent ID indexes to retrieve (1st - Clock 2nd -AMR Status and 3rd - Active Energy Import (A+))
         0x0001 for quantity of entries (we only need the last)
         The u16CoilsNo is 0 as we don't need any more inputs.
         You should always use this configuration for FCT_READLOADPROFILE
      */
    SendEBRequest(1, FCT_READLOADPROFILE, 769, 0, au16data);
    u8state++;
    break;
  case 2:
    master.poll(); // check incoming messages
    if (master.getState() == COM_IDLE)
    {
      u32wait = millis() + 2000;
      edpComm.printRawData(au16data); //prints raw load profile data coming from the EB
      u8state =6; //goes to Sigfox sending option
    }
    break;
  case 3:
    // wait state. You should always use this between SendEBRequests()
    if (millis() > u32wait)
      u8state++;
    break;
  case 4:
    /* For FCT_READSINGLEREGISTER the u16RegAdd depends on which register you want to get.
       Check the documentation for more info.
       The u16CoilsNo is 1 as in one output.
      You should always use this configuration for FCT_READSINGLEREGISTER
    */
    SendEBRequest(1, FCT_READSINGLEREGISTER, 109, 1, au16data); // #109 register is Instantaneous Current
    u8state++;
    break;
  case 5:
    master.poll(); // check incoming messages
    if (master.getState() == COM_IDLE)
    {
      u32wait = millis() + 2000;
      double current = edpComm.getLiveInstantValues(au16data);
      Serial.println(current);          //Current has a scaler of -1
      edpComm.resetDataArray(au16data); //Always reset au16data
      u32wait = millis() + 2000;
      u8state = 0;
    }
    break;
  case 6:
    /*
        In this example, we will ONLY send the last data value from the Load Profile (3rd - Active Energy Import (A+))
        You should complete the case with the other data available coming in from the EBs and send it to Sigfox. 
        Be aware of the 12 bytes payload limit!
        */
    Serial.println("**Starting PRINT comm...**");

    // Only sends to Sigfox backend with valid data
    if (edpComm.checkLoadProfileData(au16data))
    {
      int loadProfileData = edpComm.getLoadProfileData(au16data);
      
      Serial.print(".sData:" + loadProfileData);

      edpComm.resetDataArray(au16data);
      u32wait = millis() + 2000;
      u8state = 3;
      break;
    }
    else
    {
      Serial.println("**Skipped. Trying again...**");
      edpComm.resetDataArray(au16data);
      u32wait = millis() + 2000;
      u8state = 0;
      break;
    }
  }
}
