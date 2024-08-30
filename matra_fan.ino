
#include <Wire.h>

#define OLED
#ifdef OLED
  #include <SPI.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// BME280 I2C addresses
#define BME280_ADDR1 0x76 // Inside
#define BME280_ADDR2 0x77 // Outside

// Objects for the two BME280 sensors
Adafruit_BME280 bme1; // Inside
Adafruit_BME280 bme2; // Outside


// WiFi credentials
const char* ssid = "MATRA_FAN";
const char* password = "ingyenwifi";

// Web server on port 80
// 192.168.4.1:80
ESP8266WebServer server(80);

// File name for logging
const char* logFileName = "/data.log";

// char buffer for snprintf:
char charbuffer[512];

const int analogPin = A0;   // Pin to read the analog value
const int pwmPin = D8;      // Pin to output PWM (GPIO15)
const int pwmFrequency = 100; // PWM frequency in Hz
const int digitalPin = D7;


#ifdef OLED
  // Display:
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels

  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
  // The pins for I2C are defined by the Wire-library. 
  // On an arduino UNO:       A4(SDA), A5(SCL)
  // On an arduino MEGA 2560: 20(SDA), 21(SCL)
  // On an arduino LEONARDO:   2(SDA),  3(SCL), ...
  #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// Function to calculate the moisture content of the air
// returns grams per m3
float calculateMoistureContent(float temperature, float humidity) {
    // Formula for moisture content (simplified version)
    // Adjust the formula according to your specific needs

    // Adjusted for correction via desmos:
    //y\ =\ 4.89*\exp((16.65*x)/(x+243.12))
    return (humidity / 100.0) * 4.89 * exp((16.65 * temperature) / (temperature + 243.12));
}

// Function to log data to LittleFS
void logData() {
    File logFile = LittleFS.open(logFileName, "a");
    if (logFile) {
        // want:
        // %i T1:%.2f, H1:%.1f, P1:%.1f, M1%.2f, %i T1:%.2f, H1:%.1f, P1:%.1f, M1%.2f, fan=%.1f
        //logFile.printf("Sensor 1 - Temp: %.2f, Humidity: %.2f, Pressure: %.2f, Moisture Content: %.2f\n", temp1, hum1, pres1, calculateMoistureContent(temp1, hum1));
        //logFile.printf("Sensor 2 - Temp: %.2f, Humidity: %.2f, Pressure: %.2f, Moisture Content: %.2f\n", temp2, hum2, pres2, calculateMoistureContent(temp2, hum2));
        logFile.printf("%s", charbuffer);
        logFile.close();
    }
}

// Handle the root page
void handleRoot() {
    // Begin response with a HTTP 200 status code and content type
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/plain");

    // Send initial message
    server.sendContent("BME280 Sensor Data Log:\n\n");

    // Open the log file
    File logFile = LittleFS.open(logFileName, "r");
    if (logFile) {
        String line;
        // Read and send the file content line by line
        while (logFile.available()) {
            line = logFile.readStringUntil('\n'); // Read one line at a time
            line += "\n"; // Add newline character
            server.sendContent(line); // Send the line to the client
        }
        logFile.close();
    } else {
        // If the file can't be opened, send an error message
        server.sendContent("Error: Log file not found or failed to open.");
    }

    // Finalize the response
    server.sendContent(""); // This signals the end of the response
    server.client().stop(); // Close the connection to the client
}
/*
void handleRoot() {
    String message = "BME280 Sensor Data Log:\n\n";
    File logFile = LittleFS.open(logFileName, "r");
    if (logFile) {
        while (logFile.available()) {
            message += logFile.readStringUntil('\n') + "\n"; // this is gonna oom :/
        }
        logFile.close();
    }
    server.send(200, "text/plain", message);
}*/

void checkSerialCommand() {
    // Check if data is available on the serial port
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n'); // Read the incoming command

        // Remove any newline or carriage return characters
        command.trim();

        // Handle the "dump" command
        if (command.equalsIgnoreCase("dump")) {
            dumpLogToSerial();
        }
        // Handle the "delete" command
        if (command.equalsIgnoreCase("delete")) {
            deleteLogFile();
        }
    
    }
}

void dumpLogToSerial() {
    // Open the log file
    File logFile = LittleFS.open(logFileName, "r");
    
    if (logFile) {
        Serial.println("Dumping data.log contents:");
        while (logFile.available()) {
            String line = logFile.readStringUntil('\n'); // Read one line at a time
            Serial.println(line); // Print the line to the Serial Monitor
        }
        logFile.close();
        Serial.println("End of log file.");
    } else {
        Serial.println("Error: Log file not found or failed to open.");
    }
}

void deleteLogFile() {
    // Check if the file exists before attempting to delete
    if (LittleFS.exists(logFileName)) {
        if (LittleFS.remove(logFileName)) {
            Serial.println("Log file deleted successfully.");
        } else {
            Serial.println("Error: Failed to delete the log file.");
        }
    } else {
        Serial.println("Log file does not exist.");
    }
}


