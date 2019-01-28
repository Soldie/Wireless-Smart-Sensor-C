#ifdef STDIO_UART_BAUDRATE
#define SERIAL_BAUDRATE STDIO_UART_BAUDRATE
#else
#define SERIAL_BAUDRATE 115200
#endif

// Assign the default LED pin
int ledPin = 6;//ARDUINO_LED;

// input buffer for receiving chars on the serial port
int buf[64];

// counter that counts the number of received chars
int count = 0;

void setup(void)
{
    // configure the LED pin to be output
    pinMode(ledPin, OUTPUT);
    // configure the first serial port to run with the previously defined
    // baudrate
    Serial.begin(SERIAL_BAUDRATE);
    // say hello
    Serial.println("Hello Arduino!");
}

void loop(void)
{
    // toggle the LED
    digitalWrite(ledPin, !digitalRead(ledPin));
    // test if some chars were received
    while (Serial.available() > 0) {
        // read a single character
        int tmp = Serial.read();
        // if we got a line end, we echo the buffer
        if (tmp == '\n') {
            // start with printing 'ECHO: '
            Serial.write("Echo: ");
            // echo the buffer
            for (int i = 0; i < count; i++) {
                Serial.write(buf[i]);
            }
            // terminate the string with a newline
            Serial.write('\n');
            // reset the count variable
            count = 0;
        }
        // else we just remember the incoming char
        else {
            buf[count++] = tmp;
        }
    }
    // wait for half a second
    delay(500);
}
