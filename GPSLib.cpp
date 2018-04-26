#include "GPSLib.h"

GPS::GPS(void){}

// Public Functions
uint32_t GPS::begin(void){
	Wire.begin();
	Serial.begin(115200);
	delay(5000);
	// Determine the current output baud rate of the GPS unit
	// and initialize the Serial connection to that baud rate
	_baudRate = determineBaud();
	// Set the GPS update rate and reporting rate
	Serial1.println(GPS_UPDATE_RATE_1SEC);
	Serial1.readBytesUntil('\n',_cSent,128);
	Serial1.println(GPS_REPORT_RATE_1SEC);
	Serial1.readBytesUntil('\n',_cSent,128);
	// Set the output type to NMEA GPRMC, which is
	// currently the only output stream that can be parsed
	Serial1.println(GPS_SET_NMEA_OUTPUT_GPRMC);
	Serial1.readBytesUntil('\n',_cSent,128);
	// Set the units serial baud rate based on user input
	Serial1.println(GPS_BAUD_115200);
	Serial1.readBytesUntil('\n',_cSent,128);
	delay(200);
	// Momentarily shutdown the serial coms so that determineBaud() can
	// assess whether the serial baud rate was properly configured
	Serial1.end();
	delay(200);
	// Determine whether the serial baud rate was properly configured,
	// and start the serial communications at whatever rate 
	// is being output by the unit
	_baudRate = determineBaud();
	Serial1.setTimeout(0);
	return _baudRate;
}

 bool GPS::available(void){
	 uint8_t bytesRead = 0;
	if (Serial1.available()) {
		bytesRead = Serial1.readBytesUntil('\n',&_cSent[_cSentLength],128);
		_cSentLength = _cSentLength + bytesRead;
		if(_cSent[_cSentLength-1] == '\r'){
			if(verifyChecksum()){
				return true;
			}else{
				memset(_cSent,0,128);
				_cSentLength = 0;
			}
		}
	} 
	return false;
} 

// $GPRMC,000258.800,V,,,,,0.00,0.00,060180,,,N*45
// $GPRMC,194509.000,A,4042.6142,N,07400.4168,W,2.03,221.11,160412,,,A*77
// Header,Time,fix,Lat,Lat_dir,Lon,Lon_dir,knots,Heading,Date,Magnetic Variation, Magnetic Variation Direction,Checksum
bool GPS::updateData(void){
	memset(_outputSent,0,64);
	Serial.println(_cSent);
	if (strncmp(_cSent,"$GPRMC",6) == 0){
		// Find the index values of each of the 12 commas in the GPRMC string
		uint8_t outputCnt = 0;
		uint8_t cma_idx[12];
		uint8_t cma_cnt = 0;
		for(uint8_t idx=0; idx<_cSentLength; idx++){
			if(_cSent[idx] == ','){
				cma_idx[cma_cnt] = idx;
				cma_cnt++;
			}
		}
		// Update Time
		if (cma_idx[1]-cma_idx[0] > 5){
			_hr = (uint8_t)parseInt(cma_idx[0]+1,2);
			_mn = (uint8_t)parseInt(cma_idx[0]+3,2);
			_sc = parseFloat(cma_idx[0]+5,cma_idx[1]-cma_idx[0]-4);
		}
		// Update Date
		if (cma_idx[9]-cma_idx[8] == 7){
			_da = (uint8_t)parseInt(cma_idx[8]+1,2);
			_mo = (uint8_t)parseInt(cma_idx[8]+3,2);
			_yr = 2000 + (uint16_t)parseInt(cma_idx[8]+5,2);
		}
		outputCnt = sprintf(_outputSent,"%04u/%02u/%02u\t%02u:%02u:%02ld.%03ld",
			_yr,_mo,_da,_hr,_mn,(int32_t)round(_sc*1000.0)/1000,(int32_t)round(_sc*1000.0)%1000);
		
		// Update GPS Fix
		if (_cSent[cma_idx[1]+1] == 'A'){
			_fix = true;
			// Update Latitude
			if (cma_idx[3]-cma_idx[2] > 7){
				_lat = (float)parseInt(cma_idx[2]+1,2) + parseFloat(cma_idx[2]+3,cma_idx[3]-cma_idx[2]-2)/60.0;
				if (_cSent[cma_idx[3]+1] == 'S'){
					_lat = -_lat;
				}
			}
			// Update Longitude
			if (cma_idx[5]-cma_idx[4] > 8){
				_lon = (float)parseInt(cma_idx[4]+1,3) + parseFloat(cma_idx[4]+4,cma_idx[5]-cma_idx[4]-3)/60.0;
				if (_cSent[cma_idx[5]+1] == 'W'){
					_lon = -_lon;
				}				
			}
			// Update Speed and convert to m/s
			if (cma_idx[7]-cma_idx[6] > 1){
				_speed = parseFloat(cma_idx[6]+1,cma_idx[7]-cma_idx[6])*0.514444;
			}
			// Update Heading
			if (cma_idx[8]-cma_idx[7] > 1){
				_heading = parseFloat(cma_idx[7]+1,cma_idx[8]-cma_idx[7]);
			}
			// Print Data
 			sprintf(&_outputSent[outputCnt],"\t%ld.%05ld, %ld.%05ld\t%ld.%02d, %ld.%02ld",
				(int32_t)round(_lat*100000.0)/100000,abs((int32_t)round(_lat*100000.0)%100000),
				(int32_t)round(_lon*100000.0)/100000,abs((int32_t)round(_lon*100000.0)%100000),
				(int32_t)round(_speed*100.0)/100,abs((int32_t)round(_speed*100.0)%100),
				(int32_t)round(_heading*10.0)/10,abs((int32_t)round(_heading*10.0)%10)); 
 			Serial.println(_outputSent);
		} else{
			_fix = false;
			Serial.println(_outputSent);
		}
	}
	memset(_cSent,0,128);
	_cSentLength = 0;
	return true;
}


