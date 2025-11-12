
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1


// Tilakoneen esittely, alustetaan napin painallus falseksi

//enum state { WAITING=1, DATA_READY};
//enum state programState = WAITING;

enum asento { PISTE, VIIVA, VÄLI};
enum asento laiteAsento;
bool napinAsento = false;

// Vasemman napin painallus asettaa napinAsennon falseksi, joka varmistaa, halutaanko tulostaa piste, viiva vai väli.
static void btn1_fxn(uint gpio, uint32_t eventMask) {
    //toggle_led();
    napinAsento = true;
}

/*
static void btn2_fxn(uint gpio, uint32_t eventMask) {
    rgb_led_write(1, 1, 50);
}
*/


void print_asento(void) {
    if (napinAsento == true) {
        if (laiteAsento == VIIVA){
            printf("-");
            napinAsento = false;
        }
        else if (laiteAsento == PISTE){
            printf(".");
            napinAsento = false;
        }
        else if (laiteAsento == VÄLI){
            printf("  ");
        }
        else
            printf("nfaofnoasdfnsdof");
    }
    napinAsento = false;      
}

// ICM Sensorin funktio, joka lukee loopilla kokoajan sensorin tilaa.

void imu_task(void *pvParameters) {
    (void)pvParameters;
    float ax, ay, az, gx, gy, gz, t;
    float a = 0.85;
    // Setting up the sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope\n");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }

    // Start collection data here. Infinite loop.   
    while (1)
    {
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
            
            //printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f| Temp: %2.2f°C\n", ax, ay, az, gx, gy, gz, t);
            if (ay >= a){
                laiteAsento = PISTE;
            }
            else if (ax >= a){
                laiteAsento = VIIVA;
            }
            else if (az >= a){
                laiteAsento = VÄLI;
            }
            else{
            }     
        }   
        else {
            printf("Failed to read imu data\n");
        }
    print_asento();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Tilakone
static void imu_task(void *arg){
    (void)arg;
    
    while(1){
        if (programState == DATA_READY) {
            programState == WAITING;
        }
        //tight_loop_contents();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
*/


int main() {
  
  stdio_init_all();

    while (!stdio_usb_connected()){
        sleep_ms(10);
    }
    
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.
    // Luodaan taski vasemman puoliselle 1. napille.

    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, btn1_fxn);
    init_red_led();

    /*
    // Luodaan taski oikeanpuoliselle 2. napille.
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, btn2_fxn);
    init_rgb_led();
    */


    TaskHandle_t hSensorTask, hPrintTask, hUSB = NULL;

    //  Gyro sensorin taski.
    TaskHandle_t hIMUTask = NULL;
    BaseType_t result;
    result = xTaskCreate(imu_task, "IMUTask", DEFAULT_STACK_SIZE, NULL, 2, &hIMUTask);

    if(result != pdPASS) {
        printf("IMU task creation failed\n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();
    
    // Never reach this line.
    return 0;
}

