#include <Arduino.h>
#include <Solenoid.hpp>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>

// Debug mode
// Enables serial
#define DEBUG true

// Solonoid spin motor M2
#define SS_STEP_PIN 36
#define SS_DIR_PIN 35
#define SS_FAULT_PIN 30
#define SS_SLEEP_PIN 37
// Set SS direction
#define SS_DIR_SET 0 // This should be used as true for clockwise, false for counterclockwise

// Carriage control motor M1
#define CC_STEP_PIN 39
#define CC_DIR_PIN 38
#define CC_FAULT_PIN 29
#define CC_SLEEP_PIN 40
// Set CC direction
#define CC_DIR_SET 0 // This should be used as true for forwards, false for backwards

// Rotary encoder
#define RE_BUTTON_PIN 21
#define RE_A_PIN 22
#define RE_B_PIN 23

// Limit switches
#define LS_START_PIN 7
#define LS_END_PIN 8

// Misc constants
#define VERSION "V1.0"
#define BUTTON_DELAY 200
#define CARRIAGE_OFFSET 500 // 0.5 cm
#define PADDING 5 // Potentially needed error correction value to add/subtract from the start and end; 0.001 accuracy
#define MOTOR_DELAY 800 //ps

enum Tasks {
  ChoosePreset,
  ValEdit,
  ConfirmScreen,
  Spin,
  End,
};

// Variables
Tasks task = Tasks::ChoosePreset;

// Stepper Values
#define SS_STEPS_PER_REVOLUTION 200 // Whole steps
#define CC_STEPS_PER_REVOLUTION 400 // Half steps
#define DISTANCE_PER_REVOLUTION 800 // 0.8 cm of carriage travel
#define DISTANCE_PER_STEP 2 // 0.002 cm of carriage travel


// Define LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define Rotary Encoder
Encoder encoder(RE_A_PIN, RE_B_PIN);

// Define solenoid
Solenoid solenoid = Solenoid();

// Function definition
void startupAnimation();
void choosePreset();
void valSelect();
void confirmScreen();
void spin();
void zeroCarriage();
void motorFault(String);
void stepCC();
void stepSS();
void stepBoth();
void completionScreen();
bool pauseSpin();
String formatVal(uint32_t, uint32_t);
String formatTurns(uint32_t);
uint32_t valEditor(uint32_t, uint32_t);
WireGauge gaugeEditor(WireGauge);
int32_t turnsEditor(uint32_t);


void setup() {
  #if DEBUG 
    Serial.begin(9600);
    Serial.println("Swinder v1.0 - Debug Mode");
  #endif

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  // Initialize Rotary Encoder
  encoder.write(0);
  pinMode(RE_BUTTON_PIN, INPUT);

  // Initialize Solenoid
  solenoid.begin(Preset::None);

  // Initialize Limit Switches
  pinMode(LS_START_PIN, INPUT);
  pinMode(LS_END_PIN, INPUT);

  // Initialize CC Motor
  pinMode(CC_DIR_PIN, OUTPUT);
  pinMode(CC_STEP_PIN, OUTPUT);
  pinMode(CC_SLEEP_PIN, OUTPUT);
  pinMode(CC_FAULT_PIN, INPUT);
  digitalWrite(CC_DIR_PIN, CC_DIR_SET);
  digitalWrite(CC_SLEEP_PIN, LOW);


  // Initialize SS Motor
  pinMode(SS_DIR_PIN, OUTPUT);
  pinMode(SS_STEP_PIN, OUTPUT);
  pinMode(SS_SLEEP_PIN, OUTPUT);
  pinMode(SS_FAULT_PIN, INPUT);
  digitalWrite(SS_DIR_PIN, SS_DIR_SET);
  digitalWrite(SS_SLEEP_PIN, LOW);

  #if !DEBUG
    startupAnimation();
  #endif
  #if DEBUG
    // Math checks
    solenoid.setPreset(Preset::Debug);
    Serial.println("Length: " + String(solenoid.getLength()) + " Formatted: " + formatVal(solenoid.getLength(), MAX_LENGTH));
    Serial.println("Radius: " + String(solenoid.getRadius()) + " Formatted: " + formatVal(solenoid.getRadius(), MAX_RADIUS));
    Serial.println("Inductance: " + String(solenoid.getInductance()) + " Formatted: " + formatVal(solenoid.getInductance(), MAX_INDUCTANCE));
    Serial.println("Gauge: " + solenoid.gaugeString());
    Serial.println("Num Turns: " + String(solenoid.getTurns()));
    Serial.println("Expected Values: L = 1234, 12.34 : R = 123, 1.23 : I = 1234567, 12345.67 : Gauge = AWG24 : Num Turns = 50504");
  #endif
}

