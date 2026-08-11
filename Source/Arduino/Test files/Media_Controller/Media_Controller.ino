#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif



// Includes enum definition of GES_* return values from readGesture()
#include "RevEng_PAJ7620.h"

// Create gesture sensor driver object
RevEng_PAJ7620 sensor = RevEng_PAJ7620();

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Gesture last;
int reps = 1;

// *********************************************************************
void setup()
{
  Serial.begin(115200);
  u8g2.begin();
  Serial.println("PAJ7620 sensor demo: Recognizing all 9 gestures.");

  if( !sensor.begin() )        // return value of 0 == success
  {
    Serial.print("PAJ7620 I2C error - halting");
    while(true) { }
  }
  

  Serial.println("PAJ7620 init: OK");
  Serial.println("Please input your gestures:");

  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Waiting for gestures");	
  u8g2.sendBuffer();					// transfer internal memory to the display 
  Serial.println("Display initialized");

}
 

// *********************************************************************
void loop()
{
  // Serial.println("Going in reading"); 
  Gesture gesture;                // Gesture is an enum type from RevEng_PAJ7620.h
  gesture = sensor.readGesture();   // Read back current gesture (if any) of type Gesture
  // Serial.println("Finished  reading"); 
  // Make a counter for last gesture
  if (gesture == last)
  {
    reps++;
  }
  else if (gesture != GES_NONE)
  {
    reps = 1;
  }

  if(gesture != GES_NONE)
  {
    last = gesture;
  }

  switch (gesture)
  {
    case GES_FORWARD:
      {
        Serial.println("forward"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Forward");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_BACKWARD:
      {
        break;
      }
    case GES_LEFT:
      {
        Serial.println("Left"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Left");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_RIGHT:
      {
        Serial.println("Right"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Right");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_UP:
      {
        Serial.println("Up"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Up");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_DOWN:
      {
        Serial.println("Down"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Down");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_CLOCKWISE:
      {
        Serial.println("CW"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Clockwise");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_ANTICLOCKWISE:
      {
        Serial.println("CCW"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Counter-Clockwise");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_WAVE:
      {
        Serial.println("Wave"); 
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Wave");	// write something to the internal memory
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();					// transfer internal memory to the display 
        break;
      }

    case GES_NONE:
      {
        break;
      }
  }
  
  if(gesture == GES_FORWARD){
    delay(1500); // give the user enough time to pull away from the sensor
  }
  

  /*
  if( (gesture != GES_NONE) && (gesture != GES_BACKWARD) )
  {
    Serial.print(", Code: ");
    Serial.println(gesture);
  }
  */
  
  delay(100);
}