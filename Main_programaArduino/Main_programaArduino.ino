#include <Wire.h>

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
public:
    LED(int p, String n = "LED") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
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

//  INSTANCIAS 
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
    Serial.println("Commit 1: Estructura base de clases");
    
    // Inicialización básica de LEDs
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    Serial.println("\nEstructura de clases creada:");
    Serial.println("- Clase base: Componente");
    Serial.println("- Clases hijas: LED, SensorLDR, Potenciometro, Boton, PantallaLCD");
    Serial.println("- Métodos: iniciar(), escribir(), leer()");
    Serial.println("===================================\n");
}

void loop() {
    Serial.println("Sistema base funcionando...");
    delay(1000);
}