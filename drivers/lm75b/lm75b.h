#ifndef LM75B_H
#define LM75B_H

#ifdef __cplusplus
extern "C" {
#endif

// Lee la temperatura del LM75B en grados Celsius.
// device_path: por ejemplo "/dev/i2c-1"
// i2c_address: por ejemplo 0x48
// temperature_c: salida en °C
int lm75b_read_temperature_c(const char* device_path, int i2c_address, float* temperature_c);

#ifdef __cplusplus
}
#endif

#endif
