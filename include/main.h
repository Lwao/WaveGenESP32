/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date January 19 2022
 */


#ifndef _MAIN_H_ 
#define _MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "driver/i2s.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

enum events{TRANSMIT_DATA, UPDATE_BUFFER}; 

#define SAMPLING_RATE 10000
#define BUF_LEN 1024
#define BUF_NUM 64
#define BUF_NUM_QUEUE 32
#define BIT_(shift) (1<<shift)
#define MAX 255
#define MIN 0

// freertos variables
TaskHandle_t xTaskUpdateDACBuffer, xTaskTransmitData; 
QueueHandle_t xQueueData; 
EventGroupHandle_t xEvents;

static const int i2s_num = 0; // i2s port number
uint16_t acc=0;

size_t bytes_written; 
uint16_t txBuffer[BUF_LEN], upBuffer[BUF_LEN];
// uint16_t *txBuffer, *upBuffer;

static const i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
    .sample_rate = SAMPLING_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = BUF_NUM,
    .dma_buf_len = BUF_LEN,
    .use_apll = false
};

/**
 * @brief Task to update buffers to TX in DAC
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskUpdateDACBuffer(void * pvParameters);

/**
 * @brief Task to transmit data to built-in 
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskTransmitData(void * pvParameters);

/**
 * @brief Function to return data 
 *
 * @param data pointer to store data
 * @param acc accumulator to index data
 * @param freq frequency of waveform
 * @param wave_id id to choose waveform
 * 
 * @return pointer to waveform generated
 */
void get_wave(uint16_t (*data)[BUF_LEN], uint16_t* acc, uint16_t freq, uint8_t wave_id);



#endif //_MAIN_H_