#include <TimerOne.h>

#define LED_R         6
#define LED_G         9
#define LED_B        10
#define BUTTON        2
#define PUSHLOCK_NO   7
#define PUSHLOCK_NC   3

#define LOCKED      LOW
#define UNLOCKED   HIGH
#define UNDEFINED     2

byte button = 0;
byte pushlock = UNDEFINED;

int c_r = 0;
int c_g = 0;
int c_b = 0;
int fade_r = 5;
int fade_g = 15;
int fade_b = 10;
int fade_delay = 5;
int fdel = 1;
int cmode = 0;

int incomingByte = 0;
unsigned long tracker;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(PUSHLOCK_NO, INPUT_PULLUP);
  pinMode(PUSHLOCK_NC, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON), button_push, FALLING);
  attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NO), pushlock_change, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NC), pushlock_open, CHANGE);

  pushlock = digitalRead(PUSHLOCK_NO);
  tracker = millis();

  Serial.begin(115200);
}

void loop() {
  if (pushlock == LOCKED) {
    //Serial.println("LOCKED");
    if (heartbeat()) {
      
    }
  } else {
    //Serial.println("UNLOCKED");
    if (Serial.available() > 0) {
      incomingByte = Serial.read();
      Serial.println(incomingByte, HEX);
      Serial.println("HEARTBEAT");
      serial_flush();
      delay(1000);
    }
  }
  if (digitalRead(BUTTON)) {
  //if (button != 0) {
    button = 0;
    delay(100);
    cmode++;
  } else {
    Serial.println("Colormode");
    show_btn_color(cmode);
  }
  //button = 0;
}

void fade_btn(byte del) {
  analogWrite(LED_R, 255-c_r);
  analogWrite(LED_G, 255-c_g);
  analogWrite(LED_B, 255-c_b);

  //analogWrite(LED_G, 255);
  //analogWrite(LED_B, 255);

  c_r = c_r + fade_r;
  c_g = c_g + fade_g;
  c_b = c_b + fade_b;

  if ( c_r <= 0 || c_r >= 200 ) {
    fade_r = -fade_r;
  }
  if ( c_g <= 0 || c_g >= 255 ) {
    fade_g = -fade_g;
  }
  if ( c_b <= 0 || c_b >= 155 ) {
    fade_b = -fade_b;
  }

  fade_delay = fade_delay + fdel;
  if (fade_delay <= 1 || fade_delay >=del) {
    fdel = -fdel;
  }
  delay(fade_delay);
}

void show_btn_color( int mode ) {
  switch (mode) {
    case 1:
            fade_btn(10);
            
            break;
    case 2:
            fade_btn(5);
            break;        
    case 3:
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 255);
            analogWrite(LED_B, 0);
            
            break;
    case 4:
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 0);
            analogWrite(LED_B, 255);
            
            break;
    case 5:
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 255);
            analogWrite(LED_B, 255);
           
            break;
    case 6:
            analogWrite(LED_R, 255);
            analogWrite(LED_G, 255);
            analogWrite(LED_B, 0);
           
            break;
    case 7:
            fade_btn(1);
            break;
    case 8:
            c_r = 255;
            c_g = 0;
            fade_btn(10);
            break;
    default:
            cmode = 1;
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 0);
            analogWrite(LED_B, 0);
            delay(50);
            analogWrite(LED_R, 200);
            analogWrite(LED_G, 200);
            analogWrite(LED_B, 200);
            delay(50);
            analogWrite(LED_R, 0);
            analogWrite(LED_G, 0);
            analogWrite(LED_B, 0);
            break;
  }
}

void button_push() {
  button = 1;
  //delay(50);
}

void pushlock_change() {
  pushlock = digitalRead(PUSHLOCK_NO);
}

void serial_flush() {
  int buf;
  while (Serial.available() > 0) {
      buf = Serial.read();
  }
}

int heartbeat() {
  int last_heartbeat;
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    
    serial_flush();
    
    if (incomingByte == 0x42) {
      last_heartbeat = millis()-tracker;
      tracker = millis();

      return last_heartbeat;
    } else {
      return -1;
    }
}

