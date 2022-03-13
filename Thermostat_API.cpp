#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include "Thermostat_API.h"

// Constructor. Set defaults for certain member variables
Thermostat_API::Thermostat_API()
: m_therm_status(therm_status_inactive)
, m_therm_mode(therm_mode_off)
, m_temp_port(1234)
, m_temp(0.0f)
, m_temp_margin(1.0f)
, m_temp_setpoint(72.0f)
, m_is_temp_valid(false)
, m_therm_cont_err(therm_err_no_temp_data)
{
  pthread_create(&m_therm_thread, 0, therm_controller, (void*)this);
}

void* Thermostat_API::therm_controller(void* context)
{
    Thermostat_API* p_this = static_cast<Thermostat_API*>(context);

    while (1)
    {
        float current_temp = 0.0f;
        if (therm_err_no_temp_data == p_this->get_temp(current_temp))
        {
            p_this->m_therm_cont_err = therm_err_no_temp_data;
        }
        else
        {
            // If the thermostate is set to off, make sure it's not heating or cooling
            if (therm_mode_off == p_this->get_therm_mode())
            {
                if (therm_status_heating == p_this->get_therm_status()) 
                {
                    p_this->stop_heating();
                }
                if (therm_status_cooling == p_this->get_therm_status())
                {
                    p_this->stop_cooling();
                }
                else
                {
                    // Do nothing. Already inactive. Not heating or cooling
                }
            }
            // If set to cool eithin a tolerance (auto or cool), start cooling if the temperature
            // is above the setpoint + margin. Only start the AC if it hasn't already been started
            else if (   ((therm_mode_cool == p_this->get_therm_mode()) 
                                || (therm_mode_auto == p_this->get_therm_mode()))
                        && (current_temp > p_this->get_temp_setpoint() + p_this->get_temp_margin()) 
                        && (therm_status_cooling != p_this->get_therm_status()) )
            {
                p_this->start_cooling();
            }
            // If set to heat eithin a tolerance (auto or heat), start heating if the temperature
            // is below the setpoint - margin. Only start the heater if it hasn't already been started
            else if (   ((therm_mode_heat == p_this->get_therm_mode()) 
                                || (therm_mode_auto == p_this->get_therm_mode()))
                        && (current_temp < p_this->get_temp_setpoint() - p_this->get_temp_margin())
                        && (therm_status_heating != p_this->get_therm_status()) )
            {
                p_this->start_heating();
            }
            // If currently heating, and temp is above setpoint, then stop heating
            else if ( (therm_status_heating == p_this->get_therm_status()) 
                      && (current_temp >= p_this->get_temp_setpoint()) )
            {
                p_this->stop_heating();
            }
            // If currently cooling, and temp is below setpoint, then stop cooling
            else if ( (therm_status_cooling == p_this->get_therm_status()) 
                      && (current_temp <= p_this->get_temp_setpoint()) )
            {
                p_this->stop_cooling();
            }
            else
            {
                // Do nothing. No action needed
            }
        }
        // Sleep for a second to avoid busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void* Thermostat_API::temp_UDP_listener(void* context)
{
    Thermostat_API* p_this = static_cast<Thermostat_API*>(context);
    char buffer[4];
    struct sockaddr_in si_other;
    socklen_t addr_size;
    float value;
    while (1)
    {
        // Continuously attempt to receive incoming messages on the UDP socket
        recvfrom(p_this->m_socket_ID, &value, sizeof(float), 0, (struct sockaddr*)& si_other, &addr_size);
        p_this->m_temp = value;
        p_this->m_is_temp_valid = true;
        std::cout << "Information: data received from client: " << std::fixed << std::setprecision(2) << value << std::endl;
    }
}

void Thermostat_API::start_UDP_server()
{
    // Setup the UDP server socket
    struct sockaddr_in server_addr;

    m_socket_ID = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_temp_port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(m_socket_ID, (struct sockaddr*)&server_addr, sizeof(server_addr));
    // Create the UDP temperature message listener thread
    pthread_create(&m_UDP_thread, 0, temp_UDP_listener, (void*)this);
}

void Thermostat_API::start_heating()
{
    // TODO: TURN ON THE HEATER HARDWARE
    m_therm_status = therm_status_heating;
}

void Thermostat_API::stop_heating()
{
    // TODO: TURN OFF THE HEATER HARDWARE
    m_therm_status = therm_status_inactive;
}

void Thermostat_API::start_cooling()
{
    // TODO: TURN ON THE AC HARDWARE
    m_therm_status = therm_status_cooling;
}

void Thermostat_API::stop_cooling()
{
    // TODO: TURN OFF THE AC HARDWARE
    m_therm_status = therm_status_inactive;
}

therm_err Thermostat_API::get_temp(float& temp)
{
    therm_err ret_err = therm_err_none;
    if(m_is_temp_valid)
    {
        temp = m_temp;
    }
    else
    {
        ret_err = therm_err_no_temp_data;
    }
    return ret_err;
}

void Thermostat_API::set_temp_margin(float temp_margin)
{
    m_temp_margin = temp_margin;
}

float Thermostat_API::get_temp_margin()
{
    return m_temp_margin;
}

void Thermostat_API::set_temp_setpoint(float temp_setpoint)
{
    m_temp_setpoint = temp_setpoint;
}

float Thermostat_API::get_temp_setpoint()
{
    return m_temp_setpoint;
}

void Thermostat_API::set_therm_mode(therm_mode setting)
{
    m_therm_mode = setting;
}

therm_mode Thermostat_API::get_therm_mode()
{
    return m_therm_mode;
}

therm_status Thermostat_API::get_therm_status()
{
    return m_therm_status;
}

therm_err Thermostat_API::get_therm_cont_err()
{
    return m_therm_cont_err;
}
