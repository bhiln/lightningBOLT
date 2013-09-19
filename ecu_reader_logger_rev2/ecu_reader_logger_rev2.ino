





/* Welcome to the ECU Reader project. This sketch uses the Canbus library.
It requires the CAN-bus shield for the Arduino. This shield contains the MCP2515 CAN controller and the MCP2551 CAN-bus driver.
A connector for an EM406 GPS receiver and an uSDcard holder with 3v level convertor for use in data logging applications.
The output data can be displayed on a serial LCD.

The SD test functions requires a FAT16 formated card with a text file of WRITE00.TXT in the card.


SK Pang Electronics www.skpang.co.uk
v4.0 04-03-12 Updated for Arduino 1.0
v3.0 21-02-11  Use library from Adafruit for sd card instead.

*/

#include <SD.h>        /* Library from Adafruit.com */
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>


File file;
tCAN message;

/* Define Joystick connection */
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  A4
#define LEFT   A5

  
unsigned char buffer[512];  //Data will be temporarily stored to this buffer before being written to the file
char tempbuf[15];
char lat_str[14];
char lon_str[14];

int read_size=0;   //Used as an indicator for how many characters are read from the file
int count=0;       //Miscellaneous variable

int D10 = 10;

int LED2 = 8;
int LED3 = 7;
int ANALOG2 = 2;
int ANALOG3 = 3;
int ANALOG4 = 4;

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))


#define COMMAND 0xFE
//#define powerpin 4


// GPS parser for 406a
#define BUFFSIZ 90 // plenty big
//char buffer[BUFFSIZ];
char *parseptr;
char buffidx;
uint8_t hour, minute, second, year, month, date;
uint32_t latitude, longitude;
uint8_t groundspeed, trackangle;
char latdir, longdir;
char status;
uint32_t waypoint = 0;
 
void setup() {
  //Serial.begin(GPSRATE);
  pinMode(LED2, OUTPUT); 
  pinMode(LED3, OUTPUT);
  pinMode(ANALOG3, OUTPUT);
  
  digitalWrite(LED2, LOW);
  pinMode(UP,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(LEFT,INPUT);
  pinMode(RIGHT,INPUT);
  pinMode(CLICK,INPUT);

  digitalWrite(UP, HIGH);       /* Enable internal pull-ups */
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);
  digitalWrite(CLICK, HIGH);
  
  
  Serial.begin(9600);
  Serial.println("ECU Reader");  /* For debug use */
  
  if(Canbus.init(CANSPEED_250))  /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.println("CAN Init ok");
  } else
  {
    Serial.println("Can't init CAN");
  } 
  
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(9)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  // open a file
  
  logging();
  
  while(1){
    Serial.println("Waiting for button click to start logging.");
    if (digitalRead(RIGHT) == 0) {
    
      Serial.println("Logging");
      logging();
    }
  }
}
 

void loop() {
  
}


void logging(void)
{
  
  
  
  file = SD.open("datalog.txt", FILE_WRITE);
  
  if (file) {
    // print to the serial port too:
    //Serial.println("Successfully opened the SD Card");
  }  
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening datalog.txt"); return;
  }

  digitalWrite(LED2, HIGH);

  //Serial.println("Writing to: datalog.txt");
  
  // write header
  file.print("READY....");
  file.println();  
  int sensorValue = 0;
  int num = 0;
  int data;
  int length;
  double temp;
  double time;
  double rpm;
  unsigned int halfTemp;
  unsigned int halfTime;
  unsigned int halfRPM;

  while(1)    /* Main logging loop */
  {
      message = Canbus.message_rx();
      //Serial.println(String(message.id, HEX));
      //Serial.println(message.data[0]);
      if (message.id != 0x000){
        file.print(micros(),DEC);
        file.print(": ");
        file.print(message.id,HEX);
        //Serial.print(message.id,HEX);
        for(int i = 0; i < 8; i++)
               {
                 data = message.data[i];
                 
                 if (data == 0x00){
                   file.print("00");
                   //Serial.print("00");
                 }
                 else{
                   file.print(data,HEX);
                   //Serial.print(data,HEX);
                 }
                 
               }
        file.println();
        //Serial.println();
        
        //if (message.id == 0xBB){
        //  temp = message.data[0];
        //  temp = (temp/65)*255;
        //  analogWrite(ANALOG3, temp);
          
          //Serial.print("Battery SOC: ");
          //Serial.println(message.data[1], DEC);
        //}         
        
        //if message is from controller
        if (message.id == 0x181){
          
          halfTime = message.data[1];
          halfTime = halfTime << 8;
          time = halfTime | message.data[0];
                    
          //battery temperature
          halfTemp = message.data[3];
          halfTemp = halfTemp << 8;
          temp = halfTemp | message.data[2];
          Serial.print("temp: ");
          Serial.print(temp);
          temp = ((temp/14)/5)*255;
          Serial.print(" pwm: ");
          Serial.println(temp);
          
          //motor rpm
          halfRPM = message.data[5];
          halfRPM = halfRPM << 8;
          rpm = halfRPM | message.data[4];
          Serial.print("rpm: ");
          Serial.print(rpm);
          rpm = ((rpm/1600)/5)*255;
          Serial.print(" pwm: ");
          Serial.println(rpm);
          
          analogWrite(ANALOG2, time);
          analogWrite(ANALOG3, temp);
          analogWrite(ANALOG4, rpm);
        }
        
        
        //Canbus.message_rx(buffer);
        //Serial.print(buffer[10]);
        //Serial.print(buffer[1]);
        //Serial.print(buffer[2]);
        //Serial.print(buffer[3]);
        //Serial.print(buffer[4]);
        //Serial.print(buffer[5]);
        //Serial.print(buffer[6]);
        //Serial.print(buffer[7]);
        //Serial.println();
        //file.println(buffer);
        
      //  Canbus.ecu_req(O2_VOLTAGE,buffer);   
   
         if (digitalRead(CLICK) == 0){  /* Check for Click button */
             file.close();
             Serial.println("Done");
             digitalWrite(LED2, LOW);
             break;
          }
    }
  }
 
 
 
}
    
void pwm(void)
{
  int sensorValue = 0;
  int num = 1;

  while(1)    /* Main logging loop */
  {
      if (num == 1){
        digitalWrite(13, HIGH);
        num = 0;
      }
      else{
        digitalWrite(13, LOW);
      }
      sensorValue = analogRead(5);
      Serial.print("Analog in: ");
      Serial.print(sensorValue);
      Serial.println();
  }
}

void sd_test(void)
{
 Serial.println("SD card test");
   
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(9)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  // open a file
  file = SD.open("datalog.txt", FILE_WRITE);
  if (file) {
    // print to the serial port too:
    Serial.println("Successfully opened the SD Card");
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  
  file.close();
    

}
