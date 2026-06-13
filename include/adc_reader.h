#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>

/* Inicializa el ADC del ESP32 para leer el potenciómetro */
void adc_reader_init(void);

/* Lee el valor crudo del ADC: rango aproximado 0 a 4095 */
uint16_t adc_reader_get_raw(void);

/* Convierte el valor ADC a voltaje aproximado */
float adc_reader_raw_to_voltage(uint16_t raw);

#endif