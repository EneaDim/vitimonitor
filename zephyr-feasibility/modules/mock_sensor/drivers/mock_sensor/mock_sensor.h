#ifndef MOCK_SENSOR_H
#define MOCK_SENSOR_H

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/**
 * @brief Effettua il campionamento del sensore mock.
 *
 * @param dev Il device del sensore.
 * @param chan Il canale del sensore da campionare (può essere SENSOR_CHAN_ALL).
 * @return 0 se successo, errore altrimenti.
 */
int mock_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan);

/**
 * @brief Ottiene il valore di un canale dal sensore mock.
 *
 * @param dev Il device del sensore.
 * @param chan Il canale da leggere.
 * @param val Puntatore a struct sensor_value dove sarà scritto il valore.
 * @return 0 se successo, errore altrimenti.
 */
int mock_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val);

#endif /* MOCK_SENSOR_H */

