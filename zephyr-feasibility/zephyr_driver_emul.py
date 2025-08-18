import os
import argparse

def write_file(path, content=""):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write(content)
    print(f"Created: {path}")

def update_root_cmakelists(output_folder, module_name):
    cmakelists_path = os.path.join('./', "CMakeLists.txt")
    if not os.path.isfile(cmakelists_path):
        print(f"Warning: {cmakelists_path} does not exist. Skipping update.")
        return

    extra_path = f"${{CMAKE_SOURCE_DIR}}/{output_folder}/{module_name}"
    with open(cmakelists_path, "r") as f:
        lines = f.readlines()

    # Track state
    in_extra_block = False
    block_start = None
    block_end = None
    already_included = False
    cmake_min_line = None

    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith("cmake_minimum_required"):
            cmake_min_line = i

        if stripped.startswith("set(ZEPHYR_EXTRA_MODULES"):
            in_extra_block = True
            block_start = i
            if extra_path in line:
                already_included = True
        elif in_extra_block:
            if extra_path in stripped:
                already_included = True
            if ")" in stripped:
                block_end = i
                in_extra_block = False

    # If already present, no need to update
    if already_included:
        print(f"ZEPHYR_EXTRA_MODULES already includes '{extra_path}' — nothing to do.")
        return

    # Modify or insert
    if block_start is not None and block_end is not None:
        # Insert before closing parenthesis
        lines.insert(block_end, f'\t"{extra_path}"\n')
        print(f"Added {extra_path} to existing ZEPHYR_EXTRA_MODULES block.")
    else:
        # Insert new block after cmake_minimum_required
        insert_idx = cmake_min_line + 1 if cmake_min_line is not None else 0
        lines.insert(insert_idx, f'\nset(ZEPHYR_EXTRA_MODULES\n\t"{extra_path}"\n)\n')
        print(f"Inserted new ZEPHYR_EXTRA_MODULES block with {extra_path}.")

    with open(cmakelists_path, "w") as f:
        f.writelines(lines)

def update_root_prjconf(module_name):
    kconfig_path = os.path.join('./', "prj.conf")
    config_name = f"CONFIG_{module_name.upper()}=y\n"

    if not os.path.isfile(kconfig_path):
        print(f"Warning: {kconfig_path} does not exist. Creating new one.")
        with open(kconfig_path, "w") as f:
            f.write(f"CONFIG_SENSOR=y\n")  # minimal starter
            f.write(config_name)
        print(f"Created and updated: {kconfig_path}")
        return

    with open(kconfig_path, "r") as f:
        lines = f.readlines()

    if any(config_name in line for line in lines):
        print(f"{config_name} already present in {kconfig_path}")
        return

    new_lines = []
    inserted = False
    for i, line in enumerate(lines):
        new_lines.append(line)
        if not inserted and line.strip() == "CONFIG_SENSOR=y":
            new_lines.append(config_name)
            inserted = True

    if not inserted:
        # CONFIG_SENSOR=y not found, append at end
        new_lines.append("\nCONFIG_SENSOR=y\n")
        new_lines.append(config_name)

    with open(kconfig_path, "w") as f:
        f.writelines(new_lines)

    print(f"Updated: {kconfig_path} (added {config_name} after CONFIG_SENSOR=y)")

