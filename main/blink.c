///Objective: Slot in MPRLS sensor (uses SLA and SDA connection)
//Standard C library
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//RTOS library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//GPIO library
#include "driver/gpio.h"
//ADC library
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   50          //Multisampling

// init atten variables
static esp_adc_cal_characteristics_t *adc_chars;
// #if CONFIG_IDF_TARGET_ESP32
// static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
// static const adc_bits_width_t width = ADC_WIDTH_BIT_12; //10bit width for ez conversion. 
// #elif CONFIG_IDF_TARGET_ESP32S2
// static const adc_channel_t channel = ADC_CHANNEL_6;     // GPIO7 if ADC1, GPIO17 if ADC2
// static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
// #endif
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

//fsr1
static const adc_channel_t channel1 = ADC_CHANNEL_4;     //GPIO32 
//fsr2
static const adc_channel_t channel2 = ADC_CHANNEL_6;     //GPIO34
//fsr3
static const adc_channel_t channel3 = ADC_CHANNEL_5;     //GPIO33 

// adc init 
static void check_efuse(void)
{
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#elif CONFIG_IDF_TARGET_ESP32S2
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}
static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}
void init()
{
    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten);
        adc1_config_channel_atten(channel2, atten);
        adc1_config_channel_atten(channel3, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel1, atten);
        adc2_config_channel_atten((adc2_channel_t)channel2, atten);
        adc2_config_channel_atten((adc2_channel_t)channel3, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}


//task functions from one_cycle_read
float find_weight1()
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel1);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel1, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
        // vTaskDelay(pdMS_TO_TICKS(100)); //100ms delay - sample 10 -> report every 1s
    }
    adc_reading /= NO_OF_SAMPLES; // find adc_reading average
    //Convert adc_reading to voltage in mV
    double voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    double weight1 = 0.0221*voltage + 26.024;
    if (weight1 < 30.0) return 0.0;
    return (float)weight1;
}
float find_weight2()
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel2);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel2, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
        // vTaskDelay(pdMS_TO_TICKS(100)); //100ms delay - sample 10 -> report every 1s
    }
    adc_reading /= NO_OF_SAMPLES; // find adc_reading average
    //Convert adc_reading to voltage in mV
    double voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    // find resistance over thermistor
    float weight2 = 0.0268*voltage + 34.751;
    if (weight2 < 40.0) return 0.0;
    return weight2;
}
float find_weight3()
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel3);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel3, ADC_WIDTH_BIT_10, &raw);
            adc_reading += raw;
        }
        // vTaskDelay(pdMS_TO_TICKS(100)); //sense every 100ms, 20 samples -> 2s intervals
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    double voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    float weight3 = 0.0263*voltage + 35.473;
    // printf("Raw: %d\tDistance %dmm\n", adc_reading, distance);
    // vTaskDelay(pdMS_TO_TICKS(1000));
    if (weight3 < 40.0) return 0.0;
    return weight3;
}

//task function from app main
void one_cycle_read()
{
    //header for csv/nodejs
    printf("FSR1 weight (g), FSR2 weight (g), FSR3 weight (g)\n");
    while(1)
    {
        float fsr1_weight = find_weight1();
        float fsr2_weight = find_weight2();
        float fsr3_weight = find_weight3();
        printf("%.2f, %.2f, %.2f\n", fsr1_weight, fsr2_weight, fsr3_weight);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();
    //configure each adc
    init();
    //one cycle involves displaying data from each sensor once and printing onto console
    xTaskCreate(one_cycle_read, "one_cycle_read", 4096, NULL, 6, NULL); 
}