void loop() {
  /*
  Usual Task Progression:
  -Choose preset
  -Edit Values
  -Confirmation
  -Spin
  -End
  -Restart
  */
  switch (task) {
    case Tasks::ChoosePreset:
      #if DEBUG
        Serial.println("Current Task: choosePreset");
      #endif
      choosePreset();
      break;
    case Tasks::ValEdit:
      #if DEBUG
        Serial.println("Current Task: valSelect");
      #endif
      valSelect();
      break;
    case Tasks::ConfirmScreen:
      #if DEBUG
        Serial.println("Current Task: confirmScreen");
      #endif
      confirmScreen();
      break;
    case Tasks::Spin:
      #if DEBUG
        Serial.println("Current Task: spin");
      #endif
      spin();
      break;
    case Tasks::End:
      #if DEBUG
        Serial.println("Current Task: end");
      #endif
      completionScreen();
      break;
    default:
      task = Tasks::ChoosePreset;
  }
}


/*
Select preset screen
-Rotate Clockwise: Move Cursor Right
-Rotate Counterclockwise: Move Cursor Left
-Press: Select Preset
*/
void choosePreset() {
  uint8_t cursorIndex = 1;
  long reOldPosition = encoder.read() / 4;

  // Screen setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presets:");
  lcd.setCursor(0, 1);
  lcd.print(" A B C D None");
  lcd.setCursor(cursorIndex, 1);
  lcd.cursor();
  lcd.blink();

  // Selection loop
  while (true) {
    // Trigger selection on button press
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);

      #if DEBUG
        Serial.println("B!");
      #endif

      // Set preset
      switch (cursorIndex) {
        case 1:
          solenoid.setPreset(Preset::A);
          break;
        case 3:
          solenoid.setPreset(Preset::B);
          break;
        case 5:
          solenoid.setPreset(Preset::C);
          break;
        case 7:
          solenoid.setPreset(Preset::D);
          break;
        default:
          solenoid.setPreset(Preset::None);
      }
      
      // Set task to val editing
      task = Tasks::ValEdit;

      // Reset lcd
      lcd.noCursor();
      lcd.noBlink();

      return;
    }

    // Read encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && cursorIndex < 9) {
      cursorIndex += 2;
    } else if (dir < 0 && cursorIndex > 1) {
      cursorIndex -= 2;
    }
    reOldPosition = reNewPosition;

    // Update cursor
    lcd.setCursor(cursorIndex, 1);

    // Stability delay
    delay(1);
  }
}

