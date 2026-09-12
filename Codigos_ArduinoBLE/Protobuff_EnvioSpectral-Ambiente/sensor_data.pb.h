#ifndef SENSOR_DATA_PB_H_INCLUDED
#define SENSOR_DATA_PB_H_INCLUDED
#include "pb.h"

/* Estructura del mensaje según el archivo .proto */
typedef struct {
    char id[16];           // ID del dispositivo
    uint32_t sequence;     // Contador de secuencia
    char modo[16];         // Modo (CALIBRATED, RAW, etc.)
    // Canales Espectrales A-W
    float ch_A; float ch_B; float ch_C; float ch_D; float ch_E; float ch_F;
    float ch_G; float ch_H; float ch_I; float ch_J; float ch_K; float ch_L;
    float ch_R; float ch_S; float ch_T; float ch_U; float ch_V; float ch_W;
    // Temperaturas
    float temp_uv; float temp_vis; float temp_ir;
    // Ambiente
    float ambient_temp; float ambient_hum; float ambient_pres; float ambient_gas;
    // Experimento
    char experiment_id[32];
    char protocol[32];
    char distance_m[16];
} SpectralMeasurement;

/* Inicializador por defecto */
#define SpectralMeasurement_init_default {"", 0, "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0, "", "", ""}

/* Descriptor de campos para el codificador */
#ifdef __cplusplus
extern "C" {
#endif

extern const pb_field_t SpectralMeasurement_fields[32];

#ifdef __cplusplus
}
#endif

#endif