/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Standard C headers
#include <inttypes.h>            // Format specifiers for integer types
#include <stddef.h>              // Standard definitions (NULL, size_t, etc.)
#include <stdint.h>              // Fixed-width integer types
#include <stdio.h>               // Standard I/O functions

// Zephyr RTOS headers
#include <zephyr/device.h>       // Device driver API
#include <zephyr/devicetree.h>   // DeviceTree macros
#include <zephyr/drivers/adc.h>  // ADC driver API
#include <zephyr/kernel.h>       // Kernel functions (threads, sleep)
#include <zephyr/sys/printk.h>   // printk for logging
#include <zephyr/sys/util.h>     // Utility macros

// Zephyr networking headers
#include <zephyr/net/socket.h>   // BSD socket API
#include <zephyr/net/ethernet.h> // Ethernet networking
#include <zephyr/net/net_mgmt.h> // Network management API

// Application-specific headers
#include "adc_fake.h"            // Fake ADC implementation
#include "eeprom_fake.h"         // Fake EEPROM implementation
#include "zephyr/net/net_ip.h"   // IP address utilities
#include "fixedpoint-pid.h"      // Fixed-point PID controller

// Define ADC device from DeviceTree instance 0
#define ADC_DEVICE_NODE		DT_INST(0, zephyr_adc_emul)

// Global pointer to EEPROM device
static const struct device *eeprom;

// TCP server port
#define BIND_PORT 4242

// Function: Initialize and run a single-threaded TCP echo server over Ethernet
int eth_init()
{
	int opt;                     // Socket option variable
	socklen_t optlen = sizeof(int);
	int serv, ret;
	struct sockaddr_in bind_addr = {
		.sin_family = AF_INET,                // IPv4
		.sin_addr = INADDR_ANY_INIT,         // Bind to all interfaces
		.sin_port = htons(BIND_PORT),        // Bind to BIND_PORT
	};
	static int counter;          // Connection counter

	// Create TCP socket
	serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv < 0) {
		printf("error: socket: %d\n", errno);
		exit(1);
	}

	// Check IPV6_V6ONLY option and disable if needed
	ret = getsockopt(serv, IPPROTO_IP, 0, &opt, &optlen);
	if (ret == 0) {
		if (opt) {
			printf("IPV6_V6ONLY option is on, turning it off.\n");
			opt = 0;
			ret = setsockopt(serv, IPPROTO_IP, 0, &opt, optlen);
			if (ret < 0) {
				printf("Cannot turn off IPV6_V6ONLY option\n");
			} else {
				printf("Sharing same socket between IPv6 and IPv4\n");
			}
		}
	}

	// Bind socket to the specified address and port
	if (bind(serv, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
		printf("error: bind: %d\n", errno);
		exit(1);
	}

	// Start listening for incoming connections
	if (listen(serv, 5) < 0) {
		printf("error: listen: %d\n", errno);
		exit(1);
	}
	printf("Eth init finished\n");
	printf("Single-threaded TCP echo server waits for a connection on port %d...\n", BIND_PORT);

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char addr_str[32];
		int client;

		// Accept a new connection
		client = accept(serv, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client < 0) {
			printf("error: accept: %d\n", errno);
			continue;
		}

		// Convert client address to string and log connection
		inet_ntop(client_addr.sin_family, &client_addr.sin_addr, addr_str, sizeof(addr_str));
		printf("Connection #%d from %s\n", counter++, addr_str);

		while (1) {
			char buf[128], *p;
			int len, out_len;

			// Receive data from client
			len = recv(client, buf, sizeof(buf), 0);
			if (len <= 0) {
				if (len < 0) {
					printf("error: recv: %d\n", errno);
				}
				break; // Client closed connection
			}

			// Echo back received data
			p = buf;
			do {
				out_len = send(client, p, len, 0);
				if (out_len < 0) {
					printf("error: send: %d\n", errno);
					goto error;
				}
				p += out_len;
				len -= out_len;
			} while (len);
		}

error:
		close(client);
		printf("Connection from %s closed\n", addr_str);
	}

	return 0;
}

// Function: Initialize PID controller with fixed parameters
int init_pid_controller(FixedPid *pid) {
	FixedPid_Init(pid);                          // Initialize PID internals
	pid->Dt = Fixed32_FromFloat(0.1);           // Sampling interval
	pid->Max = 100;                             // Max output
	pid->Min = -100;                            // Min output
	pid->Kp  = Fixed32_FromFloat(0.1);          // Proportional gain
	pid->Kd  = Fixed32_FromFloat(0.01);         // Derivative gain
	pid->Ki  = Fixed32_FromFloat(0.5);          // Integral gain

	printf("max=%d    min=%d    dt=%d\n", pid->Max, pid->Min, pid->Dt);
	return 0;
}

// Main function
int main()
{
	int ret = 0;
	uint8_t tbuf[] = {0xAA, 0xBB};                          // Test data to write to EEPROM
	const struct device *const adc_dev = DEVICE_DT_GET(ADC_DEVICE_NODE); // Get ADC device from DT
	eeprom = DEVICE_DT_GET(DT_ALIAS(eeprom_1));             // Get EEPROM device alias

	// Configure fake ADC
	fake_adc_dev_t fake_adc = {
		.adc_dev = adc_dev,
		.input_mv = 1500,      // Initial input in mV
		.nsamples = 5,         // Number of samples
	};

	/******** EEPROM INIT ********/
	size_t size = eeprom_get_size(eeprom);                 // Read EEPROM size
	printk("EEPROM Size: %d\n", size);

	// Write test data to EEPROM
	ret = e2prom_fake_write(eeprom, 0x00, tbuf, sizeof(tbuf));
	printk("EEPROM WRITE ret: %d\n", ret);

	// Read back from EEPROM
	uint8_t rbuf[2] = {0};
	ret = e2prom_fake_read(eeprom, 0x00, rbuf, 2);
	printk("ret: %d, buf: %02x, %02x\n", ret, rbuf[0], rbuf[1]);

	/****** PID Controller init ******/
	FixedPid pid;
	FixedPid_Init(&pid);
	pid.Dt  = Fixed32_FromFloat(0.2);
	pid.Max = 16384;                                       // Max %
	pid.Min = -16384;                                      // Min %
	pid.Kp  = Fixed32_FromFloat(0.1);
	pid.Kd  = Fixed32_FromFloat(0.01);
	pid.Ki  = Fixed32_FromFloat(0.5);

	int ftemp = 2500;                                      // Desired temperature

	/******** ADC INIT *********/
	ret = adc_fake_setup(&fake_adc);                       // Setup ADC
	printk("fake adc ret: %d\n", ret);

	/******** Main Loop ********/
	while (1) {
		// Read current temperature
		int ctemp = adc_fake_read(&fake_adc);
		printk("ADC Read val: %d\n", ctemp);

		// Calculate PID output
		Fixed32 inc = FixedPid_Calculate(&pid, ftemp, ctemp);
		printk("Current Temp: %d, Desired Temp: %d, motor bias: %d\n",
		       ctemp, ftemp, inc);

		// Sleep and update ADC simulated input
		k_msleep(10);
		fake_adc.input_mv = ctemp + inc;
		adc_fake_set_value(&fake_adc);
	}

#if 0U
	// (Disabled) Run Ethernet echo server if needed
	eth_init();
#endif

	return 0;
}

