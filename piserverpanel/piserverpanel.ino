#include <TimerOne.h>

#define LED_R                       6
#define LED_G                       9
#define LED_B                      10

#define BUTTON                      2
#define PUSHLOCK_NO                 7
#define PUSHLOCK_NC                 3

#define LOCKED                    LOW
#define UNLOCKED                 HIGH
#define UNDEFINED                   2

#define HEARTBEAT_TIMEOUT       10000
#define HEARTBEAT_SMALL_TIMEOUT  3333

#define RUN                         8

#define BAUDRATE               115200

//isr
void isr_button_push();       //obsolete?
void isr_pushlock_change();

//physical routines
void serial_flush();
void touch_run_pin();

//heartbeat detection
long detect_heartbeat_any();
long detect_heartbeat();
long detect_heartbeat_ack();
long detect_heartbeat_abort();
long get_last_heartbeat();

//heartbeat animations
void led_heartbeat();
void led_heartbeat_missing();
void led_heartbeat_ack();
void led_heartbeat_abort();

//generic led routines
void led_color(byte r, byte g, byte b);
void led_off();
void led_red();
void led_green();
void led_blue();

//States                          //state matching led animations
void *server_bootable();          void animate_bootable();
void *server_booting();           void animate_booting();
void *server_running();           void animate_running();
void *server_shutdown();          void animate_shutdown();
void *server_shutdown_active();   void animate_shutdown_active();
void *server_hungup();            void animate_hungup(byte del);
void *server_reset();             void animate_reset();
void *server_locked();            void animate_locked();

//FSM
typedef void *(*StateFunc)();
StateFunc statefunc = server_bootable();

//++++ cleanup due!

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
unsigned long heartbeat, track_last_heartbeat, track_missing_heartbeat, track_uart;
int show_missing_beat = 1;

void setup() {
  pinMode(PUSHLOCK_NO, INPUT_PULLUP);
  pinMode(PUSHLOCK_NC, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(RUN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON), isr_button_push, FALLING);
  attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NO), isr_pushlock_change, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PUSHLOCK_NC), pushlock_open, CHANGE);

  pushlock = digitalRead(PUSHLOCK_NO);

  track_last_heartbeat = millis();
  track_missing_heartbeat = track_last_heartbeat;

  Serial.begin(BAUDRATE);
}

void loop() {
  heartbeat = detect_heartbeat();
  led_heartbeat_ack();
  statefunc = (StateFunc)(*statefunc)();    //FSM
}

//isr
void isr_button_push() {
  button = 1;
}

void isr_pushlock_change() {
  pushlock = digitalRead(PUSHLOCK_NO);
}

//physical routines
void serial_flush() {
  int buf;
  while (Serial.available() > 0) {
      buf = Serial.read();
  }
}

void touch_run_pin() {
  digitalWrite(RUN, HIGH);
  delay(50);
  digitalWrite(RUN, LOW);
}

//generic led routines
void led_color(byte r, byte g, byte b) {
  analogWrite(LED_R, 255-r);
  analogWrite(LED_G, 255-g);
  analogWrite(LED_B, 255-b);
}

void led_off() {
  led_color(0,0,0);
}

void led_red() {
  led_color(255, 0, 0);
}

void led_green() {
  led_color(0, 255, 0);
}

void led_blue() {
  led_color(0, 0, 255);
}

//heartbeat animations
void led_heartbeat() {
  led_green();
  delay(10);
  led_red();
  delay(10);
  led_off();
  delay(15);
}

void led_heartbeat_missing() {
  led_red();
  delay(10);
  led_red();
  delay(10);
  led_off();
  delay(15);
}

void led_heartbeat_ack() {
  led_color(255,0,255);
  delay(10);
  led_color(255,0,255);
  delay(10);
  led_color(0,0,0);
  delay(15);
}

void led_heartbeat_abort() {
  led_color(0,0,255);
  delay(10);
  led_color(255,255,255);
  delay(10);
  led_color(0,0,0);
  delay(15);
}

//heartbeat detection
long detect_heartbeat_any();

long detect_heartbeat() {
  int last_heartbeat = millis()-track_last_heartbeat;

//  if (!Serial) {
//    return -1;
//  }

  if (((millis()-track_missing_heartbeat) > 3000) && last_heartbeat > 3000) {
    if (show_missing_beat == 1) {
      led_heartbeat_missing();
    }
    track_missing_heartbeat = millis();
  }

  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    serial_flush();

    if (incomingByte == 0x42) {
      track_last_heartbeat = millis();
      track_missing_heartbeat = track_last_heartbeat;
      led_heartbeat();
      return last_heartbeat;
    } else {
      return -1;
    }
  } else {
    return 0;
  }
}

long detect_heartbeat_ack() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    serial_flush();

    if (incomingByte == 0x108) {
      Serial.println("ok");
      led_heartbeat_ack();
      return 1;
    } else {
      return 0;
    }
  } else {
    return -1;
  }
}

long detect_heartbeat_abort() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    serial_flush();

    if (incomingByte == 0x109) {
      Serial.println("ok");
      led_heartbeat_abort();
      return 1;
    } else {
      return 0;
    }
  } else {
    return -1;
  }
}

