#include <TimerOne.h>

#define LED_R                       6
#define LED_G                       9
#define LED_B                      10

#define BUTTON                      8
#define PUSHLOCK                    4
#define PUSHLOCK_NO                 4
#define PUSHLOCK_NC                 5

#define LOCKED                    LOW
#define UNLOCKED                 HIGH
#define IS_ON                    HIGH
#define PUSHED                   HIGH
#define UNDEFINED                   2

#define HEARTBEAT_TIMEOUT          10000
#define HEARTBEAT_IS_MISSING        3333
#define HEARTBEAT                   0x42
#define HEARTBEAT_SYSTEM_SHUTDOWN   0x54
#define HEARTBEAT_SHUTDOWN_ACK     0x108
#define HEARTBEAT_ABORT_ACK        0x109

#define SYSTEM_RUN                  7
#define SYSTEN_ON                  A3
#define TIME_UNTIL_REBOOT       60000UL

#define BAUDRATE               115200L
#define WAIT_TO_SEND             1000

//physical routines
void touch_run_pin();

//heartbeat detection
long detect_heartbeat_any();
long detect_heartbeat();
long detect_heartbeat_ack();
long detect_heartbeat_abort();
long get_last_heartbeat();

//heartbeat signals
void signal_heartbeat();
void signal_heartbeat_missing();
void signal_heartbeat_ack();
void signal_heartbeat_abort();
void signal_heartbeat_system_shutdown();

//generic led routines
void led_color(byte red, byte green, byte blue);
void led_off(byte value);
void led_white(byte value);
void led_red(byte value);
void led_green(byte value);
void led_blue(byte value);

//led signals
void signal_server_locked();
//void siganimate_

//uart routines
void serial_flush();
void debug_print(char *msg);
//uart messages
void send_ok();
void send_shutdown();
void send_abort_shutdown();

//States                          //state matching led animations
void *server_bootable();          void animate_bootable();
void *server_booting();           void animate_booting();
void *server_running();           void animate_running();
void *server_shutdown();          void animate_shutdown();
void *server_shutdown_active();   void animate_shutdown_active();
void *server_hungup();            void animate_hungup();
void *server_locked();            void animate_locked();
void *system_reset();             void animate_reset();
void *system_shutdown();  //async trigger in function detect_heartbeat();

//FSM
typedef void *(*StateFunc)();
StateFunc statefunc = server_bootable;

//++++ cleanup due!

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
int receivedHeartbeat = 0;
unsigned long heartbeat, last_heartbeat;
unsigned long track_last_heartbeat, track_missing_heartbeat, track_uart;


