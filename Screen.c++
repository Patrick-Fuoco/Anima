// ANIMA SCREEN CODE

/*
  16x9 LCD menu navigation using a 2-axis joystick.
  
  Circuit:
  * LCD RS pin to digital pin 6
  * LCD Enable pin to digital pin 7
  * LCD D4 pin to digital pin 5
  * LCD D5 pin to digital pin 4
  * LCD D6 pin to digital pin 3
  * LCD D7 pin to digital pin 2
  * LCD R/W pin to ground
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K resistor:
      * ends to +5V and ground
      * wiper to LCD VO pin (pin 3)

  Joystick ---> Arduino:
  * GND          GND
  * +5V          5V
  * VRx          A1
  * VRy          A0 
  * SW           9
*/

// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Define constants
#define lcd_rows 2
#define lcd_columns 16
#define i2c_addr  0x27

// LCD object for a 16x2 LCD display
LiquidCrystal_I2C lcd(i2c_addr, lcd_columns, lcd_rows);

// Arduino pins attached to joystick 
#define joystick_switch_pin  9
#define joystick_y_pin       A0

// Joystick direction definitions
#define up    0
#define down  2
#define enter 4
#define none  5

// Define menu items
String main_menu[] = {"1. Garlic", "2. Hummus", "3. Buffalo"};
String quantities[] = {"25", "50", "75", "100", "Main Menu"};
String options[] = {"Yes", "Cancel"};

// Declare global variables to track menu state
int current_menu_item = 0;
int current_quantity = 0;
int current_option = 0;
int last_joy_read = none;

// Function to read joystick input and return its state
int read_joystick() {
  int output = none;
  
  // Read the joystick's Y axis
  int Y_Axis = analogRead(joystick_y_pin);     
  Y_Axis = map(Y_Axis, 0, 1023, 1023, 0);      // Invert Y axis values
  
  // Read the joystick's switch
  int SwitchValue = digitalRead(joystick_switch_pin);  
  SwitchValue = map(SwitchValue, 0, 1, 1, 0);  // Invert switch state
  
  // Determine joystick state based on readings
  if (SwitchValue == 1){
    output = enter;
  } else if (Y_Axis >= 900) {
    output = up;
  } else if (Y_Axis <= 100) {
    output = down;
  }

  return output;
}

// Clear a specific line on the LCD
void clear_line(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");  // Print 16 blank spaces to clear the line
}

// Print text to a specific line on the LCD
void print_line(int line, String text) {
  lcd.setCursor(0, line);
  clear_line(line);  // Clear the line before printing
  lcd.setCursor(0, line);
  lcd.print(text);
}

// Navigation through main menu
void move_upM() {
  current_menu_item = (current_menu_item == 0) ? 2 : current_menu_item - 1;
}

void move_downM() {
  current_menu_item = (current_menu_item == 2) ? 0 : current_menu_item + 1;
}

// Navigation through quantities menu
void move_upQ() {
  current_quantity = (current_quantity == 0) ? 4 : current_quantity - 1;
}

void move_downQ() {
  current_quantity = (current_quantity == 4) ? 0 : current_quantity + 1;
}

// Navigation through options menu
void move_upO() {
  current_option = (current_option == 0) ? 1 : current_option - 1;
}

void move_downO() {
  current_option = (current_option == 1) ? 0 : current_option + 1;
}

// Handle the quantities selection
void handle_quantities() {
  bool selecting = true;

  while (selecting) {
    lcd.setCursor(0, 0);
    lcd.print("Quantities:  ");
    lcd.setCursor(0, 1);
    lcd.print(quantities[current_quantity]);

    int current_joy_read = read_joystick();

    if (current_joy_read != last_joy_read) {
      last_joy_read = current_joy_read;

      switch (current_joy_read) {
        case up:
          move_upQ();
          break;
        case down:
          move_downQ();
          break;
        case enter:
          if (current_quantity == 4) {
            print_line(0, "Main Menu");
            selecting = false;
          } else {
            handle_options();
          }
          break;
        default:
          break;
      }

      print_line(1, quantities[current_quantity]);
      delay(100);
    }
  }
}

// Handle the options menu
void handle_options() {
  bool selecting = true;

  while (selecting) {
    lcd.setCursor(0, 0);
    lcd.print("Are you sure?");
    lcd.setCursor(0, 1);
    lcd.print(options[current_option]);

    int current_joy_read = read_joystick();

    if (current_joy_read != last_joy_read) {
      last_joy_read = current_joy_read;

      switch (current_joy_read) {
        case up:
          move_upO();
          break;
        case down:
          move_downO();
          break;
        case enter:
          if (current_option == 1) {  // Cancel option
            selecting = false;
          } else {
            initialize_action();
          }
          break;
        default:
          break;
      }

      print_line(1, options[current_option]);
      delay(100);
    }
  }
}

// Placeholder function to initialize action
void initialize_action() {
  print_line(0, "Initializing...");
  delay(2000);
}

// Arduino setup function
void setup() {
  lcd.init();
  lcd.begin(16, 2);  // Initialize 16x2 LCD
  delay(1000);
  lcd.backlight();    // Turn on LCD backlight

  // Print initial menu
  print_line(0, "Main Menu");
  print_line(1, "1. Garlic");

  // Set up joystick pin
  pinMode(joystick_switch_pin, INPUT_PULLUP);
}

// Arduino loop function
void loop() {
  int current_joy_read = read_joystick();

  if (current_joy_read != last_joy_read) {
    last_joy_read = current_joy_read;

    switch (current_joy_read) {
      case up:
        move_upM();
        break;
      case down:
        move_downM();
        break;
      case enter:
        handle_quantities();
        break;
      default:
        break;
    }

    // Print the updated menu
    print_line(0, "Main Menu");
    print_line(1, main_menu[current_menu_item]);
    delay(100);
  }
}
