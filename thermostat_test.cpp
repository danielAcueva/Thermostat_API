#include "Thermostat_API.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

// Global variables for the UDP socket. 
int port_UDP = 1234;
int socket_ID;
struct sockaddr_in server_address;
Thermostat_API* p_test_API = nullptr;

/// Templated function to compare expected values vs. obtained values
/// This function prints to the console (using the test name to identify the test)
/// If the obtained value and expected value match, the test passes
/// Otherwise the test will fail, and the two values will be printed to show the mismatch
/// @param obtained     The value obtained using the API
/// @param expected     The expected value for the test
/// @param test_name    The name of the test for context in the ooutput
/// @return             Nothing (void)
template <class T>
int test_result(T obtained, T expected, std::string test_name)
{
    int test_failure = 0;
    std::cout << "********************************************" << std::endl;
    std::cout << "TEST: " << test_name << std::endl;
    if (obtained == expected)
    {
        std::cout << "**** PASSED" << std::endl;
    }
    else
    {
        std::cout   << "**** FAILED: "
                    << "******** Obtained value: " << obtained
                    << "******** Expected value: " << expected
                    << std::endl;
        test_failure = 1;
    }
    std::cout << std::endl;
    return test_failure;
}

/// Setup a UDP client so we can send mock temperature messages to the API
/// and test the API
/// @return Nothing (void)
void setup_UDP_client()
{
    socket_ID = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&server_address, '\0', sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_UDP);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
}

/// Send a mock temperature message the the API
/// @param temp_value   The temperature value to send to the UDP server (API)
/// @return             Nothing (void)
void send_UDP_temp(float temp_value)
{
    sendto(socket_ID, &temp_value, sizeof(float), 0, (struct sockaddr*)&server_address, sizeof(server_address));
    std::cout << "Information: temperature message sent from client" << std::endl;
}

