#include <Arduino.h>

// CLASE BASE
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
};

// CLASE LED
class LED : public Componente {
private:
    int brillo;
    bool estado;

public:
    LED(int p, String n = "LED") : Componente(p, n), brillo(0), estado(false) {}
    
    void iniciar() override {
        pinMode(pin, OUTPUT);
        analogWrite(pin, 0);
    }
    
    void escribir(int valor) override {
        brillo = constrain(valor, 0, 255);
        analogWrite(pin, brillo);
        estado = (brillo > 0);
    }
    
    int leer() override {
        return brillo;
    }
    
    void encender() {
        escribir(255);
    }
    
    void apagar() {
        escribir(0);
    }
    
    void setBrillo(int valor) {
        escribir(valor);
    }
    
    bool estaEncendido() {
        return estado;
    }
};

// INSTANCIAS DE LED
LED* leds[8];

void setup() {
    Serial.begin(115200);
    
    // Crear LEDs
    leds[0] = new LED(15, "Sala");
    leds[1] = new LED(5,  "Cocina");
    leds[2] = new LED(18, "Cuarto1");
    leds[3] = new LED(19, "Cuarto2");
    leds[4] = new LED(21, "Cuarto3");
    leds[5] = new LED(22, "PatioInt");
    leds[6] = new LED(23, "PatioFront");
    leds[7] = new LED(13, "PatioTras");
    
    // Inicializar con polimorfismo
    Componente* componentes[8];
    
    for (int i = 0; i < 8; i++) {
        componentes[i] = leds[i];
        componentes[i]->iniciar();
    }
    
    Serial.println("Sistema LED iniciado");
}

void loop() {
    // Prueba: encender todos uno por uno
    for (int i = 0; i < 8; i++) {
        leds[i]->encender();
        Serial.print("LED ");
        Serial.print(i + 1);
        Serial.print(" (");
        Serial.print(leds[i]->getNombre());
        Serial.println(") ENCENDIDO");
        delay(500);
    }
    
    delay(1000);
    
    // Apagar todos
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
        Serial.print("LED ");
        Serial.print(i + 1);
        Serial.print(" (");
        Serial.print(leds[i]->getNombre());
        Serial.println(") APAGADO");
        delay(300);
    }
    
    delay(1000);
}