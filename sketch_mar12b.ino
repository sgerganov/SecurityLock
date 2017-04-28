//#include <Password.h>
#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h> // NFC
#include <Adafruit_SSD1306.h> // Display
#include <Adafruit_GFX.h> // Display
#include <SD.h>
#include "RTClib.h"


/***********************************************************************************/
#define PN532_IRQ   (2) // NFC
#define PN532_RESET (3) // NFC
#define OLED_RESET 4 // Display
#define Password_Lenght 7
/***********************************************************************************/
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET); // NFC
Adafruit_SSD1306 display(OLED_RESET); // Display
/************************** Initialize the keypad **********************************/

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {24, 25, 26, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {28, 29, 30, 31}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
/***********************************************************************************/
int sv = 0;
byte result;
char a;
String uname;

char Data[Password_Lenght]; // 6 is the number of chars it can hold + the null char = 7
char Master[Password_Lenght] = "123456"; 
char Master1[Password_Lenght] = "654321"; 
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;

uint8_t success;
 uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
 uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
 String content= "";
 String content1;
 
File myFile;
RTC_DS3231 rtc;

 char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
/*************************** Methods ***********************************************/
void displaySetup(){
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void displayClear(){
  display.clearDisplay();
}

void displayData(){
  display.display();
}

void displayString(String a){
  display.print(a);
}

void displayStr(String b) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(b);
  display.display();
}

void displayStrI(String c) {
  //display.clearDisplay();
  display.setCursor(127,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(c);
  display.display();
}

void displayChar(char d) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(d);
  display.display();
}

 void clearData()
{
  while(data_count !=0 ||  content !="" || uname !="")
  {   // This can be used for any array size, 
    Data[data_count--] = 0; //clear array for new data
    content = "";
    uname = "";
   
  }
  return;
}

void writeToSD_OK(){
myFile = SD.open("test.txt", FILE_WRITE);
 if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println(uname);
    myFile.print("/");
    myFile.print(content);
    
    DateTime now = rtc.now();
myFile.println();
 myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(" (");
    myFile.print(daysOfTheWeek[now.dayOfTheWeek()]);
    myFile.print(") ");
   myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
   myFile.print(':');
    myFile.print(now.second(), DEC);
    
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
}
void writeToSD_NOK(){
myFile = SD.open("test1.txt", FILE_WRITE);
 if (myFile) {
    Serial.print("Writing to testNOK.txt...");
     myFile.println(uname);
    myFile.print("/");
    myFile.print(content);
    
DateTime now = rtc.now();
myFile.println();
 myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(" (");
    myFile.print(daysOfTheWeek[now.dayOfTheWeek()]);
    myFile.print(") ");
   myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
   myFile.print(':');
    myFile.print(now.second(), DEC);
    

    
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
}


/***********************************************************************************/

void setup() {

Serial.begin(115200);
//keypad.addEventListener(keypadEvent); //add an event listener for this keypad
//keypad.setDebounceTime(250);
nfc.begin(); // Initialize NFC
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize Display

 Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }


uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    displayStr("No NFC");
    while (1); // halt
  }

 /* Set the max number of retry attempts to read from a card
    This prevents us from waiting forever for a card, which is
    the default behaviour of the PN532.
 */
  nfc.setPassiveActivationRetries(0xFF);
  
// configure board to read RFID tags
  nfc.SAMConfig();
  
displayStr("Init OK"); // Check the initialization. 
delay(1000); // Wait before continue to the loop.
 
}

void loop() {

  displayStr("Waiting   for Card");

// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
switch(sv){

case 0: 
success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
Serial.println("Found a card!");

for (byte i = 0; i < uidLength; i++) {
    content.concat(String(uid[i] < 0x10 ? " 0" : " "));
    content.concat(String(uid[i], HEX));
}
  content.toUpperCase();
  Serial.println(content);

if (content.substring(1) == "30 E5 83 1E"){
 displayStr("Hi SVGE");
 uname = "SVGE";
  sv = 1;
delay(2000);
}

else if (content.substring(1) == "CD 58 07 2E"){
 
  displayStr("Hi BGB");
 uname = "BGB";
 sv = 1;
 delay(2000); 
 
}
else {
  displayStr("Wrong card!");
  delay(2000); 
 // clearData();
//  delay(2000); 
writeToSD_NOK();
  delay(1000);
  clearData();
  delay(1000);
 sv = 0;
}
 
Serial.println(sv);
Serial.println(content);
Serial.println(uname);
break;

case 1:
 displayStr("Enter PIN");
 customKey = keypad.getKey();
while(customKey == NO_KEY){
  customKey = keypad.getKey();
  Serial.print(customKey);
  
 
  } //while
  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {
    Data[data_count] = customKey; // store char into data array
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
     
  }

  if(data_count == Password_Lenght-1 ) // if the array index is equal to the number of expected chars, compare data to master
  { //if
   if(uname == "SVGE") {
    if(!strcmp(Data, Master)) { // equal to (strcmp(Data, Master) == 0)
      displayStr("Correct   PIN");
      Serial.println("Good");
      delay(2000);
     // clearData();
       sv = 3;
    } 
    else {
      displayStr("Bad PIN");
    Serial.println("Bad");
    delay(2000);
    clearData();
    sv = 0;
    }
   }
    else if (uname == "BGB") {
      if(!strcmp(Data, Master1)) { // equal to (strcmp(Data, Master) == 0)
      displayStr("Correct   PIN");
      Serial.println("Good");
      delay(2000);
     // clearData();
      sv = 3;
      
    } 
    else {
      displayStr("Bad PIN");
    Serial.println("Bad");
    delay(2000);
    clearData();
    sv = 0;
    }
    }
      
   else {
     displayStr("Unknown PIN");
    Serial.println("Bad");
    delay(2000);
    clearData();
    sv = 0;
   }

   
  } //endif
break;

case 3: 
displayStr("Opening   the Door");
//Relay run here;
delay(2000);
writeToSD_OK();
displayStr("Wellcome!");
clearData();
delay(2000);
sv=0;
break;




}// END of Switch
}

