# Outdoor Air Pollution Monitoring 

An air pollution monitoring system on campus using the ESP8266 microcontroller and arduino. The microcontroller measures levels of gases such as CO, NO2 and NH3 in the environment, and regularly uploads data to a thingspeak dashboard for monitoring. 

The system was successfully deployed in the IIIT-Hyderabad campus.

# Info

Each folder represents code for 1 node, the node_[no.] file has the arduino code to be loaded to each node for its functionality and the python file called thingspeak in each folder gives
running average of the data for that node given that a 'feeds.csv' file is downloaded into the folder 
from the Thingspeak platform(using Download button in export section of the Data Import/Export tab).  
