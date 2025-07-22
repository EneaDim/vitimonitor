/*
 * Copyright (c) 2024
 * Javad Rahimipetroudi <javad.rahimipetroud@mind.be>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Application-specific fake ADC abstraction header
#include "adc_fake.h"

// Zephyr internal assertion utilities
#include "zephyr/sys/__assert.h"

// Standard C headers
#include <inttypes.h>             // Format specifiers
#include <stddef.h>               // size_t, NULL
#include <stdint.h>               // fixed-width integer types

// Zephyr device and driver headers
#include <zephyr/device.h>        // Device abstraction
#include <zephyr/devicetree.h>    // DeviceTree macros
#include <zephyr/drivers/adc.h>   // ADC driver API
#include <zephyr/kernel.h>        // Kernel (k_sleep, etc.)
#include <zephyr/sys/printk.h>    // printk logging
#include <zephyr/sys/util.h>      // utility macros
#include <zephyr/drivers/adc/adc_emul.h> // ADC emulator driver
//#include <zephyr/ztest.h>       // Unit test framework (commented out)

/* Constants defined from DeviceTree */
#define ADC_REF_INTERNAL_MV	DT_PROP(DT_INST(0, zephyr_adc_emul), ref_internal_mv)
#define ADC_REF_EXTERNAL1_MV	DT_PROP(DT_INST(0, zephyr_adc_emul), ref_external1_mv)

#define ADC_RESOLUTION		14                       // ADC resolution (bits)
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME_DEFAULT    // Default acquisition time
#define ADC_1ST_CHANNEL_ID	0                        // First ADC channel ID
#define ADC_2ND_CHANNEL_ID	1                        // Second ADC channel ID

#define INVALID_ADC_VALUE	SHRT_MIN                 // Invalid sample marker
#define MV_OUTPUT_EPS		2                        // Allowable millivolt error
#define SEQUENCE_STEP		100                      // Sample sequence step

#define BUFFER_SIZE  6                               // ADC sample buffer size
static int16_t m_sample_buffer[BUFFER_SIZE];        // Global ADC sample buffer

/**
 * @brief Setup a single ADC channel
 *
 * @param adc_dev Pointer to ADC device
 * @param ref Reference voltage source (internal/external)
 * @param gain Gain to apply
 * @param channel ADC channel number
 *
 * @return 0 on success or error code
 */
int channel_setup(const struct device *adc_dev, enum adc_reference ref,
                  enum adc_gain gain, int channel)
{
	int ret;

	// Configure ADC channel settings
	struct adc_channel_cfg channel_cfg = {
		.gain             = gain,
		.reference        = ref,
		.acquisition_time = ADC_ACQUISITION_TIME,
		.channel_id       = channel,
	};

	// Apply configuration to ADC driver
	ret = adc_channel_setup(adc_dev, &channel_cfg);

	// Could check result here using zassert_ok() in tests
	return ret;
}

/**
 * @brief Perform an ADC read on selected channels and collect samples
 *
 * @param adc_dev Pointer to ADC device
 * @param channel_mask Bitmask of channels to sample
 * @param len Number of samples to take
 *
 * @return 0 on success or error code
 */
static int start_adc_read(const struct device *adc_dev, uint32_t channel_mask,
                          uint16_t len)
{
	int ret;

	// Optional sequence options for multiple samples
	const struct adc_sequence_options options = {
		.extra_samplings = len - 1,
	};
	const struct adc_sequence_options *options_ptr =
		(len > 1) ? &options : NULL;

	// ADC sequence configuration
	const struct adc_sequence sequence = {
		.options     = options_ptr,
		.channels    = channel_mask,
		.buffer      = m_sample_buffer,
		.buffer_size = sizeof(m_sample_buffer),
		.resolution  = ADC_RESOLUTION,
	};

	// Trigger ADC read
	ret = adc_read(adc_dev, &sequence);
	printk("adc read ret: %d\n", ret);

	return ret;
}

/**
 * @brief Initialize fake ADC device and set initial emulated value
 *
 * @param fake_adc_dev Pointer to fake ADC device structure
 *
 * @return 0 on success or error code
 */
int adc_fake_setup(const fake_adc_dev_t *fake_adc_dev)
{
	int ret;

	// Initialize sample buffer with invalid values
	for (int i = 0; i < BUFFER_SIZE; ++i) {
		m_sample_buffer[i] = INVALID_ADC_VALUE;
	}

	// Setup ADC channel
	ret = channel_setup(fake_adc_dev->adc_dev, ADC_REF_INTERNAL,
	                    ADC_GAIN_1, ADC_1ST_CHANNEL_ID);
	if (ret)
		return ret;

	// Set initial value in ADC emulator
	return adc_emul_const_value_set(fake_adc_dev->adc_dev,
	                                ADC_1ST_CHANNEL_ID,
	                                fake_adc_dev->input_mv);
}

/**
 * @brief Read value from fake ADC, average samples, convert to mV
 *
 * @param fake_adc_dev Pointer to fake ADC device structure
 *
 * @return Average measured value in millivolts, or error code
 */
int adc_fake_read(const fake_adc_dev_t *fake_adc_dev)
{
	int ret;
	int avg_val = 0;

	// Start ADC conversion and read samples
	ret = start_adc_read(fake_adc_dev->adc_dev,
	                     BIT(ADC_1ST_CHANNEL_ID),
	                     fake_adc_dev->nsamples);

	printk("ADC READ RET :%d\n", ret);
	if (ret)
		return ret;

	// Convert raw samples to millivolts and accumulate
	for (int i = 0; i < fake_adc_dev->nsamples; i++) {
		int output = m_sample_buffer[i];
		ret = adc_raw_to_millivolts(ADC_REF_INTERNAL_MV, ADC_GAIN_1,
		                            ADC_RESOLUTION, &output);
		avg_val += output;
	}

	// Return average value
	return (avg_val / fake_adc_dev->nsamples);
}

/**
 * @brief Update the emulated ADC value
 *
 * @param fake_adc_dev Pointer to fake ADC device structure
 *
 * @return 0 on success or error code
 */
int adc_fake_set_value(const fake_adc_dev_t *fake_adc_dev)
{
	return adc_emul_const_value_set(fake_adc_dev->adc_dev,
	                                ADC_1ST_CHANNEL_ID,
	                                fake_adc_dev->input_mv);
}

