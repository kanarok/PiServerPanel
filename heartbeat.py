#!/usr/bin/env python

import serial, sys, time, signal

def handle_interrupt(signal, frame):
	print("closing serial port...")
	ser.close()
	sys.exit(0)

def handle_terminate(signal, frame):
	print("terminated, sending 'system shutdown' to panel")
	send_system_shutdown()

def heartbeat():
	send_heartbeat()

def shutdown():
	command = "/sbin/shutdown -h -t sec 30"
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
	ser.flushOutput()
	send_abort()

def mode2heartbeat():
	mode = "heartbeat"

def send_heartbeat():
	ser.write(B'\x42')

def send_ack():
	ser.write(B'\x108')

def send_abort():
	ser.write(B'\x109')

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
}

signal.signal(signal.SIGINT, handle_interrupt)
signal.signal(signal.SIGTERM, handle_terminate)

ser.isOpen()
while True:
	start_routine = int(round(time.time() * 1000))
	print(mode)
	heartbeats[mode]()
	end_wait = int(round(time.time() * 1000)) + 2950
	while (int(round(time.time() * 1000)) < end_wait):
		bytesToRead = ser.inWaiting()
		if (bytesToRead > 0):
			cmd = str(ser.readline(), "utf-8")[:-2]
			print("Received command: ", cmd)
			ser.flushInput()
			try:
				commands[cmd]()
				mode = cmd
			except:
				print("Command not found: ", cmd)
				mode = "heartbeat"
	print("Routine took: ", int(round(time.time() * 1000))-start_routine, "ms")
ser.close()
