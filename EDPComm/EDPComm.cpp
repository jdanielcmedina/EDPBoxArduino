/* EDP-Comm.h - v2 [2017.01.04]
 *
 * Copyleft EDP Inovação 2017 - took inspiration from the Akeru library (Snootlab) and Modbus-Master-Slave-for-Arduino
 * This library is intended to use only with the Akeru & Modbus-Master-Slave-for-Arduino.
 * Made for the EDP IoT Hackathon 2017
 */

#include "EDPComm.h"

EDPComm::EDPComm(SoftwareSerial *serialPort)
{
	//Create a SoftwareSerial object so that we can use software serial. Search "software serial" on Arduino.cc to find out more details.
	//SoftwareSerial mySerial(10, 11);
	port = serialPort;
}

/* The clock data has the following structure:
{Year, Month, Day of the Month, Weekday, Hour, Minute, Second, Hundreths of second, Deviation, Clock status}
(Note that all of the variables have 1 byte except the year and the deviation that have 2 bytes each)
*/

/* The default load profile coming from the EB has the following structure:
{Clock{}, AMRProfileStatus, Energy A+ Inc}
*/


/* The following functions only work with FCT_READLOADPROFILE */

int EDPComm::getYearData(uint16_t dataArray[16])
{
	//year data is 2 bytes (from 2000 to 2019)
	int year= ((dataArray[0]));

	return year;
}

int EDPComm::getMonthData(uint16_t dataArray[16]){
	//month data is 1 byte only and we need to shift it
	int month= ((dataArray[1]>>8)&0xFF);

	return month;
}

int EDPComm::getDayData(uint16_t dataArray[16]){
	//day data is 1 byte only
	int day= ((dataArray[1])&0xFF);

	return day;
}

int EDPComm::getHourData(uint16_t dataArray[16]){
	//hour data is 1 byte only
	int hour = ((dataArray[2])&0xFF);

	return hour;
}

int EDPComm::getMinuteData(uint16_t dataArray[16]){
	//minutes data is 1 byte only and we need to shift it
	int min = ((dataArray[3]>>8)&0xFF);

	return min;
}

int EDPComm::getAMRProfileStatus(uint16_t dataArray[16]){
	/*Note that this variable belongs to the last load profile
	AMRProfileStatus gives a 1 byte status code during the load profile time integration
	|___BIT___________Description_______|
		 7			Incomplete Reading
		 6			RTC Sync (>=30 s)
		 5			Overflow
		 4			RTC Sync
		 3			Modified Config
		 2			Load Profile Reset
		 1			Power Down
		 0			Power Up
	|___________________________________|
	*/
	int amrProfileStatus = ((dataArray[6]>>8)&0xFF);

	 return amrProfileStatus;
}

int EDPComm::getClockStatus(uint16_t dataArray[16]){
	//Clock Status has only 2 values: 0 (Winter) and 128 (Summer)
        int clockStatus = ((dataArray[5])&0xFF);

	 return clockStatus;
}

double EDPComm::getLoadProfileData(uint16_t dataArray[16]){
	/*this function returns the last load profile data considering the default load profile structure with a variable size of 4 bytes
	*/
	 double profileValue_1half= (((dataArray[6]<<8)&0xFF00) | ((dataArray[7]>>8)&0xFF))*0x10000;
     double profileValue_2half= ((dataArray[7]<<8)&0xFF00) | ((dataArray[8]>>8)&0xFF);

	 double totalValue = profileValue_1half+profileValue_2half;

	return totalValue;
}


/* The following functions only work with FCT_READSINGLEREGISTER */
/* You can add aditional functions to other registers if needed*/

String EDPComm::getLiveFirmwareID(uint16_t dataArray[16]){
	// gets firmware ID from the EB (example: V110)
	char firmID[5+1];
	firmID[0] = ((dataArray[0]>>8)&0xFF);
	firmID[1] = ((dataArray[0])&0xFF);
	firmID[2] = ((dataArray[1]>>8)&0xFF);
	firmID[3] = ((dataArray[1])&0xFF);
	firmID[4] = ((dataArray[2]>>8)&0xFF);

	return firmID;
}

double EDPComm::getLiveTotalRegistriesValue(uint16_t dataArray[16])
{
	// Returns a live total value of 4 bytes (ex: A+_Total)
	double regValue_1half = dataArray[1];
	double regValue_2half = (dataArray[0])*0x10000;

	return regValue_1half + regValue_2half;
}

double EDPComm::getLiveInstantValues(uint16_t dataArray[16])
{
	// Returns a live instantaneous value of 2 bytes(ex: Current and Voltage)
	// Note: Live Instantaneous Values usually have a scaler of -1
	double instValue = (dataArray[0]);
	return instValue/10;
}


void EDPComm::resetSerial(long baudRate)
{
	//We need to reset the SerialPort in order to communicate again with the EB after sending something through SIGFOX
	delay(SERIAL_PORT_DELAY);
	port->begin(baudRate);
	delay(SERIAL_PORT_DELAY);
}

void EDPComm::printRawData(uint16_t dataArray[16])
{
	// print raw data coming from the EB
	for(int i = 0; i < 16;){
		Serial.print(dataArray[i],HEX);
		i++;
	}
	Serial.println("");
}


bool EDPComm::checkLoadProfileData(uint16_t dataArray[16])
{
	//Checks if the load profile data is valid
	int year = getYearData(dataArray);
	int month = getMonthData(dataArray);
	int day = getDayData(dataArray);

	if(year > 0 && month > 0 && day > 0 )
	{
		return true;
	}
	else
		return false;
}

void EDPComm::resetDataArray(uint16_t dataArray[16])
{
	// cleans buffer
	for(int i=0; i<16;i++){
		dataArray[i]=0;
	}
}
