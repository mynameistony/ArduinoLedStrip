/*TODOs:

-Add setStrip(String newData)
-  @newData = string of chars representing colors
-             eg "rroorryybb"

          
            
Make Library?


*/
#include <avr/pgmspace.h>
#include <IRremote.h>
#include <SoftwareSerial.h>

// ******** DEBUG ==== should auto config to adapt different mother board *********
//#define DATA_1 (PORTF |=  0X01)    // DATA 1    // for ATMEGA
//#define DATA_0 (PORTF &=  0XFE)    // DATA 0    // for ATMEGA
//#define STRIP_PINOUT DDRF=0xFF  // for ATMEGA
#define DATA_1 (PORTC |=  0X01)    // DATA 1    // for UNO
#define DATA_0 (PORTC &=  0XFE)    // DATA 0    // for UNO
#define STRIP_PINOUT (DDRC=0xFF)    // for UNO

#define rr  0xff0000
#define ro  0x6f002f 
#define ry  0x8f008f
#define rg  0x0000ff
#define rb  0x00ff00
#define rp  0x8f8f00
#define oo  0x000000

#define lightblue  0x008f8f

#define A  220
#define B  222
#define C  223
#define D  225
#define E  227
#define F  228
#define G  229

#define whole  214
#define half  213
#define quarter  212
#define eighth  211

#define scale3  215
#define scale4  216
#define scale5  217


unsigned long colors[10] = {
  rr,ro,ry,ry,rg,rg,rb,rb,rp,rp
};

unsigned long strip1[10]={
  rr,rr,rr,rr,rr,rr,rr,rr,rr,rr
};

unsigned long colorFadeStrip[10]={
  rr,rr,rr,rr,rr,rr,rr,rr,rr,rr  
};

SoftwareSerial lcd(8,9);
// IR Receiver Stuff
int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;
// IR Receiver Stuff

int currLength = 0;

int currRate = 100;

int currMode = 10;

int currBrightness = 125;

boolean isOff = false;

int button1;
int switch1;
int pot1;

int colorFadeMode = 0;
long thisDistance;

unsigned long lastMillis = 0;
unsigned long lastPrint = 0;

void setup(){
  pinMode(7,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);

  lcd.begin(9600);
  Serial.begin(9600);
  irrecv.enableIRIn();

  STRIP_PINOUT;

  reset_strip();
  //noInterrupts();
  mySend(colors);
  lcd.write(12);
  lcd.write(17);
 
}

void loop(){
      
    switch1 = digitalRead(7);
    
//      if(!switch1)
//        for(int i = 0; i < 10; i++)
//          strip1[i] = 0x000000;
//        while(!switch1)        
//          switch1 = digitalRead(6);
        
          
    pot1 = analogRead(A1);
    button1 = digitalRead(6);  

    if(!digitalRead(6))
      happyBirthday();

    if (irrecv.decode(&results)){
      setMode(results.value);
//      Serial.println(results.value, HEX);
      irrecv.resume();
      
      printInfo();
    }
    
    if(millis() - lastPrint > 2000)
      lcd.write(18);   
          

    switch (currMode){  
      case 0:
      break;
      case 1:
        
        if(millis() - lastMillis > currRate){        
          lastMillis = millis();
          for(int i = 0; i < 10; i++)
            colorFadeStrip[i] = newColorFade(colorFadeStrip[i]);
          
          mySend(colorFadeStrip);      
        }

      break;
      
      case 9:
        happyBirthday();         
      break;

      case 10:
        thisDistance = map(getDistance(100),0,100,0,10);
      
        if(thisDistance < 10)
          for(int i = 0; i < 10; i++)
            if(i <= thisDistance)
              strip1[i] = 0x888888;
            else
              strip1[i] = 0x000000;   
          
      
      mySend(strip1);    
      break;  
      
      case 11:
      break;
    }
}

void send_strip(uint32_t data)
{
  int i;
  unsigned long j=0x800000;
  
 
  for (i=0;i<24;i++)
  {
    if (data & j)
    {
      DATA_1;
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");    
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      
/*----------------------------*/
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");  
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");  
      __asm__("nop\n\t");  
      __asm__("nop\n\t");        
/*----------------------------*/      
      DATA_0;
    }
    else
    {
      DATA_1;
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");    
      DATA_0;
/*----------------------------*/      
       __asm__("nop\n\t");
      __asm__("nop\n\t");
      __asm__("nop\n\t");      
/*----------------------------*/         
    }

    j>>=1;
  }  
}

void reset_strip()
{
  DATA_0;
  delayMicroseconds(20);
}

