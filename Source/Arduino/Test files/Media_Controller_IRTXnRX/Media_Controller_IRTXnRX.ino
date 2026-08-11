#include <Arduino.h>


// ESP32 NVS Headers
#include <Preferences.h>
Preferences TV;

// Display Library Headers
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

bool del = 0;
volatile bool del_touch = 0;
#define delete_pin 18
void delete_function(void) {
  unsigned long trig = millis();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Keep holding to delete");
  u8g2.drawStr(0,20,"stored data.");
  u8g2_prepare();
  u8g2.drawRFrame(2, 49,124,15, 5);
  u8g2.sendBuffer();
  del = 0;
  while( (millis() - trig) < 5000)
  {
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    u8g2.drawStr(0,10,"Keep holding to delete");
    u8g2.drawStr(0,20,"stored data.");
    u8g2_prepare();
    u8g2.drawRFrame(2, 49,124,15, 5);
    u8g2.sendBuffer();
    for (int i = 10; (i < 125) && !digitalRead(delete_pin); i++)
    {
      //u8g2.clearBuffer();
      //u8g2.drawRFrame(2, 49,124,15, 5);
      u8g2.drawRBox(2, 49,i,15, 5);
      u8g2.sendBuffer();
      delay(10);
      if (i == 124)
      {
        del = 1;
      }
    }
  }
  if (del == 1)
  {
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    u8g2.drawStr(0,10,"Deleting...");
    u8g2.sendBuffer();
    TV.begin("signals", false); 
    TV.clear();
    TV.end();
    delay(3500);
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    u8g2.drawStr(0,10,"Deleted!");
    u8g2.drawStr(0,20,"Restarting..");
    u8g2.sendBuffer();
    delay(1000);
    ESP.restart(); 
  }
  del_touch = 0;
}

void IRAM_ATTR delete_isr() 
{
  del_touch = 1;
}




// A structure for IRButton data type which stores name of the command, the command and the address.
struct IRButton {
  const char* name;
  uint8_t command;
  uint8_t address;
};

IRButton buttons[] =  {
    {.name = "left"},
    {.name = "right"},
    {.name = "up"},
    {.name = "down"},
    {.name = "ok"},
    {.name = "vol_up"},
    {.name = "vol_down"}
};


// IR Library Headers
/*
#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
*/
#define NO_LED_FEEDBACK_CODE      // Saves 216 bytes program memory
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc. Sets FLASHEND and RAMSIZE and evaluates value of SEND_PWM_BY_TIMER.
#include <IRremote.hpp>
/*
#define IR_RECEIVE_PIN 4
#define IR_SEND_PIN 18
*/

//struct Buttons




// Headers for the gesture sensor and math (for circular gestures)
// Includes enum definition of GES_* return values from readGesture()
#include "RevEng_PAJ7620.h"
#include <math.h>
// Create gesture sensor driver object
RevEng_PAJ7620 sensor = RevEng_PAJ7620();

// Variables to store last gesture and how many times it was repeated
Gesture last;
int reps = 1;

// Variables for the math that's used to recognize circular gestures
// ------------------------------------------------------------------
// Continuous clockwise/counter-clockwise volume control.
// Uses getObjectCenterX/Y (works in Gesture Mode, updates every loop,
// independent of readGesture()'s discrete GES_CLOCKWISE/ANTICLOCKWISE
// which only fires once the hand leaves view).
// ------------------------------------------------------------------
const float DEG_PER_STEP     = 120.0;  // degrees of rotation per volume step
const float DEADZONE_RADIUS  = 150.0; // ignore readings too close to the adaptive center
const float CENTER_EMA_ALPHA = 0.02;  // how fast the "center" adapts to hand drift
const int   VOLUME_STEP_SIZE = 1;     // volume change per applied step
// ------------------------------------------------------------------
// Debounce / activation thresholds for rotation steps
const int DEBOUNCE_STEPS      = 3; // consecutive same-direction steps before direction is "recognized"
const int VOLUME_START_STEPS  = 5; // consecutive same-direction steps before volume actually starts changing
// ------------------------------------------------------------------
int pendingDirection = 0; // 0 = none, +1 / -1 = current streak's direction
int pendingCount      = 0; // consecutive steps counted in pendingDirection
// ------------------------------------------------------------------
float lastAngle  = NAN;
float angleAccum = 0.0;
float centerX    = NAN;
float centerY    = NAN;
// ------------------------------------------------------------------
int volume = 50; // 0..100, replace with your actual volume interface

