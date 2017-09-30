#include <TimerOne.h>

//Version: beta mk I

#define LED_R                       6
#define LED_G                       9
#define LED_B                      10

#define BUTTON                      8
#define PUSHLOCK                    5
#define PUSHLOCK_NO                 4
#define PUSHLOCK_NC                 5

#define LOCKED                    LOW
#define UNLOCKED                 HIGH
#define IS_ON                    HIGH
#define PUSHED                   HIGH
#define UNDEFINED                   2

#define HEARTBEAT_TIMEOUT         10000
#define HEARTBEAT_IS_MISSING       3333
#define HEARTBEAT                  0x42
#define HEARTBEAT_SYSTEM_SHUTDOWN  0x54
#define HEARTBEAT_SHUTDOWN_ACK     0xA0
#define HEARTBEAT_ABORT_ACK        0xA9

#define SYSTEM_RUN                  7
#define SYSTEM_ON                  A3
#define TIME_UNTIL_REBOOT       60000UL

#define BAUDRATE               115200L
#define WAIT_TO_SEND             3000

//physical routines
void touch_run_pin();

//generic led routines
void led_color(byte red, byte green, byte blue);
void led_off(byte value);
void led_white(byte value);
void led_red(byte value);
void led_green(byte value);
void led_blue(byte value);
void led_yellow(byte value);

//led signals
void signal_server_locked();
//void siganimate_

//heartbeat detection
int detect_heartbeat();       //sets receivedHeartbeat;
long get_last_heartbeat();

//heartbeat signals
void signal_heartbeat();
void signal_heartbeat_missing();
void signal_heartbeat_ack();
void signal_heartbeat_abort();
void signal_heartbeat_system_shutdown();

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
void *system_reset();             void animate_system_reset();
void *system_shutdown();          void animate_system_shutdown();
//async trigger in function detect_heartbeat();

//test routines
void test_led();
void test_animation();

//FSM
typedef void *(*StateFunc)();
StateFunc statefunc = server_bootable;

int pwm_red = 0;
int pwm_green = 0;
int pwm_blue = 0;
int pwm_yellow = 0;

int fader_red = 7;
int fader_green = 3;
int fader_blue = 15;
int fader_yellow = 10;

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
  digitalWrite(SYSTEM_RUN, HIGH);
  delay(50);
  digitalWrite(SYSTEM_RUN, LOW);
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

void led_white(byte value) {
  led_color(value, value, value);
}

void led_red(byte value) {
  led_color(value, 0, 0);
}

void led_green(byte value) {
  led_color(0, value, 0);
}

void led_blue(byte value) {
  led_color(0, 0, value);
}

void led_yellow(byte value) {
  led_color(255, 255, 0);
}

//led signals

void signal_server_locked() {
  led_red(255);
  delay(80);
}

//heartbeat detection

