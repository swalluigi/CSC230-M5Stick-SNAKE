/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)

  edited by Ian Sbar
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <M5StickCPlus.h>








RTC_TimeTypeDef RTC_TimeStruct;

class Square{//class for the square the player controls
public:

//properties of the square, such as coordinates and and size
  int size=6;
  int Xcord=0;
  int Ycord=0;


  int set_Xcord(int x){ // a function that adds a number to the current x coordinate
    Xcord=Xcord+x;
    return Xcord;
  }

  int set_Ycord(int y){// a function that adds a number to the current Y coordinate
    Ycord=Ycord+y;
    return Ycord;
  }

    
  void DrawSquare(){ //function that draws the square on the screen
    M5.Lcd.drawRect(Xcord, Ycord, size, size, BLUE);
    M5.Lcd.fillRect(Xcord, Ycord, size, size, BLUE);
  
  }

  void DrawFace(){//function that draws a smaller purple square on top of the blue square. acts as the "face"
    M5.Lcd.drawRect(Xcord+1, Ycord+1, size-2, size-2, PURPLE);
    M5.Lcd.fillRect(Xcord+1, Ycord+1, size-2, size-2, PURPLE);
  
  }

  void origin(){
    M5.Lcd.drawRect(Xcord, Ycord, 1, 1, GREEN);
    M5.Lcd.fillRect(Xcord, Ycord, 1, 1, GREEN);
  }

  
};

class Power{ //class for the power up object.
  public://object properties
  int Xcord=random(128);
  int Ycord=random(234);
  int size = 6;


  void DrawPower(){//function that plants the powerup on screen at a random location
    Xcord=random(128);//random x cord
    Ycord=random(234);//random Y cord
    M5.Lcd.drawRect(Xcord, Ycord, size, size, RED);
    M5.Lcd.fillRect(Xcord, Ycord, size, size, RED);
  
  }

  void remove(){//function that removes the powerup.
    M5.Lcd.drawRect(Xcord, Ycord, size, size, BLUE);
    M5.Lcd.fillRect(Xcord, Ycord, size, size, BLUE);
  }
};
// Set these to your desired credentials for the access point
const char *ssid = "CCASTER";
const char *password = "Rollback";

WiFiServer server(80);

//creates a pixel object of class square
Square *pixel = new Square; 

//creates two PowerUp objects of class power
Power *PowerUp = new Power;
Power *PowerUp2 = new Power;


  //numerical representation of LCD screen
  //All of these lines before the settup function were meant to contribute to making a numberical version of the pixel grid on the LCD screen.
  //The Idea was to represent all the pixels that the blue square has touched with the number "1" and everything else as "0"
  //the program would then keep track of all the "1s" in the grid, stopping the square's movement and saving the time on the Real time clock, which would be uploaded to a firebase database.
  //Unfortunatelly, mainly due to me procrastinating, I could not get this part of the code to function in time, and thus could not 
 int LCDarray[135][240];//initailizes the Array
 int blueCount;//integer variable for the number of blue pixels on the LCD (or 1s in the array)

 //this function uses "for" loops to fill the array with zeros
 void MakeLCDArray(){
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 135; x++) {
            LCDarray[x][y] = 0;

        }

    }
    
  }

  void LCDarrayFill() {//This function does not work properly for some reason. It either prevents the program from openeing, caused what I believe is some sort of infinite loop, or doesn't run at all.
   //Unfortunately this function seemed to be the only way I could count the amound of blue pixels on the LDC screen.
   
   //it was meant to fill the arrays with 1s in the same places the blue square on the LCD as passed
    for (int y = pixel->Ycord; y < pixel->size+ pixel->Ycord; y++) {
        for (int x = pixel->Xcord; x < pixel->size + pixel->Xcord; x++) {
            
            LCDarray[x][y] = 1;
            blueCount++;
          
        } 
    }

   
  }

  

void setup() {
  

  M5.begin();

     //added from rtc clock. 
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = 0;  // Set the time.  设置时间
    TimeStruct.Minutes = 0;
    TimeStruct.Seconds = 0;
    M5.Rtc.SetTime(&TimeStruct);//sets the time of the RTC to the three variables above

    // Lcd display
    

    pixel->DrawSquare();//draws the pixel object
    pixel->DrawFace();//draws the face of hte pixel object
    pixel->origin();//draws the origin point of the pixel object
    PowerUp -> DrawPower();//draws the powerup object
    PowerUp2 -> DrawPower();//draws the second powerup object

    // draw graphic.  绘图🌹
   
    
    delay(1000);
 
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  
  Serial.println("Server started");
  
}


void loop() {
  
   M5.Rtc.GetTime(&RTC_TimeStruct);//Gets the data from the RTC to start keeping track of time.
   
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";  
                 // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header. Will be used to move the pixel on the LCD using a web page
            client.print("<button><a href=\"/L\">Left</a> <button><br>");
            client.print("<button><a href=\"/D\">Down</a> <button><br>");
            client.print("<button><a href=\"/R\">Right</a> <button><br>");
            client.print("<button><a href=\"/U\">Up</a> <button><br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        //code for moving pixel and interactions.
        pixel->DrawSquare();//draws a blue line to cover the previous purple face pixel
        pixel->origin();

        if (currentLine.endsWith("GET /R")) {//moves the pixel Right
         pixel->set_Xcord(pixel->size);

          //makes sure the pixel does not go off screen
         if(pixel->Xcord >= 135-pixel->size ){
           pixel->Xcord = 135-pixel->size;
         }
         
         
        }
        if (currentLine.endsWith("GET /D")) {//moves the Pixel down. distance moved is affected by the pixel's size
          
         pixel->set_Ycord(pixel->size);

         //makes sure the pixel does not go off screen
         if(pixel->Ycord >= 240- pixel->size ){
           pixel->Ycord = 240- pixel->size;
         }
         
        }
        if (currentLine.endsWith("GET /L")) { //moves the pixel left
         pixel->set_Xcord(-pixel->size);
         //makes sure the pixel does not go off screen
         if(pixel->Xcord < 0 ){
          pixel->Xcord = 0;
         }
         
        }
        if (currentLine.endsWith("GET /U")) {//moves the pixel up
          
         pixel->set_Ycord(-pixel->size);
         
         //makes sure the pixel does not go off screen
         if(pixel->Ycord < 0 ){
           
             pixel->Ycord =0;
           
         
        }
        }

        
        //If the pixel touches PowerUp, it will grow larger and PowerUp will disappear and reappear elsewhere
        if(pixel->Xcord >= PowerUp->Xcord-(pixel->size/2) && pixel->Xcord <= PowerUp->Xcord+(pixel->size/2) ){
          if(pixel->Ycord >= PowerUp->Ycord-(pixel->size/2) && pixel->Ycord <= PowerUp->Ycord+(pixel->size/2)){
          PowerUp-> remove();
          pixel->size = pixel->size+4;
          PowerUp -> DrawPower();
          }
          
        }
        //If the pixel touches PowerUp2, it will grow larger and PowerUp2 will disappear and reappear elsewhere
        if(pixel->Xcord >= PowerUp2->Xcord-(pixel->size/2) && pixel->Xcord <= PowerUp2->Xcord+(pixel->size/2) ){
          if(pixel->Ycord >= PowerUp2->Ycord-(pixel->size/2) && pixel->Ycord <= PowerUp2->Ycord+(pixel->size/2)){
          PowerUp2-> remove();
          pixel->size = pixel->size+4;
          PowerUp2 -> DrawPower();
          }
          
        }
        pixel->DrawFace();
        pixel->origin();
        
      }
    }
    
   
    
    
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
    
  }
}
