# Build and test Information

This code was developed, built and tested on Ubuntu Linux

To build the code, run `make build` or `make all`

To run the test code after building, use `./test_therm`

The unit tests will run through a series of test cases to verify the implementation. The test code starts a 
UDP client to send fake UDP temperature messages to allow for realistic test scenarios

# Requirements

The following requirements are met for this thermostat API:
    * The thermostat consists of four settable temperature control mode settings 
        * Off: No temperature control
        * Auto: Heat or cool to maintain a specific temperature
        * Heat: Heat to guarantee a high temperature
        * Cool: Cool to guarantee a low temperature
    * Track the current thermostat status to determine current state
        * Inactive: No active heating or cooling
        * Heating: Actively heating
        * Cooling: Actively cooling
    * Allow a settable temperature setpoint as the target temperature to control to
    * Allow a settable temperature margin value
        * Start heating (assuming heat or auto mode) when the temperature is below the setpoint - margin 
        * Start cooling (assuming cool or auto mode) when the temperature is above the setpoint + margin 
        * If actively heating or cooling, stop once the setpoint is reached

# Design Decisions

The temperature sampling data was assumed to be delivered via UDP packets. This may likely be done via GPIO or I2C, 
but UDP was assumed to allow for testing.

There are two threads in the API. One thread monitors UDP temperature data and stores the current temperature. 
The other thread runns the temperature controller logic, based on the temperature settings and modes of operation.

The API creates a UDP server. Although only one UDP temperature client was used, the API can easily be extended 
to receive temperature data from multiple sources in the future. 

The test code creates a UDP client to send fake temperature data for testing thermostat scenarios.

# Future Work

As noted in "Design Decisions" The API UDP server can easily accept temperature data from additional sources. 
A use case for this would be if there is separate temperature control over multiple zones (i.e upstairs, downstairs). 
With multiple zones, there would likely be separate temperature sampling devices in each zone. The current UDP data 
could be extended to add a zone ID so the API can differentiate and track multiple zones. The current temperature 
controller function could be generalized to accept a specific zone ID, and a separate thread can be used to control 
each zone independently.

Another useful extension would be to allow thermostat programs based on the time of day. Each program can be scheduled 
to allow for custom heating and cooling at specific ranges of time, on specific days. This would be convenient for those 
who would like to have differing temperatures for waking hours than for sleeping hours as an example. This extension 
could be added by tracking several time ranges, and keeping track of the local time. When a programmed time range is met 
the appropriate thermostat settings can be applied automatically by the API. This feature may belong outside of the API 
but could also be implemented in the API as well. 

Another useful extension could be to support celcius temperatures along with the default farenheit. This would be a 
convenient feature for those living outside of the US, or for those more comfortable with celcius. This would be a simple 
conversion function which could be applied throughout the API.

The API could implement temerature setting range checking; however, I would argue that this doesn't belong in the API. 
I would assume the user interface would only allow the user to adjust the temperature within bounds. To make the API 
more flexible with other thermostat programs, I would argue to leave the range checking at the front end.

Additional error handling could be added once connected to the system to give overall system health.

