The coolerThermostat sketch continuously runs three separate loops, which are set to repeat with their own frequencies. Each loop is triggered by the time-elapsed with respect to the real time clock. These are the loops:

Sleep Loop (Every 8 seconds)
Fan Duty Cycle Loop (every 120 seconds)
Data Logging Loop (every 600 seconds or 10 minutes) 
Thermostat Loop (every 896 seconds or 15 minutes)


Sleep Loop (technically the main loop of the program):
Every 8 seconds, the Arduino will wake up and begin its main loop. It activates the peripherals and evaluates IF statements that trigger the other loops if enough time has passed. At the end, it deactivates the peripherals and puts the Arduino to sleep in watch dog timer wakeup mode using ISR (WDT_vect)


Fan Duty Cycle Loop:
Every 120 seconds (14 sleep loop cycles) the fan is activated. It runs for a short period of time, as determined by the fanRunTime variable, measured in ms. The fanRunTime is adjusted by the thermostat, which makes its decisions much less frequently than the fan duty cycle loop, allowing time for the temperature to respond to thermostat changes. Increasing the fanRunTime has the effect of decreasing the cooler temperature. 


Data Logging Loop: 
Every 10 minutes (800 seconds or 100 sleep loop cycles) the data logging subroutine is activated. A time and date stamp along with a current temperature read and states of the thermostat's variables is written as a line of text to a text document named 'datalog.txt' on the Micro SD card. 

Thermostat Loop:
Every ~=15 minutes (896 seconds or 112 sleep loop cycles) the thermostat loop runs. 
The Thermostat Loop does not directly activate or deactivate the fan. Instead, it just makes adjustments to the fan's fanRunTime variable. Whereas the fan loop occurs every 120 seconds to maintain even temperature, the thermostat loop occurs every 15 minutes to allow cooler temperature to respond to chanes in fan duty cycle. 


Expanded explanation of the Thermostat Loop:
The sketch presented for this cooler uses a 'Seek-Overshoot' control method. That is to say, the system is always operating in one of two states: seeking the set point, or overshooting the set point. The variables used by the thermostat loop are as follows:

	tstatTimeWindow - the time in seconds between the start of each thermostat cycle.
	setPoint - temperature in degrees C that system will attempt to maintain
	thisWindowTemp - current temperature reading of the cooler this thermostat time window
	prevWindowTemp - temperature reading measured during the previous thermostat window.
	currentAdjustment - time in ms by which the fanRunTime will be changed during this thermostat window.
	minAdjustment - minimum allowable adjustment time in ms
	maxAdjustment -maximum allowable adjustment time in ms
	fanRunTime - the durration that the fan remains on for each fan window, in ms. 

The 'Seek-Overshoot' control system strives to arrive quickly at the set point, and once there, to minimize the magnitude of oscillation about it. This method operates by evaluating a current temperature reading against a previous temperature reading with respect to the set point. By this method, there are four possible states that can be found: 

1. Seeking Warming Up - current temp is lower than set point, previous temp was lower than.
2. Seeking Cooling Down - current temp is higher than set point, previous temp was higher than.
3. Overshot Warming Up - current temp is higher than set point, previous temp was lower than.
4. Overshot Cooling Down - current temp is lower than set point, previous temp was higher than. 

The following is a list of the four states, along with the action taken by the thermostat:

	Seeking Warming Up -> double currentAdjustment and subtract it from fanRunTime
	Seeking Cooling Down -> double currentAdjustment and add it to fanRunTime
	Overshot Warming Up -> halve currentAdjustment and add it to fanRunTime
	Overshot Cooling Down -> halve currentAdjustment and subtract it from fanRunTime

That's it! The thermostat loop just makes adjustments to fanRunTime. The fan loop routinely runs the fan for this period of time.

Tuning the Seek-Overshoot control system:
Changing the initial currentAdjustment affects how long it will take to reach the set point the first time, but this will rapidly change once the set point has been found with an overshoot.
If the tstatTimeWindow is too short, or the fanTimewindow is too long, there will be large temperature oscillations over time because of the lag between fan action and measured temperature change. 

With a short fanTimeWindow and a long tStatTimeWindow, this system is good at maintaining the temperature of a cooler. We found an experimental accuracy of +-0.25 degrees C when using a sensor with a 0.10 degrees C resolution. The system takes around 2 hours to stabalize at 5C when buffered with water and started at 20C.

The system is easy to implement and straightforward to tune. It combines functional features of P-I-D control in its ability to respond proportionally, ability to heighten response as error accumulates over time (integration) and mechanism to actively squelch magnitude of oscillation about the set point (derivative). This method is good in laggy systems because a simple 'lag response time' in seconds can be assessed experimentally, and then used for the tstatTimeWindow variable's value.  

