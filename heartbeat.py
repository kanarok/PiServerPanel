#!/usr/bin/env python

import serial, sys, time

def shutdown():
    command = "/sbin/shutdown -h -t 30"
    import subprocess
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output = process.communicate()[0]
    print(output)

def shutdown_abort():
    command = "/sbin/shutdown -c"
    import subprocess
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output = process.communicate()[0]
    print(output)

serialPort = "/dev/ttyACM0"

ser = serial.Serial(
    port=serialPort,
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

ser.isOpen()

while True:
	print("Heartbeat")
	ser.write(B'\x42')
	bytesToRead = ser.inWaiting()
	if (bytesToRead > 0):
		cmd = ser.readline()
		ser.flushInput()
		ser.flushOutput()
		print(cmd)
		if (cmd == b'shutdown\r\n'):
			shutdown()
			ser.write(B'\x108')
		if (cmd == b'abort\r\n'):
			shutdown-abort()
			ser.write(B'\x109')
	time.sleep(3)

ser.close()
