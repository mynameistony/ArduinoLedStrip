/**********Tony's IR LED Strip**********

Parts:

-Arduino UNO (or any) w/12 Volt power supply (for the LED strip)

-TM1801 chip based LED strip 

	-I have this one:
		http://blog.radioshack.com/2013/06/tricolor-led-strip/

	-PWR -> VIN
	-DATA -> A0
	-GND -> GND
	
-IR Receiver Module (most will work)

- 4 pin Ultrasonic Rangefinder (Optional)
	-I have an HC-SR04
	
*/
/*TODOs:
-Make Library?

-Document better...


*/

#include <avr/pgmspace.h>
#include <SoftwareSerial.h>

#include <IRremote.h> // Credits to Ken Shiriff
/*
	Ken's Blog:
	Ken's Github:
*/

/*Configurations*/
#define buttonPin 6
#define switchPin 7
#define LCDRxPin 8
#define LCDTxPin 9
#define IRpin 11
#define backlightTimer 2000 
#define echoPin 12
#define trigPin 13

/*LED Strip*/

//	LEARN THIS SHIT!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ******** DEBUG ==== should auto config to adapt different mother board *********
//#define DATA_1 (PORTF |=  0X01)    // DATA 1    // for ATMEGA
//#define DATA_0 (PORTF &=  0XFE)    // DATA 0    // for ATMEGA
//#define STRIP_PINOUT DDRF=0xFF  // for ATMEGA
#define DATA_1 (PORTC |=  0X01)    // DATA 1    // for UNO
#define DATA_0 (PORTC &=  0XFE)    // DATA 0    // for UNO
#define STRIP_PINOUT (DDRC=0xFF)    // for UNO
/***********/

/*Define some colors*/
#define rr  0xff0000
#define ro  0x6f002f 
#define ry  0x8f008f
#define rg  0x0000ff
#define rb  0x00ff00
#define rp  0x8f8f00
#define oo  0x000000
#define lightblue  0x008f8f
/********************/


/*Define stuff for LCD screen speaker*/
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
/*************************************/



/*A Few Strips To Use*/
unsigned long colors[10] = {
  rr,ro,ry,ry,rg,rg,rb,rb,rp,rp
};

unsigned long strip1[10]={
  rr,rr,rr,rr,rr,rr,rr,rr,rr,rr
};

unsigned long colorFadeStrip[10]={
  rr,rr,rr,rr,rr,rr,rr,rr,rr,rr  
};
/**********************/


// LCD screen
SoftwareSerial lcd(LCDRxPin,LCDTxPin);

// IR Receiver Stuff
int RECV_PIN = IRpin;

IRrecv irrecv(RECV_PIN);

decode_results results;


// Some useful variables
int currLength = 10;

int currRate = 100;

int currMode = 11;

int currBrightness = 125;

int colorFadeMode = 0;

unsigned long currColor = 0x010101;

long thisDistance;

boolean isOff = false;

// Physical Input
int button1, switch1, pot1;

// Timing stuff
unsigned long lastMillis = 0;
unsigned long lastPrint = 0;

void setup(){
	
	//Initialize Buttons
	pinMode(switchPin,INPUT_PULLUP);
	pinMode(buttonPin,INPUT_PULLUP);
	
	//Initialize Rangefinder
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	
	
	//Start Communication W/LCD & Serial
	lcd.begin(9600);
	Serial.begin(9600);
	
	//Initalize IR Receiver;
	irrecv.enableIRIn();
	
	//Initialize LED strip
	//Not exactly sure how to configure it to use a different pin...
	STRIP_PINOUT;
	reset_strip();
	
	//Display a rainbow on startup
	mySend(colors);
	
	
	clearLCD();
	backlightOn();
 
}

