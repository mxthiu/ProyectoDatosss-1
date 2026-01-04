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
    virtual int leer() = 0;
    virtual void escribir(int valor) = 0;
    
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

class Potenciometro : public Componente {
private:
    int ultimoModo;
    const int MIN_VALOR = 208;
    const int MAX_VALOR = 4095;
    
public:
    Potenciometro(int p, String n = "Pot") 
        : Componente(p, n), ultimoModo(-1) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);
    }
    
    int leer() override {
        return analogRead(pin);
    }
    
    void escribir(int valor) override {}
    
    int getModo() {
        int valor = leer();
        int ajustado = map(valor, MIN_VALOR, MAX_VALOR, 0, 4095);
        ajustado = constrain(ajustado, 0, 4095);
        
        return map(ajustado, 0, 4095, 0, 6);
    }
    
    String getNombreModo() {
        switch(getModo()) {
            case 0: return "NOCHE";
            case 1: return "LECTURA";
            case 2: return "RELAJACION";
            case 3: return "FIESTA";
            case 4: return "AUTOMATICO";
            case 5: return "MANUAL";
            default: return "DESCONOCIDO";
        }
    }
    
    bool cambioModo() {
        int modoActual = getModo();
        if (modoActual != ultimoModo) {
            ultimoModo = modoActual;
            return true;
        }
        return false;
    }
};

class LCD : public Componente {
public:
    LCD(int sda, int scl, String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(100);
        
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
    
    int leer() override { return 0; }
    
    void escribir(int valor) override {}
    
    void mostrarModo(String modo) {
        // Limpiar pantalla
        enviar(0x01, 0);
        delay(2);
        
        escribirTexto("MODO ACTUAL:", 0, 0);
        
        int espacios = (16 - modo.length()) / 2;
        String linea2 = "";
        for (int i = 0; i < espacios; i++) linea2 += " ";
        linea2 += modo;
        
        escribirTexto(linea2, 0, 1);
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


Potenciometro pot(35, "Pot Modos");
LCD pantalla(17, 16, "LCD");

Componente* componentes[2];

void setup() {
    Serial.begin(115200);
    
    componentes[0] = &pot;
    componentes[1] = &pantalla;
    
    for (int i = 0; i < 2; i++) {
        componentes[i]->iniciar();
    }
    
    Serial.println("Potenciometro controla modos en LCD");
    Serial.println("Gira para cambiar entre 6 modos");
    
    pantalla.mostrarModo(pot.getNombreModo());
}

void loop() {
    // Verificar si cambió el modo
    if (pot.cambioModo()) {
        String modoActual = pot.getNombreModo();
        
        pantalla.mostrarModo(modoActual);
        
        Serial.print("Modo cambiado: ");
        Serial.println(modoActual);
    }
    
    delay(100);
}