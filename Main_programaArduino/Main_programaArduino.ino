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
// SECCIÓN 2: CLASE BASE PRINCIPAL - COMPONENTE (SIN CAMBIOS)
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

// CLASE: LED (COMPLETA) - SIN CAMBIOS SIGNIFICATIVOS
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

// ============================================================================
// NUEVO: CLASE SENSOR LDR COMPLETAMENTE IMPLEMENTADA
// ============================================================================

class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    // MÉTODO: iniciar() - Configura el pin como entrada analógica
    void iniciar() override {
        pinMode(pin, INPUT);  // Los pines analógicos no necesitan configuración especial
        Serial.print("Sensor LDR inicializado en GPIO");
        Serial.println(pin);  // Confirmación de inicialización
    }
    
    // MÉTODO: escribir() - Los sensores solo leen, no escriben
    void escribir(int valor) override {
        // No aplica para sensores (método vacío pero requerido por la interfaz)
    }
    
    // MÉTODO: leer() - Lectura real del sensor (0-4095 en ESP32)
    int leer() override {
        return analogRead(pin);  // Valor entre 0 y 4095
    }
    
    // MÉTODO NUEVO: esOscuro() - Interpretación del valor del sensor
    // Retorna true si la lectura está por debajo del umbral (<= 400)
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;  // Menor valor = más luz, Mayor valor = más oscuro
    }
};

// ============================================================================
// NUEVO: CLASE POTENCIÓMETRO COMPLETAMENTE IMPLEMENTADA
// ============================================================================

class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    // MÉTODO: iniciar() - Configuración del pin analógico
    void iniciar() override {
        pinMode(pin, INPUT);  // Entrada analógica
        Serial.print("Potenciómetro inicializado en GPIO");
        Serial.println(pin);  // Confirmación
    }
    
    void escribir(int valor) override {}  // No aplica
    
    // MÉTODO: leer() - Lectura del valor actual (0-4095)
    int leer() override {
        return analogRead(pin);
    }
    
    // MÉTODO NUEVO: getModo() - Convierte valor analógico en modo (0-5)
    // Mapea el rango 0-4095 a 0-6, luego limita a 0-5
    int getModo() {
        int valor = leer();  // Lee valor actual
        int modo = map(valor, 0, 4095, 0, 6);  // Convierte rango
        return constrain(modo, 0, 5);  // Asegura que sea 0-5
    }
};

// ============================================================================
// CLASE: Boton (ESQUELETO) - SIN CAMBIOS
// ============================================================================

class Boton : public Componente {
public:
    Boton() : Componente(0, "Boton") {}
    Boton(int p, String n = "Boton") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return HIGH; }
};

// ============================================================================
// CLASE: PantallaLCD (ESQUELETO) - SIN CAMBIOS
// ============================================================================

class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ============================================================================
// NUEVO: SISTEMA DE MODOS MEJORADO CON LÓGICA COMPLETA
// ============================================================================

class SistemaModos {
private:
    // ARREGLO DE NOMBRES: Cada índice corresponde a un modo específico
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;  // Almacena el modo activo actualmente (0-5)
    
public:
    // CONSTRUCTOR: Inicia en modo NOCHE (0)
    SistemaModos() : modoActual(0) {}
    
    // MÉTODO: getModoActual() - Devuelve nombre del modo como String
    String getModoActual() {
        return String(nombresModos[modoActual]);
    }
    
