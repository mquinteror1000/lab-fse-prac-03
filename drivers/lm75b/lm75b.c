#include "lm75b.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/*
===============================================================================
                    CONFIGURACIÓN DESDE EL DATASHEET
===============================================================================

El alumno debe verificar en el datasheet del LM75B:

1) Dirección del registro de temperatura.
2) Número de bits significativos.
3) Resolución (°C por bit).
4) Formato de número (¿complemento a dos?).

Modificar las constantes.
*/

// Registro donde se almacena la temperatura
#define LM75B_REG_TEMPERATURE 0x00    // <-- Obtener en datasheet

// Resolución en °C por bit
#define LM75B_LSB_C 0.125f             // <-- Obtener en datasheet

/*
===============================================================================
                FUNCIÓN GENÉRICA DE TRANSACCIÓN I2C
===============================================================================

Realiza:
    write(registro) + repeated start + read(N bytes)

Esto es lo típico para leer registros en dispositivos I2C.
*/
static int i2c_write_then_read(int file_descriptor,
                              uint8_t i2c_address,
                              const uint8_t* write_data,
                              uint16_t write_len,
                              uint8_t* read_data,
                              uint16_t read_len)
{
    struct i2c_msg messages[2];

    // Mensaje 1: escribir (selección de registro interno)
    messages[0].addr  = i2c_address;
    messages[0].flags = 0;          // write
    messages[0].len   = write_len;
    messages[0].buf   = (uint8_t*)write_data;

    // Mensaje 2: leer datos del registro seleccionado
    messages[1].addr  = i2c_address;
    messages[1].flags = I2C_M_RD;   // read
    messages[1].len   = read_len;
    messages[1].buf   = read_data;

    struct i2c_rdwr_ioctl_data packet;
    packet.msgs  = messages;
    packet.nmsgs = 2;

    if (ioctl(file_descriptor, I2C_RDWR, &packet) < 0)
        return -errno;

    return 0;
}

/*
===============================================================================
                LECTURA RAW DEL SENSOR (2 BYTES)
===============================================================================

Devuelve el valor bruto tal como viene del registro de temperatura.

El alumno NO debe modificar esta función.
*/
static int lm75b_read_temperature_raw(const char* device_path,
                                      int i2c_address,
                                      uint8_t* msb,
                                      uint8_t* lsb)
{
    int fd = open(device_path, O_RDWR);
    if (fd < 0)
        return -errno;

    uint8_t reg = LM75B_REG_TEMPERATURE;
    uint8_t data[2] = {0};

    int rc = i2c_write_then_read(fd,
                                 (uint8_t)i2c_address,
                                 &reg, 1,
                                 data, 2);

    close(fd);

    if (rc < 0)
        return rc;

    *msb = data[0];
    *lsb = data[1];

    return 0;
}

/*
===============================================================================
            CONVERSIÓN DE RAW A °C (PARTE A COMPLETAR)
===============================================================================

El alumno debe:

1) Combinar MSB y LSB.
2) Determinar cuántos bits son significativos.
3) Aplicar desplazamiento si es necesario.
4) Manejar signo (si aplica).
5) Multiplicar por la resolución correcta.

Consultar el datasheet cuidadosamente.
*/
static float lm75b_raw_to_celsius(uint8_t msb, uint8_t lsb)
{
    /*
    Paso 1:
        Combinar los dos bytes en una variable de 16 bits.
    */
    int16_t raw16 = (msb << 8) | lsb;  // <-- COMPLETAR

    /*
    Paso 2:
        Ajustar bits significativos (¿hay que desplazar? ¿cuántos bits?)
    */
    int16_t valor_significativo = raw16 >> 5;  // <-- COMPLETAR

    /*
    Paso 3:
        Manejar signo si el formato es complemento a dos.
        (¿es necesario extender signo?)
    */

    /*
    Paso 4:
        Convertir a °C usando la resolución.
    */
    float temperatura_c = valor_significativo * LM75B_LSB_C;  // <-- COMPLETAR

    return temperatura_c;
}

/*
===============================================================================
                    FUNCIÓN PRINCIPAL DE LECTURA
===============================================================================

Esta es la función pública que usará la aplicación.

Flujo:
    1) Leer temperatura RAW.
    2) Convertir a °C.
    3) Devolver resultado.
*/
int lm75b_read_temperature_c(const char* device_path,
                             int i2c_address,
                             float* temperature_c)
{
    if (!device_path || !temperature_c)
        return -EINVAL;

    uint8_t msb = 0;
    uint8_t lsb = 0;

    int rc = lm75b_read_temperature_raw(device_path,
                                        i2c_address,
                                        &msb,
                                        &lsb);

    if (rc < 0)
        return rc;

    *temperature_c = lm75b_raw_to_celsius(msb, lsb);

    return 0;
}