/*
Value select Screen
-Rotate Clockwise: Move to next value
-Rotate Counterclockwise: Move to previous value
-Press: Begin editing value
*/
void valSelect() {
  // Local vars
  uint8_t screenIndex = 0;
  bool screenChange = true;
  long reOldPosition = encoder.read() / 4;

  // Setup screen
  lcd.clear();

  while (true) {
    if (screenChange) {
      lcd.clear();
      switch (screenIndex) {
        case 0: // Length (cm)
          lcd.setCursor(0, 0);
          lcd.print("Length (cm)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(solenoid.getLength(), MAX_LENGTH));
          break;
        case 1: // Radius (cm)
          lcd.setCursor(0, 0);
          lcd.print("Radius (cm)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(solenoid.getRadius(), MAX_RADIUS));
          break;
        case 2: // Inductance (mH)
          lcd.setCursor(0, 0);
          lcd.print("Inductance (mH)");
          lcd.setCursor(0, 1);
          lcd.print(formatVal(solenoid.getInductance(), MAX_INDUCTANCE));
          break;
        case 3:
          lcd.setCursor(0, 0);
          lcd.print("Wire Gauge");
          lcd.setCursor(0, 1);
          lcd.print(solenoid.gaugeString());
          break;
        case 4: // Confirmation Screen
          lcd.setCursor(0, 0);
          lcd.print("Turns");
          lcd.setCursor(0, 1);
          lcd.print(formatTurns(solenoid.getTurns()));
          if (solenoid.getOverride()) {
            lcd.print(" OVERRIDE");
          }
          break;
        case 5: // Confirmation Screen
          lcd.setCursor(0, 0);
          lcd.print("Turns: ");
          lcd.print(String(solenoid.getTurns()));
          lcd.setCursor(0, 1);
          lcd.print("Confirm?");
      }
      screenChange = false;
    }

    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      switch (screenIndex) {
        case 0: // Length
          solenoid.setLength(valEditor(solenoid.getLength(), MAX_LENGTH));
          break;
        case 1: // Radius
          solenoid.setRadius(valEditor(solenoid.getRadius(), MAX_RADIUS));
          break;
        case 2: // Inductance
          solenoid.setInductance(valEditor(solenoid.getInductance(), MAX_INDUCTANCE));
          break;
        case 3: // Gauge
          solenoid.setGauge(gaugeEditor(solenoid.getGauge()));
          break;
        case 4:
          solenoid.turnsOverride(turnsEditor(solenoid.getTurns()));
          break;
        case 5: // Turns
          task = Tasks::ConfirmScreen;
          return;
      }
      reOldPosition = encoder.read() / 4;
      screenChange = true;
    }

    // Read Encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && screenIndex < 5) {
      screenIndex += 1;
      screenChange = true;
    } else if (dir < 0 && screenIndex > 0) {
      screenIndex -= 1;
      screenChange = true;
    }
    reOldPosition = reNewPosition;

    delay(1);
  }
}

/*
Decimal value editing Screen
While not editing:
-Rotate Clockwise: Move cursor right
-Rotate Counterclockwise: Move cursor left
-Press: Begin editing digit or return if on 'Done'
While editing:
-Rotate Clockwise: Increase current digit by 1
-Rotate Counterclockwise: Decrease current digit by 1
-Press: Finish editing
*/
uint32_t valEditor(uint32_t num, uint32_t max) {
  uint32_t scaler = 1;
  uint8_t maxLength = String(max).length() + 1;
  uint8_t cursor_idx = maxLength - 1;
  long reOldPosition = encoder.read() / 4;
  bool editingDigit = false;
  bool screenChange = true;

  
  lcd.setCursor(11, 1);
  lcd.print("Done");
  lcd.setCursor(cursor_idx, 1);
  lcd.cursor_on();

  while (true) {
    
    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY * 2);
      if (cursor_idx == 11) {
        lcd.cursor_off();
        lcd.blink_off();
        return num;
      } else {
        editingDigit = !editingDigit;
        if (editingDigit) {
          lcd.blink_on();
        } else {
          lcd.blink_off();
        }
      }
    }

    // Read Encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (editingDigit) { // Editing digit
      if (dir > 0 && num + scaler <= max) {
        num += scaler;
        screenChange = true;
      } else if (dir < 0 && num - scaler >= 0) {
        num -= scaler;
        screenChange = true;
      }
    } else { // Selecting digit
      if (dir > 0) {
        if (cursor_idx < maxLength - 1) {
          cursor_idx++;
          scaler /= 10;
          // Skip .
          if (cursor_idx == maxLength - 3) {
            cursor_idx++;
          }
        } else if (cursor_idx == maxLength - 1) { // Jump to done
          cursor_idx = 11;
        }
        
        screenChange = true;
      } else if (dir < 0) {
        if (cursor_idx > 0 && cursor_idx < maxLength) {
          cursor_idx--;
          scaler *= 10;
          // Skip .
          if (cursor_idx == maxLength - 3) {
            cursor_idx--;
          }
        } else if (cursor_idx == 11) { // Jump from done
          cursor_idx = maxLength - 1;
        }
        
        screenChange = true;
      }
    }
    reOldPosition = reNewPosition;

    // Update screen
    if (screenChange) {
      lcd.setCursor(0, 1);
      lcd.print(formatVal(num, max));
      lcd.setCursor(cursor_idx, 1);
      screenChange = false;
    }
    
    // Stability delay
    delay(1);
  }
}

