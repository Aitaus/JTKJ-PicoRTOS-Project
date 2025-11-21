// Iida Hotti ja Elias Herranen
// Jaetaan 6 pistettä tasan kummankin kesken, koska teimme projektia aina yhdessä.



#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1


// Picon asennot, jotka mitataan ICM sensorilla.
enum asento {PISTE, VIIVA, VÄLI, NEWLINE};

// Picon asennot alustettuna
enum asento laiteAsento;
bool napinAsento = false;

// Napin painaminen asettaa napinAsento:n falseksi, joka varmistaa printtaamisen.
static void btn1_fxn(uint gpio, uint32_t eventMask) {
    napinAsento = true;
}


// Funktio, joka printtaa viivan, välin, pisteen tai newlinen sen mukaan, missä asennossa pico on ja kun nappia on painettu.
void print_asento(void *arg) {
    while(1) {
        if (napinAsento == true) {
            if (laiteAsento == VIIVA){
                printf("-");
            }
            else if (laiteAsento == PISTE){
                printf(".");
            }
            else if (laiteAsento == VÄLI){
                printf(" ");
            }
            else if (laiteAsento == NEWLINE){
                printf("\n");
            }
            else {
                printf("ei onnistunut grrr");
            }
            vTaskDelay(pdMS_TO_TICKS(500));
            napinAsento = false;
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }   
}


// ICM sensorifunktio, joka lukee loopilla kokoajan sensorin tilaa.
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
    }
    else {
        printf("Failed to initialize ICM-42670P.\n");
    }

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
            else if (ay <= -a){
                laiteAsento = NEWLINE;
            }
            else{
            }     
        }   
        else {
            printf("Failed to read imu data\n");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


int main() {
  
  stdio_init_all();
    while (!stdio_usb_connected()){
        sleep_ms(10);
    }
    
    init_hat_sdk();
    sleep_ms(300); // Wait some time so initialization of USB and hat is done.

    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, btn1_fxn);

    BaseType_t result;

    // Taski print_asento funktiolle.
    TaskHandle_t printTask = NULL;
    result = xTaskCreate(print_asento, "print_asento", DEFAULT_STACK_SIZE, NULL, 2, &printTask);

    if (result != pdPASS) {
        printf("Print task creation failed\n");
        return 0;
    }

    // ICM sensorin taski.
    TaskHandle_t hIMUTask = NULL;
    result = xTaskCreate(imu_task, "IMUTask", DEFAULT_STACK_SIZE, NULL, 2, &hIMUTask);

    if (result != pdPASS) {
        printf("IMU task creation failed\n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();
    
    // Never reach this line.
    return 0;
}
