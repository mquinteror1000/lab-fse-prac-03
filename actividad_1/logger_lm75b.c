#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// Inclusión del driver respetando la estructura de directorios
#include "../drivers/lm75b/lm75b.h"

int main() {
    // 1. Obtener una lectura del LM75B mediante el driver
    float temp_c = 0.0f;
    // Se asume el bus /dev/i2c-1 y la dirección 0x48 por defecto
    int rc = lm75b_read_temperature_c("/dev/i2c-1", 0x48, &temp_c);
    
    if (rc < 0) {
        fprintf(stderr, "Error al leer el sensor LM75B: %s\n", strerror(-rc));
        return 1;
    }

    // 2. Obtener la fecha y hora actuales como marca de tiempo Unix (segundos)
    time_t timestamp = time(NULL);

    // Construir la ruta absoluta ~/practica03/actividad_1/lm75b_log.csv
    char filepath[512];
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: No se pudo obtener la variable de entorno HOME.\n");
        return 1;
    }
    snprintf(filepath, sizeof(filepath), "%s/practica03/actividad_1/lm75b_log.csv", home);

    // 3. Abrir el archivo en modo de adición y lectura ("a+")
    FILE *f = fopen(filepath, "a+");
    if (!f) {
        fprintf(stderr, "Error al abrir o crear el archivo %s: %s\n", filepath, strerror(errno));
        return 1;
    }

    // Comprobar el tamaño del archivo para saber si está vacío
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET); // Regresar al inicio para contar líneas si es necesario

    int lecture_num = 0;

    // 4. Escribir el encabezado únicamente si el archivo no existe o está vacío
    if (size == 0) {
        fprintf(f, "timestamp,lecture_num,temperature_c\n");
    } else {
        // Calcular el número de lectura contando las líneas existentes
        char buffer[256];
        int line_count = 0;
        while (fgets(buffer, sizeof(buffer), f)) {
            line_count++;
        }
        // Restar 1 para excluir el encabezado; el conteo inicia en 0
        lecture_num = (line_count > 0) ? line_count - 1 : 0;
    }

    // 5. Agregar una fila con la marca de tiempo, número de lectura y temperatura
    // Se da formato de 3 decimales para coincidir con el ejemplo (ej. 24.375)
    if (fprintf(f, "%ld,%d,%.3f\n", (long)timestamp, lecture_num, temp_c) < 0) {
        fprintf(stderr, "Error al escribir los datos en el archivo.\n");
        fclose(f);
        return 1;
    }

    // 6. Cerrar el archivo y terminar exitosamente
    fclose(f);
    return 0;
}