/*
Gauge editing Screen
While not editing:
-Rotate Clockwise: Move cursor right
-Rotate Counterclockwise: Move cursor left
-Press: Begin editing gauge or return if on 'Done'
While editing:
-Rotate Clockwise: Increase gauge by 1
-Rotate Counterclockwise: Decrease gauge by 1
-Press: Finish editing
*/
WireGauge gaugeEditor(WireGauge gauge) {
  uint8_t cursorIndex = 4;
  bool editingGauge = false;
  bool screenUpdate = true;
  long reOldPosition = encoder.read() / 4;

  lcd.setCursor(11, 1);
  lcd.print("Done");
  lcd.setCursor(cursorIndex, 1);
  lcd.cursor_on();

  while (true) {
    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY * 2);
      if (cursorIndex == 11) {
        lcd.cursor_off();
        lcd.blink_off();
        return gauge;
      } else {
        editingGauge = !editingGauge;
        if (editingGauge) {
          lcd.blink_on();
        } else {
          lcd.blink_off();
        }
      }
    }

    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (editingGauge) {
      if (dir > 0 && gauge < MAX_GAUGE) {
        gauge = static_cast<WireGauge>(gauge + 1);
        screenUpdate = true;
      } else if (dir < 0 && gauge > 0) {
        gauge = static_cast<WireGauge>(gauge - 1);
        screenUpdate = true;
      }
    } else {
      if (dir > 0 && cursorIndex == 4) {
        cursorIndex = 11;
        screenUpdate = true;
      } else if (dir < 0 && cursorIndex == 11) {
        cursorIndex = 4;
        screenUpdate = true;
      }
    }
    reOldPosition = reNewPosition;

    if (screenUpdate) {
      lcd.setCursor(0, 1);
      solenoid.setGauge(gauge);
      lcd.print(solenoid.gaugeString());
      lcd.setCursor(cursorIndex, 1);
      screenUpdate = false;
    }
  }
}