// *********************************************************************
void setup()
{
  Serial.begin(115200);

  pinMode(delete_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(delete_pin), delete_isr, FALLING);

  // u8g2 initialization
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"System booted up!");
  u8g2.sendBuffer();					// transfer internal memory to the display
  Serial.println("Display initialized");
  delay(1000);

  // PAJ7620 initialization
  Serial.println("PAJ7620 sensor demo: Recognizing all 9 gestures.");
  if( !sensor.begin() )        // return value of 0 == success
  {
    Serial.print("PAJ7620 I2C error - halting");
    while(true) { }
  }
  Serial.println("PAJ7620 init: OK");
  Serial.println("Please input your gestures:");

  // IR initialization
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("IR Receiver ready");

  //NVS initialization (IR signals and codes)
  int stat; // storing that status of the namespace, 1 = exists, 0 = does not exist, 2 = error reading.
  TV.begin("signals", false);
  if (TV.isKey("signals")) // checking to see if a key was previously stored
  {
    // read the data
    size_t bytesRead = TV.getBytes("signals", &buttons, sizeof(buttons));
    if (bytesRead == sizeof(buttons))
    {
      Serial.println("Settings loaded successfully");
      stat = 1;
    }
    else
    {
      Serial.println("Error reading settings.");
      stat = 2;
    }
  } 
  else
  {
    // write the data
    stat = 0;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,10,"No previously record-");
    u8g2.drawStr(0,20,"ed data found.");
    u8g2.sendBuffer();
    delay(1000);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,10,"Learning new data...");
    u8g2.sendBuffer();
    delay(1000);
    for (int i = 0; i < 7; i++)  // go through seven gestures
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0,10,"Press the following-");
      u8g2.drawStr(0,20,"button:");
      u8g2.setCursor(40, 20);
      u8g2.print(buttons[i].name);
      u8g2.sendBuffer();

      while (!IrReceiver.decode())
      {
        delay(10);
      }

      int isnec = 0;
      while (isnec == 0)
      {
        if (IrReceiver.decode()) {
          if (IrReceiver.decodedIRData.protocol == NEC) {
              buttons[i].address = IrReceiver.decodedIRData.address; // already extracted, no manual bit math needed
              buttons[i].command = IrReceiver.decodedIRData.command;
              u8g2.clearBuffer();
              u8g2.setFont(u8g2_font_ncenB08_tr);
              u8g2.drawStr(0,10,"Signal recorded and stored!");
              u8g2.sendBuffer();
              delay(1500);
              // store into your own buffer/struct here
              isnec = 1;
          } else {
              u8g2.drawStr(0,50,"Can't store this signal.");
              u8g2.drawStr(0,60,"Not NEC protocol!");
              u8g2.sendBuffer();
              isnec = 0;
          }
          IrReceiver.resume(); // ready for next signal
        }
      }
    }
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,10,"All signals recorded!");
    u8g2.drawStr(0,20,"Continuing..");
    u8g2.sendBuffer();
    delay(2000);

  }

  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Waiting for gestures...");
  u8g2.sendBuffer();
  TV.putBytes("signals", &buttons, sizeof(buttons));
  TV.end();
}

// *********************************************************************
void loop()
{
  handleDiscreteGestures();
  handleRotationVolume();
  if (del_touch)
    delete_function();
  delay(10); // lowered from 100ms: rotation tracking needs finer polling
}

