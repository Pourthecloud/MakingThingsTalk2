

/*
 Twitter RFID Web Client
 Language: Arduino
 */
#include <SPI.h>
#include <Ethernet.h>
#include <TextFinder.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SonMicroReader.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };
//IPAddress ip(192,168,1,20);               // will only be used if DHCP fails
IPAddress ip(128,122,151,6);               // will only be used if DHCP fails

IPAddress server(199,59,149,200);    // Twitter's API address
Client client;                            // the client connection

String twitterHandle = "";       // tweet handle to come from RFID tag
String tweet = "";               // the tweet
char tweetBuffer[150];
boolean readingTweet = false;    // if you're in the middle of a tweet

long lastRequestTime = 0;       // last time you connected to the server, in milliseconds
int requestDelay = 10 * 1000;   // time between HTTP requests to the same twitter ID

SonMicroReader rfid;                // instance of the SonMicroReader library
unsigned long lastTag = 0;          // address of the previous tag
int addressBlock = 4;               // memory block on the tag to read

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9,8, 5, 4, 3, 2);
const int screenWidth = 16;         // width of the LCD in characters
String shortString = "";           // the string to display on the screen
long lastScrollTime = 0;            // last time you scrolled the LCD   
int scrollDelay = 250;              // delay between LCD moves
int cursorPosition = 0;                   // first position of the tweet that's on the LCD

int status = 0;

void setup() {
  // start the serial library:
  Serial.begin(9600);
  //if(!Ethernet.begin(mac)) {
    // start the Ethernet connection:
    Ethernet.begin(mac, ip);
 // }
   // reserve space for the tweet strings:
  twitterHandle.reserve(50);
  tweet.reserve(160);
  shortString.reserve(20);
  // initialize and reset the reader:
  rfid.begin();

  // set up the LCD's number of columns and rows: 
  lcd.begin(screenWidth,2);
  // print something to the LCD:
  scrollLongString(0);
  // give the Ethernet shield and RFID reader
  // two seconds to initialize:
  delay(2000);
  lcd.println("Starting");
}

void loop() {

  unsigned long tag = 0;
  switch(status) {
  case 0:    // get tag
    // try to get a tag:
    tag = rfid.selectTag();
    if (tag != 0) {
      // you have a tag, so print it:
      Serial.println(tag, HEX);  
      status++;
    }
    break;
  case 1:    // read block
    if (rfid.authenticate(addressBlock)) {
      // read the tag for the twitter handle:
      rfid.readBlock(addressBlock);
      twitterHandle = rfid.getString();
      Serial.println(twitterHandle);
      status++;
    }

    break;
  case 2:    //connect to server
    // if this is a new tag, or if the request delay
    // has passed since the last time you made a HTTP request:
    if (tag != lastTag || 
      millis() - lastRequestTime > requestDelay) {
      // attempt to connect:
      connectToServer();
      Serial.println("Back from request");
      status++;
    } 
    break;
  case 3:    // read response
    readResponse();
    break;
  }

  // if you haven't moved the LCD recently:
  if (millis() - lastScrollTime > scrollDelay) {
    // advance the LCD:
    scrollLongString(cursorPosition);
    // increment the LCD cursor position:
    if (cursorPosition < tweet.length()) {
      cursorPosition++;
    } 
    else {
      cursorPosition = 0;
    }
    // note the last time you moved the LCD:
    lastScrollTime = millis();    
  } 
}

void readResponse() {
  // if there are bytes available from the server:
  if (client.available()) {
    // make an instance of TextFinder to search the response:
    TextFinder response(client);
    Serial.write(client.read());
    // see if the response from the server contains <text>:
    response.getString("<text>", "</text>", tweetBuffer, 150);
    // print the tweet string:
    Serial.println(tweetBuffer);
    // you only care about the tweet:
    client.stop();
    status = 0;
  }
}

// this method takes a substring of the long
// tweet string to display on the screen

void scrollLongString(int startPos) {
  // string that will be on the screen:
  shortString = "";
  // make sure there's enough of the long string left:
  if (startPos < tweet.length() - screenWidth) {
    // take a 16-character substring:
    shortString = tweet.substring(startPos, startPos + screenWidth);
  } 
  // refresh the LCD:
  lcd.clear();              // clear previous stuff
  lcd.print(twitterHandle);       // tweet handle on the top line
  lcd.setCursor(0,1);       // move cursor down
  lcd.print(shortString);   // tweet, scrolling, on the bottom
}