int32_t turnsEditor(uint32_t turns) {
  uint32_t scaler = 1;
  uint8_t maxLength = 5;
  uint8_t cursor_idx = maxLength - 1;
  uint32_t max = 10000;
  long reOldPosition = encoder.read() / 4;
  bool editingDigit = false;
  bool screenChange = true;

  
  lcd.setCursor(6, 1);
  lcd.print("Done Reset");
  lcd.setCursor(cursor_idx, 1);
  lcd.cursor_on();

  while (true) {
    
    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      if (cursor_idx == 6) {
        lcd.cursor_off();
        lcd.blink_off();
        return turns;
      } else if (cursor_idx == 11) {
        lcd.cursor_off();
        lcd.blink_off();
        return -1;
      } else {
        editingDigit = !editingDigit;
        if (editingDigit) {
          lcd.blink_on();
        } else {
          lcd.blink_off();
        }
        delay(BUTTON_DELAY);
      }
    }

    // Read Encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (editingDigit) { // Editing digit
      if (dir > 0 && turns + scaler <= max) {
        turns += scaler;
        screenChange = true;
      } else if (dir < 0 && turns - scaler >= 0) {
        turns -= scaler;
        screenChange = true;
      }
    } else { // Selecting digit
      if (dir > 0) {
        if (cursor_idx < maxLength - 1) {
          cursor_idx++;
          scaler /= 10;
        } else if (cursor_idx == maxLength - 1) { // Jump to done
          cursor_idx = 6;
        } else if (cursor_idx == 6) { // Jump to reset
          cursor_idx = 11;
        }
        
        screenChange = true;
      } else if (dir < 0) {
        if (cursor_idx > 0 && cursor_idx < maxLength) {
          cursor_idx--;
          scaler *= 10;
        } else if (cursor_idx == 6) { // Jump from done
          cursor_idx = maxLength - 1;
        } else if (cursor_idx == 11) { // Jump from reset
          cursor_idx = 6;
        }
        
        screenChange = true;
      }
    }
    reOldPosition = reNewPosition;

    // Update screen
    if (screenChange) {
      lcd.setCursor(0, 1);
      lcd.print(formatTurns(turns));
      lcd.setCursor(cursor_idx, 1);
      screenChange = false;
    }
    
    // Stability delay
    delay(1);
  }
}

/*
Confirmation screen
-Rotate Clockwise: Move Cursor Right
-Rotate Counterclockwise: Move Cursor Left
-Press: Confirm Y/N
*/
void confirmScreen() {
  uint8_t cursor_idx = 0;
  long reOldPosition = encoder.read() / 4;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Begin Process?");
  lcd.setCursor(0, 1);
  lcd.print("Y/N");
  lcd.setCursor(cursor_idx, 1);
  lcd.cursor_on();
  lcd.blink_on();

  while (true) {
    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      lcd.blink_off();
      lcd.cursor_off();

      if (cursor_idx == 0) {
        task = Tasks::Spin;
        return;
      } else {
        task = Tasks::ValEdit;
        return;
      }
    }

    // Read encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && cursor_idx == 0) {
      cursor_idx = 2;
    } else if (dir < 0 && cursor_idx == 2) {
      cursor_idx = 0;
    }
    reOldPosition = reNewPosition;

    lcd.setCursor(cursor_idx, 1);

    // Stability delay
    delay(1);
  } 
}

