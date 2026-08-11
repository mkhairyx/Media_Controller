#include <IRremote.hpp>

#define IR_RECEIVE_PIN 4

void setup() {
    Serial.begin(115200);
    delay(1000); // let serial monitor connect
    Serial.println("IR Receiver ready");

    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
    if (IrReceiver.decode()) {
        // IrReceiver.printIRResultShort(&Serial); // prints protocol, address, command
        IrReceiver.printIRSendUsage(&Serial);   // shows the send() call to replay this code
        IrReceiver.resume();                    // ready for next signal
    }
}