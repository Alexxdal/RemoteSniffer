#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include "Sniffer.h"

#define btn1 T4
#define btn1ts 64
#define btn2 T5
#define btn2ts 70
#define tx 15

#define CS_PIN 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET 0
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Sniffer sniffer = Sniffer();
unsigned int Code[128];
uint8_t npulse;

File myfile;
File root;
File entry;

void setup() {
  pinMode(tx, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
  }
  if (!SD.begin(CS_PIN)) {
    Serial.println(F("SD fail"));
    display.clearDisplay();
    display.setRotation(2);
    display.setTextSize(1);// Normal 1:1 pixel scale
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);//Draw white text
    display.setCursor(0, 10);
    display.println(F(" SD FAIL "));
    display.display();
    while (1);
  }
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(2);// Normal 1:1 pixel scale
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);//Draw white text
  display.setCursor(0, 0);
  display.println(F(" TESTING "));
  display.display();

  root = SD.open("/codes");
  delay(1000);
}

void loop() {
  //Send Code from sd card
  if(touched(btn1, btn1ts) == true)
  { 
    entry =  root.openNextFile();
    if(!entry){ root.close(); root = SD.open("/codes"); entry = root.openNextFile(); }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);// Normal 1:1 pixel scale
    display.setTextColor(BLACK, WHITE);//Draw white text
    display.print(F(" Choose file "));
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print(entry.name());
    display.setTextColor(BLACK, WHITE);
    display.setCursor(0, 24);
    display.print(" NEXT ");
    display.setCursor(92, 24);
    display.print(" SEND ");
    display.display();
    int curr = 0;
    npulse = (int)entry.parseInt();
    while (entry.available()) {
      Code[curr] = (int)entry.parseInt();
      curr++;
    }
    entry.close();
    delay(1000);    
  }
  if(touched(btn2, btn2ts) == true)
  { 
    sniffer.disableReceive();
    delay(100);
    SendCode();
    delay(1000);
    sniffer.continueReceive();
  }
 
  
  // put your main code here, to run repeatedly:
  if(sniffer.available())
  {    
    int startPulse = sniffer.GetStartPulseLenght();
    int minPulse = sniffer.GetMinPulseLenght();
    int maxPulse = sniffer.GetMaxPulseLenght();
    int pauseL = sniffer.GetPauseLenght();
    npulse = sniffer.nPulse();
    for(int i = 0; i < npulse; i++)
    {
      Code[i] = sniffer.GetCode(i);
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);// Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);//Draw white text
    display.print(F("Start Pulse: "));
    display.print(startPulse);
    display.setCursor(0, 10);
    display.print(F("Min:"));
    display.print(minPulse);
    display.print(F(" Max:"));
    display.print(maxPulse);
    display.print(F(" "));
    display.print((npulse - 2)/2);
    display.print(F("Bit"));
    display.setTextColor(BLACK, WHITE);
    display.setCursor(0, 24);
    display.print(" SAVE ");
    display.setCursor(92, 24);
    display.print(" EXIT ");
    display.display();

    while(touched(btn1, btn1ts) == false  && touched(btn2, btn2ts) == false){
    }
    //EXIT
    if(touched(btn2, btn2ts) == true)
    {
      display.clearDisplay();
      display.display();
      sniffer.continueReceive();
      delay(1000);
    }
    //SAVE CODE
    if(touched(btn1, btn1ts) == true)
    {
      String filename;
      randomSeed(millis());
      int rand1 = random(99);
      filename += "/codes/" + String(startPulse) + "-" + String(rand1) + ".txt";
      myfile = SD.open(filename, FILE_WRITE);
      if(myfile) 
      {
        myfile.println(npulse);
        for(int i = 0; i < npulse; i++)
        {
          myfile.println(Code[i]);
        }
        myfile.close();
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);// Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);//Draw white text
      display.println(F("File Saved"));
      display.println(filename);
      display.display();
      delay(1500);
      display.clearDisplay();
      display.display();
      sniffer.continueReceive();
      delay(500);
    }
  }
}


void SendCode()
{
  //Quick resend
    for(int a = 0; a < 4; a++)
    {
      for(int i = 0; i < npulse; i = i + 2)
      {
        digitalWrite(tx, LOW);
        delayMicroseconds(Code[i]);
        digitalWrite(tx, HIGH);
        delayMicroseconds(Code[i+1]);
        digitalWrite(tx, LOW);
      }
    }
    Serial.println(F("Code sent"));
    delay(50);
}

bool touched(int pin, int threshold)
{
  int readed = 0;
  for(int i=0; i< 50; i++)
  { 
    readed += touchRead(pin); // adding the hundred readings within the for loop
    delay(2);
  }
  readed = readed / 50;
  if(readed <= threshold)
  {
    return true;
  }
  else
  {
    return false;
  }
}
