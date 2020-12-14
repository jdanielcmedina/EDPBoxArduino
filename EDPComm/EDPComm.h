/* EDP-Comm.h - v2 [2017.01.04]
 *
 * Copyleft EDP Inovação 2017 - took inspiration from the Akeru library (Snootlab) and Modbus-Master-Slave-for-Arduino
 * This library is intended to use with the Akeru & Modbus-Master-Slave-for-Arduino.
 * Made for the EDP IoT Hackathon 2017
 */

#ifndef EDPCOMM_H
#define EDPCOMM_H

#include <SoftwareSerial.h>


#define SERIAL_PORT_DELAY (1000)
#define BAUD_RATE (9600) //9600 means baud-rate at 9600

/**
 * @enum FCT
 * function codes
 */
enum FCT
{
    FCT_READSINGLEREGISTER = 4,
    FCT_READLOADPROFILE = 68
};

class EDPComm
{
	public:
		EDPComm(SoftwareSerial *serialPort);
		double getRawData(uint16_t dataArray[16]);
		int getYearData(uint16_t dataArray[16]);
		int getHourData(uint16_t dataArray[16]);
		int getMinuteData(uint16_t dataArray[16]);
		int getMonthData(uint16_t dataArray[16]);
		int getDayData(uint16_t dataArray[16]);
		int getAMRProfileStatus(uint16_t dataArray[16]);
		int getClockStatus(uint16_t dataArray[16]);
		double getLoadProfileData(uint16_t dataArray[16]);
		void resetSerial(long baudRate);
		bool checkLoadProfileData(uint16_t dataArray[16]);
		String getLiveFirmwareID(uint16_t dataArray[16]);
		double getLiveTotalRegistriesValue(uint16_t dataArray[16]);
		double getLiveInstantValues(uint16_t dataArray[16]);
		void printRawData(uint16_t dataArray[16]);
		void resetDataArray(uint16_t dataArray[16]);

	private:
		SoftwareSerial *port;
};
#endif // EDPCOMM_H
