# PiServerPanel
Arcade Button with self wired RGB led and Metal Emergency Switch

This project started with an arcade button and the wish for an RGB led inside.
An emergency switch and the idea of some kind of "panel" für my raspberry pi was born.

Please note, there is a picture to visualize the components and simple wiring used.

The code features a callback state machine.
There are 8 states:
server_bootable (initial state)
server_booting (kind of useless \*1)
server_running
server_hungup (untested)
server_shutdown (when emergency switch is locked)
server_shutdown_active (when pi issued shutdown command)
system_reset (touches run pins on pi)
system_shutdown (untested, when pi is shutdown a special heartbeat is issued to the panel)

\*1 The idea was to use a 5V pin of the pi, in case the pi can power off, which is cannot. The workaround is a GPIO Pin Setup instead which renders the booting state useless.

On the pi side there is a heartbeat.py:
send a heartbeat every 3 seconds and monitors ttyACM0 for communication.
takes serial devices as first parameter.

More Documentation to come...
