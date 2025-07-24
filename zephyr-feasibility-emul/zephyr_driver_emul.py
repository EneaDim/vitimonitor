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

    with open(cmakelists_path, "r") as f:
        lines = f.readlines()

    new_lines = []
    inserted = False
    # Compose relative path for ZEPHYR_EXTRA_MODULES
    extra_modules_path = f"${{CMAKE_SOURCE_DIR}}/{output_folder}/{module_name}"

    for line in lines:
        new_lines.append(line)
        # Insert after cmake_minimum_required line (first occurrence)
        if not inserted and line.strip().startswith("cmake_minimum_required"):
            new_lines.append(f'\nset(ZEPHYR_EXTRA_MODULES "{extra_modules_path}")\n')
            inserted = True

    if not inserted:
        # If cmake_minimum_required not found, add at start
        new_lines.insert(0, f'set(ZEPHYR_EXTRA_MODULES "{extra_modules_path}")\n')

    with open(cmakelists_path, "w") as f:
        f.writelines(new_lines)

    print(f"Updated: {cmakelists_path} (added ZEPHYR_EXTRA_MODULES)")

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
          This is an emulator for the {module_nammodule_namee} sensor.
"""

    c_content = f"""\
/*
 * {module_name}.c
 * Interface: {interface}
 */

#define DT_DRV_COMPAT {module_name}

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER({module_name.split('_')[-1]}, CONFIG_{interface.upper()}_LOG_LEVEL);

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/{interface}.h>
#include <zephyr/drivers/{interface}_emul.h>
#include <zephyr/sys/byteorder.h>
#include <string.h>
#include <errno.h>

#include "{module_name}.h"

struct {module_name}_data {{
    struct {interface}_emul emul;
    const struct {module_name}_cfg *cfg;
    struct device *{interface};
    uint16_t data_raw;
    bool cmd_ready;
    uint8_t cmd_buf[2];
}};

static int {module_name}_transfer(const struct emul *target,
                                 struct {interface}_msg *msgs, int num_msgs, int addr)
{{
    const struct {module_name}_cfg *cfg = target->cfg;
    struct {module_name}_data *data = target->data;

    if (cfg->addr != addr) {{
        LOG_ERR("{interface.upper()} address mismatch");
        return -EIO;
    }}

    // TODO: implement your transfer logic here

    return -EIO;
}}

static int {module_name}_api_set(const struct device *dev, uint16_t data_raw)
{{
    struct {module_name}_data *data = dev->data;
    if (!data) return -EINVAL;

    data->data_raw = data_raw;
    return 0;
}}

static const struct {module_name}_api {module_name}_driver_api = {{
    .set = {module_name}_api_set,
}};

static struct {interface}_emul_api {module_name}_{interface}_api = {{
    .transfer = {module_name}_transfer,
}};

int {module_name}_emul_sample_fetch(const struct emul *emul, float *temp_c, float *hum_pct)
{{
    const struct {module_name}_emul_cfg *cfg = emul->cfg;
    struct {module_name}_emul_data *data = emul->data;

    // TODO

    return 0;
}}

static int {module_name}_init(const struct emul *target, const struct device *parent)
{{
    struct {module_name}_data *data = target->data;

    data->emul.api = &{module_name}_{interface}_api;
    data->emul.addr = ((const struct {module_name}_cfg *)target->cfg)->addr;
    data->emul.target = target;
    data->{interface} = parent;

    data->cmd_ready = false;
    data->data_raw = 0x6666;  // Default dummy temperature

    return 0;
}}

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
        &{module_name}_{interface}_api, &{module_name}_driver_api);

DT_INST_FOREACH_STATUS_OKAY({module_name.upper()}_EMUL)
"""

    h_content = f"""\
#ifndef ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_
#define ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_

#include <zephyr/device.h>
#include <zephyr/drivers/{interface}_emul.h>
#include <zephyr/drivers/emul.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {{
#endif

struct {module_name}_emul_api {{
    int (*set)(const struct device *dev, uint16_t temp_raw, uint16_t hum_raw);
}};

struct {module_name}_emul_cfg {{
    uint16_t addr;
}};

struct {module_name}_emul_data {{
    struct {interface}_emul emul;
    const struct device *{interface};

    uint8_t cmd_buf[2];
    bool cmd_ready;

    uint16_t data_raw;
}};

static inline int {module_name}_emul_set_measurement(const struct device *dev,
                                                    uint16_t data_raw)
{{
    const struct {module_name}_emul_api *api = (const struct {module_name}_emul_api *)dev->api;

    if (!api || !api->set) {{
        return -ENOTSUP;
    }}

    return api->set(dev, data_raw);
}}

int {module_name}_emul_sample_fetch(const struct emul *emul, float *data_c);

#ifdef __cplusplus
}}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_{module_name.upper()}_H_ */
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
    parser.add_argument("-c", "--category", default="sensor", help="Interface type, e.g., i2c, spi")
    parser.add_argument("-o", "--output", default=".", help="Base output directory (default current directory)")

    args = parser.parse_args()

    create_structure(args.output, args.module_name, args.interface, args.category)
    update_root_cmakelists(args.output, args.module_name)
    update_root_prjconf(args.module_name)
if __name__ == "__main__":
    main()

