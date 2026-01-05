#include <Wire.h>

//  CONFIGURACIÓN 
#define LCD_ADDR 0x27
#define SDA_PIN 17
#define SCL_PIN 16
#define UMBRAL_LDR 400

//  CLASE BASE 
class Componente {
protected:
    int pin;
    String nombre;
    
public:
    Componente(int p, String n) : pin(p), nombre(n) {}
    Componente() : pin(0), nombre("") {}
    virtual ~Componente() {}
    
    // MÉTODOS VIRTUALES PUROS
    virtual void iniciar() = 0;
    virtual void escribir(int valor) = 0;
    virtual int leer() = 0;
    
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

//  CLASE LED 
class LED : public Componente {
private:
    int brillo;
    bool encendido;
    
public:
    LED(int p, String n = "LED") : Componente(p, n), brillo(0), encendido(false) {}
    
    void iniciar() override {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        Serial.print("LED inicializado: ");
        Serial.println(nombre);
    }
    
    void escribir(int valor) override {
        brillo = constrain(valor, 0, 255);
        analogWrite(pin, brillo);
        encendido = (brillo > 0);
    }
    
    int leer() override {
        return brillo;
    }
    
    void encender() { 
        escribir(255); 
        encendido = true;
        Serial.print("LED encendido: ");
        Serial.println(nombre);
    }
    
    void apagar() { 
        escribir(0); 
        encendido = false;
        Serial.print("LED apagado: ");
        Serial.println(nombre);
    }
    
    void alternar() {
        if (encendido) {
            apagar();
        } else {
            encender();
        }
    }
    
    bool estaEncendido() { return encendido; }
    void setBrillo(int valor) { escribir(valor); }
};

//  CLASE SENSOR LDR 
class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

//  CLASE POTENCIÓMETRO 
class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

//  CLASE BOTÓN 
class Boton : public Componente {
public:
    Boton() : Componente(0, "Boton") {}
    Boton(int p, String n = "Boton") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return HIGH; }
};

//  CLASE LCD 
class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

//  SISTEMA DE MODOS  
class SistemaModos {
public:
    SistemaModos() {}
    
    String getModoActual() { return "NOCHE"; }
    void cambiarModo(int nuevoModo) {}
    void cambiarModoAuto() {}
    bool isAutoActivo() { return true; }
    int getBrilloPorModo(int valorLDR) { return 0; }
    bool esModoManual() { return false; }
    bool esModoAuto() { return false; }
    bool esModoFiesta() { return false; }
};

LED* leds[8];
SensorLDR ldr(34, "Sensor Luz");
Potenciometro pot(35, "Pot Modos");
PantallaLCD lcd("Pantalla LCD");
Boton* botones[9];
SistemaModos modos;
Componente* componentes[30];
int numComponentes = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 2: Clase LED implementada completamente");
    
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    for (int i = 0; i < 8; i++) {
        leds[i]->iniciar();
        componentes[numComponentes++] = leds[i];
    }
    
    Serial.println("\n=== DEMOSTRACIÓN LEDs ===");
    delay(1000);
    leds[0]->encender();
    delay(500);
    leds[0]->apagar();
    
    leds[1]->setBrillo(128); // 50% brillo
    delay(500);
    leds[1]->apagar();
    
    Serial.println("Demostración completada");
    Serial.println("===================================\n");
}

void loop() {
    // Prueba cíclica de LEDs
    static int ledActual = 0;
    static unsigned long ultimoCambio = 0;
    
    if (millis() - ultimoCambio > 1000) {
        // Apagar todos
        for (int i = 0; i < 8; i++) {
            leds[i]->apagar();
        }
        
        // Encender LED actual
        leds[ledActual]->encender();
        
        Serial.print("LED activo: ");
        Serial.println(leds[ledActual]->getNombre());
        
        // Siguiente LED
        ledActual = (ledActual + 1) % 8;
        ultimoCambio = millis();
    }
    
    delay(100);
}