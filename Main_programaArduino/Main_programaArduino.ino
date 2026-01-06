// ============================================================================
// SECCIÓN 1: CONFIGURACIONES DE HARDWARE Y CONSTANTES
// ============================================================================

#include <Wire.h>  // Biblioteca para comunicación I2C (necesaria para LCD)

// Configuración del hardware - valores fijos del sistema
#define LCD_ADDR 0x27      // Dirección I2C estándar para pantallas LCD
#define SDA_PIN 17         // Pin de datos I2C (Serial Data)
#define SCL_PIN 16         // Pin de reloj I2C (Serial Clock)
#define UMBRAL_LDR 400     // Valor límite para sensor de luz (más oscuro = mayor valor)

// ============================================================================
// SECCIÓN 2: CLASE BASE PRINCIPAL - COMPONENTE 
// ============================================================================

class Componente {
protected:
    int pin;
    String nombre;
    
public:
    Componente(int p, String n) : pin(p), nombre(n) {}
    Componente() : pin(0), nombre("") {}
    virtual ~Componente() {}
    
    virtual void iniciar() = 0;
    virtual void escribir(int valor) = 0;
    virtual int leer() = 0;
    
    String getNombre() { return nombre; }
    int getPin() { return pin; }
};

// ============================================================================
// SECCIÓN 3: CLASES DERIVADAS - IMPLEMENTACIONES COMPLETAS
// ============================================================================

// CLASE: LED (COMPLETA)
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
    
    int leer() override { return brillo; }
    
    void encender() { escribir(255); encendido = true; }
    void apagar() { escribir(0); encendido = false; }
    void alternar() { encendido ? apagar() : encender(); }
    bool estaEncendido() { return encendido; }
    void setBrillo(int valor) { escribir(valor); }
};

// CLASE: SensorLDR (COMPLETA)
class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);
    }
    
    void escribir(int valor) override {}
    
    int leer() override {
        return analogRead(pin);
    }
    
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;
    }
};

// CLASE: Potenciometro (COMPLETA)
class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);
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

// CLASE: Boton (COMPLETA)
class Boton : public Componente {
private:
    bool estadoAnterior;
    unsigned long ultimoDebounce;
    
public:
    Boton() : Componente(0, "Boton"), estadoAnterior(HIGH), ultimoDebounce(0) {}
    Boton(int p, String n = "Boton") : Componente(p, n), estadoAnterior(HIGH), ultimoDebounce(0) {}
    
    void iniciar() override {
        if (pin != 0) {
            pinMode(pin, INPUT_PULLUP);
        }
    }
    
    void escribir(int valor) override {}
    
    int leer() override {
        if (pin == 0) return HIGH;
        return digitalRead(pin);
    }
    
    bool presionado() {
        if (pin == 0) return false;
        
        bool actual = digitalRead(pin);
        bool presion = (estadoAnterior == HIGH && actual == LOW);
        
        if (presion && (millis() - ultimoDebounce > 50)) {
            estadoAnterior = actual;
            ultimoDebounce = millis();
            return true;
        }
        
        estadoAnterior = actual;
        return false;
    }
};

// CLASE: PantallaLCD (ESQUELETO)
class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ============================================================================
// NUEVO: SISTEMA DE MODOS COMPLETAMENTE IMPLEMENTADO
// ============================================================================

class SistemaModos {
private:
    // Nombres de los 6 modos disponibles
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    
    // Estado actual del sistema
    int modoActual;                     // Modo activo (0-5)
    bool autoActivo;                    // Estado del modo AUTO
    unsigned long ultimoCambioFiesta;   // Control de tiempo para modo FIESTA
    int ledFiestaActual;                // LED activo en modo FIESTA
    bool estadoAutoAnterior;            // Estado previo del AUTO (para detección de cambios)
    unsigned long ultimoCambioLED;      // Control de tiempo para cambios en AUTO
    
public:
    // CONSTRUCTOR: Inicializa todas las variables de estado
    SistemaModos() : modoActual(0), autoActivo(true), ultimoCambioFiesta(0), 
                     ledFiestaActual(0), estadoAutoAnterior(false), ultimoCambioLED(0) {}
    
    String getModoActual() {
        return String(nombresModos[modoActual]);
    }
    
