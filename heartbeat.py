#!/usr/bin/env python

#Version: beta mk II

import serial, sys, time, signal
import RPi.GPIO as GPIO

def gpio_setup():
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup(40, GPIO.OUT)

def gpio_on():
	GPIO.output(40, 1)

def gpio_off():
	GPIO.output(40, 0)

def handle_interrupt(signal, frame):
	GPIO.cleanup()
	print("closing serial port...")
	ser.close()
	sys.exit(0)

def handle_terminate(signal, frame):
	print("terminated, sending 'system shutdown' to panel")
	send_system_shutdown()
	gpio_off()

def heartbeat():
	send_heartbeat()

def shutdown():
	command = "/sbin/shutdown -h -t +1"
	import subprocess
	process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
	output = process.communicate()[0]
	print(output)
	ser.flushOutput()
	send_ack()

def abort_shutdown():
	command = "/sbin/shutdown -c"
	import subprocess
	process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
	output = process.communicate()[0]
	print(output)
	print("Shutdown aborted")
	ser.flushOutput()
	send_abort()

def mode2heartbeat():
	mode = "heartbeat"

def send_heartbeat():
	ser.write(B'\x42')

def send_ack():
	ser.write(B'\xA0')

def send_abort():
	ser.write(B'\xA9')

def send_system_shutdown():
	ser.write(B'\x54')

try:
	serialPort = sys.argv[1]
except:
	serialPort = "/dev/ttyACM0"

ser = serial.Serial(
    port=serialPort,
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

mode = "heartbeat"

commands = {	"heartbeat":      	heartbeat,
		"shutdown":       	shutdown,
		"abort shutdown": 	abort_shutdown,
		"ok":             	mode2heartbeat,
}

heartbeats = {	"heartbeat":      	send_heartbeat,
		"shutdown":       	send_ack,
		"abort shutdown": 	send_abort,
		"system shutdown":	send_system_shutdown,
		"ok":             	mode2heartbeat,
}

gpio_setup()
gpio_on()

signal.signal(signal.SIGINT, handle_interrupt)
signal.signal(signal.SIGTERM, handle_terminate)

ser.isOpen()
while True:
#	start_routine = int(round(time.time() * 1000))
#	print("Heartbeat: ", mode)
	if (mode == "ok"):
		mode = "heartbeat"
	heartbeats[mode]()
	end_wait = int(round(time.time() * 1000)) + 2950
	while (int(round(time.time() * 1000)) < end_wait):
		bytesToRead = ser.inWaiting()
		if (bytesToRead > 0):
			cmd = str(ser.readline())[:-2]
#			print("Received command: ", cmd)
			ser.flushInput()
			try:
				mode = cmd
				commands[cmd]()
			except:
#				print("Command not found: ", cmd)
				mode = "heartbeat"
#	print("Routine took: ", int(round(time.time() * 1000))-start_routine, "ms")
ser.close()