void loop(){
      
	pot1 = analogRead(A1);
	button1 = digitalRead(buttonPin);  
	switch1 = digitalRead(switchPin);

	// Quick physical off switch    
	// Its input pullup so its backwards

	//	if(!switch1){
	//		mode = 11;
	//		currBrightness = 0;
	//	}

	// Play Happy Birthday :P
	// if(!button1)
	//	happyBirthday();


	// Auto Backlight off
	if(millis() - lastPrint > backlightTimer)
		backlightOff();   

	// Recieve IR Code
	if (irrecv.decode(&results)){
		//Serial.println(results.value, HEX);
		setMode(results.value);

		irrecv.resume();
	      
		printInfo();
	}

	// Define more modes in setMode()
	switch (currMode){  
		case 0:
		break;

		// Color Fade mode 
		case 1:
			// If it been longer than the desired interval ...
			if(millis() - lastMillis > currRate){      
				lastMillis = millis();
		
				// Increment all the segments current color
				for(int i = 0; i < 10; i++)
					colorFadeStrip[i] = newColorFade(colorFadeStrip[i]);
			  	
				// Update the strip
				mySend(colorFadeStrip);      
			}
		break;
                          
                case 2:
			if(millis() - lastMillis > currRate){      
				lastMillis = millis();

				// Increment all the segments current color
       				for(int i = 0; i < 10; i++)
					strip1[i] = fadeToColor(colorFadeStrip[i],colors[i]);

				// Update the strip
				mySend(strip1);      
			}                  
                
                break;

		// Play Happy Birthday	      
		case 9:
			happyBirthday();         
		break;
	
		// Rangefinder mode
		case 10:
			// Get new distance up to 100cm and map it to 10 units (1 for each segment)
			thisDistance = map(getDistance(100),0,100,0,10);
		      	
			// Make sure...
			if(thisDistance < 10)

				// Set the strip accordingly
				for(int i = 0; i < 10; i++)
					if(i <= thisDistance)
						strip1[i] = currColor * currBrightness;
					else
						strip1[i] = 0x000000;   
			mySend(strip1);    
		break;  

		// This is actually the general mode
		// All the magic happens elsewhere but this needs to be here
		case 11:
		break;
	}
}

