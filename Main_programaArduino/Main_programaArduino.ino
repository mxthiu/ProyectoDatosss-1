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

// CLASE: Componente (Base Abstracta)
// ----------------------------------------------------------------------------
// Propósito: Ser la clase padre de todos los elementos electrónicos del sistema
// Características: Clase abstracta (no se pueden crear objetos de esta clase)
// Uso de OOP: Define una interfaz común mediante métodos virtuales puros

class Componente {
protected:
    // ATRIBUTOS PROTEGIDOS (accesibles por clases hijas)
    int pin;           // Guarda el número del pin GPIO de la ESP32
    String nombre;     // Nombre identificador (ej: "LED_SALA", "SENSOR_LDR")
    
public:
    // --- CONSTRUCTORES ---
    // Constructor principal: inicializa con pin y nombre específicos
    Componente(int p, String n) : pin(p), nombre(n) {}
    
    // Constructor por defecto: para cuando no se tienen valores iniciales
    Componente() : pin(0), nombre("") {}
    
    // Destructor virtual: necesario para liberar memoria correctamente en herencia
    virtual ~Componente() {}
    
    // --- MÉTODOS VIRTUALES PUROS (OBLIGATORIOS PARA CLASES HIJAS) ---
    // Estas funciones DEBEN ser implementadas por todas las clases hijas
    
    virtual void iniciar() = 0;      // Configura el hardware del componente
    virtual void escribir(int valor) = 0;  // Envía datos/instrucciones al componente
    virtual int leer() = 0;          // Obtiene datos/estado del componente
    
    // --- MÉTODOS CONCRETOS (COMPARTIDOS) ---
    // Estas funciones son heredadas tal cual por las clases hijas
    
    String getNombre() { return nombre; }  // Devuelve el nombre del componente
    int getPin() { return pin; }           // Devuelve el número del pin
};

// ============================================================================
// SECCIÓN 3: CLASES DERIVADAS (HIJAS DE COMPONENTE)
// ============================================================================

// CLASE: LED - Control de luces (VERSIÓN COMPLETA)
// ----------------------------------------------------------------------------
// NOVEDAD: Ahora tiene implementación completa con control de brillo y estado
// Mejoras respecto a versión anterior: Atributos privados, métodos utilitarios

class LED : public Componente {
private:
    // NUEVOS ATRIBUTOS PRIVADOS (encapsulamiento mejorado)
    int brillo;      // Almacena la intensidad actual del LED (0-255 para PWM)
    bool encendido;  // Estado booleano del LED (true = encendido, false = apagado)
    
public:
    // Constructor mejorado: inicializa atributos adicionales
    LED(int p, String n = "LED") : Componente(p, n), brillo(0), encendido(false) {}
    
    // MÉTODO: iniciar() - Implementación COMPLETA
    // Configura el pin físico y muestra confirmación por Serial
    void iniciar() override {
        pinMode(pin, OUTPUT);      // Establece pin como salida
        digitalWrite(pin, LOW);    // Asegura que empiece apagado
        Serial.print("LED inicializado: ");
        Serial.println(nombre);    // Mensaje de confirmación
    }
    
    // MÉTODO: escribir() - Implementación COMPLETA con control PWM
    // Recibe valor entre 0-255 y lo aplica al LED mediante analogWrite()
    void escribir(int valor) override {
        brillo = constrain(valor, 0, 255);  // Limita valor al rango PWM (0-255)
        analogWrite(pin, brillo);           // Escribe valor analógico (PWM)
        encendido = (brillo > 0);           // Actualiza estado según brillo
    }
    
    // MÉTODO: leer() - Implementación COMPLETA
    // Devuelve el brillo actual almacenado (no lee del pin físico)
    int leer() override {
        return brillo;  // Retorna intensidad actual (0-255)
    }
    
    // ============================================================================
    // NUEVOS MÉTODOS UTILITARIOS - INTERFAZ MEJORADA PARA CONTROL DE LEDS
    // ============================================================================
    
    // MÉTODO: encender() - Enciende el LED al máximo brillo
    void encender() { 
        escribir(255);  // Brillo máximo (255 = 100%)
        encendido = true;
        Serial.print("LED encendido: ");
        Serial.println(nombre);
    }
    
    // MÉTODO: apagar() - Apaga completamente el LED
    void apagar() { 
        escribir(0);  // Brillo cero (0 = 0%)
        encendido = false;
        Serial.print("LED apagado: ");
        Serial.println(nombre);
    }
    
    // MÉTODO: alternar() - Cambia entre encendido y apagado
    // Útil para botones de toggle
    void alternar() {
        if (encendido) {
            apagar();
        } else {
            encender();
        }
    }
    
    // MÉTODO: estaEncendido() - Consulta el estado actual
    // Devuelve true si el LED está encendido (brillo > 0)
    bool estaEncendido() { return encendido; }
    
    // MÉTODO: setBrillo() - Alias para escribir() con nombre más descriptivo
    void setBrillo(int valor) { escribir(valor); }
};

