// This code is to read data from the analog pins of the Arduino UNO saving it to a SD card.
//all the data is processed post processing to save precious sampling time.  
//Before you run the code connect the Arduino UNO to a computer via USB and then open the Serial Monitor


#include <SD.h>
#include <SPI.h>
#include <RTClib.h>

// Init the DS3231 using the hardware interface
RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


//Initialize the Analog pin variables. 
int potPin0=A0;
int potPin1=A1;
int potPin2=A2;
int potPin3=A3;

//Initilaize the analogread variables. 
int Av0; 
int Av1;
int Av2;
int Av3;

//Initialize the voltage variables
float Voltage0; 
float Voltage1; 
float Voltage2;
float Voltage3; 


//++++++++++++++++++++++++++++++++++++++++++++++ INITIALIZATION OF THE SD CARD READER
File DataLog; //Data object you will write your sesnor data to
int chipSelect=10;
//_____________SETUP FOR THE FILE NAME

//Now we need the variables to make the file name: 
//The file name has to be 8.3 to be used with the SD.h library. So, based on the program found on the 
//thread https://forum.arduino.cc/index.php?topic=543788.0
//We need to make sure filename is 12Bytes and set it up as follows. 
char filename[12]="Dt";

//the creation of the "date" part of the program follows the info found on this thread:
// https://forum.arduino.cc/index.php/topic,148888.0.html
//the program of the forum thread is in my file SD-Onenate-LogFilename_from_DATE_DS1307_20200609.ino
char datename[30];


//----------------------------------VOID SETUP -----------------------------------------
void setup() {

  
//++++++++++++++++++++++++++++++++++++++++++++Setup the RTC module 

//wiring: 
//Connect SDA with A4 or SDA on the Arduino
//Connect SCL with A5 or SCL on the Arduino

//Usually the rtc will initialize at the next "if" statement but I saw that some card readers don't understand that and need the "rtc.begin()" command.
// THIS IS NEEDED FOR SOME MOUDULES SUCH AS THE DS1307. IT MAY BE COMMENTED FOR THE DS3231: CHECK BEFORE DECIDING. 
rtc.begin();

//----------------------NOTE YOU NEED TO USE THE FOLLOWING LINE TO SET THE TIME ON THE RTC. UNSE THIS LINE ONE TIME WHEN YOU UPLOAD THE CODE THE FIRST TIME. 
//----------------------THEN COMMENT AND RE-UPLOAD THE CODE AGAIN. ------------------------------------------------------------------------------------------------------------
//COMMENT OR UNCOMMENT THE FOLLOWING LINE AS NEEDED. IF YOU USE THE DS1307 YOU MAY WANT TO SYNC TIME EVERY TIME YOU CONNECT THE BOARD TO THE COMPUTER
//BECAUSE IT LOOSES PRECISION. CHECK WHAT HAPPENS FOR EACH MODULE USED AND THEN DECIDE WHAT TO DO. 
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ SETUP the analog reference to be internal.
//Choices are INTERNAL, EXTERNAL, DEFAULT. 
//IF YOU DECIDE TO MEASURE THE VREF AS PER ANDREAS SPIESS' VIDEO THEN YOU NEED TO COMMENT THE FOLLOWING LINE
//LEAVE THE burn8Reading() FUNCTION RUNNING BECAUSE YOU ARE CHANGING VOLTAGE REFRERENCE. 
analogReference(EXTERNAL); 

//++++++++++++++++++++Burn the first readings as suggested by FORCETRONIC 
////http://forcetronic.blogspot.com/2015/01/maximizing-arduinos-adc-resolution-and.html
burn8Readings();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ SETUP the Analog Pins 
pinMode(potPin0, INPUT);  //declare the type of PotPin
pinMode(potPin1, INPUT);
pinMode(potPin2, INPUT);
pinMode(potPin3, INPUT);

 //++++++++++++++++++++++++++++++++++++INITIALIZE THE SERIAL PORT:
Serial.begin(9600);

//+++++++++++++++++++++++++++++++++++++INITIALIZE THE SD CARD READER:
//Wiring:
//Connect SCK to pin 13
//Connect MISO to pin 12
//Connect MOSI to pin 11
//Connect CS to pin 10 (chipSelect=10)

pinMode(chipSelect, OUTPUT); //Must declare 10 an output and reserve it

// --------------------------------- CREATE THE FILE NAME:

DateTime now = rtc.now();
sprintf(datename, "%02d%02d%02d.csv", now.hour(), now.minute(), now.second());
strcat(filename,datename);
    
//you don't need to run the SD.begin because is run inside the if function below.
//i will leave it here commented because it may be needed depending on the module used. 
//SD.begin(chipSelect); //Initialize the SD card reader

if (!SD.begin(chipSelect)) { // see if the card is present and can be initialized:
  Serial.println("Card failed, or not present");
  while (1); // wait until card is recognized... 
  //return;//exit the setup function. This quits the setup() and the program counter jumps to the loop().
  }
//If the file exists, then wait one second a create a new file name.
  while (SD.exists(filename)) {
    delay(1000);
    DateTime now = rtc.now();
    sprintf(datename, "%02d%02d%02d.csv", now.hour(), now.minute(), now.second());
    strcat(filename,datename);
    Serial.print("I had to wait an extra second to create the file name:");Serial.println(filename);
  }
Serial.print("SD Card OK - Filename:");Serial.println(filename);

DataLog = SD.open(filename, FILE_WRITE);
if (!DataLog) {
    Serial.println(F("open failed"));
    return;
  }
  Serial.print(F("Check file Ok!! The file "));
  Serial.print(filename);
  Serial.println(" was opened succesfully!");
  DataLog.close();
  
//Create headers
DataLog = SD.open(filename, FILE_WRITE);
DataLog.print("RTCDate and time = ");DataLog.print(",");
DataLog.print(now.year());
DataLog.print("-");
DataLog.print(now.month());
DataLog.print("-");
DataLog.print(now.day());
DataLog.print("  ");
DataLog.print(now.hour());
DataLog.print(":");
DataLog.print(now.minute());
DataLog.print(":");
DataLog.print(now.second());
DataLog.print(",");
DataLog.print("Aref= 4960mV"); //put the value of Aref used for the sampling of the data
DataLog.print(",");
DataLog.println("64 sps (15.625[mS] btw samples)"); //verify the sampling rate for the data recorded during this istance. 
DataLog.print("A0");DataLog.print(",");
DataLog.print("A1");DataLog.print(",");
DataLog.print("A2");DataLog.print(",");
DataLog.println("A3");
DataLog.close();

Serial.println("Headers Created...ready to go!");

}