    void cambiarModo(int nuevoModo) {
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
        }
    }
    
    void cambiarModoAuto() {
        modoActual = 4; // Índice 4 = AUTO
        autoActivo = true;
        Serial.println("Atajo P4: Modo AUTO activado");
    }
    
    void toggleAuto() {
        autoActivo = !autoActivo;
        Serial.print("Auto: ");
        Serial.println(autoActivo ? "ACTIVADO" : "DESACTIVADO");
    }
    
    bool isAutoActivo() {
        return autoActivo;
    }
    
    // ============================================================================
    // NUEVO: LÓGICA MEJORADA PARA DETECCIÓN DE CAMBIOS EN MODO AUTO
    // ============================================================================
    
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE - 20% brillo fijo
                return 51;
            case 1: // LECTURA - 40% brillo fijo
                return 102;
            case 2: // RELAJACIÓN - 30% brillo fijo
                return 76;
            case 3: // FIESTA - Se maneja separadamente
                return 0;
            case 4: // AUTOMÁTICO - Control inteligente por LDR
            {
                bool oscuroAhora = (valorLDR <= UMBRAL_LDR);
                
                // Detección de cambio de estado con debounce temporal
                if (oscuroAhora != estadoAutoAnterior && (millis() - ultimoCambioLED > 1000)) {
                    estadoAutoAnterior = oscuroAhora;
                    ultimoCambioLED = millis();
                    Serial.print("AUTO (");
                    Serial.print(valorLDR);
                    Serial.print(" <= 400): ");
                    Serial.println(oscuroAhora ? "OSCURO -> LEDs ON" : "CLARO -> LEDs OFF");
                }
                
                return oscuroAhora ? 255 : 0;
            }
            case 5: // MANUAL - Control por usuario
                return -1;
            default:
                return 100;
        }
    }
    
    // ============================================================================
    // NUEVO: MÉTODO PARA MODO FIESTA - ANIMACIÓN SECUENCIAL DE LEDs
    // ============================================================================
    
    void manejarModoFiesta(LED** leds, int totalLEDs) {
        // Cambia de LED cada 200ms para crear efecto de animación
        if (millis() - ultimoCambioFiesta > 200) {
            // Apagar todos los LEDs primero
            for (int i = 0; i < totalLEDs; i++) {
                leds[i]->apagar();
            }
            
            // Encender solo el LED actual de la secuencia
            leds[ledFiestaActual]->encender();
            
            // Avanzar al siguiente LED (cíclico)
            ledFiestaActual = (ledFiestaActual + 1) % totalLEDs;
            ultimoCambioFiesta = millis();
        }
    }
    
    bool esModoFiesta() {
        return modoActual == 3;
    }
    
    bool esModoManual() {
        return modoActual == 5;
    }
    
    bool esModoAuto() {
        return modoActual == 4;
    }
};

// ============================================================================
// SECCIÓN 5: DECLARACIÓN DE OBJETOS GLOBALES
// ============================================================================

LED* leds[8];
SensorLDR ldr(34, "Sensor Luz");
Potenciometro pot(35, "Pot Modos");
PantallaLCD lcd("Pantalla LCD");
Boton* botones[9];
SistemaModos modos;
Componente* componentes[30];
int numComponentes = 0;

// ============================================================================
// SECCIÓN 6: SETUP CON DESCRIPCIÓN COMPLETA DE MODOS
// ============================================================================