// ----------------------------------------------------------------------------
// CLASE: SensorLDR - Sensor de luz ambiental (SIN CAMBIOS)
// ----------------------------------------------------------------------------

class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ----------------------------------------------------------------------------
// CLASE: Potenciometro - Control manual rotatorio (SIN CAMBIOS)
// ----------------------------------------------------------------------------

class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ----------------------------------------------------------------------------
// CLASE: Boton - Entrada manual por pulsación (SIN CAMBIOS)
// ----------------------------------------------------------------------------

class Boton : public Componente {
public:
    Boton() : Componente(0, "Boton") {}
    Boton(int p, String n = "Boton") : Componente(p, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return HIGH; }
};

// ----------------------------------------------------------------------------
// CLASE: PantallaLCD - Interfaz visual para el usuario (SIN CAMBIOS)
// ----------------------------------------------------------------------------

class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}
    void escribir(int valor) override {}
    int leer() override { return 0; }
};

// ============================================================================
// SECCIÓN 4: SISTEMA DE MODOS DE OPERACIÓN (SIN CAMBIOS)
// ============================================================================

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

// ============================================================================
// SECCIÓN 5: DECLARACIÓN DE OBJETOS GLOBALES (SIN CAMBIOS)
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
// SECCIÓN 6: FUNCIONES PRINCIPALES DE ARDUINO/ESP32 (MEJORADAS)
// ============================================================================

// FUNCIÓN: setup() - Configuración inicial MEJORADA
// ----------------------------------------------------------------------------
// NOVEDAD: Ahora inicializa TODOS los LEDs y los registra en el sistema
// También incluye una demostración automática de funcionalidad

void setup() {
    Serial.begin(115200);  // Inicia comunicación serial para depuración
    
    // Mensaje inicial de identificación (actualizado)
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 2: Clase LED implementada completamente");
    
    // INICIALIZACIÓN DE LEDs - Creación de objetos dinámicos
    // Cada LED representa una habitación/área de la casa
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    // ============================================================================
    // NUEVO: INICIALIZACIÓN AUTOMÁTICA DE TODOS LOS LEDs
    // ============================================================================
    
    // Bucle que inicializa cada LED y lo registra en el arreglo polimórfico
    for (int i = 0; i < 8; i++) {
        leds[i]->iniciar();                    // Llama al método iniciar() de cada LED
        componentes[numComponentes++] = leds[i]; // Agrega al arreglo de componentes
    }
    
    // ============================================================================
    // NUEVO: DEMOSTRACIÓN AUTOMÁTICA DE FUNCIONALIDAD
    // ============================================================================
    
    Serial.println("\n=== DEMOSTRACIÓN LEDs ===");
    delay(1000);  // Pausa para que se pueda leer el mensaje
    
    // Demostración 1: Encender y apagar LED 0
    leds[0]->encender();  // Enciende completamente
    delay(500);           // Espera medio segundo
    leds[0]->apagar();    // Apaga completamente
    
    // Demostración 2: Control de brillo en LED 1
    leds[1]->setBrillo(128);  // Establece brillo al 50% (128/255 ≈ 50%)
    delay(500);                // Mantiene encendido
    leds[1]->apagar();         // Apaga
    
    Serial.println("Demostración completada");
    Serial.println("===================================\n");
}

// FUNCIÓN: loop() - Bucle principal MEJORADO
// ----------------------------------------------------------------------------
// NOVEDAD: Implementa un demostrador cíclico que enciende LEDs uno por uno
// Muestra el uso práctico de los métodos implementados

void loop() {
    // ============================================================================
    // NUEVO: DEMOSTRADOR CÍCLICO DE LEDs
    // ============================================================================
    
    // Variables estáticas mantienen su valor entre llamadas a loop()
    static int ledActual = 0;           // LED que se está mostrando actualmente
    static unsigned long ultimoCambio = 0;  // Marca de tiempo del último cambio
    
    // Cambia de LED cada 1000 milisegundos (1 segundo)
    if (millis() - ultimoCambio > 1000) {
        
        // Paso 1: Apagar TODOS los LEDs antes de encender uno nuevo
        for (int i = 0; i < 8; i++) {
            leds[i]->apagar();  // Llama al método apagar() de cada LED
        }
        
        // Paso 2: Encender el LED actual
        leds[ledActual]->encender();  // Enciende el LED correspondiente
        
        // Paso 3: Mostrar información por Serial
        Serial.print("LED activo: ");
        Serial.println(leds[ledActual]->getNombre());  // Usa getNombre() heredado
        
        // Paso 4: Avanzar al siguiente LED (cíclico 0→1→2...→7→0)
        ledActual = (ledActual + 1) % 8;  // Operador módulo para ciclo continuo
        
        // Paso 5: Actualizar marca de tiempo
        ultimoCambio = millis();  // Registra el momento del cambio
    }
    
    delay(100);  // Pequeña pausa para evitar sobrecargar el procesador
}
