
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1


// tilakoneen esittely

enum state { WAITING=1, DATA_READY};
enum state programState = WAITING;

enum laiteAsento { PISTE, VIIVA, VÄLI};
enum laiteAsento asento = VÄLI;

bool napinAsento = false;

// Napin painallus laittaa valosensorin päälle ja pois.
static void btn_fxn(uint gpio, uint32_t eventMask) {
    toggle_led();
    napinAsento = true;
}

//if

int print_asento(void) {
    if (asento == VIIVA && napinAsento = true){
        printf("-\n");
        napinAsento = false;
    }
    else if (asento == PISTE && napinAsento = true){
        printf(".\n");
        napinAsento = false;
    }
    else
        printf("nfaofnoasdfnsdof");
}


static void sensor_task(void *arg){
    (void)arg;

    for(;;){
        if (programState == WAITING) {
            programState = DATA_READY;
        }
        // Do not remove this
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


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
        /*int _enablegyro = ICM42670_enable_accel_gyro_ln_mode();
        printf ("Enable gyro: %d\n",_enablegyro);
        int _gyro = ICM42670_startGyro(ICM42670_GYRO_ODR_DEFAULT, ICM42670_GYRO_FSR_DEFAULT);
        printf ("Gyro return:  %d\n", _gyro);
        int _accel = ICM42670_startAccel(ICM42670_ACCEL_ODR_DEFAULT, ICM42670_ACCEL_FSR_DEFAULT);
        printf ("Accel return:  %d\n", _accel);*/
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }
    // Start collection data here. Infinite loop. 

    while (1)
    {
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
            
            //printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f| Temp: %2.2f°C\n", ax, ay, az, gx, gy, gz, t);
            if (ay >= a){
                //printf(".\n");
                laiteAsento = PISTE;
            }
            else if (ax >= a){
                //printf("-\n");
                laiteAsento = VIIVA;
            }
            else{
                //printf("Not a clear state\n");
                }

        }
        
        else {
            printf("Failed to read imu data\n");
            }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
static void print_task(void *arg){
    (void)arg;
    
    while(1){
        if (programState == DATA_READY) {
            //printf("Valoisuus: %u \n", ambientLight);
            programState = WAITING;
        }
        //tight_loop_contents();

        // Do not remove this
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
*/


int main() {
  
  stdio_init_all();

    // Uncomment this lines if you want to wait till the serial monitor is connected
    while (!stdio_usb_connected()){
        sleep_ms(10);
    }
    
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, btn_fxn);
    init_red_led();

   TaskHandle_t hSensorTask, hPrintTask, hUSB = NULL;

    //  Luodaan gyro sensori taski.

    printf("Start acceleration test\n");
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

