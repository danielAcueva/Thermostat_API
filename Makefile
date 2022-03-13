all: clean build

build: thermostat_test.cpp Thermostat_API.cpp
	g++ -pthread -o test_therm thermostat_test.cpp Thermostat_API.cpp

.PHONY: clean
clean:
	rm -rf test_therm
