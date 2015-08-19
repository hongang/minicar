#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//right a1,a2: control bit for L298
int moto_in_a1 = 2;
int moto_in_a2 = 4;
//a3: pwm signal for L298
int moto_in_a3 = 5;

//right
int moto_in_b1 = 7;
int moto_in_b2 = 8;
int moto_in_b3 = 6;

//remote control , consider 3 in 1
int rc_rate = 10;
int rc_direct = 11;
int rc_switch = 12;
#define RC_HIGH  1975
#define RC_LOW   1000
#define RC_TIMEOUT 30000

#define LEVEL_UP      1000
#define LEVEL_MID     1500
#define LEVEL_DOWN    1950

//IR detect 
int ir_pin = 13;


//ultra sound detect 
int us_echo_pin = 9;
int us_trig_pin = 3;

unsigned long Time_Echo_us = 0;
unsigned long Len_mm  = 0;

int beep_pin = 14; //A0 

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display


void init_us()
{
	pinMode(us_echo_pin, INPUT);                    //Set EchoPin as input, to receive measure result from US-100
    pinMode(us_trig_pin, OUTPUT);                   //Set TrigPin as output, used to send high pusle to trig measurement (>10us)	
}

int init_beep()
{
	pinMode(beep_pin,OUTPUT);
}

void init_left()
{
	pinMode(moto_in_a1,OUTPUT);
    pinMode(moto_in_a2,OUTPUT);
    pinMode(moto_in_a3,OUTPUT);
}
void init_right()
{
	  pinMode(moto_in_b1,OUTPUT);
    pinMode(moto_in_b2,OUTPUT);
    pinMode(moto_in_b3,OUTPUT);
}

void init_rc()
{
   	pinMode(rc_rate,INPUT);
    pinMode(rc_direct,INPUT);
    pinMode(rc_switch,INPUT);
}

void init_lcd()
{
  lcd.init();                      // initialize the lcd  
  lcd.backlight();
  lcd.print("StartUP, Welcome");
}
void moto_forward()
{
	digitalWrite(moto_in_a1,HIGH);
	digitalWrite(moto_in_a2,LOW);
	digitalWrite(moto_in_b1,HIGH);
	digitalWrite(moto_in_b2,LOW);
}

void moto_back()
{
	digitalWrite(moto_in_a1,LOW);
	digitalWrite(moto_in_a2,HIGH);
	digitalWrite(moto_in_b1,LOW);
	digitalWrite(moto_in_b2,HIGH);
}


void moto_left_rate( int value)
{
	analogWrite(moto_in_a3,value);
}

void moto_right_rate( int value)
{
	analogWrite(moto_in_b3,value);
}

void moto_stop()
{
	digitalWrite(moto_in_a1,HIGH);
	digitalWrite(moto_in_a2,HIGH);
	digitalWrite(moto_in_b1,HIGH);
	digitalWrite(moto_in_b2,HIGH);
}

int back_len_detect()
{
    digitalWrite(us_trig_pin, HIGH);              //begin to send a high pulse, then US-100 begin to measure the distance
    delayMicroseconds(50);                       //set this high pulse width as 50us (>10us)
    digitalWrite(us_trig_pin, LOW);               //end this high pulse
    
    Time_Echo_us = pulseIn(us_echo_pin, HIGH);               //calculate the pulse width at EchoPin, 
    if((Time_Echo_us < 60000) && (Time_Echo_us > 1))     //a valid pulse width should be between (1, 60000).
    {
      Len_mm = (Time_Echo_us*34/100)/2;      //calculate the distance by pulse width, Len_mm = (Time_Echo_us * 0.34mm/us) / 2 (mm)
    }
	lcd.setCursor(0,1);
	lcd.print("Distance:");
	lcd.print(Len_mm);
	lcd.print(" mm .");
	return Len_mm;
}

int front_len_detect()
{
	int alarm = 0;
	if(digitalRead(ir_pin) == LOW)
	{
	   alarm = 1;
	}
	return alarm;
}

void beep()
{
	for(int j= 0; j<10; j++)
	{
			for(int i =200; i <= 800; i++)
			{	
				tone(beep_pin,i,3);
			}
      delay(10);
	}
}

void setup()
{
	init_left();
	init_right();
	init_rc();
	init_us();
	init_lcd();
	init_beep();
}


void loop()
{
	int duration,drive; 
	duration = pulseIn(rc_switch, HIGH,RC_TIMEOUT);
   //stop when up, move when down
	if(duration > LEVEL_UP)
	{
		//moving
		duration = pulseIn(rc_direct, HIGH,RC_TIMEOUT);
       //forward by default, back when down
		if(duration < LEVEL_UP)
		{
			moto_forward();
			lcd.setCursor(0,0);
			lcd.print("Forward");
		}
		else
		{
			moto_back();
			lcd.setCursor(0,0);
			lcd.print("Back");
		}
		duration = pulseIn(rc_rate, HIGH,RC_TIMEOUT);
		drive = duration * 255/(RC_HIGH - RC_HIGH); 
		moto_left_rate(drive);
		moto_right_rate(drive);	
	}
	else
	{
		//stop
		moto_stop();
	}
	if( (back_len_detect() < 100) || ( front_len_detect() == 1))
	{
		moto_stop();
		beep();
	}
	delay(500);
}

