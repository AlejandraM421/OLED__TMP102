/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include <cstring>  // 

// Definiciones de pines y puertos
I2C i2c(D14, D15); // Pines I2C para la NUCLEO-F411RE
Adafruit_SSD1306_I2c oled(i2c, D0); // Pantalla OLED conectada
AnalogIn ain(A0); // Entrada analógica (A0)
BufferedSerial serial(USBTX, USBRX); // Comunicación serial

// Variables globales
float Vin = 0.0;
float Temperatura = 0.0;
int ent = 0;
int dec = 0;
char men[40];

// Implementación manual de strlen()
int my_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Funciones auxiliares
void inicializar_oled(Adafruit_SSD1306_I2c &oled) {
    oled.begin();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.display();
    ThisThread::sleep_for(3000ms); 
    oled.clearDisplay();
    oled.display();
}

void leer_temperatura(I2C &i2c, float &Temperatura, int &ent, int &dec, char *men) {
    char comando[3] = {0x01, 0x60, 0xA0};
    char data[2];
    i2c.write(0x90, comando, 1); 
    i2c.read(0x90, data, 2); 
    int16_t temp = (data[0] << 4) | (data[1] >> 4);
    Temperatura = temp * 0.0625;
    ent = int(Temperatura); 
    dec = int((Temperatura - ent) * 10000);
    sprintf(men, "La Temperatura es:\n\r %01u.%04u Celsius\n\r", ent, dec);
}

void leer_voltaje(AnalogIn &ain, float &Vin, int &ent, int &dec, char *men) {
    Vin = ain.read() * 3.3;
    ent = int(Vin);
    dec = int((Vin - ent) * 10000);
    sprintf(men, "El voltaje es:\n\r %01u.%04u volts\n\r", ent, dec);
}

void visualizar_oled(Adafruit_SSD1306_I2c &oled, const char *men) {
    oled.clearDisplay();
    oled.setTextCursor(0, 2);
    oled.printf(men);
    oled.display();
}

// Hilos

void hilo_lectura_voltaje() {
    while (true) {
        leer_voltaje(ain, Vin, ent, dec, men);
        visualizar_oled(oled, men);
        serial.write((const char*)men, my_strlen(men));  
        ThisThread::sleep_for(1000ms);  
    }
}

void hilo_lectura_temperatura() {
    while (true) {
        leer_temperatura(i2c, Temperatura, ent, dec, men);
        visualizar_oled(oled, men);
        serial.write((const char*)men, my_strlen(men));  // Uso de la función manual de strlen
        ThisThread::sleep_for(2000ms);  // Usamos milisegundos en lugar de "2s"
    }
}

int main() {
    // Inicialización de la pantalla OLED
    inicializar_oled(oled);

    // Saludo por el puerto serial
    const char *mensaje_inicio = "Arranque del programa\n\r";
    serial.write((const char*)mensaje_inicio, my_strlen(mensaje_inicio));  // Uso de la función manual de strlen

    // Crear los hilos para la lectura de voltaje y temperatura
    Thread thread_voltaje;
    Thread thread_temperatura;

    // Iniciar los hilos
    thread_voltaje.start(hilo_lectura_voltaje);
    thread_temperatura.start(hilo_lectura_temperatura);

    // El hilo principal se suspende indefinidamente
    while (true) {
        ThisThread::sleep_for(1000ms);  // Mantiene vivo el hilo principal, usando milisegundos
    }
}