def update_native_sim_overlay(module_name, i2c_addr, interface="i2c0"):
    overlay_path = os.path.join('./boards/native_sim.overlay')

    # Prepare node label and compatible string
    node_parts = module_name.split('_')[:-1]
    label = node_parts[1].upper() if len(node_parts) > 1 else node_parts[0].upper()
    node_label = '_'.join(node_parts)
    compat = node_label.replace('_', ',') + "-emul"

    new_node = f"""\
    {node_label}: {node_label}@{i2c_addr} {{
        compatible = "{compat}";
        reg = <0x{i2c_addr}>;
        status = "okay";
        label = "{label}";
    }};
"""

    # If overlay file does not exist, create it
    if not os.path.isfile(overlay_path):
        os.makedirs(os.path.dirname(overlay_path), exist_ok=True)
        with open(overlay_path, "w") as f:
            f.write(f"&{interface} {{\n    status = \"okay\";\n{new_node}}};\n")
        print(f"Created and added node to: {overlay_path}")
        return

    with open(overlay_path, "r") as f:
        lines = f.readlines()

    if f"{node_label}@{i2c_addr}" in ''.join(lines):
        print(f"Node '{node_label}@{i2c_addr}' already present in {overlay_path}")
        return

    new_lines = []
    inside_iface = False
    brace_level = 0
    inserted = False

    for line in lines:
        stripped = line.strip()

        if stripped.startswith(f"&{interface}"):
            inside_iface = True

        if inside_iface:
            brace_level += line.count("{") - line.count("}")
            if brace_level == 0 and not inserted:
                # Right before the closing brace of &i2c0
                new_lines.append(new_node)
                inserted = True

        new_lines.append(line)

    # If interface block not found, append new full block
    if not inserted:
        print(f"Interface '&{interface}' not found. Appending new block at end.")
        new_lines.append(f"\n&{interface} {{\n    status = \"okay\";\n{new_node}}};\n")

    with open(overlay_path, "w") as f:
        f.writelines(new_lines)

    print(f"Updated: {overlay_path} (added node '{node_label}@{i2c_addr}')")

def update_main_c(module_name):
    main_c_path = os.path.join('./src/', "main.c")

    if not os.path.isfile(main_c_path):
        print(f"Warning: {main_c_path} does not exist. Skipping update.")
        return

    with open(main_c_path, "r") as f:
        lines = f.readlines()

    # Determine the correct interval for the sensor from the module name
    driver_name = module_name.split('_')[0]
    sensor_name = module_name.split('_')[1]

    # Define the interval
    interval_define = f"#define {sensor_name.upper()}_INTERVAL_MS   1000\n"
    sensor_define = f"#define {module_name.upper()}_NODE DT_NODELABEL({module_name})\n"

    # Prepare new lines for the update
    new_lines = []
    inserted = False
    inside_driver_includes = False
    inside_constants_section = False
    inside_thread_section = False
    inside_main_function = False

    for line in lines:
        # Insert driver include section after kernel includes
        if not inserted and line.strip().startswith("#include <zephyr/kernel.h>"):
            new_lines.append(line)
            new_lines.append(f"#include \"{module_name}_emul.h\"\n")  # Add the driver include
            inserted = True
            continue

        # Place the interval define in constants section after other defines
        if not inside_constants_section and line.strip().startswith("#define LED_BLINK_INTERVAL_MS"):
            new_lines.append(line)
            new_lines.append(interval_define)  # Add interval for the sensor
            new_lines.append(sensor_define)    # Add sensor node label
            inside_constants_section = True
            continue

        # Insert thread stack and control block setup for the sensor in the thread section
        if not inside_thread_section and line.strip().startswith("K_THREAD_STACK_DEFINE(led_stack"):
            new_lines.append(line)
            new_lines.append(f"K_THREAD_STACK_DEFINE({module_name}_stack, STACK_SIZE);\n")  # Add thread stack for sensor
            new_lines.append(f"static struct k_thread {module_name}_thread_data;\n")  # Add thread control block for sensor
            inside_thread_section = True
            continue
        
        # Insert the sensor thread function definition in the thread section
        if not inside_main_function and line.strip().startswith("int main(void)"):
            new_lines.append(line)
            new_lines.append(f"""
void {module_name}_thread(void *arg1, void *arg2, void *arg3)
{{
    struct sensor_value lux;

    while (1) {{
        if (sensor_sample_fetch({module_name}_dev) == 0 &&
            sensor_channel_get({module_name}_dev, SENSOR_CHAN_LIGHT, &lux) == 0) {{
            float lf = sensor_value_to_double(&lux);
            LOG_INF("{module_name} Light Intensity: %.2f lux", lf);
        }} else {{
            LOG_WRN("Failed to fetch {module_name} sample");
        }}

        k_msleep({sensor_name.upper()}_INTERVAL_MS);
    }}
}}
            """)
            inside_main_function = True
            continue
        
        # Add the main function logic to create the sensor thread
        if line.strip().startswith("int main(void)"):
            new_lines.append(line)
            new_lines.append(f"""
    if (!device_is_ready({module_name}_dev)) {{
        LOG_ERR("{module_name} sensor not ready");
        return 0;
    }}

#ifdef CONFIG_EMUL
    if (!device_is_ready({module_name}_emul->dev)) {{
        LOG_ERR("{module_name} emulator not ready");
        return 0;
    }}
#endif

    k_thread_create(&{module_name}_thread_data, {module_name}_stack, STACK_SIZE,
                    {module_name}_thread, NULL, NULL, NULL,
                    {sensor_name.upper()}_PRIORITY, 0, K_NO_WAIT);
            """)

        # Add the rest of the lines unchanged
        new_lines.append(line)

    # If sections are not found, append them at the end
    if not inserted:
        new_lines.insert(0, f"#include \"{module_name}_emul.h\"\n")  # Add the driver include
    if not inside_constants_section:
        new_lines.append(interval_define)  # Add interval for the sensor
        new_lines.append(sensor_define)    # Add sensor node label
    if not inside_thread_section:
        new_lines.append(f"K_THREAD_STACK_DEFINE({module_name}_stack, STACK_SIZE);\n")
        new_lines.append(f"static struct k_thread {module_name}_thread_data;\n")
    if not inside_main_function:
        new_lines.append(f"""
void {module_name}_thread(void *arg1, void *arg2, void *arg3)
{{
    struct sensor_value lux;

    while (1) {{
        if (sensor_sample_fetch({module_name}_dev) == 0 &&
            sensor_channel_get({module_name}_dev, SENSOR_CHAN_LIGHT, &lux) == 0) {{
            float lf = sensor_value_to_double(&lux);
            LOG_INF("{module_name} Light Intensity: %.2f lux", lf);
        }} else {{
            LOG_WRN("Failed to fetch {module_name} sample");
        }}

        k_msleep({sensor_name.upper()}_INTERVAL_MS);
    }}
}}
        """)

    with open(main_c_path, "w") as f:
        f.writelines(new_lines)

    print(f"Updated: {main_c_path} (added driver and thread for '{module_name}')")

