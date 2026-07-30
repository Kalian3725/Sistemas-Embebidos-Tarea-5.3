# Sistemas-Embebidos-Tarea-5.3

# Descripción

El sistema constituye una integración entre la comunicación serial, el procesamiento concurrente mediante FreeRTOS y la visualización gráfica en una pantalla OLED. El sistema recibe comandos de un solo carácter mediante el monitor serial comandos dados por el usuario y este en procesado mediante una tarea dedicada para así modificar los parámetros internos del sistema según el comando dado, reflejándose esto también en el parpadeo de una LED así como de una gráfica de línea de tiempo mostrada en la pantalla OLED que indica en cuál de las 2 tareas que modifican el estado de la LED se encuentra el sistema.

# Explicación del sistema

La función `setup` inicializa el sistema según los parámetros dados así como la configuración del bus I2C nativo mediante la función `init_i2c_native()`, estableciendo los pines SDA y SCL junto con una velocidad de reloj de 400kHz, y el controlador de la pantalla OLED mediante la función `ssd1306_init((i2c_port_t)I2C_PORT)`, para finalmente crear una cola de comandos capaz de almacenar hasta 10 caracteres mediante `xQueueCreate()`.

Una vez terminada la configuración de los parámetros del sistema, se inicializan las 4 tareas correspondientes al control de la interfaz de la OLED, procesamiento de comandos, recepción de datos mediante UART y el reporte periódico en el mismo orden de prioridad.

La tarea `vTaskUART(void *pvParameters)` lee las entradas dadas por el usuario desde el monitor serial revisando este último cada 50ms mediante `Serial.available`, y cuando detecta una entrada válida, envía este a la cola `commandQueue` mediante la función `xQueueSend`.

La tarea `vTaskProcess(void *pvParameters)` permanece bloqueada mediante `xQueueReceive`, y al momento de recibir un comando, este lo interpreta y ejecuta la acción correspondiente, siendo `'1'` el comando de activación del sistema, `'0'` el comando que desactiva el sistema, y `'T'` el comando que alterna entre la tarea 1 y tarea 2 correspondiente a la LED, alternando la variable `activeTask` entre los valores de `1` y `2` determinando la tarea en la que se encuentra la LED.

La tarea `vTaskReport(void *pvParameters)` verifica cada 2 segundos el valor de la variable `systemActive` y según este, alterna el estado lógico de la LED y a su vez informa por el puerto serial si el sistema se encuentra activo o en pausa.

Finalmente, la tarea `vTaskOLED(void *pvParameters)` es ejecutada cada 100ms y grafica sobre la pantalla OLED líneas horizontales sobre la altura de las etiquetas `T1` y `T2` dependiendo de los elementos que se encuentran en el arreglo `taskHistory`, arreglo que registra 4 actividades y es actualizada constantemente según las entradas del usuario, desplazando sus elementos por las nuevas actividades que vayan surgiendo.

La comunicación entre las tareas se resuelve exclusivamente mediante la cola `commandQueue`, que actúa como mecanismo seguro de paso de datos entre la tarea `vTaskUART`, y la tarea `vTaskProcess`, evitando así condiciones de carrera al desacoplar la recepción de datos crudos de su interpretación lógica, mientras que las variables `systemActive` y `activeTask`, al ser leídas periódicamente por las demás tareas sin necesidad de escritura concurrente compleja.

# Instrucciones de compilación y ejecución.
1. Bajar el repositorio
2. Importar la carpeta del repositorio al area de trabajo de Platform.io
3. Abrir el archivo `diagram.json`
4. Ejecutar la simulación




