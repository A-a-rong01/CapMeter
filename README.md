# Capacitance Meter Project - BEE 425 Microprocessor Design
## Overview
This project is a battery-operated capacitance meter designed as part of the BEE 425 Microprocessor Design course. The meter operates independently from a PC and measures capacitance values ranging from 20 pF to 200 μF with an accuracy of up to 2%. The design utilizes a Teensy 4.1 microcontroller and a 555 timer, along with relays and a custom PCB to provide reliable and accurate capacitance readings.

## Features
Power Supply: Operates either from a battery or a 5V-15V wall power supply ("wall wart"), making it fully self-sufficient.
Auto-Ranging: Automatically selects the appropriate range for best accuracy over the 20 pF to 200 μF capacitance range.
Resolution: 3-1/2 digit display (0000 - 1999).
Accuracy:
2% for 2 nF to 2 μF.
10% for 20 pF to 2 nF and 2 μF to 200 μF.
Update Frequency: Provides at least 5 updates per second.
Flying Leads: Comes equipped with flying leads and alligator clips for easy connection to unknown capacitors.
Independence from PC: Fully operational without the need for a PC or laptop connection.