def create_structure(base_path, module_name, interface, category):
    module_path = os.path.join(base_path, module_name)  # module root dir

    # Prepare contents for root files
    cmake_root_content = """\
add_subdirectory(drivers)
zephyr_include_directories(drivers)
"""

    kconfig_root_content = """\
rsource "drivers/Kconfig"
"""

    cmake_drivers_content = f"""\
add_subdirectory_ifdef(CONFIG_{module_name.upper()} {module_name})
"""

    kconfig_drivers_content = f"""\
rsource "{module_name}/Kconfig"
"""

    cmake_emul_content = f"""\
zephyr_library()
zephyr_library_sources({module_name}.c)
zephyr_include_directories(.)
"""

    kconfig_emul_content = f"""\
config {module_name.upper()}
        bool "Emulate {module_name} with {interface} interface"
  default n
        depends on EMUL
        help
          This is an emulator for the {module_name} sensor.
"""

    c_content = f"""\
/*
 * {module_name}.c
 * Interface: {interface}
 */

#define DT_DRV_COMPAT {module_name}  // TODO: assicurati che corrisponda a 'compatible' nel devicetree

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER({'_'.join(module_name.split('_')[-2:])}, CONFIG_{interface.upper()}_LOG_LEVEL);

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/{interface}.h>
#include <zephyr/drivers/{interface}_emul.h>
#include <zephyr/drivers/sensor.h>  // TODO: rimuovi se non è un sensore
#include <zephyr/random/random.h>
#include <string.h>
#include <errno.h>

// -----------------------------------------------------------------------------
// Strutture dati del driver emulato

// TODO: adatta i campi secondo le caratteristiche del tuo dispositivo
struct {module_name}_data {{
    uint16_t raw_data;           // esempio: valore grezzo
    //bool powered_on;
}};

// Configurazione statica
// TODO: estendi se servono altri parametri dal devicetree
struct {module_name}_cfg {{
    uint16_t addr;
}};

// -----------------------------------------------------------------------------
// Funzione di conversione raw → unità fisica (se sensore)

static float raw_to_unit(uint16_t raw)
{{
    // TODO: personalizza la formula secondo il tuo sensore
    return raw / 1.2f;
}}

// -----------------------------------------------------------------------------
// API standard (sensor_driver_api) se usi driver sensor Zephyr

// TODO: rimuovi se non usi il framework sensor

static int {module_name}_sample_fetch(const struct device *dev, enum sensor_channel chan)
{{
    struct {module_name}_data *data = dev->data;
    ARG_UNUSED(chan);

    // Check if powered or not
    //if (!data->powered_on) {{
    //    return -EIO;
    //}}

    data->raw_data = 0x2000 + (sys_rand32_get() % 0x1000);  // TODO: sostituisci con logica realistica
    return 0;
}}

static int {module_name}_channel_get(const struct device *dev,
                                     enum sensor_channel chan,
                                     struct sensor_value *val)
{{
    struct {module_name}_data *data = dev->data;

    // TODO: personalizza il canale
    if (chan != SENSOR_CHAN_LIGHT) {{
        return -EIO;
    }}

    float value = raw_to_unit(data->raw_data);
    sensor_value_from_double(val, value);
    return 0;
}}

static const struct sensor_driver_api {module_name}_driver_api = {{
    .sample_fetch = {module_name}_sample_fetch,
    .channel_get = {module_name}_channel_get,
}};

// -----------------------------------------------------------------------------
// I2C Emulator API

static int {module_name}_transfer(const struct emul *target,
                                  struct {interface}_msg *msgs, int num_msgs, int addr)
{{
    const struct {module_name}_cfg *cfg = target->cfg;
    struct {module_name}_data *data = target->data;

    if (cfg->addr != addr) {{
        return -EIO;
    }}

    // TODO: personalizza la gestione dei comandi I2C

    // Caso: scrittura comando
    if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {{
        uint8_t cmd = msgs[0].buf[0];

        switch (cmd) {{
        case 0x00:  // Power down
            //data->powered_on = false;
            break;
        case 0x01:  // Power on
            //data->powered_on = true;
            break;
        case 0x07:  // Reset
            //if (data->powered_on) {{
            //    data->raw_data = 0;
            //}}
            break;
        case 0x20: case 0x23:  // Modalità misura
            //if (!data->powered_on) return -EIO;
            break;
        default:
            return -EIO;
        }}
        return 0;
    }}

    // Caso: lettura dati (2 byte)
    if (num_msgs == 1 && (msgs[0].flags & I2C_MSG_READ)) {{
        //if (!data->powered_on) return -EIO;
        if (msgs[0].len != 2) return -EIO;

        msgs[0].buf[0] = data->raw_data >> 8;
        msgs[0].buf[1] = data->raw_data & 0xFF;
        return 0;
    }}

    return -EIO;
}}

static struct {interface}_emul_api {module_name}_api = {{
    .transfer = {module_name}_transfer,
}};

// -----------------------------------------------------------------------------
// Inizializzazione dell'emulatore

static int {module_name}_init(const struct emul *target, const struct device *parent)
{{
    struct {module_name}_data *data = target->data;

    //data->powered_on = false;
    data->raw_data = 0x6666;  // TODO: valore iniziale sensato
    return 0;
}}

// -----------------------------------------------------------------------------
// Macro Devicetree per istanziare l’emulatore

#define {module_name.upper()}_EMUL(n) \\
    static struct {module_name}_data {module_name}_data_##n; \\
    static const struct {module_name}_cfg {module_name}_cfg_##n = {{ \\
        .addr = DT_INST_REG_ADDR(n), \\
    }}; \\
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, \\
        &{module_name}_data_##n, &{module_name}_cfg_##n, \\
        POST_KERNEL, I2C_INIT_PRIORITY + 1, &{module_name}_driver_api); \\
    EMUL_DT_INST_DEFINE(n, {module_name}_init, \\
        &{module_name}_data_##n, &{module_name}_cfg_##n, \\
        &{module_name}_api, &{module_name}_driver_api);

DT_INST_FOREACH_STATUS_OKAY({module_name.upper()}_EMUL)
"""

    h_content = f"""\
#ifndef ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_
#define ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_

// -----------------------------------------------------------------------------
// Zephyr core includes

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/{interface}_emul.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {{
#endif

/**
 * @brief {module_name.replace('_', ' ').title()} Emulator API
 *
 * API opzionale per test o manipolazione manuale dell'emulatore.
 */
struct {module_name}_api {{
    // TODO: adatta i parametri del metodo 'set' secondo le esigenze del sensore
    int (*set)(const struct device *dev, uint16_t data_raw);
}};

/**
 * @brief Configurazione statica dell'emulatore (da Devicetree)
 */
struct {module_name}_cfg {{
    uint16_t addr;  ///< Indirizzo I2C assegnato nel devicetree
}};

/**
 * @brief Stato dinamico del dispositivo emulato
 */
struct {module_name}_data {{
    struct {interface}_emul emul;     ///< Struttura base Zephyr per I2C emulator
    const struct device *{interface}; ///< Controller I2C associato

    // TODO: personalizza i campi runtime secondo il tuo dispositivo
    uint16_t data_raw;
}};

/**
 * @brief Imposta un valore RAW simulato per l'emulatore
 *
 * Utile per test automatici (ztest) o simulazioni forzate.
 *
 * @param dev        Puntatore al device emulato
 * @param data_raw   Valore grezzo da iniettare
 * @return 0 se ok, -ENOTSUP se API mancante
 */
static inline int {module_name}_set_raw(const struct device *dev, uint16_t data_raw)
{{
    const struct {module_name}_api *api =
        (const struct {module_name}_api *)dev->api;

    if (!api || !api->set) {{
        return -ENOTSUP;
    }}

    return api->set(dev, data_raw);
}}

/**
 * @brief Simula una lettura e restituisce il valore convertito
 *
 * Può essere invocato direttamente in test, senza driver Zephyr.
 *
 * @param emul   Puntatore all'emulatore
 * @param value  Output: unità fisica simulata (es. lux, °C, %RH, ecc.)
 * @return 0 se ok, errore negativo altrimenti
 */
int {module_name}_sample_fetch(const struct emul *emul, float *value);

#ifdef __cplusplus
}}
#endif

#endif  // ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_
"""

    zephyr_module_yaml_content = f"""\
name: {module_name}
build:
  cmake: .
  kconfig: Kconfig
  settings:
    dts_root: .
"""

    # Write root files
    write_file(os.path.join(module_path, "CMakeLists.txt"), cmake_root_content)
    write_file(os.path.join(module_path, "Kconfig"), kconfig_root_content)

    # Write drivers files
    drivers_path = os.path.join(module_path, "drivers")
    write_file(os.path.join(drivers_path, "CMakeLists.txt"), cmake_drivers_content)
    write_file(os.path.join(drivers_path, "Kconfig"), kconfig_drivers_content)

    # Write module driver files
    emul_path = os.path.join(drivers_path, module_name)
    write_file(os.path.join(emul_path, "CMakeLists.txt"), cmake_emul_content)
    write_file(os.path.join(emul_path, "Kconfig"), kconfig_emul_content)
    write_file(os.path.join(emul_path, f"{module_name}.c"), c_content)
    write_file(os.path.join(emul_path, f"{module_name}.h"), h_content)

    # Write DTS YAML file with your filename rule
    yaml_path = os.path.join(module_path, "dts", "bindings", category)
    parts = module_name.split('_', 1)
    if len(parts) == 2:
        first_part = parts[0]
        rest = parts[1].replace('_', '-')
        yaml_filename = f"{first_part},{rest}.yaml"
    else:
        yaml_filename = f"{module_name}.yaml"

    # dts yaml content
    dts_yaml_content = f"""\
description: Emulator for {module_name}

compatible: "{yaml_filename}"

include: [sensor-device.yaml, {interface}-device.yaml]
"""


    write_file(os.path.join(yaml_path, yaml_filename), dts_yaml_content)

    # Write zephyr module.yaml
    write_file(os.path.join(module_path, "zephyr", "module.yaml"), zephyr_module_yaml_content)


def main():
    parser = argparse.ArgumentParser(description="Create Zephyr driver module structure.")
    parser.add_argument("-m", "--module_name", required=True, help="Name of the module, e.g., sensirion_sht3xd_emul")
    parser.add_argument("-i", "--interface", required=True, help="Interface type, e.g., i2c, spi")
    parser.add_argument("-a", "--address", required=True, help="Address at interface node")
    parser.add_argument("-c", "--category", default="sensor", help="Driver type, e.g., sensor")
    parser.add_argument("-o", "--output", default=".", help="Base output directory (default current directory)")

    args = parser.parse_args()

    create_structure(args.output, args.module_name, args.interface, args.category)
    update_root_cmakelists(args.output, args.module_name)
    update_root_prjconf(args.module_name)
    update_native_sim_overlay(args.module_name, args.address)
    #update_main_c(args.module_name)

if __name__ == "__main__":
    main()

