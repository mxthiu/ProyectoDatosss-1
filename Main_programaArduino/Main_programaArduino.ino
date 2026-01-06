#include <Wire.h>

// ================= CONFIGURACIÓN =================
#define LCD_ADDR 0x27
#define SDA_PIN 17
#define SCL_PIN 16
#define UMBRAL_LDR 400

// ================= CLASE BASE =================
class Componente {
protected:
    int pin;
    String nombre;
    
public:
    Componente(int p, String n) : pin(p), nombre(n) {}
    Componente() : pin(0), nombre("") {}
    virtual ~Componente() {}
    
    // MÉTODOS VIRTUALES PUROS (Polimorfismo obligatorio)
    virtual void iniciar() = 0;
    virtual void escribir(int valor) = 0;
    virtual int leer() = 0;
    
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

// ================= CLASE LED (COMPLETA) =================
class LED : public Componente {
private:
    int brillo;
    bool encendido;
    
public:
    LED(int p, String n = "LED") : Componente(p, n), brillo(0), encendido(false) {}
    
    void iniciar() override {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
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
    }
    
    void apagar() { 
        escribir(0); 
        encendido = false;
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

// ================= CLASE SENSOR LDR (COMPLETA) =================
class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);
        Serial.print("Sensor LDR inicializado en GPIO");
        Serial.println(pin);
    }
    
    void escribir(int valor) override {
        // No aplica para sensor
    }
    
    int leer() override {
        return analogRead(pin);
    }
    
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;  // <= 400 es oscuro
    }
};

// ================= CLASE POTENCIÓMETRO (COMPLETA) =================
class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);
        Serial.print("Potenciómetro inicializado en GPIO");
        Serial.println(pin);
    }
    
    void escribir(int valor) override {}
    
    int leer() override {
        return analogRead(pin);
    }
    
    int getModo() {
        int valor = leer();
        int modo = map(valor, 0, 4095, 0, 6);
        return constrain(modo, 0, 5);
    }
};

// ================= CLASE BOTÓN (ESQUELETO) =================
class Boton : public Componente {
public:
    Boton() : Componente(0, "Boton") {}
    Boton(int p, String n = "Boton") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return HIGH; }
};

// ================= CLASE LCD (ESQUELETO) =================
class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ================= SISTEMA DE MODOS (MEJORADO) =================
class SistemaModos {
private:
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;
    
public:
    SistemaModos() : modoActual(0) {}
    
    String getModoActual() {
        return String(nombresModos[modoActual]);
    }
    
    void cambiarModo(int nuevoModo) {
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
            Serial.print("Modo cambiado a: ");
            Serial.println(getModoActual());
        }
    }
    
    void cambiarModoAuto() {
        modoActual = 4; // Modo AUTO
        Serial.println("Modo AUTO activado");
    }
    
    bool isAutoActivo() { return true; }
    
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE
                return 51; // 20%
            case 1: // LECTURA
                return 102; // 40%
            case 2: // RELAJACIÓN
                return 76; // 30%
            case 3: // FIESTA
                return 0; // Se maneja aparte
            case 4: // AUTOMÁTICO
                return (valorLDR <= UMBRAL_LDR) ? 255 : 0;
            case 5: // MANUAL
                return -1; // Control manual
            default:
                return 100;
        }
    }
    
    bool esModoManual() { return modoActual == 5; }
    bool esModoAuto() { return modoActual == 4; }
    bool esModoFiesta() { return modoActual == 3; }
};

// ================= INSTANCIAS =================
LED* leds[8];
SensorLDR ldr(34, "Sensor Luz");
Potenciometro pot(35, "Pot Modos");
PantallaLCD lcd("Pantalla LCD");
Boton* botones[9];
SistemaModos modos;
Componente* componentes[30];
int numComponentes = 0;

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 3: Sensores LDR y potenciómetro implementados");
    Serial.println("Umbral LDR: <= 400 = OSCURO (LEDs ON)");
    
    // 1. INICIALIZAR LEDs
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    // 2. INICIALIZAR COMPONENTES
    for (int i = 0; i < 8; i++) {
        leds[i]->iniciar();
        componentes[numComponentes++] = leds[i];
    }
    
    ldr.iniciar();
    componentes[numComponentes++] = &ldr;
    
    pot.iniciar();
    componentes[numComponentes++] = &pot;
    
    componentes[numComponentes++] = &lcd;
    
    Serial.println("\n=== SENSORES INICIALIZADOS ===");
    Serial.print("Total componentes: ");
    Serial.println(numComponentes);
    
    // 3. DEMOSTRACIÓN: Apagar todos los LEDs
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    Serial.println("Todos los LEDs apagados");
    Serial.println("===================================\n");
}

// ================= LOOP =================
void loop() {
    // 1. LEER SENSORES
    int valorLDR = ldr.leer();
    bool estaOscuro = ldr.esOscuro();
    int modoPot = pot.getModo();
    
    // 2. ACTUALIZAR MODO
    modos.cambiarModo(modoPot);
    
    // 3. APLICAR MODO A LEDs (excepto MANUAL)
    if (!modos.esModoManual()) {
        int brillo = modos.getBrilloPorModo(valorLDR);
        
        if (brillo >= 0) {
            if (modos.esModoAuto() || modos.isAutoActivo()) {
                for (int i = 0; i < 8; i++) {
                    leds[i]->setBrillo(brillo);
                }
            }
        }
    }
    
    // 4. CONTAR LEDs ENCENDIDOS
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // 5. LOG EN SERIAL
    static unsigned long ultimoLog = 0;
    if (millis() - ultimoLog > 2000) {
        Serial.print("LDR: ");
        Serial.print(valorLDR);
        Serial.print(" | ");
        Serial.print(estaOscuro ? "OSCURO" : "CLARO");
        Serial.print(" (<=400)");
        Serial.print(" | Modo: ");
        Serial.print(modos.getModoActual());
        Serial.print(" | LEDs: ");
        Serial.print(ledsOn);
        Serial.println("/8");
        
        ultimoLog = millis();
    }
    
    delay(100);
}