void setup() {
  pinMode(PUSHLOCK_NO, INPUT_PULLUP);
  pinMode(PUSHLOCK_NC, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(SYSTEM_ON, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(SYSTEM_RUN, OUTPUT);

  pushlock = digitalRead(PUSHLOCK);

  track_last_heartbeat = millis();
  track_missing_heartbeat = track_last_heartbeat;

  Serial.begin(BAUDRATE);
}

void loop() {
  heartbeat = detect_heartbeat();
  last_heartbeat = get_last_heartbeat();

  statefunc = (StateFunc)(*statefunc)();    //FSM
}

//physical routines

void touch_run_pin() {
  digitalWrite(RUN, HIGH);
  delay(50);
  digitalWrite(RUN, LOW);
}

//generic led routines
void led_color(byte red, byte green, byte blue) {
  analogWrite(LED_R, 255-red);
  analogWrite(LED_G, 255-green);
  analogWrite(LED_B, 255-blue);
}

void led_off() {
  led_color(0,0,0);
}

void led_white(byte values {
  led_color(255-value,255-value,255-value)
}

void led_red(byte value) {
  led_color(255-value, 255, 255);
}

void led_green(byte value) {
  led_color(255, 255-byte value, 255);
}

void led_blue(byte value) {
  led_color(255, 255, 255-byte value);
}


//heartbeat signals
void signal_heartbeat() {
  led_green();
  delay(10);
  led_red();
  delay(10);
  led_off();
  delay(15);
}

void signal_heartbeat_missing() {
  led_red();
  delay(10);
  led_red();
  delay(10);
  led_off();
  delay(15);
}

void signal_heartbeat_ack() {
  led_color(255,0,255);
  delay(10);
  led_color(255,0,255);
  delay(10);
  led_color(0,0,0);
  delay(15);
}

void signal_heartbeat_abort() {
  led_color(0,0,255);
  delay(10);
  led_color(255,255,255);
  delay(10);
  led_color(0,0,0);
  delay(15);
}

<<<<<<< HEAD
void signal_heartbeat_system_shutdown() {
  led_color(0,0,255);
  delay(10);
  led_color(90,20,200);
  delay(10);
  led_color(0,0,0);
  delay(15);
=======
//heartbeat detection
long detect_heartbeat_any() {

>>>>>>> c774d8b760525d033ce5c35391565738e5727941
}

//heartbeat detection
long detect_heartbeat_any() {

}

int detect_heartbeat() {
  if (((millis()-track_missing_heartbeat) >= HEARTBEAT_IS_MISSING) && get_last_heartbeat() >= HEARTBEAT_IS_MISSING) {
    signal_heartbeat_missing();
    track_missing_heartbeat = millis();
  }

  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    serial_flush();

    switch (incomingByte) {
      case 0x42:  // heartbeat
                  signal_heartbeat();
                  break;
      case 0x54:  // system shutdown
                  statefunc = system_shutdown;
                  signal_heartbeat_system_shutdown();
                  break;
      case 0x108: // ack
                  signal_heartbeat_ack();
                  send_ok();
                  break;
      case 0x109: // abort (ack)
                  signal_heartbeat_abort();
                  send_ok();
                  break;
    }

    switch (incomingByte) {
      case 0x42:  // heartbeat
      case 0x54:  // system shutdown
      case 0x108: // ack
      case 0x109: // cancel (ack)
                  receivedHeartbeat = incomingByte;
                  track_last_heartbeat = millis();
                  track_missing_heartbeat = track_last_heartbeat;
                  return receivedHeartbeat;
      default:
                  receivedHeartbeat = -1;
                  return receivedHeartbeat;
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
      send_ok();
      signal_heartbeat_ack();
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
      send_ok();
      signal_heartbeat_abort();
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

void animate_hungup() {
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

//uart routines

void serial_flush() {
  int buf;
  while (Serial.available() > 0) {
      buf = Serial.read();
  }
}

void debug_print(char *msg) {
  Serial1.println("ok");
}

//uart messages

void send_ok() {
  Serial.println("ok");
  return;
}

void send_shutdown() {
  Serial.println("shutdown");
  return;
}

void send_abort_shutdown() {
  Serial.println("abort shutdown");
  return;
}

//states

void *server_bootable () {
  animate_bootable();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("BOOTABLE");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {
    return server_booting;
  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      if (last_heartbeat >= HEARTBEAT_TIMEOUT) {
        signal_server_locked();
        return server_locked;
      }
    } else {
      if (digitalRead(BUTTON) == PUSHED) {
        delay(100);
        return system_reset;
      }
    }
    return server_bootable;
  }
}

void *server_booting () {
  animate_booting();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("BOOTING");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if (receivedHeartbeat > 0) {
      return server_running;
    } else {
      return server_booting;
    }

} else {

    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }

  }
}

void *server_running () {
  animate_running();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("RUNNING");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {
      if (receivedHeartbeat > 0) {
        return server_running;
      }

      if (digitalRead(PUSHLOCK) == LOCKED) {
        send_shutdown();
        return server_shutdown;
      }
  } else {
    if (digitalRead(BUTTON) == PUSHED) {
      delay(100);
      //return system_reset;
    }

    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

void *server_shutdown() {
  animate_shutdown();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SHUTDOWN");
    send_shutdown();
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if (digitalRead(BUTTON) == PUSHED) {
      delay(100);
      send_abort_shutdown();
    }

    if (receivedHeartbeat == HEARTBEAT_SHUTDOWN_ACK) {
      return server_shutdown_active;
    } else if (receivedHeartbeat == HEARTBEAT_ABORT_ACK) {
      return server_running;
    } else {
      return server_shutdown;
    }

  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

void *server_shutdown_active() {
  animate_shutdown_active();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SHUTDOWN ACTIVE");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if ((digitalRead(BUTTON) == PUSHED) || (digitalRead(PUSHLOCK) == UNLOCKED)) {
      delay(100);
      send_abort_shutdown();
    }

    if (receivedHeartbeat == HEARTBEAT_ABORT_ACK) {
      send_ok();
      return server_running;
    } else {
      return server_shutdown_active;
    }

  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

void *server_hungup() {
  animate_hungup();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("HUNG UP");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if ()(digitalRead(BUTTON) == PUSHED) || (get_last_heartbeat() >= TIME_UNTIL_REBOOT)) {
      delay(100);
      return system_reset;
    }

    if (receivedHeartbeat > 0) {}
      return server_running;
    } else {
      return server_hungup;
    }

  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

void *server_locked() {
  animate_locked();

  if (((millis()-track_uart) >= WAIT_TO_SEND*10)) {
    //send_();
    debug_print("LOCKED");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_shutdown;
    } else {
      return server_running;
    }
  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

void *system_reset() {
  animate_reset();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SYSTEM RESET");
    track_uart = millis();
  }
  touch_run_pin();
  return server_booting;
}

void *system_shutdown() {
  animate_shutdown_active();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SYSTEM SHUTDOWN");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if ((digitalRead(BUTTON) == PUSHED) || (digitalRead(PUSHLOCK) == UNLOCKED)) {
      delay(100);
      send_abort_shutdown();
    }

    if (receivedHeartbeat > 0) {
      send_ok();
      return server_running;
    } else {
      return server_shutdown_active;
    }

  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}