// *********************************************************************
void handleDiscreteGestures()
{
  Gesture gesture;                  // Gesture is an enum type from RevEng_PAJ7620.h
  gesture = sensor.readGesture();   // Read back current gesture (if any) of type Gesture

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
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Forward");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        IrSender.sendNEC(buttons[4].address, buttons[4].command, 0); //ok button
        break;
      }

    case GES_BACKWARD:
      {
        break;
      }
    case GES_LEFT:
      {
        Serial.println("Left");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Left");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        IrSender.sendNEC(buttons[0].address, buttons[0].command, 0);
        break;
      }

    case GES_RIGHT:
      {
        Serial.println("Right");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Right");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        IrSender.sendNEC(buttons[1].address, buttons[1].command, 0);
        break;
      }

    case GES_UP:
      {
        Serial.println("Up");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Up");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        IrSender.sendNEC(buttons[2].address, buttons[2].command, 0);
        break;
      }

    case GES_DOWN:
      {
        Serial.println("Down");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Down");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        IrSender.sendNEC(buttons[3].address, buttons[3].command, 0);
        break;
      }

    // GES_CLOCKWISE / GES_ANTICLOCKWISE intentionally NOT handled here.
    // They only fire once, after the hand fully leaves the sensor's
    // view, so they can't drive continuous volume control. That's
    // handled separately in handleRotationVolume() below, using live
    // position tracking instead of the discrete gesture event.

    case GES_WAVE:
      {
        Serial.println("Wave");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0,10,"Wave");
        u8g2.setCursor(0, 20);
        u8g2.print(reps);
        u8g2.sendBuffer();
        // no tv command
        break;
      }

    default:
      {
        break;
      }
  }

  if(gesture == GES_FORWARD){
    delay(1500); // give the user enough time to pull away from the sensor
  }
}

// *********************************************************************
void handleRotationVolume()
{
  if (sensor.isObjectInView())
  {
    int x = sensor.getObjectCenterX();
    int y = sensor.getObjectCenterY();

    if (isnan(centerX))
    {
      // first valid reading after acquiring the object: seed the center
      centerX = x;
      centerY = y;
    }
    else
    {
      // slowly drift the center toward the current position, so it
      // settles near the middle of whatever circle is being traced.
      centerX += (x - centerX) * CENTER_EMA_ALPHA;
      centerY += (y - centerY) * CENTER_EMA_ALPHA;
    }

    float dx = x - centerX;
    float dy = y - centerY;
    float radius = sqrtf(dx * dx + dy * dy);

    if (radius > DEADZONE_RADIUS)
    {
      float angle = atan2f(dy, dx) * 180.0 / PI; // -180..180

      if (!isnan(lastAngle))
      {
        float delta = angle - lastAngle;

        // handle wraparound at +/-180
        if (delta > 180.0)  delta -= 360.0;
        if (delta < -180.0) delta += 360.0;

        angleAccum += delta;

        while (angleAccum >= DEG_PER_STEP)
        {
          angleAccum -= DEG_PER_STEP;
          processRotationStep(-1);
        }
        while (angleAccum <= -DEG_PER_STEP)
        {
          angleAccum += DEG_PER_STEP;
          processRotationStep(+1);
        }
      }
      lastAngle = angle;
    }
  }
  else
  {
    // hand left view: reset so we don't get a spurious jump on return
    lastAngle = NAN;
    angleAccum = 0.0;
    centerX = NAN;
    centerY = NAN;
    pendingDirection = 0;
    pendingCount = 0;
  }
}

// *********************************************************************
// Gate raw rotation steps through debounce + activation thresholds
// before letting them actually touch the volume.
void processRotationStep(int dir)
{
  if (dir == pendingDirection)
  {
    pendingCount++;
  }
  else
  {
    // direction changed (or first step ever): restart the streak
    pendingDirection = dir;
    pendingCount = 1;
  }

  if (pendingCount == DEBOUNCE_STEPS)
  {
    // direction just became "recognized" - optional feedback point
    Serial.println(dir > 0 ? "Direction recognized: Up" : "Direction recognized: Down");
  }

  if (pendingCount >= VOLUME_START_STEPS)
  {
    // streak is long enough: this step (and every one after it, as
    // long as direction keeps holding) actually changes the volume
    volumeStep(dir);
  }
}

// *********************************************************************
void volumeStep(int dir)
{
  volume = constrain(volume + dir * VOLUME_STEP_SIZE, 0, 100);
  Serial.print("Volume: ");
  Serial.println(volume);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,10, dir > 0 ? "Volume Up" : "Volume Down");
  u8g2.setCursor(0, 20);
  u8g2.print(volume);
  u8g2.sendBuffer();
  if (dir == +1){
    IrSender.sendNEC(buttons[5].address, buttons[5].command, 0);
  }
  else if (dir == -1){
    IrSender.sendNEC(buttons[6].address, buttons[6].command, 0);
  }
}