/// Test use cases when the thermostat is in heat mode
/// Each test case will be described in the test description
/// @param fail_count   Track the number of failed tests. Append to the count if a test fails
/// @return             Nothing (void)
void test_heating_mode(int& fail_count)
{
    std::cout << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "Heating Mode Test Cases" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;
    if (nullptr != p_test_API)
    {
        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");

        // set the temperature to something higher than the setpoint. Verify it's still inactive
        p_test_API->set_temp_setpoint(70.0f);
        p_test_API->set_temp_margin(2.0f);
        send_UDP_temp(72.0f);
        p_test_API->set_therm_mode(therm_mode_heat);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Heat mode enabled. Above setpoint. Verify the thermostat is inactive");

        // Reduce the temperature to below setpoint, but still within margin. Verify inactive
        send_UDP_temp(68.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Heat mode enabled. Below setpoint, but within margin. Verify the thermostat is inactive");

        // Reduce the temperature to below setpoint and margin. Verify heating
        send_UDP_temp(67.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Heat mode enabled. Below setpoint and margin. Verify the thermostat is heating");

        // Increase the temperature to just above margin. Verify still heating
        send_UDP_temp(68.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Heat mode enabled. Below setpoint but above margin. Verify the thermostat still heating");

        // Increase the temperature to just below setpoint. Verify still heating
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Heat mode enabled. Just below setpoint. Verify the thermostat still heating");

        // Increase the temperature to just above setpoint. Verify inactive
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Heat mode enabled. Just above setpoint. Verify the thermostat is inactive");

        // Reduce the temperature below the setpoint but still within margin. Verify still inactive
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Heat mode enabled. Just below setpoint, but within margin. Verify the thermostat is inactive");

        // Reduce the temperature to below setpoint and margin. Verify heating
        send_UDP_temp(67.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Heat mode enabled. Below setpoint and margin. Verify the thermostat is heating");

        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");
    }
    std::cout << std::endl;
}

/// Test use cases when the thermostat is in cool mode
/// Each test case will be described in the test description
/// @param fail_count   Track the number of failed tests. Append to the count if a test fails
/// @return             Nothing (void)
void test_cooling_mode(int& fail_count)
{
    std::cout << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "Cooling Mode Test Cases" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;
    if (nullptr != p_test_API)
    {
        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");

        // set the temperature to something lower than the setpoint. Verify it's still inactive
        p_test_API->set_temp_setpoint(70.0f);
        p_test_API->set_temp_margin(2.0f);
        send_UDP_temp(68.0f);
        p_test_API->set_therm_mode(therm_mode_cool);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Cool mode enabled. Below setpoint. Verify the thermostat is inactive");

        // increase the temperature to above setpoint, but still within margin. Verify inactive
        send_UDP_temp(71.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Cool mode enabled. Above setpoint, but within margin. Verify the thermostat is inactive");

        // Increase the temperature to above setpoint and margin. Verify cooling
        send_UDP_temp(72.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Cool mode enabled. Above setpoint and margin. Verify the thermostat is cooling");

        // Reduce the temperature to just below margin. Verify still cooling
        send_UDP_temp(71.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Cool mode enabled. Above setpoint but below margin. Verify the thermostat still cooling");

        // reduce the temperature to just above setpoint. Verify still cooling
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Cool mode enabled. Just above setpoint. Verify the thermostat still cooling");

        // Reduce the temperature to just below setpoint. Verify inactive
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Cool mode enabled. Just below setpoint. Verify the thermostat is inactive");

        // Increase the temperature above the setpoint but still within margin. Verify still inactive
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Cool mode enabled. Just above setpoint, but within margin. Verify the thermostat is inactive");

        // Increase the temperature to above setpoint and margin. Verify cooling
        send_UDP_temp(72.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Cool mode enabled. Above setpoint and margin. Verify the thermostat is cooling");

        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");
    }
    std::cout << std::endl;
}

/// Test use cases when the thermostat is in auto mode
/// Each test case will be described in the test description
/// @param fail_count   Track the number of failed tests. Append to the count if a test fails
/// @return             Nothing (void)
void test_auto_mode(int& fail_count)
{
    std::cout << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "Auto Mode Test Cases" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;
    if (nullptr != p_test_API)
    {
        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");

        // set the temperature to something higher than the setpoint. Verify it's still inactive
        p_test_API->set_temp_setpoint(70.0f);
        p_test_API->set_temp_margin(2.0f);
        send_UDP_temp(70.0f);
        p_test_API->set_therm_mode(therm_mode_auto);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "auto mode enabled. At setpoint. Verify the thermostat is inactive");

        // Reduce the temperature to below setpoint, but still within margin. Verify inactive
        send_UDP_temp(68.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Below setpoint, but within margin. Verify the thermostat is inactive");

        // Reduce the temperature to below setpoint and margin. Verify heating
        send_UDP_temp(67.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Auto mode enabled. Below setpoint and margin. Verify the thermostat is heating");

        // Increase the temperature to just above margin. Verify still heating
        send_UDP_temp(68.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Auto mode enabled. Below setpoint but above margin. Verify the thermostat still heating");

        // Increase the temperature to just below setpoint. Verify still heating
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Auto mode enabled. Just below setpoint. Verify the thermostat still heating");

        // Increase the temperature to just above setpoint. Verify inactive
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Just above setpoint. Verify the thermostat is inactive");

        // Reduce the temperature below the setpoint but still within margin. Verify still inactive
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Just below setpoint, but within margin. Verify the thermostat is inactive");


        // increase the temperature to above setpoint, but still within margin. Verify inactive
        send_UDP_temp(71.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Above setpoint, but within margin. Verify the thermostat is inactive");

        // Increase the temperature to above setpoint and margin. Verify cooling
        send_UDP_temp(72.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Above setpoint and margin. Verify the thermostat is cooling");

        // Reduce the temperature to just below margin. Verify still cooling
        send_UDP_temp(71.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Above setpoint but below margin. Verify the thermostat still cooling");

        // reduce the temperature to just above setpoint. Verify still cooling
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Just above setpoint. Verify the thermostat still cooling");

        // Reduce the temperature to just below setpoint. Verify inactive
        send_UDP_temp(69.9f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Just below setpoint. Verify the thermostat is inactive");

        // Increase the temperature above the setpoint but still within margin. Verify still inactive
        send_UDP_temp(70.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Just above setpoint, but within margin. Verify the thermostat is inactive");

        // Increase the temperature to above setpoint and margin. Verify cooling
        send_UDP_temp(72.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Above setpoint and margin. Verify the thermostat is cooling");

        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");
    }
    std::cout << std::endl;
}

/// Test use cases that are unusual to verify behavior
/// @param fail_count   Track the number of failed tests. Append to the count if a test fails
/// @return             Nothing (void)
void test_boundary_cases(int& fail_count)
{
    std::cout << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "Boundary Test Cases" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;
    if (nullptr != p_test_API)
    {
        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");

        // set the temperature to something very low. Verify it's in heating mode
        p_test_API->set_temp_setpoint(70.0f);
        p_test_API->set_temp_margin(2.0f);
        send_UDP_temp(30.0f);
        p_test_API->set_therm_mode(therm_mode_auto);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Auto mode enabled. Very low temp. Verify the thermostat is heating");

        // set the temperature to something very low. Verify it's in cooling mode
        send_UDP_temp(100.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Very high temp. Verify the thermostat is cooling");

        // set the temperature to something below the set point, but within margin. Verify it's inactive
        send_UDP_temp(68.5f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Auto mode enabled. Lower than set point but within margin. Verify the thermostat is inactive");

        // Change the margin, so the previous temp is below margin. Verify it's heating
        p_test_API->set_temp_margin(1.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_heating, 
                                   "Auto mode enabled. Same temp, reduce margin. Verify the thermostat is heating");

        // Change the setpoint, so the current temp is above the setpoing. Verify it's cooling
        p_test_API->set_temp_setpoint(60.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_cooling, 
                                   "Auto mode enabled. Same temp, reduce setpoint. Verify the thermostat is cooling");

        // Turn the thermostat off, wait for the controller thread, then verify it's off
        p_test_API->set_therm_mode(therm_mode_off);
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        fail_count += test_result(p_test_API->get_therm_status(), therm_status_inactive, 
                                   "Set the thermostat to off. Verify the thermostat is inactive");
    }
    std::cout << std::endl;
}

int main()
{
    //Create a test API object
    p_test_API = new Thermostat_API();

    if (nullptr == p_test_API)
    {
        std::cout << "Failure to create test API object. All test are aborting!" << std::endl;
    }
    else
    {
        // Start the API UDP server to start listening for temperature messages
        (void)p_test_API->start_UDP_server();

        // Start the test client to mock temperature messages to the API
        setup_UDP_client();

        // Tracks the total number of failed tests
        int test_fail_count = 0;

        // Run test suite for the thermostat in heating mode
        test_heating_mode(test_fail_count);

        // Run test suite for the thermostat in cooling mode
        test_cooling_mode(test_fail_count);

        // Run test suite for the thermostat in auto mode
        test_auto_mode(test_fail_count);

        // Test boundary cases
        test_boundary_cases(test_fail_count);

        std::cout << std::endl;
        std::cout << "|||||||||||||||||||||||||" << std::endl;
        std::cout << "TEST SUMMARY:" << std::endl;
        std::cout << test_fail_count << " TESTS FAILED" << std::endl;
        std::cout << "|||||||||||||||||||||||||" << std::endl;
    }
}
