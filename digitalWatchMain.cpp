#define DEBUG 
#include <Arduino.h>
#include <TinyWireM.h>
#include <TinyDebug.h>
#include <TM1637Display.h>

// TM1637 PINS
#define DIO 3 
#define CLK 4 

#define DS1307_ADDR 0X68        //< DS1307 slave address 

TM1637Display display(CLK, DIO); // Init TM1637  


/**
 * DS1307 struct 
*/ 
struct RTC_time_t {
	uint8_t seconds;  //< 0-59  
	uint8_t minutes;  //< 0-59 
	uint8_t hours;    //< 0-23
}

RTC_time_t currentTime;


// Timer variables
unsigned long previousMillis = 0UL;
unsigned long interval = 1000UL;

void init_system(); 
void ds1307_get_time(RTC_time_t* rtc_time); 
void ds1307_set_time(RTC_time_t* rtc_time);
uint8_t decToBcd(uint8_t val);
uint8_t bcdToDecd(uint8_t val);

/**
 * Main program 
 */
int main(void) {
  init_system(); // Init. the system  

  while(1) {
    unsigned long currentMillis = millis();

    if(currentMillis - previousMillis > interval) {
      ds1307_get_time(&currentTime); // Get current time 
      display.showNumberDecEx(currentTime.hours * 100 + currentTime.minutes, //Display time 
                          (currentTime.seconds % 2) ? 0x00: 0x80, true);
    }
    previousMillis = currentMillis;
  }
}

/**
 * Init. the system
 */
void init_system() {
  #ifdef DEBUG 
    Debug.begin();
  #endif 
  
  // Setup TinyWire I2C
  TinyWireM.begin();

  display.setBrightness(0x0f); // Set the display brightness (0x00 to 0x0f)
  
  /**
   * Sets desired time (current time) 
   * NOTE - uncomment after setting the time  
  */   
  currentTime.hours = 00; 
  currentTime.minutes= 06; 
  currentTime.seconds = 30; 
  
  ds1307_set_time(&currentTime); 
}

/**
 * Get current time from DS1307
 * 
 * @param rtc_time - pointer to RTC_time_t struct 
 */
void ds1307_get_time(RTC_time_t* rtc_time)
{
  TinyWireM.beginTransmission(DS1307_ADDR); // Reset DS1307 register pointer
  TinyWireM.send(0x00);
  TinyWireM.endTransmission();

  // Request 7 bytes from DS1307
  TinyWireM.requestFrom(DS1307_ADDR, 7);

  rtc_time->seconds = bcdToDec(TinyWireM.receive() & 0x7F);
  rtc_time->minutes = bcdToDec(TinyWireM.receive());
  rtc_time->hours = bcdToDec(TinyWireM.receive());

  #ifdef DEBUG
    Debug.print("Hours:");
    Debug.println(rtc_time->hours);
    Debug.print("Minutes:");
    Debug.println(rtc_time->minutes);
    Debug.print("Seconds:");
    Debug.println(rtc_time->seconds);
    Debug.println();
    Debug.println();
  #endif
}

/**
 * Set desired time to DS1307
 * 
 * @param rtc_time - pointer to RTC_time_t struct 
 */
void ds1307_set_time(RTC_time_t* rtc_time) {
  TinyWireM.beginTransmission(DS1307_ADDR); 
  TinyWireM.send(0x00); // Point to seconds regsiter 
  TinyWireM.send(decToBcd(rtc_time->seconds)); 
  TinyWireM.send(decToBcd(rtc_time->minutes)); 
  TinyWireM.send(decToBcd(rtc_time->hours));
  TinyWireM.endTransmission(); 
}


/**
 * Convert binary coded decimal to normal decimal numbers
 * 
 * @param val - binary coded decimal number
 * @return - normal decimal number 
 */
uint8_t bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}


/**
 * Convert normal decimal numbers to binary coded decimal
 * 
 * @param val - normal decimal number
 * @return - binary coded decimal number 
 */
uint8_t decToBcd(uint8_t val) { 
  return ((val / 10 * 16) + (val % 10)); 
}