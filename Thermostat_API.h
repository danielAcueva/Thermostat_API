#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

// Track thermostat errors
// TODO (future): Error handling counld be extended
enum therm_err{
    therm_err_none = 0,
    therm_err_no_temp_data,
};

// Thermostat modes of operation. Each will determine the 
// neating/cooling logic to be used. 
// TODO (future): A manual mode could be added in the future
enum therm_mode{
    therm_mode_off = 0,
    therm_mode_heat,
    therm_mode_cool,
    therm_mode_auto
};

// Status of the current status of the temperature controller
enum therm_status{
    therm_status_inactive = 0,
    therm_status_heating,
    therm_status_cooling
};

class Thermostat_API {

public:
Thermostat_API();

~Thermostat_API();

/// Start the UDP server for listening to UDP temperature data messages
/// This function starts the UDP temperature listening thread
/// @return             Nothing (void)
void start_UDP_server();

// TODO (Future): Could track the latest temperature and use an error state if the temperature
// is stale
/// Get the current temperature read from the temperature sampling device
/// @param temp     Floating point temperature value
/// @return         therm_err. None, or temperature_not_valid if no temperature has been read
therm_err get_temp(float& temp);

/// Set the temperature margin which provides an extra margin from the setpoint before 
/// temperature controlling starts. Cooling will start if above the setpoint + margin (assuming
/// cooling or auto mode) and heating will start if below the setpoint - margin (heat/auto mode)
/// @param temp_margin  Floating point temperature margin
/// @return             Nothing (void)
void set_temp_margin(float temp_margin);

/// Return the current temperature margin value
/// @return             floating point temperature margin
float get_temp_margin();

/// Set the temperature setpoint. This is the target value to temperature control to
/// Heating/cooling will stop when the target temperature value is crossed
/// @param temp_setpoint    Target temperature setpoint
/// @return                 Nothing (void)
void set_temp_setpoint(float temp_setpoint);

/// Return the current temperature setpoint
/// @return             floating point temperature setpoint value
float get_temp_setpoint();

/// Set the thermostat mode of operation. See therm_mode enum for options
/// @param setting      therm_mode mode of operation setting
/// @return             Nothing (void)
void set_therm_mode(therm_mode setting);

/// Return the current thermostat mode of operation. See therm_mode enum for options
/// @return             therm_mode thermostat mode of operation
therm_mode get_therm_mode();

/// Get the thermostat temperature control status. see therm_status for options
/// @return             therm_status status of the temperature controlled (heating, cooling, inactive)
therm_status get_therm_status();

/// Get the current temperature controller error. See therm_err for options
/// @return             therm_err thermostat controller error
therm_err get_therm_cont_err();

private:
/// Thread callback function.
/// Thermostat controller function. Setermines what action the temperature controller must take 
/// given the thermostat settings, mode, status, and the current temperature
/// Determine if the temperature controller should be heating, cooling, or if it should be inactive
/// @param context      void* context parameter so the context can be passed into the static thread
///                     function, which will allow access to member functions and member variables
/// @return             void* to comply with thread callback function expectations
static void* therm_controller(void* context);

/// Thread callback function.
/// UDP temperature message listener function. Attemps to receive temperature messages on the server 
/// UDP socket to store temperature sampling device data from the client
/// @param context      void* context parameter so the context can be passed into the static thread
///                     function, which will allow access to member functions and member variables
/// @return             void* to comply with thread callback function expectations
static void* temp_UDP_listener(void* sock_UDP_ID);

/// Turn the heating device on. This is determined by the therm_controller thread callback
/// start the device and set the status
/// @return             Nothing (void)
void start_heating();

/// Turn the heating device off. This is determined by the therm_controller thread callback
/// stop the device and set the status
/// @return             Nothing (void)
void stop_heating();

/// Turn the cooling device on. This is determined by the therm_controller thread callback
/// start the device and set the status
/// @return             Nothing (void)
void start_cooling();

/// Turn the cooling device off. This is determined by the therm_controller thread callback
/// start the device and set the status
/// @return             Nothing (void)
void stop_cooling();

// Member variables
therm_mode m_therm_mode;
therm_status m_therm_status;
int m_temp_port;
int m_socket_ID;
struct sockaddr_in m_server_addr, m_client_addr;
float m_temp;
float m_temp_margin;
float m_temp_setpoint;
bool m_is_temp_valid;
pthread_t m_UDP_thread;
pthread_t m_therm_thread;
therm_err m_therm_cont_err;

};
