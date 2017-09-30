# PiServerPanel<br />
Arcade Button with self wired RGB led and Metal Emergency Switch<br />
<br />
This project started with an arcade button and the wish for an RGB led inside.<br />
An emergency switch and the idea of some kind of "panel" für my raspberry pi was born.<br />
<br />
Please note, there is a picture to visualize the components and simple wiring used.<br />
<br />
The code features a callback state machine.<br />
There are 8 states:<br />
server_bootable (initial state)<br />
server_booting (kind of useless \*1)<br />
server_running<br />
server_hungup (untested)<br />
server_shutdown (when emergency switch is locked)<br />
server_shutdown_active (when pi issued shutdown command)<br />
system_reset (touches run pins on pi)<br />
system_shutdown (untested, when pi is shutdown a special heartbeat is issued to the panel)<br />
<br />
\*1 The idea was to use a 5V pin of the pi, in case the pi can power off, which is cannot. The workaround is a GPIO Pin Setup instead which renders the booting state useless.<br />
<br />
On the pi side there is a heartbeat.py:<br />
send a heartbeat every 3 seconds and monitors ttyACM0 for communication.<br />
takes serial devices as first parameter.<br />