/*
Major spin task
-Press: Pauses
*/
void spin() {
  // Start by zeroing carriage
  zeroCarriage();
  
  // Calculate necessary values
  const uint32_t SS_STEPS = solenoid.getTurns() * SS_STEPS_PER_REVOLUTION;
  const uint32_t CC_DISTANCE_PER_REVOLUTION = (solenoid.gaugeDiameter() * 100) / DISTANCE_PER_STEP;
  const uint32_t SS_STEP_PER_CC_STEP = (CC_STEPS_PER_REVOLUTION * 1000) / CC_DISTANCE_PER_REVOLUTION;

  uint32_t stepCount = 0; // Step count
  uint32_t subStepCount = 0; // Specifically to time carriage steps to avoid costly mod ops
  int32_t carriagePosition = 0; // Should be zero after zeroing; 0.001 cm accuracy
  bool direction = true; // True = forward
  uint8_t oldPercentComplete = 0;

  // Wake CC Motor
  digitalWrite(CC_SLEEP_PIN, HIGH);
  delay(20);

  // Set starting directions
  digitalWrite(CC_DIR_PIN, CC_DIR_SET);

  // Apply starting offset
  while (carriagePosition < CARRIAGE_OFFSET + PADDING) {
    // Check for motor fault
    if (digitalRead(CC_FAULT_PIN) == LOW) {
      motorFault("CC");
    }

    stepCC();
    carriagePosition += DISTANCE_PER_STEP;
  }
  // Set new offset position as 0 position
  carriagePosition = 0;

  // Wake SS motor
  digitalWrite(SS_SLEEP_PIN, HIGH);
  delay(20);

  // Set starting dir
  digitalWrite(SS_DIR_PIN, SS_DIR_SET);

  // Setup Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Percent Complete");
  lcd.setCursor(0, 1);
  lcd.print(String(oldPercentComplete) + "%");

  #if DEBUG
    long startTime = micros();
  #endif
  while (stepCount < SS_STEPS) {
    // Check for motor faults
    if (digitalRead(CC_FAULT_PIN) == LOW) {
      motorFault("CC");
    }
    if (digitalRead(SS_FAULT_PIN) == LOW) {
      motorFault("SS");
    }

    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);

      if (pauseSpin()) {
        return;
      }

      // Reset display after pause
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Percent Complete");
      lcd.setCursor(0, 1);
      lcd.print(String(oldPercentComplete) + "%");
    }

    // Reverse carriage
    if (carriagePosition > int(solenoid.getLength()) * 10 + PADDING) {
      digitalWrite(CC_DIR_PIN, !CC_DIR_SET);
      direction = false;
    }
    if (carriagePosition < PADDING) {
      digitalWrite(CC_DIR_PIN, CC_DIR_SET);
      direction = true;
    }

    // Step motor(s)
    if (subStepCount == SS_STEP_PER_CC_STEP) {
      stepBoth();
      carriagePosition += (direction ? DISTANCE_PER_STEP : -DISTANCE_PER_STEP);
      subStepCount = 0;
    } else {
      stepSS();
    }

    // Update counts
    subStepCount++;
    stepCount++;

    // Update % completion
    
    uint8_t newPercentComplete = (stepCount * 100) / SS_STEPS;
    if (newPercentComplete != oldPercentComplete) {
      lcd.setCursor(0, 1);
      lcd.print(String(newPercentComplete) + "%");
      oldPercentComplete = newPercentComplete;
    }
    

    #if DEBUG
      long endTime = micros();
      Serial.println("Loop Time: " + String(endTime - startTime) + "ms");
      startTime = endTime;
    #endif
  }

  task = Tasks::End;
  return;
}

// Moves carriage towards 0 position till the start limit switch is hit
void zeroCarriage() {
  // Setup Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Zeroing...");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");

  // Wake Motor
  digitalWrite(CC_SLEEP_PIN, HIGH);
  delay(20);

  // Set direction
  digitalWrite(CC_DIR_PIN, !CC_DIR_SET);

  while (true) {
    // Check for fault
    if (digitalRead(CC_FAULT_PIN) == LOW) {
      motorFault("CC");
    }

    // Usual behavior is to check start limit switch, but allow manual zero as well for debugging
    if (digitalRead(LS_START_PIN) == HIGH || digitalRead(RE_BUTTON_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Zeroing Complete");
      delay(BUTTON_DELAY);

      return;
    } else {
      stepCC();
    }
  }
}

/*
Pause screen
-Rotate Clockwise: Move Cursor Right
-Rotate Counterclockwise: Move Cursor Left
-Press: Confirm Resume/Restart
*/
bool pauseSpin() {
  uint8_t cursorIndex = 0;
  long reOldPosition = encoder.read() / 4;

  // Sleep Motors
  digitalWrite(CC_SLEEP_PIN, LOW);
  digitalWrite(SS_SLEEP_PIN, LOW);

  // Setup Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Paused");
  lcd.setCursor(0, 1);
  lcd.print("Resume  Restart");
  lcd.setCursor(cursorIndex, 1);
  lcd.cursor_on();
  lcd.blink_on();

  while (true) {
    // Read Button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      lcd.blink_off();
      lcd.cursor_off();
      if (cursorIndex == 0) {
        // Wake motors and return
        digitalWrite(CC_SLEEP_PIN, HIGH);
        digitalWrite(SS_SLEEP_PIN, HIGH);
        delay(100);
        return false;
      } else {
        // Return to value editor
        task = Tasks::ValEdit;
        return true;
      }
    }

    // Read encoder
    long reNewPosition = encoder.read() / 4;
    int16_t dir = reNewPosition - reOldPosition;
    if (dir > 0 && cursorIndex != 8) {
      cursorIndex = 8;
    } else if (dir < 0 && cursorIndex != 0) {
      cursorIndex = 0;
    }
    lcd.setCursor(cursorIndex, 1);
    reOldPosition = reNewPosition;
    Serial.println(cursorIndex);

    // Stability delay
    delay(1);
  }
}