void mySend(unsigned long data[10]){
  int j=0;
  uint32_t temp_data;
    noInterrupts();
    for (j=0;j<10;j++)
    {
      temp_data=data[j];
      send_strip(temp_data);
    }
    interrupts();

}
void setMode(unsigned long value){
  
    Serial.println(value, HEX);
    
    switch(value){ 
     // #1
     case 0xE0E020DF:
       currMode = 1;
     break;
     // #2
     case 0xE0E0A05F:
       currMode = 2;
     break;
     // #3
     case 0xE0E0609F:
       currMode = 3;
     break;
     // #4
     case 0xE0E010EF:
       currMode = 4;
     break;
     // #5
     case 0xE0E0906F:
       currMode = 5;
     break;
     // #6
     case 0xE0E050AF:
       currMode = 6;
     break;
     // #7
     case 0xE0E030CF:
       currMode = 7;
     break;
     // #8
     case 0xE0E0B04F:
       currMode = 8;
     break;
     // #9
     case 0xE0E0708F:
       currMode = 9;
     break;
     // #0
     case 0xE0E040BF:
       currMode = 0;
       for(int i = 0; i < 10; i++)
         strip1[i] = 0x000000;
       mySend(strip1);
     break;  
     
     // '-'
     case 0xE0E0C43B:      
        currMode = 10;          
     break;
     
     // Play/Pause
     case 0xE0E052AD:
        currMode = 10;
     break;
     //CH UP     
     case 0xE0E048B7:
       currBrightness += 10;     
       if(currBrightness > 255)
         currBrightness = 255;
         
        for(int i = 0; i < 10; i++)
          if(i <= currLength)
            strip1[i] = 0x010101 * currBrightness;
          else
            strip1[i] = 0x000000;
        mySend(strip1);
     break;
     //CH DOWN
     case 0xE0E008F7:
       currBrightness -= 10;

       if(currBrightness < 0)
         currBrightness = 0;

       for(int i = 0; i < 10; i++)
          if(i <= currLength)
            strip1[i] = 0x010101 * currBrightness;
          else
            strip1[i] = 0x000000;
        mySend(strip1);         
     break;
     
     // VUP
     case 0xE0E0E01F:
       currMode = 11;
       currLength++;
       if(currLength > 10)
         currLength = 10;
         
       for(int i = 0; i < 10; i++)
         if(i <= currLength)
           strip1[i] = 0x010101 * currBrightness;
         else
           strip1[i] = 0x000000;
        mySend(strip1);        
       
     break;
     
     //VDOWN
     case 0xE0E0D02F:
       currMode = 11;
       currLength--;
       if(currLength < 0)
         currLength = 0;
         
       for(int i = 0; i < 10; i++)
         if(i < currLength)
           strip1[i] = 0x888888;
         else
           strip1[i] = 0x000000;
        mySend(strip1);        

     break;
     
     
       
    }    

  
  
}


void printInfo(){
  lcd.write(17);
  lcd.write(12);
  lcd.print("Rate: ");
  lcd.print(currRate);
  lcd.write(13);
  lcd.write("Mode: ");
  lcd.print(currMode);
  
  lastPrint = millis();  
}

long getDistance(int limit){
  long duration, distance;
  
  digitalWrite(13, LOW);
  delayMicroseconds(2);
  
  digitalWrite(13, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(13, LOW);
  
  duration = pulseIn(12, HIGH);
  distance = (duration/2) / 29.1;
  
//  Serial.print("Distance ");
//  Serial.println(distance);
  
  if(distance > limit)
    return limit;
  else
    return distance;
}

unsigned long newColorFade(unsigned long data){

    
  int r =  data / 0x010000;
  int b = (data / 0x000100) % 0x000100 ;
  int g = data % 0x000100;  
  
//  Serial.print("Data: ");
//  Serial.print(data, HEX);
//  Serial.print("\nR: ");
//  Serial.print(r, HEX);
//  Serial.print("\tB: ");
//  Serial.print(b, HEX);
//  Serial.print("\tG: ");
//  Serial.print(g, HEX);
//  Serial.print("\tMode: ");
//  Serial.println(colorFadeMode);


  
  switch(colorFadeMode){
  
      case 0:
        if(r > 0){
          data -= 0x010000;
          data += 0x000100;
        }
        else
          colorFadeMode = 1;
          
      break;
      
      case 1:
        if(b > 0){
          data -= 0x000100;
          data += 0x000001; 
        }
        else
          colorFadeMode = 2;
      break;
      
      case 2:
        if(g > 0){
          data += 0x010000;
          data -= 0x000001; 
        }
        else
          colorFadeMode = 0;        
      break;
      
      default:
        Serial.print("\nShits fucked\n");     
  }    

  return data;
}


void happyBirthday(){
  playNote(D,eighth,scale3);
  playNote(D,eighth,scale3);
  playNote(E,quarter,scale3);
  playNote(D,quarter,scale3);
  playNote(G,quarter,scale3);
  playNote(F,half,scale3);
  
  playNote(D,eighth,scale3);
  playNote(D,eighth,scale3);
  playNote(E,quarter,scale3);
  playNote(D,quarter,scale3);
  playNote(A,quarter,scale4);
  playNote(G,half,scale3);
  
  playNote(D,eighth,scale3);
  playNote(D,eighth,scale3);
  playNote(D,quarter,scale4);
  playNote(B,quarter,scale4);
  playNote(G,quarter,scale3);
  playNote(F,quarter,scale3);
  playNote(E,half,scale3);
  
  playNote(C,eighth,scale4);
  playNote(C,eighth,scale4);
  playNote(B,quarter,scale4);
  playNote(G,quarter,scale3);
  playNote(A,quarter,scale4);
  playNote(G,half,scale3);   
}

void playNote(int note, int duration, int scale){
  lcd.write(scale);
  lcd.write(duration);
  lcd.write(note);

}