int detect_heartbeat() {
  if (((millis()-track_missing_heartbeat) >= HEARTBEAT_IS_MISSING) && get_last_heartbeat() >= HEARTBEAT_IS_MISSING) {
    if ((statefunc != server_bootable) && (statefunc != server_locked)) {
      signal_heartbeat_missing();
    }
    track_missing_heartbeat = millis();
  }

  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    serial_flush();

    switch (incomingByte) {
      case HEARTBEAT:
                  signal_heartbeat();
                  break;
      case HEARTBEAT_SYSTEM_SHUTDOWN:
                  statefunc = system_shutdown;
                  signal_heartbeat_system_shutdown();
                  break;
      case HEARTBEAT_SHUTDOWN_ACK:
                  signal_heartbeat_ack();
                  send_ok();
                  break;
      case HEARTBEAT_ABORT_ACK:
                  signal_heartbeat_abort();
                  send_ok();
                  break;
    }

    switch (incomingByte) {
      case HEARTBEAT:
      case HEARTBEAT_SYSTEM_SHUTDOWN:
      case HEARTBEAT_SHUTDOWN_ACK:
      case HEARTBEAT_ABORT_ACK:
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

long get_last_heartbeat() {
  return millis()-track_last_heartbeat;
}

//heartbeat signals
void signal_heartbeat() {
  led_blue(255);
  delay(10);
  led_off();
  delay(10);
  led_blue(150);
  delay(15);
  led_off();
}

void signal_heartbeat_missing() {
  led_red(255);
  delay(10);
  led_red(255);
  delay(10);
  led_off();
  delay(15);
}

void signal_heartbeat_ack() {
  led_color(255,0,255);
  delay(20);
  led_off();
  delay(10);
  led_color(255,0,255);
  delay(10);
  led_off();
  delay(15);
}

void signal_heartbeat_abort() {
  led_blue(255);
  delay(10);
  led_white(255);
  delay(10);
  led_off();
  delay(15);
}

void signal_heartbeat_system_shutdown() {
  led_blue(255);
  delay(30);
  led_color(90,20,200);
  delay(20);
  led_color(0,0,0);
  delay(15);
}

//uart routines

void serial_flush() {
  int buf;
  while (Serial.available() > 0) {
      buf = Serial.read();
  }
}

void debug_print(char *msg) {
  //Serial.println(msg);
}

//uart messages

void send_ok() {
  Serial.println("ok");
}

void send_shutdown() {
  Serial.println("shutdown");
}

void send_abort_shutdown() {
  Serial.println("abort shutdown");
}

//state matching led animations

void animate_bootable() {
  int ms = 45;
  int min_value = 0;
  int max_value = 150;
  
  led_green(pwm_green);
  
  pwm_green = pwm_green + fader_green;
  
  if ( pwm_green > max_value+1 ) {
    pwm_green = max_value;
  }

  if ( pwm_green <= min_value || pwm_green >= max_value ) {
    fader_green = -fader_green;
  }

  if (fader_green > 0) {
   delay(ms/2);
  } else {
   delay(ms);
  }
}

void animate_booting() {
  int ms = 45;
  int min_value = 0;
  int max_value = 255;
  
  led_color(pwm_red, pwm_green, pwm_blue);
  
  pwm_red = pwm_red + fader_red;
  pwm_green = pwm_green + fader_green;
  pwm_blue = pwm_blue + fader_blue;

  if ( pwm_red <= min_value || pwm_red >= max_value ) {
    fader_red = -fader_red;
  }

  if ( pwm_green <= min_value || pwm_green >= max_value ) {
    fader_green = -fader_green;
  }
  
  if ( pwm_blue <= min_value || pwm_blue >= max_value ) {
    fader_blue = -fader_blue;
  }

  if (fader_green > 0) {
   delay(ms);
  } else {
   delay(ms/2);
  }
}

void animate_running() {
  int ms = 20;
  int min_value = 0;
  int max_value = 255;
  
  led_color(pwm_red, pwm_green, pwm_blue);
  
  pwm_red = pwm_red + fader_red;
  pwm_green = pwm_green + fader_green;
  pwm_blue = pwm_blue + fader_blue;

  if ( pwm_red <= min_value || pwm_red >= max_value ) {
    fader_red = -fader_red;
  }

  if ( pwm_green <= min_value || pwm_green >= max_value ) {
    fader_green = -fader_green;
  }
  
  if ( pwm_blue <= min_value || pwm_blue >= max_value ) {
    fader_blue = -fader_blue;
  }

  if (fader_green > 0) {
   delay(ms);
  } else {
   delay(ms/2);
  }
}

void animate_shutdown() {
  int ms = 45;
  int min_value = 0;
  int max_value = 150;
  
  led_yellow(pwm_yellow);
  
  pwm_yellow = pwm_yellow + fader_yellow;
  
  if ( pwm_yellow > max_value+1 ) {
    pwm_yellow = max_value;
  }

  if ( pwm_yellow <= min_value || pwm_yellow >= max_value ) {
    fader_yellow = -fader_yellow;
  }

  if (fader_yellow > 0) {
   delay(ms);
  } else {
   delay(ms);
  }
}

void animate_shutdown_active() {
  int ms = 15;
  int min_value = 0;
  int max_value = 150;
  
  led_red(pwm_yellow);
  
  pwm_yellow = pwm_yellow + fader_yellow;
  
  if ( pwm_yellow > max_value+1 ) {
    pwm_yellow = max_value;
  }

  if ( pwm_yellow <= min_value || pwm_yellow >= max_value ) {
    fader_yellow = -fader_yellow;
  }

  if (fader_yellow > 0) {
   delay(ms);
  } else {
   delay(ms/3);
  }
}

void animate_hungup() {
  led_yellow(255);
  delay(80);
  led_off();
  delay(20);
}

void animate_locked() {

}

void animate_system_reset() {
  led_red(255);
  delay(20);
  led_yellow(255);
  delay(40);
  led_off();
  delay(10);
  led_red(255);
  delay(20);
  led_yellow(255);
  delay(40);
  delay(10);
}

void animate_system_shutdown() {
  led_color(190, 70, 100);
  delay(80);
  led_off();
  delay(20);
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
    
    if (digitalRead(PUSHLOCK) == LOCKED) {
      send_shutdown();
      return server_shutdown;
    } else {
      if (receivedHeartbeat > 0) {
        return server_running;
      }
    }


    if (digitalRead(BUTTON) == PUSHED) {
      delay(100);
      //print mail or sth. else
    }
      
  } else {
    
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

    if (digitalRead(PUSHLOCK) == LOCKED) {
      
      if (receivedHeartbeat == HEARTBEAT_SHUTDOWN_ACK) {
        send_ok();
        return server_shutdown_active;
      }      
      
    } else {
      
      if (digitalRead(BUTTON) == PUSHED) {
        delay(100);
        send_abort_shutdown();
      }  

      if (receivedHeartbeat == HEARTBEAT_ABORT_ACK) {
        return server_running;
      }
      
    }

    return server_shutdown;

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

    if ((digitalRead(BUTTON) == PUSHED) && (digitalRead(PUSHLOCK) == UNLOCKED)) {
      delay(300);
      send_abort_shutdown();
    }

    if (receivedHeartbeat == HEARTBEAT_ABORT_ACK) {
      send_ok();
      return server_running;
    }

  } else {
    if (get_last_heartbeat() > HEARTBEAT_TIMEOUT) {
      if (digitalRead(PUSHLOCK) == LOCKED) {
        return server_locked;
      } else {
        return server_bootable;
      }
    }
  }
  
  return server_shutdown_active;
}

void *server_hungup() {
  animate_hungup();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("HUNG UP");
    track_uart = millis();
  }

  if (digitalRead(SYSTEM_ON) == IS_ON) {

    if ((digitalRead(BUTTON) == PUSHED) || (get_last_heartbeat() >= TIME_UNTIL_REBOOT)) {
      delay(100);
      return system_reset;
    }

    if (receivedHeartbeat > 0) {
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
  animate_system_reset();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SYSTEM RESET");
    track_uart = millis();
  }
  touch_run_pin();
  return server_booting;
}

void *system_shutdown() {
  animate_system_shutdown();

  if (((millis()-track_uart) >= WAIT_TO_SEND)) {
    //send_();
    debug_print("SYSTEM SHUTDOWN");
    track_uart = millis();
  }

  if ((digitalRead(SYSTEM_ON) == IS_ON) || (get_last_heartbeat() < HEARTBEAT_TIMEOUT)) {
    return system_shutdown;
  } else {
    if (digitalRead(PUSHLOCK) == LOCKED) {
      return server_locked;
    } else {
      return server_bootable;
    }
  }
}

//test routines

void test_led() {
  debug_print("LED TEST");
  debug_print("led off");
  led_off();
  delay(2000);
  debug_print("led red");
  led_red(255);
  delay(2000);
  debug_print("led white");
  led_white(255);
  delay(2000);
  debug_print("led green");
  led_green(100);
  delay(2000);
  debug_print("led blue");
  led_blue(255);
  delay(2000);
  debug_print("led yellow");
  led_yellow(255);
  delay(2000);
}

void test_animation() {
  debug_print("ANIMATION TEST");
  animate_bootable();
  delay(100);
  animate_booting();
  delay(100);
  animate_running();
  delay(100);
  animate_shutdown();
  delay(100);
  animate_shutdown_active();
  delay(100);
  animate_hungup();
  delay(100);
  animate_locked();
  delay(100);
  animate_system_reset();
  delay(100);
  animate_system_shutdown();
}
