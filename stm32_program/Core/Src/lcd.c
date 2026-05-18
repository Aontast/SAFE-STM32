#include "lcd.h"

// Mappatura dei pin (Modifica qui se in futuro cambi pin)
#define LCD_PORT GPIOD
#define RS_PIN GPIO_PIN_8
#define EN_PIN GPIO_PIN_9
#define D4_PIN GPIO_PIN_10
#define D5_PIN GPIO_PIN_11
#define D6_PIN GPIO_PIN_12
#define D7_PIN GPIO_PIN_13

// Funzione interna per inviare un "impulso" al pin Enable
void lcd_send_enable() {
    HAL_GPIO_WritePin(LCD_PORT, EN_PIN, 1);
    HAL_Delay(1); // L'LCD ha bisogno di tempo per leggere
    HAL_GPIO_WritePin(LCD_PORT, EN_PIN, 0);
    HAL_Delay(1);
}

// Invia 4 bit di dati all'LCD
void lcd_send_4bits(uint8_t data) {
    HAL_GPIO_WritePin(LCD_PORT, D4_PIN, ((data >> 0) & 0x01));
    HAL_GPIO_WritePin(LCD_PORT, D5_PIN, ((data >> 1) & 0x01));
    HAL_GPIO_WritePin(LCD_PORT, D6_PIN, ((data >> 2) & 0x01));
    HAL_GPIO_WritePin(LCD_PORT, D7_PIN, ((data >> 3) & 0x01));
    lcd_send_enable();
}

void lcd_send_cmd(char cmd) {
    HAL_GPIO_WritePin(LCD_PORT, RS_PIN, 0); // RS=0 per i comandi
    lcd_send_4bits(cmd >> 4);               // Invia i 4 bit più alti
    lcd_send_4bits(cmd & 0x0F);             // Invia i 4 bit più bassi
}

void lcd_send_data(char data) {
    HAL_GPIO_WritePin(LCD_PORT, RS_PIN, 1); // RS=1 per i dati (testo)
    lcd_send_4bits(data >> 4);
    lcd_send_4bits(data & 0x0F);
}

void lcd_init(void) {
    HAL_Delay(50); // Attendi l'accensione dello schermo
    HAL_GPIO_WritePin(LCD_PORT, RS_PIN, 0);
    HAL_GPIO_WritePin(LCD_PORT, EN_PIN, 0);

    // Sequenza magica di inizializzazione a 4-bit per l'HD44780
    lcd_send_4bits(0x03);
    HAL_Delay(5);
    lcd_send_4bits(0x03);
    HAL_Delay(1);
    lcd_send_4bits(0x03);
    HAL_Delay(10);
    lcd_send_4bits(0x02); // Passa alla modalità 4-bit

    // Configurazione finale
    lcd_send_cmd(0x28); // 4-bit mode, 2 righe, font 5x8
    lcd_send_cmd(0x0C); // Display ON, Cursore OFF
    lcd_send_cmd(0x06); // Incrementa il cursore in automatico
    lcd_clear();
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    HAL_Delay(2); // Il clear richiede un po' di tempo in più
}

void lcd_put_cur(int row, int col) {
    switch (row) {
        case 0: col |= 0x80; break; // Riga superiore
        case 1: col |= 0xC0; break; // Riga inferiore
    }
    lcd_send_cmd(col);
}

void lcd_send_string(char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}
