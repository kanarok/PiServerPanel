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
#define HEARTBEAT_TIMEOUT       10000
#define HEARTBEAT_SMALL_TIMEOUT  3333

#define RUN           8

byte button = 0;
byte pushlock = UNDEFINED;

int c_r = 0;
int c_g = 0;
int c_b = 0;
int fade_r = 7;
int fade_g = 3;
int fade_b = 15;
int fade_delay = 5;
int fdel = 1;
int cmode = 0;

int incomingByte = 0;
unsigned long tracker, tracker2, beat;

typedef void *(*StateFunc)();

//States
void *server_bootable();
void *server_start();
void *server_booting();
void *server_running();
void *server_shutdown();
void *server_hungup();
void *server_reset();
void *server_locked();

StateFunc statefunc;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(PUSHLOCK_NO, INPUT_PULLUP);
  pinMode(PUSHLOCK_NC, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(RUN, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON), button_push, FALLING);
  attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NO), pushlock_change, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NC), pushlock_open, CHANGE);

  pushlock = digitalRead(PUSHLOCK_NO);
  
//  if (pushlock == LOCKED) {
//    statefunc = server_locked;
//  } else {
    statefunc = server_booting;
//  }
  tracker = millis();
  tracker2 = tracker;

  Serial.begin(115200);
}

void loop() {
  statefunc = (StateFunc)(*statefunc)();
}

void touch_run_pins() {
  digitalWrite(RUN, HIGH);
  delay(50);
  digitalWrite(RUN, LOW);
}

void fade_bootable(byte del) {
  analogWrite(LED_R, 255);
  analogWrite(LED_G, 255-c_g);
  analogWrite(LED_B, 255);

  c_g = c_g + fade_g;
  
  if ( c_g <= 0 || c_g >= 87 ) {
    fade_g = -fade_g;
  }

  if (fade_g > 0) {
   delay(del/2); 
  } else {
   delay(del); 
  }
}

void fade_shutdown(byte del) {
  analogWrite(LED_R, 255-c_r);
  analogWrite(LED_G, 255-c_r);
  analogWrite(LED_B, 255);

  c_r = c_r + fade_b;
  
  if ( c_r <= 0 || c_r >= 140 ) {
    fade_b = -fade_b;
  }

  if (fade_b > 0) {
   delay(del); 
  } else {
   delay(del/3); 
  }
}

void fade_hungup(byte del) {
  analogWrite(LED_R, 255-c_r);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 255);

  c_r = c_r + fade_r;
  
  if ( c_r <= 0 || c_r >= 240 ) {
    fade_r = -fade_r;
  }
  
  delay(del);
}

void show_red() {
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 255);
}

void show_color(byte r, byte g, byte b) {
  analogWrite(LED_R, 255-r);
  analogWrite(LED_G, 255-g);
  analogWrite(LED_B, 255-b);
}

void show_none() {
  show_color(0,0,0);
}

void show_heartbeat() {
  show_color(0,255,0);
  delay(10);
  show_color(255,0,0);
  delay(10);
  show_color(0,0,0);
  delay(15);
}

void show_missing_heartbeat() {
  show_color(255,0,0);
  delay(10);
  show_color(255,0,0);
  delay(10);
  show_color(0,0,0);
  delay(15);
}

void fade_boot(byte del) {
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

long heartbeat() {
  int last_heartbeat = millis()-tracker;

//  if (!Serial) {
//    return -1;
//  }

  if (((millis()-tracker2) > 3000) && last_heartbeat > 3000) {
    show_missing_heartbeat();
    tracker2 = millis();
  }
  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    
    serial_flush();
    
    if (incomingByte == 0x42) {
      tracker = millis();
      tracker2 = tracker;
      show_heartbeat();
      return last_heartbeat;
    } else {
      return -1;
    }
  } else {
    return 0;
  }
}

long last_heartbeat() {
  return millis()-tracker;
}

void *server_bootable() {
  if (pushlock == LOCKED) {
    show_red();
    delay(80);
    return server_locked;
  } else {
    fade_bootable(45);
    
    if ((last_heartbeat() < HEARTBEAT_TIMEOUT || heartbeat() > 0 )) { // && Serial
      serial_flush();
      return server_booting;
    }
    
    if (digitalRead(BUTTON)) {
      delay(100);
      tracker = millis();
      serial_flush();
      return server_start;
    } else {
      serial_flush();
      return server_bootable;
    }
  }
}

void *server_start() {
//  if (Serial) {
//    Serial.println("touching run pins on pi");
//  }
  touch_run_pins();
  serial_flush();
  return server_booting;
}

void *server_booting() {
//  if (Serial) {
//    Serial.println("Waiting for Heartbeat");
//    Serial.println("?");
//  }
  if (last_heartbeat() > 10000) {
    
    return server_bootable;
  }
  
  if (heartbeat() <= 0) {
    fade_boot(40);
    
    return server_booting;
  } else {
    
    return server_running;
  }
}

void *server_running() {
  show_none();

  beat = heartbeat();
//  if (Serial) {
//    Serial.println("running");
//    Serial.println(last_heartbeat());
//  }

  if (pushlock == LOCKED) {
    if (last_heartbeat() > 30000 || heartbeat() < 0 ) {
      
      return server_locked;
    } else {
      
      return server_shutdown;
    }
  } else {
    if (last_heartbeat() > HEARTBEAT_TIMEOUT) {
      
      return server_hungup;
    } else {
      
      return server_running;
    }
  }  

}

void *server_shutdown() {
  if (Serial) {
    Serial.println("shutdown");
  } else {
    show_missing_heartbeat();
    delay(40);
    show_missing_heartbeat();
    delay(40);
    show_missing_heartbeat();
    delay(40);
    show_missing_heartbeat();
    delay(40);
    show_missing_heartbeat();
    delay(40);
  }
  
  beat = heartbeat();
  fade_shutdown(35);
  
  if (last_heartbeat() < HEARTBEAT_TIMEOUT) {
    serial_flush();
    return server_shutdown;
  } else {  
    serial_flush();
    return server_locked;
  }
}

void *server_hungup() {
//  if (Serial) {
//    Serial.println("hung up");
//    Serial.println(last_heartbeat());
//  }
  
  beat = heartbeat();
  beat = last_heartbeat();
  
  if (beat < HEARTBEAT_TIMEOUT || heartbeat() > 0) {
    
    return server_running;
  } else {
    
    if (beat > HEARTBEAT_TIMEOUT && beat < 20000UL) {
      fade_hungup(25);
    } else if (beat > 20000UL && beat < 30000UL) {
      fade_hungup(15);
    } else if (beat > 30000UL && beat < 450000UL) {
      fade_hungup(5);
    } 
    
    if (beat > 45000UL) {
      show_red();
    }
    
    if (last_heartbeat() > 60000UL) {
      
      return server_reset;
    } else {
      
      return server_hungup;
    }
    
  }
}

void *server_reset() {
//  if (Serial) {
//    Serial.println("touching run pins on pi");
//  }
  touch_run_pins();
  serial_flush();
  return server_booting;
}

void *server_locked() {
  if (pushlock == LOCKED) {
    show_none();
    serial_flush();
    return server_locked;
  } else {
    serial_flush();
    return server_bootable;
  }
}
