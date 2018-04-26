#ifndef _GPSLIB__
#define _GPSLIB_

#include <Arduino.h>
#include <Wire.h>

// to generate your own sentences, check out the MTK command datasheet and use a checksum calculator
// such as the awesome http://www.hhhh.org/wiml/proj/nmeaxor.html

// PMTK GPS Ping
#define GPS_PING              		"$PMTK000*32"

// PMTK GPS Update Rate
#define GPS_UPDATE_RATE_10SEC		"$PMTK300,10000,0,0,0,0*2C" 
#define GPS_UPDATE_RATE_5SEC		"$PMTK300,5000,0,0,0,0*18"  
#define GPS_UPDATE_RATE_1SEC		"$PMTK300,1000,0,0,0,0*1C"
#define GPS_UPDATE_RATE_5HZ			"$PMTK300,200,0,0,0,0*2F"
#define GPS_UPDATE_RATE_10HZ		"$PMTK300,100,0,0,0,0*2C"

// PMTK GPS Report Rate
#define GPS_REPORT_RATE_10SEC		"$PMTK220,10000*2F" // Once every 10 seconds, 100 mHz.
#define GPS_REPORT_RATE_5SEC		"$PMTK220,5000*1B"  // Once every 5 seconds, 200 mHz.
#define GPS_REPORT_RATE_1SEC		"$PMTK220,1000*1F"
#define GPS_REPORT_RATE_5HZ			"$PMTK220,200*2C"
#define GPS_REPORT_RATE_10HZ		"$PMTK220,100*2F"

// PMTK GPS Baud Rate
#define GPS_BAUD_115200				"$PMTK251,115200*1F"
#define GPS_BAUD_57600				"$PMTK251,57600*2C"
#define GPS_BAUD_38400				"$PMTK251,38400*27"
#define GPS_BAUD_19200				"$PMTK251,19200*22"
#define GPS_BAUD_14400				"$PMTK251,14400*29"
#define GPS_BAUD_9600				"$PMTK251,9600*17"
#define GPS_BAUD_4800				"$PMTK251,4800*14"

// PMTK GPS Set NMEA sentence structure
#define GPS_SET_NMEA_OUTPUT_GPGLL 	"$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_GPRMC 	"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_GPVTG 	"$PMTK314,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_GPGGA 	"$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_GPGSA 	"$PMTK314,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_GPGSV 	"$PMTK314,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define GPS_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
#define GPS_SET_NMEA_OUTPUT_OFF 	"$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

// PMTK GPS Hot Start
#define GPS_HOT_START				"$PMTK101*32"

class GPS {
	public:
		GPS();
		uint32_t begin(void);
		bool available(void);
		bool updateData(void);
		char _outputSent[64];
	private:
		uint32_t determineBaud(void);
		bool verifyChecksum(void);
		int32_t parseInt(uint8_t,uint8_t);
		float parseFloat(uint8_t,uint8_t);
		char _cSent[128];
		uint8_t _cSentLength;
		const int32_t _pow10[8] = {1,10,100,1000,10000,100000,1000000,10000000};
		uint32_t _baudRates[7] = {115200,57600,38400,19200,14400,9600,4800};
		uint32_t _baudRate;
		bool _fix;
		uint16_t _yr;
		uint8_t _mo;
		uint8_t _da;
		uint8_t _hr;
		uint8_t _mn;
		float _sc;
		float _lat;
		float _lon;
		float _speed;
		float _heading;
};

#endif
