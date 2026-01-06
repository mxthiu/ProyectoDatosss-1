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

// ============================================================================
// NUEVO: CLASE BOTÓN COMPLETAMENTE IMPLEMENTADA
// ============================================================================

class Boton : public Componente {
private:
    // ATRIBUTOS PARA DEBOUNCE (eliminar falsas pulsaciones)
    bool estadoAnterior;          // Estado previo del botón (HIGH/LOW)
    unsigned long ultimoDebounce; // Tiempo de la última pulsación válida
    
public:
    // Constructores: inicializan variables de debounce
    Boton() : Componente(0, "Boton"), estadoAnterior(HIGH), ultimoDebounce(0) {}
    Boton(int p, String n = "Boton") : Componente(p, n), estadoAnterior(HIGH), ultimoDebounce(0) {}
    
    // MÉTODO: iniciar() - Configura el pin con INPUT_PULLUP (resistencia interna)
    void iniciar() override {
        if (pin != 0) {
            pinMode(pin, INPUT_PULLUP);  // Pin con resistencia pull-up interna
        }
    }
    
    void escribir(int valor) override {}  // No aplica para botones
    
    // MÉTODO: leer() - Lectura directa del pin (HIGH/LOW)
    int leer() override {
        if (pin == 0) return HIGH;  // Si no hay pin configurado
        return digitalRead(pin);
    }
    
    // MÉTODO NUEVO: presionado() - Detecta pulsación con debounce
    // Retorna true solo en el flanco descendente (HIGH → LOW)
    bool presionado() {
        if (pin == 0) return false;  // Verifica que haya pin configurado
        
        bool actual = digitalRead(pin);  // Lee estado actual
        // Detecta flanco descendente (botón presionado: HIGH → LOW)
        bool presion = (estadoAnterior == HIGH && actual == LOW);
        
        // DEBOUNCE: Evita falsas detecciones por ruido eléctrico
        if (presion && (millis() - ultimoDebounce > 50)) {
            estadoAnterior = actual;        // Actualiza estado
            ultimoDebounce = millis();      // Registra tiempo
            return true;                    // Pulsación válida
        }
        
        estadoAnterior = actual;  // Actualiza para siguiente lectura
        return false;             // No hubo pulsación válida
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
// MEJORA: SISTEMA DE MODOS CON CONTROL DE AUTO ACTIVO
// ============================================================================

class SistemaModos {
private:
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;
    bool autoActivo;  // NUEVO: Control independiente para AUTO
    
public:
    // CONSTRUCTOR: Inicia en modo NOCHE con AUTO activo
    SistemaModos() : modoActual(0), autoActivo(true) {}
    
    String getModoActual() {
        return String(nombresModos[modoActual]);
    }
    
    void cambiarModo(int nuevoModo) {
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
        }
    }
    
    // MÉTODO: cambiarModoAuto() - Atajo directo al modo AUTO
    void cambiarModoAuto() {
        modoActual = 4;  // Índice 4 = AUTO
        autoActivo = true;
        Serial.println("Atajo P4: Modo AUTO activado");
    }
    
    // MÉTODO NUEVO: toggleAuto() - Alterna estado del modo AUTO
    void toggleAuto() {
        autoActivo = !autoActivo;
        Serial.print("Auto: ");
        Serial.println(autoActivo ? "ACTIVADO" : "DESACTIVADO");
    }
    
    // MÉTODO: isAutoActivo() - Consulta si AUTO está activo
    bool isAutoActivo() {
        return autoActivo;
    }
    
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE - 20%
                return 51;
            case 1: // LECTURA - 40%
                return 102;
            case 2: // RELAJACIÓN - 30%
                return 76;
            case 3: // FIESTA - Lógica especial
                return 0;
            case 4: // AUTOMÁTICO - Depende del LDR
                return (valorLDR <= UMBRAL_LDR) ? 255 : 0;
            case 5: // MANUAL - Control usuario
                return -1;
            default:
                return 100;
        }
    }
    
    bool esModoManual() { return modoActual == 5; }
    bool esModoAuto() { return modoActual == 4; }
    bool esModoFiesta() { return modoActual == 3; }
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
// SECCIÓN 6: SETUP CON SISTEMA COMPLETO DE COMPONENTES
// ============================================================================

