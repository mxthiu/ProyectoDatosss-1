#include <Wire.h>

#define LCD_ADDR 0x27
#define SDA_PIN 17
#define SCL_PIN 16

class Potenciometro {
private:
    int pin;
    int ultimoValor;
    
public:
    Potenciometro(int p) : pin(p), ultimoValor(-1) {}
    
    void iniciar() {
        pinMode(pin, INPUT);
    }
    
    int leer() {
        return analogRead(pin);
    }
    
    bool cambioSignificativo() {
        int actual = leer();
        
        // Cambio si difiere más de 50 unidades
        if (abs(actual - ultimoValor) > 50) {
            ultimoValor = actual;
            return true;
        }
        return false;
    }
    
    String getValorTexto() {
        int val = leer();
        return String(val) + "/4095";
    }
};

void enviarLCD(uint8_t valor, uint8_t modo) {
    uint8_t alta = (valor & 0xF0) | 0x08 | modo;
    uint8_t baja = ((valor << 4) & 0xF0) | 0x08 | modo;
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(alta); Wire.write(alta | 0x04); Wire.write(alta);
    Wire.write(baja); Wire.write(baja | 0x04); Wire.write(baja);
    Wire.endTransmission();
    delayMicroseconds(100);
}

void escribirLCD(String texto, uint8_t col, uint8_t fila) {
    uint8_t addr = (fila == 0) ? 0x80 : 0xC0;
    addr += col;
    enviarLCD(addr, 0);
    
    for (int i = 0; i < texto.length(); i++) {
        enviarLCD(texto.charAt(i), 1);
    }
}

void inicializarLCD() {
    delay(100);
    enviarLCD(0x03, 0); delay(5);
    enviarLCD(0x03, 0); delay(1);
    enviarLCD(0x03, 0); delay(1);
    enviarLCD(0x02, 0);
    enviarLCD(0x28, 0);
    enviarLCD(0x0C, 0);
    enviarLCD(0x06, 0);
    enviarLCD(0x01, 0);
    delay(2);
}

Potenciometro pot(35);  // GPIO35

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    
    pot.iniciar();
    inicializarLCD();
    
    escribirLCD("PRUEBA POT", 3, 0);
    escribirLCD("Gira potenciometro", 0, 1);
    
    Serial.println("Prueba potenciometro GPIO35");
    Serial.println("Conecta: 3.3V -> Pot medio -> GPIO35 -> GND");
}

void loop() {
    int valor = pot.leer();
    
    // Mostrar en LCD
    escribirLCD("Valor: " + pot.getValorTexto(), 0, 0);
    
    // Mostrar barra visual
    int barras = map(valor, 0, 4095, 0, 16);
    String barra = "";
    for (int i = 0; i < 16; i++) {
        barra += (i < barras) ? "#" : ".";
    }
    escribirLCD(barra, 0, 1);
    
    // Mostrar en Serial
    Serial.print("GPIO35: ");
    Serial.print(valor);
    Serial.print("/4095 (");
    Serial.print(map(valor, 0, 4095, 0, 100));
    Serial.println("%)");
    
    delay(200);
}