void setup() {
    Serial.begin(115200);
    
    // NUEVO: Información específica del commit 5
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 5: Sistema de modos completo implementado");
    Serial.println("Umbral LDR: <= 400 = OSCURO (LEDs ON)");
    
    // 1. INICIALIZAR LEDs (sin cambios)
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    // 2. INICIALIZAR BOTONES (sin cambios)
    botones[0] = new Boton(4, "P4 AtajoAuto");
    botones[1] = new Boton(26, "Boton C1");
    botones[2] = new Boton(27, "Boton C2");
    botones[3] = new Boton(14, "Boton C3");
    botones[4] = new Boton(2, "Boton Sala");
    botones[5] = new Boton(12, "Boton Cocina");
    botones[6] = new Boton(25, "Boton P.Del");
    botones[7] = new Boton(33, "Boton P.Tras");
    botones[8] = new Boton(32, "Boton P.Int");
    
    // 3. AGREGAR COMPONENTES AL ARRAY POLIMÓRFICO
    for (int i = 0; i < 8; i++) {
        componentes[numComponentes++] = leds[i];
    }
    
    componentes[numComponentes++] = &ldr;
    componentes[numComponentes++] = &pot;
    componentes[numComponentes++] = &lcd;
    
    for (int i = 0; i < 9; i++) {
        componentes[numComponentes++] = botones[i];
    }
    
    // 4. INICIALIZAR COMPONENTES
    Serial.println("\n=== INICIALIZANDO COMPONENTES ===");
    for (int i = 0; i < numComponentes; i++) {
        componentes[i]->iniciar();
        Serial.print("- ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" en GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
    // NUEVO: Descripción detallada de botones
    Serial.println("\n=== BOTONES CONFIGURADOS ===");
    Serial.println("P4 (GPIO4): Atajo a Modo AUTO");
    Serial.println("GPIO26: Cuarto 1");
    Serial.println("GPIO27: Cuarto 2");
    Serial.println("GPIO14: Cuarto 3");
    Serial.println("GPIO2: Sala");
    Serial.println("GPIO12: Cocina");
    Serial.println("GPIO25: Patio Delantero");
    Serial.println("GPIO33: Patio Trasero");
    Serial.println("GPIO32: Patio Interior");
    
    // ============================================================================
    // NUEVO: DESCRIPCIÓN COMPLETA DE LOS 6 MODOS IMPLEMENTADOS
    // ============================================================================
    
    Serial.println("\n=== MODOS IMPLEMENTADOS ===");
    Serial.println("NOCHE: 20% (51)");
    Serial.println("LECTURA: 40% (102)");
    Serial.println("RELAJ: 30% (76)");
    Serial.println("FIESTA: Animación secuencial");
    Serial.println("AUTO: Control por LDR (umbral 400)");
    Serial.println("MANUAL: Control por botones");
    
    // 5. ESTADO INICIAL: Apagar todos los LEDs
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    Serial.println("\n=== SISTEMA LISTO ===");
    Serial.println("===================================\n");
}

// ============================================================================
// SECCIÓN 7: LOOP PRINCIPAL CON LÓGICA COMPLETA DE 6 MODOS
// ============================================================================

void loop() {
    // 1. LEER SENSORES
    int valorLDR = ldr.leer();
    bool estaOscuro = ldr.esOscuro();
    
    // 2. ACTUALIZAR MODO CON POTENCIÓMETRO
    int modoPot = pot.getModo();
    modos.cambiarModo(modoPot);
    
    // 3. BOTÓN P4 COMO ATAJO AL MODO AUTO
    if (botones[0]->presionado()) {
        modos.cambiarModoAuto();
    }
    
    // 4. CONTROL MANUAL DE LEDs (solo en modo MANUAL)
    if (modos.esModoManual()) {
        for (int i = 1; i <= 8; i++) {
            if (botones[i]->presionado()) {
                leds[i-1]->alternar();
                Serial.print("MANUAL - LED ");
                Serial.print(i);
                Serial.print(" (");
                Serial.print(leds[i-1]->getNombre());
                Serial.print("): ");
                Serial.println(leds[i-1]->estaEncendido() ? "ON" : "OFF");
            }
        }
    }
    
    // ============================================================================
    // NUEVO: LÓGICA COMPLETA PARA LOS 6 MODOS DE OPERACIÓN
    // ============================================================================
    
    // 5. APLICAR MODO ACTUAL A LEDs
    if (!modos.esModoManual()) {
        if (modos.esModoFiesta()) {
            // MODO FIESTA: Animación secuencial de LEDs
            modos.manejarModoFiesta(leds, 8);
        } else {
            // MODOS NOCHE, LECTURA, RELAJ, AUTO
            int brillo = modos.getBrilloPorModo(valorLDR);
            
            if (brillo >= 0) {
                if (modos.esModoAuto() || modos.isAutoActivo()) {
                    for (int i = 0; i < 8; i++) {
                        leds[i]->setBrillo(brillo);
                    }
                }
            }
        }
    }
    
    // 6. CONTAR LEDs ENCENDIDOS (para monitoreo)
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // lcd.mostrarInfo(valorLDR, modos.getModoActual(), ledsOn);
    
    // 8. LOG EN SERIAL (mejorado)
    static unsigned long ultimoLog = 0;
    if (millis() - ultimoLog > 1000) {
        Serial.print("LDR: ");
        Serial.print(valorLDR);
        Serial.print(" | ");
        Serial.print(estaOscuro ? "OSCURO" : "CLARO");
        Serial.print(" (<=400)");
        Serial.print(" | Modo: ");
        Serial.print(modos.getModoActual());
        Serial.print(" | Auto: ");
        Serial.print(modos.isAutoActivo() ? "ON" : "OFF");
        Serial.print(" | LEDs: ");
        Serial.print(ledsOn);
        Serial.println("/8");
        
        ultimoLog = millis();
    }
    
    delay(50);  // Pausa para estabilidad y respuesta
}
