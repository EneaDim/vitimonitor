// Application-specific header for fake EEPROM abstraction
#include "eeprom_fake.h"

// Zephyr driver API for EEPROM devices
#include "zephyr/drivers/eeprom.h"

// Standard types header (off_t, size_t, etc.)
#include <sys/types.h>

// Function: Write data to fake EEPROM
// Arguments:
//   fake_e2p_dev - pointer to EEPROM device
//   address      - EEPROM memory offset to write to
//   buf          - pointer to data buffer
//   buf_len      - number of bytes to write
// Returns:
//   0 on success, negative error code on failure
int e2prom_fake_write(const struct device* fake_e2p_dev, const off_t address,
                      const uint8_t* buf, const size_t buf_len)
{
	int ret = 0;

	// Call Zephyr EEPROM write API
	ret = eeprom_write(fake_e2p_dev, address, buf, buf_len);

	return ret;
}

// Function: Read data from fake EEPROM
// Arguments:
//   fake_e2p_dev - pointer to EEPROM device
//   address      - EEPROM memory offset to read from
//   buf          - pointer to buffer to store read data
//   buf_len      - number of bytes to read
// Returns:
//   0 on success, negative error code on failure
int e2prom_fake_read(const struct device *fake_e2p_dev, const off_t address,
                     uint8_t *buf, size_t buf_len)
{
	int ret = 0;

	// Call Zephyr EEPROM read API
	ret = eeprom_read(fake_e2p_dev, address, buf, buf_len);

	return ret;
}