/*
Motor fault screen
Unresolvable error - requires restart
*/
void motorFault(String motorName) {
  #if DEBUG
    Serial.println("Motor fault on motor: " + motorName);
  #endif

  // Sleep both motors
  digitalWrite(SS_SLEEP_PIN, LOW);
  digitalWrite(CC_SLEEP_PIN, LOW);

  // Error message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!!MOTOR  FAULT!!");
  lcd.setCursor(0, 1);
  lcd.print(motorName + " Motor");
  
  // Infinite loop till restart
  while (true) {delay(1);}
}

// Step carriage control motor one step
void stepCC() {
  digitalWrite(CC_STEP_PIN, HIGH);
  delayMicroseconds(MOTOR_DELAY);
  digitalWrite(CC_STEP_PIN, LOW);
  delayMicroseconds(MOTOR_DELAY);
}

// Step solenoid spin motor one step
void stepSS() {
  digitalWrite(SS_STEP_PIN, HIGH);
  delayMicroseconds(MOTOR_DELAY);
  digitalWrite(SS_STEP_PIN, LOW);
  delayMicroseconds(MOTOR_DELAY);
}

// Combined step function to eliminate out of sync steps and stuttering
void stepBoth() {
  digitalWrite(CC_STEP_PIN, HIGH);
  digitalWrite(SS_STEP_PIN, HIGH);
  delayMicroseconds(MOTOR_DELAY);
  digitalWrite(CC_STEP_PIN, LOW);
  digitalWrite(SS_STEP_PIN, LOW);
  delayMicroseconds(MOTOR_DELAY);
}

void completionScreen() {
  // Setup Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Completed!");
  lcd.setCursor(0, 1);
  lcd.print("Press to restart");

  while (true) {
    // Read button
    if (digitalRead(RE_BUTTON_PIN) == LOW) {
      delay(BUTTON_DELAY);
      
      task = Tasks::ValEdit;
      return;
    }

    delay(1);
  }
}

/*
  Simple animation to play at startup
  Total delay: 1600ms
*/
void startupAnimation() {
  String s = "Robojackets!";
  lcd.clear();
  lcd.setCursor(2, 0);
  for (size_t i = 0; i < s.length(); i++) {
    lcd.print(s.charAt(i));
    delay(100);
  }
  lcd.setCursor(5, 1);
  lcd.print(VERSION);
  delay(500);
}

// Assumes 2 decimal place precision
String formatVal(uint32_t num, uint32_t max) {
  uint8_t maxLength = String(max).length() + 1; // Cannot be greater than 10
  String returnString = "";
  String numberString = String(num);
  
  // Add leading zeros to match length
  for (size_t i = 0; i < maxLength - numberString.length() - 1; i++) {
    if (int(i) == maxLength - 3) {
        returnString += ".";
    } else {
        returnString += "0";
    }
  }
  
  // Add actual value
  // Short value case
  if (numberString.length() < 3) {
    returnString += "." + numberString;
    return returnString;
  }

  // Split case
  returnString += numberString.substring(0, numberString.length() - 2);
  returnString += ".";
  returnString += numberString.substring(numberString.length() - 2);
  return returnString;
}

String formatTurns(uint32_t turns) {
  String returnString = "";
  String numberString = String(turns);

  for (size_t i = 0; i < 5 - numberString.length(); i++) {
    returnString += "0";
  }
  returnString += numberString;
  return returnString;
}