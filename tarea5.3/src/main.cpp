#include <Arduino.h>
#include <driver/i2c.h>
extern "C" {
    #include "ssd1306.h"
}

#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define LED_PIN 2

QueueHandle_t commandQueue;
TaskHandle_t TaskOLEDHandle;

bool systemActive = true;
int activeTask = 1;
int timeTick = 0;

void init_i2c_native() {
    i2c_config_t i2c_conf = {};
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_SDA_PIN;
    i2c_conf.scl_io_num = I2C_SCL_PIN;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = 400000;

    i2c_param_config(I2C_PORT, &i2c_conf);
    i2c_driver_install(I2C_PORT, i2c_conf.mode, 0, 0, 0);
}

void vTaskUART(void *pvParameters) {
    char cmd;
    for (;;) {
        if (Serial.available() > 0) {
            cmd = Serial.read();
            if (cmd != '\n' && cmd != '\r') {
                xQueueSend(commandQueue, &cmd, portMAX_DELAY);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vTaskProcess(void *pvParameters) {
    char receivedCmd;
    for (;;) {
        if (xQueueReceive(commandQueue, &receivedCmd, portMAX_DELAY) == pdPASS) {
            Serial.print("Comando recibido: ");
            Serial.println(receivedCmd);

            switch (receivedCmd) {
                case '1':
                    systemActive = true;
                    Serial.println("Sistema ACTIVADO");
                    break;
                case '0':
                    systemActive = false;
                    Serial.println("Sistema DESACTIVADO");
                    break;
                case 'T':
                    activeTask = (activeTask == 1) ? 2 : 1; 
                    Serial.print("Cambio a la Tarea: ");
                    Serial.println(activeTask);
                    break;
                default:
                    Serial.println("Comando desconocido. Usa '1', '0' o 'T'.");
                    break;
            }
        }
    }
}

void vTaskReport(void *pvParameters) {
    pinMode(LED_PIN, OUTPUT);
    for (;;) {
        if (systemActive) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            Serial.println("[Estado] Sistema Operativo y funcionando.");
        } else {
            digitalWrite(LED_PIN, LOW);
            Serial.println("[Estado] Sistema en pausa.");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vTaskOLED(void *pvParameters) {
    ssd1306_clear();
    
    const int task1_y = 15;
    const int task2_y = 35;
    const int axis_y = 55;
    
    int taskHistory[4] = {1, 1, 1, 1};

    for (;;) {
        ssd1306_clear();

        ssd1306_draw_string(0, task1_y - 3, "T1");
        ssd1306_draw_string(0, task2_y - 3, "T2");
        ssd1306_draw_string(40, axis_y + 2, "Time");
        ssd1306_draw_hline(20, 120, axis_y, true);

        timeTick++;
        if (timeTick % 10 == 0) { 
            for (int i = 0; i < 3; i++) {
                taskHistory[i] = taskHistory[i + 1];
            }
            taskHistory[3] = activeTask;
        }

        int draw_x = 20;
        
        for (int i = 0; i < 4; i++) {
            int next_x = draw_x + 25;
            int current_state = taskHistory[i];

            ssd1306_draw_vline_dashed(draw_x, 5, axis_y);

            if (current_state == 1) {
                ssd1306_draw_hline(draw_x, next_x, task1_y, true);
                
                if (i > 0 && taskHistory[i - 1] == 2) {
                    ssd1306_draw_vline(draw_x, task1_y, task2_y, true);
                }
            } else {
                ssd1306_draw_hline(draw_x, next_x, task2_y, true);
                
                if (i > 0 && taskHistory[i - 1] == 1) {
                    ssd1306_draw_vline(draw_x, task1_y, task2_y, true);
                }
            }
            
            draw_x = next_x;
        }

        ssd1306_update();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    init_i2c_native();
    ssd1306_init((i2c_port_t)I2C_PORT);
    commandQueue = xQueueCreate(10, sizeof(char));

    if (commandQueue != NULL) {
        xTaskCreate(vTaskUART, "UART_Task", 2048, NULL, 1, NULL);
        xTaskCreate(vTaskProcess, "Process_Task", 2048, NULL, 2, NULL);
        xTaskCreate(vTaskReport, "Report_Task", 2048, NULL, 1, NULL);
        xTaskCreate(vTaskOLED, "OLED_Task", 4096, NULL, 3, &TaskOLEDHandle);
    }
}

void loop() {}