// Private Functions
uint32_t GPS::determineBaud(void){
	for(uint8_t baudCnt = 0; baudCnt<sizeof(_baudRates)/sizeof(_baudRates[0]); baudCnt++){
		Serial1.begin(_baudRates[baudCnt]);
		Serial1.setTimeout(50);
		delay(1000);
		for(uint8_t readAttempt=0; readAttempt<=100; readAttempt++){
			if (Serial1.available()){
				memset(_cSent,0,128);
				Serial1.readBytesUntil('\n',_cSent,128);
				if (strncmp(_cSent,"$GP",3) == 0 || strncmp(_cSent,"$PM",3) == 0){
					return _baudRates[baudCnt];
				} 
			} else{
				delay(100);
			}
		}
		Serial1.end();
	}
	return 0;
}

bool GPS::verifyChecksum(void){
	uint8_t calc_chksum = 0;
	uint8_t read_chksum = 0;
	for(int i=1; i<_cSentLength-4; i++){
		calc_chksum ^= (uint8_t)_cSent[i];
	}
	read_chksum = (uint8_t)strtol(&_cSent[_cSentLength-3], NULL, 16);
	return read_chksum == calc_chksum;
}

int32_t GPS::parseInt(uint8_t idx, uint8_t nChars){
	int32_t value = 0;
	bool negative = false;
	if (_cSent[idx] == '-'){
		idx = idx + 1;
		nChars = nChars - 1;
	}
	for (uint8_t i=0; i<nChars; i++){
		value = 10*value + (int32_t)(_cSent[idx+i]-'0');
	}
	if (negative){
		return -value;
	} else{
		return value;
	}
}


float GPS::parseFloat(uint8_t idx, uint8_t nChars){
	float integer_value = 0;
	float decimal_value = 0;
	uint8_t dp_idx = nChars+1;
	for (uint8_t i = 0; i<nChars; i++){
		if (_cSent[idx+i] == '.'){
			dp_idx = i;
			break;
		}
	}
	integer_value = (float)parseInt(idx,dp_idx);
	decimal_value = (float)parseInt(idx+dp_idx+1,nChars-1-dp_idx)*pow(10.0,-(nChars-(dp_idx+1)));
	if (integer_value < 0){
		return integer_value - decimal_value;
	} else{
		return integer_value + decimal_value;
	}
}
