#!/usr/bin/env python

import serial, sys, time

def heartbeat():
	ser.write(B'\x42')

def shutdown():
	command = "/sbin/shutdown -h -t sec 30"
	import subprocess
	process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
	output = process.communicate()[0]
	print(output)
	ser.flushOutput()
	ser.write(B'\x108')

def abort_shutdown():
	command = "/sbin/shutdown -c"
	import subprocess
	process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
	output = process.communicate()[0]
	print(output)
	ser.flushOutput()
	ser.write(B'\x109')

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

commands = {	"heartbeat": heartbeat,
		"shutdown": shutdown,
		"abort shutdown": abort_shutdown,
}

millis = int(round(time.time() * 1000))

ser.isOpen()
while True:
	print(mode)
	commands[mode]()
	end_wait = int(round(time.time() * 1000)) + 2900
	while (int(round(time.time() * 1000)) < end_wait):
		bytesToRead = ser.inWaiting()
		if (bytesToRead > 0):
			cmd = str(ser.readline(), "utf-8")[:-2]
			print(cmd)
			ser.flushInput()
			commands[cmd]()

ser.close()
