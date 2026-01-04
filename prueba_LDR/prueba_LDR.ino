#include <Arduino.h>

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
    
    int leer() override { return brillo; }
    
    void encender() { escribir(255); }
    void apagar() { escribir(0); }
    bool estaEncendido() { return estado; }
};

class SensorLDR : public Componente {
private:
    const int LUZ_NORMAL = 700;   // Valor con luz
    const int OSCURO_MAX = 300;   // Máximo valor considerado oscuro
    
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override { 
        pinMode(pin, INPUT); 
    }
    
    void escribir(int valor) override {}
    
    int leer() override { 
        return analogRead(pin); 
    }
    
    bool estaOscuro() {
        return leer() < OSCURO_MAX;
    }
    
    int getPorcentajeLuz() {
        // Convierte 300-700 a 0-100%
        return map(constrain(leer(), OSCURO_MAX, LUZ_NORMAL), 
                   OSCURO_MAX, LUZ_NORMAL, 0, 100);
    }
};

LED led(15, "LED Sala");
SensorLDR ldr(34, "Sensor LDR");
Componente* componentes[2];

void setup() {
    Serial.begin(115200);
    
    componentes[0] = &led;
    componentes[1] = &ldr;
    
    for (int i = 0; i < 2; i++) {
        componentes[i]->iniciar();
    }
    
    Serial.println("Sistema LDR-LED ajustado a tu rango");
    Serial.println("Rango: 300 (oscuridad) - 700 (luz normal)");
    Serial.println("<300 = LED ON | >300 = LED OFF");
}

void loop() {
    int valorLDR = ldr.leer();
    int porcentaje = ldr.getPorcentajeLuz();
    
    if (ldr.estaOscuro()) {
        led.encender();
    } else {
        led.apagar();
    }
    
    Serial.print("LDR: ");
    Serial.print(valorLDR);
    
    Serial.print(" (");
    Serial.print(porcentaje);
    Serial.print("%)");
    
    Serial.print(" | LED: ");
    Serial.print(led.estaEncendido() ? "ON" : "OFF");
    
    if (valorLDR < 100) Serial.print(" [MUY OSCURO]");
    else if (valorLDR < 300) Serial.print(" [OSCURO]");
    else if (valorLDR < 500) Serial.print(" [POCA LUZ]");
    else if (valorLDR < 700) Serial.print(" [LUZ NORMAL]");
    else Serial.print(" [MUCHA LUZ]");
    
    Serial.println();
    
    delay(500);
}