#include <Wire.h>

#define LCD_ADDR 0x27
#define SDA_PIN 17
#define SCL_PIN 16

class Componente {
protected:
    int pin;
    String nombre;
public:
    Componente(int p, String n) : pin(p), nombre(n) {}
    virtual ~Componente() {}
    virtual void iniciar() = 0;
    virtual void escribir(int valor) = 0;
    virtual int leer() = 0;
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

class LCD : public Componente {
public:
    LCD(int sda, int scl, String n = "LCD") : Componente(0, n) {
    }
    
    void iniciar() override {
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(100);
        
        // Inicialización LCD
        enviar(0x03, 0); delay(5);
        enviar(0x03, 0); delay(1);
        enviar(0x03, 0); delay(1);
        enviar(0x02, 0);
        enviar(0x28, 0);
        enviar(0x0C, 0);
        enviar(0x06, 0);
        enviar(0x01, 0);
        delay(2);
    }
    
    void escribir(int valor) override {
    }
    
    int leer() override {
        return 0; // LCD no lee
    }
    
    void mostrarInfo(int ldrVal, String modo, int ledsOn) {
        escribirTexto("LDR:" + String(ldrVal) + " Modo:" + modo, 0, 0);
        
        escribirTexto("LEDs On:" + String(ledsOn) + "/8", 0, 1);
    }
    
private:
    void enviar(uint8_t valor, uint8_t modo) {
        uint8_t alta = (valor & 0xF0) | 0x08 | modo;
        uint8_t baja = ((valor << 4) & 0xF0) | 0x08 | modo;
        
        Wire.beginTransmission(LCD_ADDR);
        Wire.write(alta); Wire.write(alta | 0x04); Wire.write(alta);
        Wire.write(baja); Wire.write(baja | 0x04); Wire.write(baja);
        Wire.endTransmission();
        delayMicroseconds(100);
    }
    
    void escribirTexto(String texto, uint8_t col, uint8_t fila) {
        uint8_t addr = (fila == 0) ? 0x80 : 0xC0;
        addr += col;
        enviar(addr, 0);
        
        for (int i = 0; i < texto.length(); i++) {
            enviar(texto.charAt(i), 1);
        }
    }
};

class SensorSimulado : public Componente {
private:
    int valorSimulado;
    
public:
    SensorSimulado(int p, String n = "Sensor") : Componente(p, n), valorSimulado(0) {}
    
    void iniciar() override {
        randomSeed(analogRead(0));
    }
    
    void escribir(int valor) override {
    }
    
    int leer() override {
        valorSimulado = random(300, 701);
        return valorSimulado;
    }
};

class SistemaModos {
private:
    const char* modos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;
    
public:
    SistemaModos() : modoActual(0) {}
    
    String getModoActual() {
        return String(modos[modoActual]);
    }
    
    void cambiarModo() {
        modoActual = (modoActual + 1) % 6;
    }
    
    int getLEDsEncendidos() {
        return random(0, 9);
    }
};

LCD pantalla(17, 16, "Pantalla LCD");
SensorSimulado ldrSimulado(34, "LDR Simulado");
SistemaModos modos;
Componente* componentes[2];

void setup() {
    Serial.begin(115200);
    
    componentes[0] = &pantalla;
    componentes[1] = &ldrSimulado;
    
    for (int i = 0; i < 2; i++) {
        componentes[i]->iniciar();
    }
    
    Serial.println("Sistema LCD listo");
    Serial.println("Mostrando: LDR, Modo, LEDs On");
}

void loop() {
    int valorLDR = ldrSimulado.leer();
    String modo = modos.getModoActual();
    int ledsOn = modos.getLEDsEncendidos();
    
    pantalla.mostrarInfo(valorLDR, modo, ledsOn);
    
    Serial.print("LDR: ");
    Serial.print(valorLDR);
    Serial.print(" | Modo: ");
    Serial.print(modo);
    Serial.print(" | LEDs: ");
    Serial.print(ledsOn);
    Serial.println("/8");
    
    modos.cambiarModo();
    
    delay(3000);
}