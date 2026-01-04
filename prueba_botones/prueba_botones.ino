#include <Arduino.h>

class Componente {
protected:
    int pin;
    String nombre;
    
public:
    Componente(int p, String n) : pin(p), nombre(n) {}
    virtual ~Componente() {}
    
    virtual void iniciar() = 0;
    virtual void verificar() = 0;
    
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

class Boton : public Componente {
private:
    int numeroBoton;
    bool estadoAnterior;
    int ledAsignado;
    
public:
    Boton(int p, int num, String n = "Boton") 
        : Componente(p, n), numeroBoton(num), estadoAnterior(HIGH), ledAsignado(num) {}
    
    void iniciar() override {
        pinMode(pin, INPUT_PULLUP);
    }
    
    void verificar() override {
        bool estadoActual = digitalRead(pin);
        
        if (estadoAnterior == HIGH && estadoActual == LOW) {
            Serial.print("Boton ");
            Serial.print(numeroBoton);
            Serial.print(" | Pin P");
            Serial.print(pin);
            Serial.print(" | LED Asignado: ");
            Serial.println(ledAsignado);
            delay(50);
        }
        
        estadoAnterior = estadoActual;
    }
    
    int getNumeroBoton() { return numeroBoton; }
};


Boton botones[] = {
    Boton(26, 1, "Boton1_LED1"),  // GPIO26 -> Boton 1 -> LED 1
    Boton(27, 2, "Boton2_LED2"),  // GPIO27 -> Boton 2 -> LED 2
    Boton(14, 3, "Boton3_LED3"),  // GPIO14 -> Boton 3 -> LED 3
    Boton(2,  4, "Boton4_LED4"),  // GPIO2  -> Boton 4 -> LED 4
    Boton(12, 5, "Boton5_LED5"),  // GPIO12 -> Boton 5 -> LED 5
    Boton(25, 6, "Boton6_LED6"),  // GPIO25 -> Boton 6 -> LED 6
    Boton(32, 7, "Boton7_LED7"),  // GPIO32 -> Boton 7 -> LED 7
    Boton(33, 8, "Boton8_LED8")   // GPIO33 -> Boton 8 -> LED 8
};

Componente* componentes[8];

void setup() {
    Serial.begin(115200);
    
    for (int i = 0; i < 8; i++) {
        componentes[i] = &botones[i];
        componentes[i]->iniciar();
        
        Serial.print("Config: ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" | GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
    Serial.println("\nListo - Presiona botones...");
}

void loop() {
    for (int i = 0; i < 8; i++) {
        componentes[i]->verificar();
    }
    delay(10);
}