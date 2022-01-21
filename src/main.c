/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date January 19 2022
 */

#include "main.h"

void app_main()
{
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL); // install and start i2s driver
    i2s_set_pin(i2s_num, NULL); // for internal DAC, this will enable both of the internal channels
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); // DAC in gpio 25
    i2s_set_sample_rates(i2s_num, SAMPLING_RATE); //set sample rate

    xEvents = xEventGroupCreate();
    xQueueData = xQueueCreate(BUF_NUM_QUEUE,BUF_LEN*sizeof(uint16_t)); 

    xTaskCreatePinnedToCore(vTaskUpdateDACBuffer, "TASK_UPDATE_DAC",   configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-1, &xTaskUpdateDACBuffer, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(vTaskTransmitData,    "TASK_TRANSMIT_DAC", configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-1, &xTaskTransmitData,    APP_CPU_NUM);
    
    xEventGroupSetBits(xEvents, BIT_(TRANSMIT_DATA));
    xEventGroupSetBits(xEvents, BIT_(UPDATE_BUFFER));

    vTaskDelete(NULL);
}

void vTaskUpdateDACBuffer(void * pvParameters)
{
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(UPDATE_BUFFER), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(UPDATE_BUFFER))
        {
            while(xQueueIsQueueFullFromISR(xQueueData)==pdFALSE)
            {
                // ESP_LOGI("update_data", "Data updated!");
                get_wave(&upBuffer, &acc, (uint16_t)60, (uint8_t)0);
                // for(int itr=0; itr<BUF_LEN; itr++) printf("%d ", upBuffer[itr]>>8);
                // printf("\n\n\n");
                // for(int ii=0; ii<BUF_LEN; ii++) upBuffer[ii] = (ii%256) << 8;
                xQueueSend(xQueueData,&upBuffer,portMAX_DELAY);
                // free(upBuffer);
            }
        } 
        vTaskDelay(1);
    }
}

void vTaskTransmitData(void * pvParameters)
{
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(TRANSMIT_DATA), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(TRANSMIT_DATA))
        {
            while(xQueueData!=NULL && xQueueReceive(xQueueData, &txBuffer, portMAX_DELAY)==pdTRUE)
            {
                // ESP_LOGI("transmit_data", "Data transmitted!");
                i2s_write(i2s_num, (void*) txBuffer, BUF_LEN, &bytes_written, portMAX_DELAY);
            }
        } 
        vTaskDelay(1);
    }
}

void get_wave(uint16_t (*data)[BUF_LEN], uint16_t* acc, uint16_t freq, uint8_t wave_id)
{
    uint16_t period;
    float inc, pi=3.14159265359;
    period=SAMPLING_RATE/freq;
    switch(wave_id) 
    {
        case 0: // sine
            for(int itr=0; itr<BUF_LEN; itr++)
            {
                if(*acc>period) *acc=0;
                (*data)[itr] = (uint16_t) ((MAX-MIN)*(sin(2*pi*(float)(*acc)/(float)period)+1)/2 + MIN) << 8;
                (*acc)++;
            }
            break;
        case 1: // square
            for(int itr=0; itr<BUF_LEN; itr++)
            {
                if(*acc>period) *acc=0;
                if(*acc<period/2) (*data)[itr]=((uint16_t)MAX) << 8;
                else if(*acc>period/2) (*data)[itr]=((uint16_t)MIN) << 8;
                else (*data)[itr]=((uint16_t)(MAX-MIN)/2) << 8;
                (*acc)++;
            }
            break;
        case 2: // triangle
            inc=2*(float)(MAX-MIN)/(float)period;
            for(int itr=0; itr<BUF_LEN; itr++)
            {
                if(*acc>period) *acc=0;
                if(*acc<period/2) (*data)[itr]=(MIN+(uint16_t)round((float)(*acc)*inc)) << 8;
                else (*data)[itr]=(MIN+2*(MAX-MIN)-(uint16_t)round((float)(*acc)*inc)) << 8;
                (*acc)++;
            }
            break;
        case 3: // sawtooth
            inc=(float)(MAX-MIN)/(float)period;
            for(int itr=0; itr<BUF_LEN; itr++)
            {
                if(*acc>period) *acc=0;
                (*data)[itr]=(MIN + (uint16_t) round((float)(*acc)*inc)) << 8;
                (*acc)++;
            }
            break;
        default:
            break;
    }
}