long get_last_heartbeat() {
  return millis()-track_last_heartbeat;
}

//state matching led animations

void animate_bootable() {
  int del = 45;
  analogWrite(LED_R, 255);
  analogWrite(LED_G, 255-c_g);
  analogWrite(LED_B, 255);

  c_g = c_g - fade_g;

  if ( c_g > 88 ) {
    c_g = 87;
  }

  if ( c_g <= 0 || c_g >= 87 ) {
    fade_g = -fade_g;
  }

  if (fade_g > 0) {
   delay(del/2);
  } else {
   delay(del);
  }
}

void animate_booting() {
  int del = 20;
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

void animate_running();

void animate_shutdown() {
  int del = 20;
  analogWrite(LED_R, 255-c_r);
  analogWrite(LED_G, 255-c_r);
  analogWrite(LED_B, 255);

  c_r = c_r + fade_b;

  if (c_r > 141) {
    c_r = 140;
  }

  if ( c_r <= 0 || c_r >= 140 ) {
    fade_b = -fade_b;
  }

  if (fade_b > 0) {
   delay(del);
  } else {
   delay(del/3);
  }
}

void animate_shutdown_active() {

}

void animate_hungup(byte del) {
  analogWrite(LED_R, 255-c_r);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 255);

  c_r = c_r + fade_r;

  if ( c_r <= 0 || c_r >= 255 ) {
    fade_r = -fade_r;
  }

  delay(del);
}

void animate_reset() {

}

void animate_locked() {

}

//states

void *server_bootable() {
  if (Serial) {
    Serial.println("bootable");
  }

  if (pushlock == LOCKED) {
    led_red();
    delay(80);
    return server_locked;
  } else {
    animate_bootable();

    if ((get_last_heartbeat() < HEARTBEAT_TIMEOUT || detect_heartbeat() > 0 )) { // && Serial
      serial_flush();
      return server_booting;
    }

    if (digitalRead(BUTTON)) {
      delay(100);
      track_last_heartbeat = millis();
      serial_flush();
      return server_reset;
    } else {
      serial_flush();
      return server_bootable;
    }
  }
}

void *server_booting() {
  if (Serial) {
    Serial.println("?");
  }

  if (get_last_heartbeat() > 10000) {

    return server_bootable;
  }

  if (detect_heartbeat() <= 0) {
    animate_booting();

    return server_booting;
  } else {

    return server_running;
  }
}

void *server_running() {
  led_off();

  heartbeat = detect_heartbeat();

  if (Serial) {
    Serial.println("running");
    Serial.println(get_last_heartbeat());
  }

  if (pushlock == LOCKED) {
    if (get_last_heartbeat() > 30000 || detect_heartbeat() < 0 ) {

      return server_locked;
    } else {
      track_uart = millis();
      return server_shutdown;
    }
  } else {
    if (get_last_heartbeat() > HEARTBEAT_TIMEOUT) {

      return server_hungup;
    } else {

      return server_running;
    }
  }

}

void *server_shutdown() {

  if ((millis()-track_uart) > 5000) {
    if (Serial) {
      Serial.println("shutdown");
      delay(100);
    }
    track_uart = millis();
  }

  //fade_shutdown(35);
  led_red();

  if (detect_heartbeat_ack() == 1) {
    return server_shutdown_active;
  } else {
    return server_shutdown;
  }
}

void *server_shutdown_active() {
  heartbeat = detect_heartbeat();
  led_heartbeat_missing();
  delay(100);
  led_red();

  if (digitalRead(BUTTON)) {
    delay(100);
    track_last_heartbeat = millis();
    serial_flush();
    if (detect_heartbeat_abort() == 1) {
      if (Serial) {
        Serial.println("abort shutdown");
      }
      return server_running;
    }
  }

  if (get_last_heartbeat() < HEARTBEAT_TIMEOUT) {
    serial_flush();
    return server_shutdown_active;
  } else {
    serial_flush();
    return server_locked;
  }
}

void *server_hungup() {
  if (Serial) {
    Serial.println("hung up");
    Serial.println(get_last_heartbeat());
  }

  heartbeat = detect_heartbeat();
  heartbeat = get_last_heartbeat();

  if (heartbeat < HEARTBEAT_TIMEOUT || detect_heartbeat() > 0) {

    return server_running;
  } else {

    if (heartbeat > HEARTBEAT_TIMEOUT && heartbeat < 20000UL) {
      animate_hungup(25);
    } else if (heartbeat > 20000UL && heartbeat < 30000UL) {
      animate_hungup(15);
    } else if (heartbeat > 30000UL && heartbeat < 450000UL) {
      animate_hungup(5);
    }

    if (heartbeat > 45000UL) {
      led_red();
    }

    if (get_last_heartbeat() > 60000UL) {

      return server_reset;
    } else {

      return server_hungup;
    }

  }
}

void *server_reset() {
  if (Serial) {
    Serial.println("touching run pins on pi");
  }
  touch_run_pin();
  serial_flush();
  return server_booting;
}

void *server_locked() {
  if (Serial) {
    Serial.println("LOCKED");
  }
  if (pushlock == LOCKED) {
    led_off();
    serial_flush();
    return server_locked;
  } else {
    serial_flush();
    return server_bootable;
  }
}