//----------------------------------VOID LOOP -----------------------------------------
void loop() {

//Reading analog inputs: 
Av0 = analogRead(potPin0);
Av1 = analogRead(potPin1);
Av2 = analogRead(potPin2);
Av3 = analogRead(potPin3);

//Convert the recoded values above into voltage values multiplying by Aref and dividing by the total number of bits
//which is 1024. (I DO THIS POST PROCESSING).
//If Arduino is doing the calculation, then you need to add a "dot" after the number to instruct Arduino 
//to consider the nubmers as decimals.Without the "dot" Arduino will round the number to the closes integer.
// The formula is: Voltage=(Aref/1024.)*Av0;   


//Write the data on the SD card:
DataLog = SD.open(filename, FILE_WRITE);
if (DataLog) {
DataLog.seek(EOF);
DataLog.print(Av0);
DataLog.print(",");
DataLog.print(Av1);
DataLog.print(",");
DataLog.print(Av2);
DataLog.print(",");
DataLog.println(Av3);
DataLog.close();
}
else {
      Serial.println("ERROR OPENING THE FILE");
    } 

//DISPLAY THE VALUES ON THE SERIAL MONITOR WITH THE CODE BELOW: 
//COMMENT OUT IF NECESSARY. 
//DateTime now = rtc.now();
//Serial.print(F("A0= "));Serial.println(Av0); //Serial.print(F("  "));Serial.print(F("V0="));Serial.print(Voltage0);Serial.print(F(" - "));

//+++++++++++++++++++++++++++   END OF VOID LOOP +++++++++++++++++++++++++++++++++++++++++
}

//++++++++++++++++++++++++++BURN8READINGS FUNCTION FROM FORCETRONICS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//http://forcetronic.blogspot.com/2015/01/maximizing-arduinos-adc-resolution-and.html
//This function makes 8 ADC measurements but does nothing with them
//Since after a reference change the ADC can return bad readings at first. This function is used to get rid of the first 
//8 readings to ensure an accurate one is displayed
void burn8Readings() {
  for(int i=0; i<8; i++) {
    analogRead(A0);
    delay(1);
  }
}
