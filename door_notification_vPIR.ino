/*
 * Door Notification System - Arduino Code
 * Detects person presence and displays "Room Occupied" on LCD
 * 
 * Components:
 * - PIR Motion Sensor
 * - 16x2 LCD Display (I2C)
 * - Arduino Uno/Nano
 */

#include <LiquidCrystal_I2C.h>

// Pin definitions
const int PIR_PIN = 2;           // PIR sensor digital pin
const int LED_PIN = 13;          // Built-in LED for visual indicator

// LCD setup (I2C address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables for motion detection
bool motionDetected = false;
bool roomOccupied = false;
unsigned long lastMotionTime = 0;
unsigned long occupancyStartTime = 0;

// Timing constants
const unsigned long MOTION_TIMEOUT = 30000;    // 30 seconds - time to wait before clearing "occupied" status
const unsigned long DEBOUNCE_DELAY = 2000;     // 2 seconds - debounce time to prevent false triggers
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // 1 second - how often to update display

unsigned long lastDisplayUpdate = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Door Notification System Starting...");
  
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("Door Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  delay(2000);
  lcd.clear();
  
  // Initial display
  displayRoomStatus();
  
  Serial.println("System initialized successfully!");
}

void loop() {
  // Read PIR sensor
  int pirState = digitalRead(PIR_PIN);
  unsigned long currentTime = millis();
  
  // Check for motion detection
  if (pirState == HIGH) {
    if (!motionDetected || (currentTime - lastMotionTime > DEBOUNCE_DELAY)) {
      motionDetected = true;
      lastMotionTime = currentTime;
      
      if (!roomOccupied) {
        roomOccupied = true;
        occupancyStartTime = currentTime;
        digitalWrite(LED_PIN, HIGH);
        
        Serial.println("Motion detected! Room is now occupied.");
        displayRoomStatus();
      } else {
        Serial.println("Motion detected while room occupied - extending occupancy time.");
      }
    }
  }
  
  // Check if room should be marked as vacant
  if (roomOccupied && (currentTime - lastMotionTime > MOTION_TIMEOUT)) {
    roomOccupied = false;
    motionDetected = false;
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("No motion for timeout period. Room marked as vacant.");
    displayRoomStatus();
  }
  
  // Update display periodically to show occupancy duration
  if (currentTime - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentTime;
    if (roomOccupied) {
      updateOccupancyDuration();
    }
  }
  
  delay(100); // Small delay to prevent excessive polling
}

void displayRoomStatus() {
  lcd.clear();
  
  if (roomOccupied) {
    lcd.setCursor(0, 0);
    lcd.print("ROOM OCCUPIED");
    lcd.setCursor(0, 1);
    lcd.print("Duration: 0m 0s");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Room Available");
    lcd.setCursor(0, 1);
    lcd.print("Ready to Monitor");
  }
}

void updateOccupancyDuration() {
  if (roomOccupied) {
    unsigned long occupancyDuration = (millis() - occupancyStartTime) / 1000; // Convert to seconds
    int minutes = occupancyDuration / 60;
    int seconds = occupancyDuration % 60;
    
    lcd.setCursor(0, 1);
    lcd.print("Duration: ");
    lcd.print(minutes);
    lcd.print("m ");
    lcd.print(seconds);
    lcd.print("s");
    
    // Clear any remaining characters
    if (minutes < 10 && seconds < 10) {
      lcd.print("  ");
    } else if (minutes < 10 || seconds < 10) {
      lcd.print(" ");
    }
  }
}

// Function to manually reset the system (can be called if needed)
void resetSystem() {
  roomOccupied = false;
  motionDetected = false;
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("System manually reset.");
  displayRoomStatus();