// Setup function
void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(500);
    Serial.println("Serial init at 115200");
    
    // Initialize BME280 sensors
    if (!bme1.begin(BME280_ADDR1)) {
        Serial.println("Could not find BME280 sensor 1!");
        //while (1);
    }

       // Initialize BME280 sensors
    if ( !bme2.begin(BME280_ADDR2)) {
        Serial.println("Could not find BME280 sensor 2!");
        //while (1);
    }
    
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        //return;
    }
    
    // Initialize WiFi in AP mode
    WiFi.softAP(ssid, password);
    Serial.println("WiFi AP started");
    
    // Initialize web server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    
    analogWriteFreq(pwmFrequency);
    pinMode(pwmPin, OUTPUT);
    digitalWrite(pwmPin, LOW);
    pinMode(digitalPin, OUTPUT);
    digitalWrite(digitalPin, LOW);


    #ifdef OLED
      // Init display:
      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
      if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
          Serial.println(F("SSD1306 allocation failed"));

        }
        display.clearDisplay();
        Serial.println("Display started");
    #endif
}

static unsigned long lastTime = 0;
static unsigned long logTime = 0;
// Loop function
void loop() {
    // Read sensor data every minute
    unsigned long currentTime = millis();
    
    if (currentTime - lastTime >= 10000) {
        checkSerialCommand(); // Continuously check for serial commands

        lastTime = currentTime;

        // Sensor 1 readings INDOORS
        float tempIn = bme1.readTemperature();
        float humIn = bme1.readHumidity();
        float presIn = bme1.readPressure(); // Pa
        float moistIn = calculateMoistureContent(tempIn, humIn);
        
        // Sensor 2 readings OUTDOORS
        float tempOut = bme2.readTemperature();
        float humOut = bme2.readHumidity();
        float presOut = bme2.readPressure(); // Pa
        float moistOut = calculateMoistureContent(tempOut, humOut);
        
        int analogValue = analogRead(analogPin);


        // sanity checks
        // temps < 50C, > -20C
        // humidity > 2%
        // Pressure > 500 hPa

        //

        // Rules for fan:
        // 1. Sanity check passed, fan can be on
        // 2. near freezing outdoors, fan must be off
        // 3. Hotter outside than inside:
        // 
        bool fanON = false; // start at off

        if ((tempIn > -20.0) && (tempOut > -20) &&
            (tempIn < 50.0) && (tempIn < 50.0) && 
            (humIn > 1.0) && (humOut > 1.0) && 
            (presIn > 100.0) && (presOut > 100.0)){

          if (tempOut > 15.0){ // Warm outdoors
            if (moistOut < moistIn){ // Dryer outdoors
              fanON = true;
            }
          }
          else if (tempOut > 5.0){ // not freezing outdoors
            if (tempOut > tempIn){ // warmer outdoors than in
              if (moistOut < moistIn){ // dryer outdoors than indoors
                fanON = true;
              }
            }
          }else{
            //  freezing outdoors

          }
        }
        sprintf(charbuffer, "%d, T1:%.2f, H1:%.1f, P1:%.1f, M1:%.2f, T2:%.2f, H2:%.1f, P2:%.1f, M2:%.2f, fan=%i, A=%d\n", 
          currentTime/1000,
          tempIn, humIn, presIn, moistIn,
          tempOut, humOut, presOut, moistOut,
          fanON, analogValue
          );

        Serial.print(charbuffer);

        if (fanON){
          analogWrite(pwmPin,analogValue);
          digitalWrite(digitalPin, HIGH);
        }else{
          digitalWrite(pwmPin, LOW);
          digitalWrite(digitalPin, LOW);
        }

        // Update Display
          // Clear the buffer.

          if (1){
            
          display.clearDisplay();

          // Display Text
          display.setTextSize(1);
          display.setTextColor(WHITE);
          int ypos = 0;

          display.setCursor(0,ypos);
          display.print("TI="); display.print(tempIn,1);
          display.setCursor(64,ypos);
          display.print("TO="); display.print(tempOut,1);
          ypos = ypos + 11;

          display.setCursor(0,ypos);
          display.print("HI="); display.print(humIn,1);
          display.setCursor(64,ypos);
          display.print("HO="); display.print(humOut,1);
          ypos = ypos + 11;

          display.setCursor(0,ypos);
          display.print("MI="); display.print(moistIn,2);
          display.setCursor(64,ypos);
          display.print("MO="); display.print(moistOut,2);
          ypos = ypos + 11;

          display.setCursor(0,ypos);
          display.print("PI="); display.print(presIn,1);
          display.setCursor(64,ypos);
          display.print("PO="); display.print(presOut,1);
          ypos = ypos + 11;



          display.setCursor(0,ypos);
          display.print("F="); display.print(fanON);
          display.setCursor(32,ypos);
          // Read the analog value from A0 (range is 0-1023)

          display.print("A="); display.print(analogValue);

          display.setCursor(64,ypos);
          display.print("t="); display.print(currentTime/1000);
          display.display();
          
          }
        const int tSecond = 1000;
        const int tMinute = 60;
        const int tHour = 20;
        if (currentTime - logTime >= (tSecond * tMinute * tHour)) { // 20 mins
          logTime = currentTime;
          // Log data to LittleFS
          // log every 15 mins only
          logData();
        }
    }
    
    // Handle client requests
    server.handleClient();


    if (currentTime >= 86400000) { // 86400000 ms = 24 hours
        Serial.println("24 hours have passed. Resetting the ESP8266.");
        ESP.restart(); // Reset the microcontroller
    }

}
