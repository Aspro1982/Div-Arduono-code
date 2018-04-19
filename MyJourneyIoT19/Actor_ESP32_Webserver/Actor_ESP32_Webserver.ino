/*
IOT Actor Board Webserver Demo
for ESP32 DevKitC
Elektor / Jens Nickel
My Journey in the IoT
*/


#include <WiFi.h>


// +++++++++++++++++++ Start of Webserver Library +++++++++++++++++++

// as ESP32 Arduino Version

WiFiClient myclient;
WiFiServer server(80);


void Webserver_Start()
{
  server.begin();     // Start TCP/IP-Server on ESP32
}


//  Call this function regularly to look for client requests
//  template see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/SimpleWiFiServer/SimpleWiFiServer.ino
//  returns empty string if no request from any client
//  returns GET Parameter: everything after the "/?" if ADDRESS/?xxxx was entered by the user in the webbrowser
//  returns "-" if ADDRESS but no GET Parameter was entered by the user in the webbrowser
//  remark: client connection stays open after return
String Webserver_GetRequestGETParameter()
{
  String GETParameter = "";
  
  myclient = server.available();   // listen for incoming clients

  //Serial.print(".");
  
  if (myclient) {                            // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                 // make a String to hold incoming data from the client
    
    while (myclient.connected()) {           // loop while the client's connected
      
      if (myclient.available()) {            // if there's bytes to read from the client,
        
        char c = myclient.read();            // read a byte, then
        Serial.write(c);                     // print it out the serial monitor

        if (c == '\n') {                     // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request
          if (currentLine.length() == 0) {
            
            if (GETParameter == "") {GETParameter = "-";};    // if no "GET /?" was found so far in the request bytes, return "-"
            
            // break out of the while loop:
            break;
        
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
          
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (c=='\r' && currentLine.startsWith("GET /?")) 
        // we see a "GET /?" in the HTTP data of the client request
        // user entered ADDRESS/?xxxx in webbrowser, xxxx = GET Parameter
        {
          
          GETParameter = currentLine.substring(currentLine.indexOf('?') + 1, currentLine.indexOf(' ', 6));    // extract everything behind the ? and before a space
                    
        }
        
      }
      
    }
    
  }

  return GETParameter;
}



// Send HTML page to client, as HTTP response
// client connection must be open (call Webserver_GetRequestGETParameter() first)
void Webserver_SendHTMLPage(String HTMLPage)
{
   String httpResponse = "";

   // begin with HTTP response header
   httpResponse += "HTTP/1.1 200 OK\r\n";
   httpResponse += "Content-type:text/html\r\n\r\n";

   // then the HTML page
   httpResponse += HTMLPage;

   // The HTTP response ends with a blank line:
   httpResponse += "\r\n";
    
   // send it out to TCP/IP client = webbrowser 
   myclient.println(httpResponse);

   // close the connection
   myclient.stop();
    
   Serial.println("Client Disconnected.");
   
};




// +++++++++++++++++++ End of Webserver library +++++++++++++++++++++




// SSID and Password of your Wifi network
const char* txtSSID = "YOUR SSID";
const char* txtPassword = "YOUR PASSWORD";




#define PIN_LED_BOARD  12
#define PIN_ACTOR      12   // the digital output we want to control
#define PIN_LED_GREEN  27
#define PIN_LED_RED    26

#define LED_YELLOW  3
#define LED_GREEN   2
#define LED_RED     1

#define LED_ON    1
#define LED_OFF   0

#define ACTOR_ON    1
#define ACTOR_OFF   0




// Switch Actor Output
// ActorState must be ACTOR_OFF or ACTOR_ON
void SwitchActor(byte ActorState)
{
    if (ActorState == ACTOR_OFF)
    {
      digitalWrite(PIN_ACTOR, LOW);
    }
    else
    {      
      digitalWrite(PIN_ACTOR, HIGH);
    }
}


// Switch LED on the board
// LEDState must be LED_OFF or LED_ON
void SwitchBoardLED(byte LEDState)
{
    if (LEDState == LED_OFF)
    {
      digitalWrite(PIN_LED_BOARD, LOW);
    }
    else
    {      
      digitalWrite(PIN_LED_BOARD, HIGH);
    }
}


// Switch LEDs of RGBLED
// LEDColor must be LED_GREEN, LED_RED, LED_YELLOW or LED_OFF
void SwitchRGBLED(byte LEDColor)
{
    switch(LEDColor)
    {     
      case LED_GREEN:
         digitalWrite(PIN_LED_GREEN, HIGH);
         digitalWrite(PIN_LED_RED, LOW);
      break;
      
      case LED_RED:
         digitalWrite(PIN_LED_GREEN, LOW);
         digitalWrite(PIN_LED_RED, HIGH);
      break;
      
      case LED_YELLOW:
         digitalWrite(PIN_LED_GREEN, HIGH);
         digitalWrite(PIN_LED_RED, HIGH);
      break;
      
      case LED_OFF:
         digitalWrite(PIN_LED_GREEN, LOW);
         digitalWrite(PIN_LED_RED, LOW);
      break;     
    }
}



// this section handles configuration values which can be configured via webpage form in a webbrowser

// 8 configuration values max
String ConfigName[8];     // name of the configuration value
String ConfigValue[8];    // the value itself (String)
int    ConfigStatus[8];   // status of the value    0 = not set    1 = valid   -1 = not valid


// Initalize the values 
void InitializeConfigValues()
{
  for (int count = 0; count < 8; count++)
  {
    ConfigName[count] = "";
    ConfigValue[count] = "";
    ConfigStatus[count] = 0;
  }
}


// Build a HTML page with a form which shows textboxes to enter the values
// returns the HTML code of the page
String EncodeFormHTMLFromConfigValues(String TitleOfForm, int CountOfConfigValues)
{
   // Head of the HTML page
   String HTMLPage = "<!DOCTYPE html><html><body><h2>" + TitleOfForm + "</h2><form><table>";

   // for each configuration value
   for (int c = 0; c < CountOfConfigValues; c++)
   {
    // set background color by the status of the configuration value
    String StyleHTML = "";
    if (ConfigStatus[c] == 0) { StyleHTML = " Style =\"background-color: #FFE4B5;\" " ;};   // yellow
    if (ConfigStatus[c] == 1) { StyleHTML = " Style =\"background-color: #98FB98;\" " ;};   // green
    if (ConfigStatus[c] == -1) { StyleHTML = " Style =\"background-color: #FA8072;\" " ;};  // red

    // build the HTML code for a table row with configuration value name and the value itself inside a textbox   
    String TableRowHTML = "<tr><th>" + ConfigName[c] + "</th><th><input name=\"" + ConfigName[c] + "\" value=\"" + ConfigValue[c] + "\" " + StyleHTML + " /></th></tr>";

    // add the table row HTML code to the page
    HTMLPage += TableRowHTML;
   }

   // add the submit button
   HTMLPage += "</table><br/><input type=\"submit\" value=\"Submit\" />";

   // footer of the webpage
   HTMLPage += "</form></body></html>";
   
   return HTMLPage;
}


// Decodes a GET parameter (expression after ? in URI (URI = expression entered in address field of webbrowser)), like "Country=Germany&City=Aachen"
// and set the ConfigValues
int DecodeGETParameterAndSetConfigValues(String GETParameter)
{
   
   int posFirstCharToSearch = 1;
   int count = 0;
   
   // while a "&" is in the expression, after a start position to search
   while (GETParameter.indexOf('&', posFirstCharToSearch) > -1)
   {
     int posOfSeparatorChar = GETParameter.indexOf('&', posFirstCharToSearch);  // position of & after start position
     int posOfValueChar = GETParameter.indexOf('=', posFirstCharToSearch);      // position of = after start position
  
     ConfigValue[count] = GETParameter.substring(posOfValueChar + 1, posOfSeparatorChar);  // extract everything between = and & and enter it in the ConfigValue
      
     posFirstCharToSearch = posOfSeparatorChar + 1;  // shift the start position to search after the &-char 
     count++;
   }

   // no more & chars found
   
   int posOfValueChar = GETParameter.indexOf('=', posFirstCharToSearch);       // search for =
   
   ConfigValue[count] = GETParameter.substring(posOfValueChar + 1, GETParameter.length());  // extract everything between = and end of string
   count++;

   return count;  // number of values found in GET parameter
}





void setup() 
{
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BOARD, OUTPUT);
  pinMode(PIN_ACTOR, OUTPUT);

  pinMode(0, INPUT);

  

  SwitchRGBLED(LED_YELLOW); 
  SwitchActor(ACTOR_OFF);
  
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Hello ESP32!"); 

  boolean success = true;

 
  // connect to WiFi network
  // see https://www.arduino.cc/en/Reference/WiFiBegin
  
  WiFi.begin(txtSSID, txtPassword);
  
  // we wait until connection is established
  // or 10 seconds are gone
  
  int WiFiConnectTimeOut = 0;
  while ((WiFi.status() != WL_CONNECTED) && (WiFiConnectTimeOut < 10))
  {
    delay(1000);
    WiFiConnectTimeOut++;
  }

  // not connected
  if (WiFi.status() != WL_CONNECTED)
  {
    success = false;
  }

  if (success == true)
  {
    SwitchRGBLED(LED_GREEN); 
  }
  else
  {
    SwitchRGBLED(LED_RED);  
  }   

  // print out local address of ESP32 in LAN
  Serial.println(WiFi.localIP());

  // switch LED off
  SwitchActor(ACTOR_OFF);

  // initialize config values and set first 3 names of values to LED1...LED3
  InitializeConfigValues();
  ConfigName[0] = "LED1";
  ConfigName[1] = "LED2";
  ConfigName[2] = "LED3";

  // start the webserver to listen for request of clients in LAN
  Webserver_Start();

}

// check the ConfigValues and set ConfigStatus
// process the ConfigValues to switch something
void ProcessAndValidateConfigValues(int countValues)
{
  if (countValues > 8) {countValues = 8;};

  // for each ConfigValue
  for (int cn = 0; cn < countValues; cn++) 
  {
    // in our application the values must be "00" or "FF" (as text string)
    if ((ConfigValue[cn].equals("00")) || (ConfigValue[cn].equals("FF")))
    {
      ConfigStatus[cn] = 1;    // Value is valid
    }
    else
    {
      ConfigStatus[cn] = -1;   // Value is not valid
    }
  }

  // first config value is used to switch LED ( = Actor)
  if (ConfigValue[0].equals("00"))   
  {
    SwitchActor(ACTOR_OFF);
  }
  
  if (ConfigValue[0].equals("FF"))
  {
    SwitchActor(ACTOR_ON);
  }
  
}



void loop() {

  String GETParameter = Webserver_GetRequestGETParameter();   // look for client request

  if (GETParameter.length() > 0)        // we got a request, client connection stays open
  {
    if (GETParameter.length() > 1)      // request contains some GET parameter
    {
      int countValues = DecodeGETParameterAndSetConfigValues(GETParameter);     // decode the GET parameter and set ConfigValues

      ProcessAndValidateConfigValues(countValues);                              // check and process ConfigValues
    }

    String HTMLPageWithConfigForm = EncodeFormHTMLFromConfigValues("ESP32 Webserver Demo", 3);   // build a new webpage with form and new ConfigValues entered in textboxes
   
    Webserver_SendHTMLPage(HTMLPageWithConfigForm);    // send out the webpage to client = webbrowser and close client connection
     
  }

  delay(50);
}