    // MÉTODO: cambiarModo() - Cambia modo con validación de rango
    void cambiarModo(int nuevoModo) {
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
            Serial.print("Modo cambiado a: ");
            Serial.println(getModoActual());  // Feedback por Serial
        }
    }
    
    // MÉTODO: cambiarModoAuto() - Atajo para modo AUTO (índice 4)
    void cambiarModoAuto() {
        modoActual = 4;  // Modo AUTO
        Serial.println("Modo AUTO activado");
    }
    
    bool isAutoActivo() { return true; }  // Por implementar
    
    // MÉTODO: getBrilloPorModo() - LÓGICA PRINCIPAL DEL SISTEMA
    // Decide el brillo según modo actual y lectura del LDR
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE - 20% de brillo fijo
                return 51;  // 20% de 255 ≈ 51
            case 1: // LECTURA - 40% de brillo fijo
                return 102; // 40% de 255 ≈ 102
            case 2: // RELAJACIÓN - 30% de brillo fijo
                return 76;  // 30% de 255 ≈ 76
            case 3: // FIESTA - Se maneja aparte con lógica especial
                return 0;   // Cero indica manejo especial
            case 4: // AUTOMÁTICO - Depende del sensor LDR
                return (valorLDR <= UMBRAL_LDR) ? 255 : 0;  // Oscuro=ON, Claro=OFF
            case 5: // MANUAL - Control usuario (brillo especial -1)
                return -1;  // -1 indica control manual
            default:
                return 100; // Valor por defecto
        }
    }
    
    // MÉTODOS DE CONSULTA: Identifican el modo actual
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
// SECCIÓN 6: FUNCIONES PRINCIPALES - SETUP MEJORADO
// ============================================================================

void setup() {
    Serial.begin(115200);
    
    // NUEVO: Información específica del commit
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 3: Sensores LDR y potenciómetro implementados");
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
    
    // 2. INICIALIZAR COMPONENTES CON REGISTRO EN SISTEMA
    for (int i = 0; i < 8; i++) {
        leds[i]->iniciar();
        componentes[numComponentes++] = leds[i];  // Registra en arreglo polimórfico
    }
    
    // NUEVO: Inicialización de sensores con registro
    ldr.iniciar();
    componentes[numComponentes++] = &ldr;  // Agrega sensor LDR al sistema
    
    pot.iniciar();
    componentes[numComponentes++] = &pot;  // Agrega potenciómetro al sistema
    
    componentes[numComponentes++] = &lcd;  // Registra pantalla LCD
    
    // NUEVO: Informe de componentes registrados
    Serial.println("\n=== SENSORES INICIALIZADOS ===");
    Serial.print("Total componentes: ");
    Serial.println(numComponentes);
    
    // 3. ESTADO INICIAL: Apagar todos los LEDs
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    Serial.println("Todos los LEDs apagados");
    Serial.println("===================================\n");
}

// ============================================================================
// SECCIÓN 7: LOOP PRINCIPAL CON LÓGICA COMPLETA DEL SISTEMA
// ============================================================================

void loop() {
    // 1. LEER SENSORES (NUEVO: Lectura real de hardware)
    int valorLDR = ldr.leer();          // Valor analógico 0-4095
    bool estaOscuro = ldr.esOscuro();   // Interpretación booleana (<=400)
    int modoPot = pot.getModo();        // Modo seleccionado por potenciómetro (0-5)
    
    // 2. ACTUALIZAR MODO DEL SISTEMA (NUEVO: Cambio dinámico según potenciómetro)
    modos.cambiarModo(modoPot);  // Actualiza modo según selección del usuario
    
    // 3. APLICAR MODO A LEDs (excepto modo MANUAL que requiere intervención)
    if (!modos.esModoManual()) {
        int brillo = modos.getBrilloPorModo(valorLDR);
        
        if (brillo >= 0) {  // Si no es modo especial (-1 = manual)
            if (modos.esModoAuto() || modos.isAutoActivo()) {
                // Aplica brillo a TODOS los LEDs
                for (int i = 0; i < 8; i++) {
                    leds[i]->setBrillo(brillo);
                }
            }
        }
    }
    
    // 4. CONTAR LEDs ENCENDIDOS (para monitoreo)
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // 5. LOG EN SERIAL (NUEVO: Informe periódico cada 2 segundos)
    static unsigned long ultimoLog = 0;  // Variable estática para control de tiempo
    
    if (millis() - ultimoLog > 2000) {  // Cada 2000ms (2 segundos)
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
        
        ultimoLog = millis();  // Reinicia contador
    }
    
    delay(100);  // Pequeña pausa para estabilidad
}
