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
#include <math.h>

// Create gesture sensor driver object
RevEng_PAJ7620 sensor = RevEng_PAJ7620();

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Gesture last;
int reps = 1;

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

// Debounce / activation thresholds for rotation steps
const int DEBOUNCE_STEPS      = 3; // consecutive same-direction steps before direction is "recognized"
const int VOLUME_START_STEPS  = 5; // consecutive same-direction steps before volume actually starts changing

int pendingDirection = 0; // 0 = none, +1 / -1 = current streak's direction
int pendingCount      = 0; // consecutive steps counted in pendingDirection

float lastAngle  = NAN;
float angleAccum = 0.0;
float centerX    = NAN;
float centerY    = NAN;

int volume = 50; // 0..100, replace with your actual volume interface

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
  u8g2.setFont(u8g2_font_t0_30_tf);	// choose a suitable font
  u8g2.drawStr(0,30,"Waiting for gestures");
  u8g2.sendBuffer();					// transfer internal memory to the display
  Serial.println("Display initialized");
}

// *********************************************************************
void loop()
{
  handleDiscreteGestures();
  handleRotationVolume();

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
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Forward");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
        break;
      }

    case GES_BACKWARD:
      {
        break;
      }
    case GES_LEFT:
      {
        Serial.println("Down");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Down");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
        break;
      }

    case GES_RIGHT:
      {
        Serial.println("Up");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Up");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
        break;
      }

    case GES_UP:
      {
        Serial.println("Left");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Left");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
        break;
      }

    case GES_DOWN:
      {
        Serial.println("Right");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Right");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
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
        u8g2.setFont(u8g2_font_t0_30_tf);
        u8g2.drawStr(0,20,"Wave");
        u8g2.setCursor(0,50);
        u8g2.print(reps);
        u8g2.sendBuffer();
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
  u8g2.setFont(u8g2_font_t0_30_tf);
  u8g2.drawStr(0,20, dir > 0 ? "Volume Up" : "Volume Down");
  u8g2.setCursor(0,50);
  u8g2.print(volume);
  u8g2.sendBuffer();

  // TODO: call your actual volume-change function here
}
