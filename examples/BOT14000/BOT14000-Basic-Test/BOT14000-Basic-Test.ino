/**
 * BOT14000 WisBlock Core OLED - Improved Debounce
 * 
 * Features:
 * - Non-blocking debounced button detection
 * - Catches quick button presses
 * - Separate display update timing
 * - Responsive input handling
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display configuration
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32  
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions
#define BTN1_HALL1_PIN PIN_SERIAL1_TX
#define BTN2_HALL2_PIN PIN_SERIAL1_RX

// Timing constants
#define DEBOUNCE_TIME 20        // 20ms debounce
#define DISPLAY_UPDATE_TIME 100 // Update display every 100ms

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button state variables
struct ButtonState {
  bool current_reading;
  bool last_reading;
  bool last_stable_state;
  unsigned long last_change_time;
  bool pressed_event;  // Flag for new press events
};

ButtonState btn1_state = {false, false, false, 0, false};
ButtonState btn2_state = {false, false, false, 0, false};

// Display variables
unsigned long last_display_update = 0;
int counter = 0;

void setup() {
  // Power up modules first
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);

  Serial.begin(115200);
  delay(2000);

  Serial.println("BOT14000 WisBlock Core OLED - Improved Debounce");
  Serial.println("===============================================");
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERROR: SSD1306 allocation failed!");
    while(1);
  }
  
  Serial.println("Display initialized successfully");
  
  // Configure input pins
  pinMode(BTN1_HALL1_PIN, INPUT_PULLUP);
  pinMode(BTN2_HALL2_PIN, INPUT_PULLUP);
  
  // Initialize button states
  btn1_state.last_reading = digitalRead(BTN1_HALL1_PIN);
  btn2_state.last_reading = digitalRead(BTN2_HALL2_PIN);
  btn1_state.last_stable_state = btn1_state.last_reading;
  btn2_state.last_stable_state = btn2_state.last_reading;
  
  // Show startup message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("BOT14000");
  display.println("Ready!");
  display.display();
  delay(2000);
  
  Serial.println("Setup complete - ready for button presses");
}

void loop() {
  // Handle buttons (non-blocking, fast response)
  handleButton(&btn1_state, BTN1_HALL1_PIN);
  handleButton(&btn2_state, BTN2_HALL2_PIN);
  
  // Check for button press events
  if (btn1_state.pressed_event) {
    Serial.println("Button/Hall 1 activated!");
    counter++;
    btn1_state.pressed_event = false; // Clear the event flag
  }
  
  if (btn2_state.pressed_event) {
    Serial.println("Button/Hall 2 activated!");
    counter--;
    btn2_state.pressed_event = false; // Clear the event flag
  }
  
  // Update display at regular intervals (non-blocking)
  if (millis() - last_display_update >= DISPLAY_UPDATE_TIME) {
    updateDisplay();
    last_display_update = millis();
  }
  
  // Very short delay to prevent overwhelming the CPU
  delay(1);
}

void handleButton(ButtonState* btn, int pin) {
  // Read current state (inverted because INPUT_PULLUP is active low)
  btn->current_reading = !digitalRead(pin);
  
  // Check if the reading has changed
  if (btn->current_reading != btn->last_reading) {
    // Reset debounce timer
    btn->last_change_time = millis();
  }
  
  // Check if enough time has passed for debouncing
  if ((millis() - btn->last_change_time) > DEBOUNCE_TIME) {
    // If the reading has been stable and different from last stable state
    if (btn->current_reading != btn->last_stable_state) {
      btn->last_stable_state = btn->current_reading;
      
      // Detect press event (transition from not pressed to pressed)
      if (btn->current_reading == true) {
        btn->pressed_event = true;
      }
    }
  }
  
  // Save the current reading for next time
  btn->last_reading = btn->current_reading;
}

void updateDisplay() {
  display.clearDisplay();
  
  // Title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("BOT14000");
  
  // Counter value
  display.setCursor(0, 10);
  display.print("Count: ");
  display.println(counter);
  
  // Button states (show current stable states)
  display.setCursor(0, 20);
  display.print("B1:");
  display.print(btn1_state.last_stable_state ? "ON " : "-- ");
  display.print("B2:");
  display.print(btn2_state.last_stable_state ? "ON" : "--");
  
  // Simple animation
  int animPos = (millis() / 100) % 64;
  display.drawPixel(animPos, 31, SSD1306_WHITE);
  
  display.display();
}