/**
 * BOT14000 WisBlock Core OLED - Simple Menu System with Working Brightness
 * 
 * Navigation:
 * - Left Button (TX Pin): SELECT menu item / BACK to previous menu
 * - Right Button (RX Pin): NAVIGATE between items / CHANGE selected value
 * 
 * Brightness only (0-30 range where changes are visible)
 * 
 * Author: PreciousRoy0
 * Date: 2025-08-12
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
#define BTN_LEFT PIN_SERIAL1_TX     // Pin 1 (TX) - SELECT/BACK
#define BTN_RIGHT PIN_SERIAL1_RX    // Pin 0 (RX) - NAVIGATE/CHANGE

// Alternative pin definitions
#ifndef PIN_SERIAL1_TX
  #define BTN_LEFT 1
#endif
#ifndef PIN_SERIAL1_RX
  #define BTN_RIGHT 0
#endif

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Menu states
enum MenuLevel {
  MAIN_LEVEL,      // Main menu selection
  SUB_LEVEL,       // Submenu navigation
  EDIT_LEVEL       // Editing a value
};

enum MainMenus {
  MENU_DISPLAY = 0,
  MENU_SYSTEM,
  MENU_DEMO,
  MAIN_COUNT
};

enum SubMenus {
  // Display submenu (only brightness now)
  DISP_BRIGHT = 0,
  DISP_BACK,
  DISP_COUNT,
  
  // System submenu
  SYS_VERSION = 0,
  SYS_UPTIME,
  SYS_BACK,
  SYS_COUNT,
  
  // Demo submenu
  DEMO_COUNTER = 0,
  DEMO_ANIM,
  DEMO_BACK,
  DEMO_COUNT
};

// State variables
MenuLevel currentLevel = MAIN_LEVEL;
int mainSelection = 0;
int subSelection = 0;
int maxSubItems = 0;

// Display Settings (0-30 range where changes are actually visible)
int brightness = 20;    // Default 20/30

// Demo settings
int demoCounter = 0;
int animFrame = 0;

// Button handling
bool leftPressed = false;
bool rightPressed = false;
bool leftLast = false;
bool rightLast = false;
unsigned long lastButtonTime = 0;
unsigned long lastUpdate = 0;
unsigned long startTime;

// Menu item names
const char* mainMenuNames[] = {"Display", "System", "Demo"};
const char* displayMenuNames[] = {"Bright", "Back"};
const char* systemMenuNames[] = {"Version", "Uptime", "Back"};
const char* demoMenuNames[] = {"Counter", "Animation", "Back"};

void setup() {
  // Enable 3V3_S power rail
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("BOT14000 Menu with Brightness Control");
  Serial.println("L=Select/Back R=Navigate/Change");
  Serial.println("Brightness range: 0-30 (visible changes)");
  
  startTime = millis();
  
  // Initialize display
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display init failed!");
    while(1);
  }
  
  // Apply initial display settings
  applyBrightness();
  
  // Configure buttons
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  
  // Show startup
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("BOT14000");
  display.println("Ready!");
  display.display();
  delay(1000);
  
  Serial.print("Initial brightness: ");
  Serial.println(brightness);
  
  updateDisplay();
}

void loop() {
  handleButtons();
  
  if (millis() - lastUpdate > 100) {
    updateDisplay();
    lastUpdate = millis();
  }
  
  delay(10);
}

void applyBrightness() {
  // Map 0-30 range to the useful SSD1306 brightness range
  // Use the range where you can actually see changes (0x01 to 0x80)
  uint8_t brightnessValue = map(brightness, 0, 30, 0x01, 0x80);
  
  display.ssd1306_command(0x81); // Set contrast/brightness command
  display.ssd1306_command(brightnessValue);
  
  Serial.print("Brightness ");
  Serial.print(brightness);
  Serial.print("/30 -> 0x");
  Serial.println(brightnessValue, HEX);
}

void handleButtons() {
  if (millis() - lastButtonTime < 150) return; // Debounce
  
  bool leftCurrent = !digitalRead(BTN_LEFT);
  bool rightCurrent = !digitalRead(BTN_RIGHT);
  
  leftPressed = leftCurrent && !leftLast;
  rightPressed = rightCurrent && !rightLast;
  
  if (leftPressed) {
    handleLeftButton();
    lastButtonTime = millis();
  }
  
  if (rightPressed) {
    handleRightButton();
    lastButtonTime = millis();
  }
  
  leftLast = leftCurrent;
  rightLast = rightCurrent;
}

void handleLeftButton() {
  if (currentLevel == MAIN_LEVEL) {
    // Enter submenu
    currentLevel = SUB_LEVEL;
    subSelection = 0;
    
    switch (mainSelection) {
      case MENU_DISPLAY:
        maxSubItems = DISP_COUNT;
        break;
      case MENU_SYSTEM:
        maxSubItems = SYS_COUNT;
        break;
      case MENU_DEMO:
        maxSubItems = DEMO_COUNT;
        break;
    }
    Serial.print("Enter submenu: ");
    Serial.println(mainSelection);
    
  } else if (currentLevel == SUB_LEVEL) {
    // Check if Back option is selected
    bool isBackOption = false;
    if ((mainSelection == MENU_DISPLAY && subSelection == DISP_BACK) ||
        (mainSelection == MENU_SYSTEM && subSelection == SYS_BACK) ||
        (mainSelection == MENU_DEMO && subSelection == DEMO_BACK)) {
      isBackOption = true;
    }
    
    if (isBackOption) {
      // Go back to main menu
      currentLevel = MAIN_LEVEL;
      Serial.println("Back to main menu");
    } else {
      // Check if this item can be edited
      bool canEdit = false;
      
      if (mainSelection == MENU_DISPLAY && subSelection == DISP_BRIGHT) {
        canEdit = true;
      } else if (mainSelection == MENU_DEMO && subSelection == DEMO_COUNTER) {
        canEdit = true;
      }
      
      if (canEdit) {
        currentLevel = EDIT_LEVEL;
        Serial.println("Enter edit mode");
      } else {
        // Can't edit, treat as info display
        Serial.println("Info display - use Back to exit");
      }
    }
    
  } else if (currentLevel == EDIT_LEVEL) {
    // Exit edit mode back to submenu
    currentLevel = SUB_LEVEL;
    Serial.println("Exit edit mode");
  }
}

void handleRightButton() {
  if (currentLevel == MAIN_LEVEL) {
    // Navigate main menu
    mainSelection++;
    if (mainSelection >= MAIN_COUNT) {
      mainSelection = 0;
    }
    Serial.print("Main menu: ");
    Serial.println(mainSelection);
    
  } else if (currentLevel == SUB_LEVEL) {
    // Navigate submenu
    subSelection++;
    if (subSelection >= maxSubItems) {
      subSelection = 0;
    }
    Serial.print("Sub menu: ");
    Serial.println(subSelection);
    
  } else if (currentLevel == EDIT_LEVEL) {
    // Change values and apply them immediately
    if (mainSelection == MENU_DISPLAY && subSelection == DISP_BRIGHT) {
      brightness += 1;  // Single step increments for fine control
      if (brightness > 30) brightness = 0;
      
      applyBrightness();
      delay(50); // Brief delay to let settings take effect
      
      Serial.print("Brightness: ");
      Serial.print(brightness);
      Serial.println("/30");
      
    } else if (mainSelection == MENU_DEMO && subSelection == DEMO_COUNTER) {
      demoCounter++;
      Serial.print("Counter: ");
      Serial.println(demoCounter);
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  if (currentLevel == MAIN_LEVEL) {
    drawMainMenu();
  } else {
    drawSubMenu();
  }
  
  display.display();
}

void drawScrollingMenu(const char* title, const char** menuItems, int itemCount, int selection) {
  // Draw title
  display.setCursor(0, 0);
  display.println(title);
  
  // Calculate which items to show (2 items max)
  int startItem = 0;
  if (itemCount > 2) {
    if (selection >= 1) {
      startItem = selection - 1;
    }
    if (startItem > itemCount - 2) {
      startItem = itemCount - 2;
    }
  }
  
  // Draw up to 2 menu items
  for (int i = 0; i < 2 && (startItem + i) < itemCount; i++) {
    int itemIndex = startItem + i;
    int yPos = 12 + (i * 10);
    
    display.setCursor(0, yPos);
    
    // Show selection indicator
    if (itemIndex == selection) {
      display.print(">");
    } else {
      display.print(" ");
    }
    
    display.println(menuItems[itemIndex]);
  }
  
  // Show scroll indicators if needed
  if (itemCount > 2) {
    if (startItem > 0) {
      // Up arrow
      display.setCursor(56, 12);
      display.print("^");
    }
    if (startItem < itemCount - 2) {
      // Down arrow
      display.setCursor(56, 22);
      display.print("v");
    }
  }
}

void drawMainMenu() {
  drawScrollingMenu("MAIN", mainMenuNames, MAIN_COUNT, mainSelection);
}

void drawSubMenu() {
  switch (mainSelection) {
    case MENU_DISPLAY:
      drawDisplayMenu();
      break;
    case MENU_SYSTEM:
      drawSystemMenu();
      break;
    case MENU_DEMO:
      drawDemoMenu();
      break;
  }
}

void drawDisplayMenu() {
  // For Back option, show as regular menu
  if (subSelection == DISP_BACK) {
    drawScrollingMenu("DISPLAY", displayMenuNames, DISP_COUNT, subSelection);
    return;
  }
  
  // Show title with edit indicator
  display.setCursor(0, 0);
  display.print("DISPLAY");
  if (currentLevel == EDIT_LEVEL) {
    display.print(" *");  // Show we're in edit mode
  }
  
  // Show brightness with bar graph
  display.setCursor(0, 12);
  if (subSelection == DISP_BRIGHT) {
    if (currentLevel == EDIT_LEVEL) {
      display.print("*");  // Edit indicator
    } else {
      display.print(">");  // Navigate indicator
    }
  } else {
    display.print(" ");
  }
  
  display.print("Bright:");
  display.println(brightness);
  
  // Draw brightness bar (visual feedback)
  display.setCursor(0, 22);
  display.print("[");
  int bars = map(brightness, 0, 30, 0, 8);
  for (int i = 0; i < 8; i++) {
    if (i < bars) {
      display.print("=");
    } else {
      display.print("-");
    }
  }
  display.print("]");
}

void drawSystemMenu() {
  // For info items, show full screen when selected
  if (subSelection == SYS_VERSION) {
    display.setCursor(0, 0);
    display.println("VERSION");
    display.setCursor(0, 12);
    display.println("BOT14000");
    display.setCursor(0, 22);
    display.println("v1.0");
  } else if (subSelection == SYS_UPTIME) {
    display.setCursor(0, 0);
    display.println("UPTIME");
    
    unsigned long uptime = (millis() - startTime) / 1000;
    unsigned long minutes = uptime / 60;
    unsigned long seconds = uptime % 60;
    
    display.setCursor(0, 12);
    display.print(minutes);
    display.print(" min");
    display.setCursor(0, 22);
    display.print(seconds);
    display.print(" sec");
  } else {
    // Show menu navigation
    drawScrollingMenu("SYSTEM", systemMenuNames, SYS_COUNT, subSelection);
  }
}

void drawDemoMenu() {
  if (subSelection == DEMO_COUNTER) {
    display.setCursor(0, 0);
    display.print("COUNTER");
    if (currentLevel == EDIT_LEVEL) {
      display.print(" *");  // Show we're in edit mode
    }
    
    display.setTextSize(2);
    display.setCursor(5, 12);
    display.println(demoCounter);
  } else if (subSelection == DEMO_ANIM) {
    display.setCursor(0, 0);
    display.println("ANIMATION");
    
    // Simple animation
    animFrame = (millis() / 100) % 48;
    display.fillCircle(8 + animFrame, 20, 2, SSD1306_WHITE);
    display.drawCircle(32, 20, 6, SSD1306_WHITE);
  } else {
    // Show menu navigation including Back option
    drawScrollingMenu("DEMO", demoMenuNames, DEMO_COUNT, subSelection);
  }
}