// Not my function, this came from the radioshack support files here:
// http://blog.radioshack.com/uploads/2013/06/RadioShack-Tricolor-LED-Strip-Support-Files1.zip
void send_strip(uint32_t data){
	int i;
	unsigned long j=0x800000;
 
	for (i=0;i<24;i++){
		if (data & j){
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
			/*---------------------------*/      
			DATA_0;
		}
		else{
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

//Neither is this
void reset_strip(){
	DATA_0;
	delayMicroseconds(20);
}

// I modified the original send_strip function from the support files
// so that patterns are dynamic and not written and read from EEPROM.
/*
	@param data: An array containing HEX values representing each segments color
*/
void mySend(unsigned long data[10]){
	
	// 32 bit temporary variable
	uint32_t temp_data;

	// Disable Interupts for time-critical operation
	noInterrupts();
	
	// Loop thru input array
	// Sending each element to the LED strip
	for(int i = 0; i < 10; i++){
		temp_data=data[i];
		send_strip(temp_data);
	}
	
	// Re-enable interrupts
	interrupts();

}

// Function to switch modes
/*
	@param value: The received value from the IR receiever 
*/
void setMode(unsigned long value){
	
	// Serial.println(value, HEX);

	switch(value){ 
		
		// Set to Red
		// #1
		case 0xE0E020DF:
			currMode = 11;
			currColor = 0x010000;       
			updateStrip();
		break;
		
		// Set to Blue
		// #2
		case 0xE0E0A05F:
			currMode = 11;
			currColor = 0x000100;      
			updateStrip();
		break;

		// Set to Green
		// #3
		case 0xE0E0609F:
			currMode = 11;
			currColor = 0x000001;
			updateStrip();
		break;
		
		// Set to Cyan
		// #4
		case 0xE0E010EF:    
			currMode = 11;
			currColor = 0x010100;
			updateStrip();
		break;

		// Set to Yellow
		// #5
		case 0xE0E0906F:
			currMode = 11;
			currColor = 0x010001;
			updateStrip();
		break;

		// Set to Magenta
		// #6
		case 0xE0E050AF:
			currMode = 11;
			currColor = 0x000101;
			updateStrip();
		break;

		
		// #7
		case 0xE0E030CF:
			currMode = 7;
		break;

		// Set to Color Fade Mode
		// #8
		case 0xE0E0B04F:
			currMode = 1;
		break;

		// Set to Random Mode
		// #9
		case 0xE0E0708F:
			currMode = 2;
		break;

		// Set to White
                // #0
		case 0xE0E08877:
			currMode = 11;
			currColor = 0x010101;
			updateStrip();
		break;

		// Turn Off (Just sets length to 0)	
		// Power
		case 0xE0E040BF:
			currMode = 11;
			currLength = 0;
			updateStrip();
		break;       
		
		// Set to ColorFade Mode		     
		// '-'
		case 0xE0E0C43B:      
			currMode = 1;          
		break;
		
		// Set to Random Mode
		// 'Mute'
		case 0xE0E0F00F:
			currMode = 2;
		break;

		// Set to Color Fade Mode		     
		// 'Play/Pause'
		case 0xE0E052AD:
			currMode = 1;
		break;

		// If in ColorFade, increase rate
		// Otherwise increase brightness
		//ARROW UP     
		case 0xE0E006F9:
			if(currMode == 1)
				currRate += 10;
			else{
				if(currMode == 2){}
				else{
					currMode = 11;
					currBrightness += 10;     

				        if(currBrightness > 255)
				        currBrightness = 255;
				 
          				updateStrip();     
                                }    
			}
		break;

		// If in Color Fade decrease rate
		// Otherwise decrease brightness
		//ARROW DOWN  
		case 0xE0E08679:
			if(currMode == 1)
				currRate -= 10;
			else{
                                if(currMode == 2){}
                                else{
					currMode = 11;
					currBrightness -= 10;

          				if(currBrightness < 0)
					currBrightness = 0;
		                
	        			updateStrip();
                                }
			}
		break;

		// Increase segments		     
		// ARROW Right
          	case 0xE0E046B9:
			// Set to general mode
			currMode = 11;
			
			// Add a segment
			currLength++;
			
			// Limit to 10
			if(currLength > 10)
				currLength = 10;
			
			updateStrip();       
		break;
		
		// Decrease segments     
		// ARROW LEFT
		case 0xE0E0A659:
			currMode = 11;
			currLength--;

			if(currLength < 0)
				currLength = 0;
				 
			updateStrip();
		break;
		     
		case 0xE0E0807F:
			currMode = 10;
		break;       
	}      
}

// Print some info to the LCD
void printInfo(){

	backlightOn();
	clearLCD();

	// Print rate
	lcd.print("Rate: ");
	lcd.print(currRate);

	// New Line
	lcd.write(13);

	// Print Mode
	lcd.write("Mode: ");
	lcd.print(currMode);
	
	// Update last print
	lastPrint = millis();  
}

// Receive a distance from Rangefinder
/*
	@param limit: Upper limit of distance
	@return: the distance received
*/
long getDistance(int limit){
	// Some variables
	long duration, distance;
	
	// Send a pulse...
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);
	
	//...
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	
	//...
	digitalWrite(trigPin, LOW);
	
	// Receive the pulse
	duration = pulseIn(echoPin, HIGH);
	
	// Calculate distance
	distance = (duration/2) / 29.1;
	  
	//  Serial.print("Distance ");
	//  Serial.println(distance);
	
	// Limit distance
	if(distance > limit)
		return limit;
	else
		return distance;
}

// "Asynchronus" color fader
// Does not loop, allowing for other stuff to happen
/*
	@param data: value representing 1 segments current color
	@return: values next iteration in color fade
*/
unsigned long newColorFade(unsigned long data){

	// Break down value into Red, Green and Blue values
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
	
	// Red -> Blue  
	case 0:
		if(r > 0){
			data -= 0x010000;
			data += 0x000100;
		}
		else
			colorFadeMode = 1;
	break;

	// Blue -> Green
	case 1:
		if(b > 0){
			data -= 0x000100;
			data += 0x000001; 
		}
		else
			colorFadeMode = 2;
	break;

	// Green -> Red
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
	break;
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
	// Set scale command
	lcd.write(scale);
	
	// Set duration command
	lcd.write(duration);

	// Set note command
	lcd.write(note);
}

void clearLCD(){
	// Send clear screen command
	lcd.write(12);
}

void backlightOn(){
	// Send backlight on command
	lcd.write(17);	
}

void backlightOff(){
	// Send backlight off command
	lcd.write(18);	
}

// Update strip with current data
void updateStrip(){
	for(int i = 0; i < 10; i++)
		if(i < currLength)
			strip1[i] = currColor * currBrightness;
		else
			strip1[i] = 0x000000;
	mySend(strip1);
}

unsigned long fadeToColor(unsigned long startColor, unsigned long endColor){
	int r1 =  startColor / 0x010000;
	int b1 = (startColor / 0x000100) % 0x000100 ;
	int g1 = startColor % 0x000100;  

	int r2 =  endColor / 0x010000;
	int b2 = (endColor / 0x000100) % 0x000100 ;
	int g2 = endColor % 0x000100;  
  		
        if(r1 > r2)
          startColor -= 0x010000;
        else 
        if(r1 < r2)
          startColor += 0x010000;

        if(b1 > b2)
          startColor -= 0x000100;
        else 
        if(b1 < b2)
          startColor += 0x000100;
          
        if(g1 > g2)
          startColor -= 0x000001;
        else 
        if(g1 < g2)
          startColor += 0x000001;          
        
  
         return startColor;
}