void setup() {
    Serial.begin(115200);
    
    // NUEVO: Información específica del commit 4
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 4: Sistema de botones implementado");
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
    
    // ============================================================================
    // NUEVO: CREACIÓN E INICIALIZACIÓN DE BOTONES
    // ============================================================================
    
    botones[0] = new Boton(4, "P4 AtajoAuto");
    botones[1] = new Boton(26, "Boton C1");
    botones[2] = new Boton(27, "Boton C2");
    botones[3] = new Boton(14, "Boton C3");
    botones[4] = new Boton(2, "Boton Sala");
    botones[5] = new Boton(12, "Boton Cocina");
    botones[6] = new Boton(25, "Boton P.Del");
    botones[7] = new Boton(33, "Boton P.Tras");
    botones[8] = new Boton(32, "Boton P.Int");
    
    // 3. AGREGAR COMPONENTES AL ARRAY POLIMÓRFICO (MEJORADO)
    for (int i = 0; i < 8; i++) {
        componentes[numComponentes++] = leds[i];
    }
    
    componentes[numComponentes++] = &ldr;
    componentes[numComponentes++] = &pot;
    componentes[numComponentes++] = &lcd;
    
    // NUEVO: Agregar botones al sistema polimórfico
    for (int i = 0; i < 9; i++) {
        componentes[numComponentes++] = botones[i];
    }
    
    // ============================================================================
    // NUEVO: INICIALIZACIÓN POLIMÓRFICA DE TODOS LOS COMPONENTES
    // ============================================================================
    
    Serial.println("\n=== INICIALIZANDO COMPONENTES ===");
    for (int i = 0; i < numComponentes; i++) {
        componentes[i]->iniciar();  // Polimorfismo: llama al iniciar() correcto
        Serial.print("- ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" en GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
    // NUEVO: Descripción detallada de los botones
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
    
    // 5. ESTADO INICIAL: Apagar todos los LEDs
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    Serial.println("\n=== SISTEMA LISTO ===");
    Serial.println("Modo MANUAL: Use botones 1-8 para controlar LEDs");
    Serial.println("===================================\n");
}

// ============================================================================
// SECCIÓN 7: LOOP PRINCIPAL CON CONTROL COMPLETO POR BOTONES
// ============================================================================

void loop() {
    // 1. LEER SENSORES
    int valorLDR = ldr.leer();
    bool estaOscuro = ldr.esOscuro();
    
    // 2. ACTUALIZAR MODO CON POTENCIÓMETRO
    int modoPot = pot.getModo();
    modos.cambiarModo(modoPot);
    
    // ============================================================================
    // NUEVO: DETECCIÓN DE BOTÓN P4 (ATAJO AL MODO AUTO)
    // ============================================================================
    
    if (botones[0]->presionado()) {
        modos.cambiarModoAuto();
    }
    
    // ============================================================================
    // NUEVO: CONTROL MANUAL DE LEDs CON BOTONES
    // ============================================================================
    
    if (modos.esModoManual()) {
        // Botones 1-8 controlan LEDs 0-7 respectivamente
        for (int i = 1; i <= 8; i++) {
            if (botones[i]->presionado()) {
                leds[i-1]->alternar();  // Alterna estado del LED correspondiente
                Serial.print("MANUAL - LED ");
                Serial.print(i);
                Serial.print(" (");
                Serial.print(leds[i-1]->getNombre());
                Serial.print("): ");
                Serial.println(leds[i-1]->estaEncendido() ? "ON" : "OFF");
            }
        }
    }
    
    // 5. APLICAR MODO ACTUAL A LEDs (modos automáticos)
    if (!modos.esModoManual()) {
        int brillo = modos.getBrilloPorModo(valorLDR);
        
        if (brillo >= 0) {  // Si no es modo manual (-1)
            if (modos.esModoAuto() || modos.isAutoActivo()) {
                for (int i = 0; i < 8; i++) {
                    leds[i]->setBrillo(brillo);
                }
            }
        }
    }
    
    // 6. CONTAR LEDs ENCENDIDOS (para monitoreo)
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // 7. LOG EN SERIAL (mejorado con estado de AUTO)
    static unsigned long ultimoLog = 0;
    if (millis() - ultimoLog > 1000) {  // Cada 1 segundo (antes 2)
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
    
    delay(50);  // Pausa reducida para mejor respuesta a botones
}
