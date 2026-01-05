// ============================================================================
// SECCIÓN 1: CONFIGURACIONES DE HARDWARE Y CONSTANTES
// ============================================================================

#include <Wire.h>  // Biblioteca para comunicación I2C (necesaria para LCD)

// Configuración del hardware - valores fijos del sistema

#define LCD_ADDR 0x27
// Dirección I2C estándar para pantallas LCD

#define SDA_PIN 17
// Pin de datos I2C (Serial Data)

#define SCL_PIN 16
// Pin de reloj I2C (Serial Clock)

#define UMBRAL_LDR 400
// Valor límite para sensor de luz (más oscuro = mayor valor)

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

// CLASE: LED - Control de luces
// ----------------------------------------------------------------------------
// Hereda de: Componente
// Propósito: Representar y controlar un LED individual
// Nota: Los métodos están vacíos en este ejemplo, pero se implementarían según el hardware

class LED : public Componente {
public:
    // Constructor: llama al constructor de Componente
    LED(int p, String n = "LED") : Componente(p, n) {}
    
    // Implementaciones requeridas (por ahora vacías)
    void iniciar() override {}        // Aquí iría pinMode(pin, OUTPUT)
    void escribir(int valor) override {}  // Aquí iría digitalWrite/analogWrite
    int leer() override { return 0; }     // Podría leer el estado actual
};

// CLASE: SensorLDR - Sensor de luz ambiental
// ----------------------------------------------------------------------------
// Hereda de: Componente  
// Propósito: Leer intensidad de luz ambiental mediante sensor LDR
// Funcionamiento: Mayor resistencia = menos luz = mayor valor leído

class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {}        // Configuración de pin analógico
    void escribir(int valor) override {}  // Los sensores normalmente no reciben escritura
    int leer() override { return 0; }     // Aquí iría analogRead(pin)
};

// CLASE: Potenciometro - Control manual rotatorio
// ----------------------------------------------------------------------------
// Hereda de: Componente
// Propósito: Permitir ajuste manual mediante perilla giratoria
// Uso típico: Selección de modos, ajuste de intensidad

class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {}        // Configura pin como entrada analógica
    void escribir(int valor) override {}  // No aplica para potenciómetros
    int leer() override { return 0; }     // Lectura de posición (0-1023 o 0-4095)
};

// CLASE: Boton - Entrada manual por pulsación
// ----------------------------------------------------------------------------
// Hereda de: Componente
// Propósito: Detectar pulsaciones del usuario
// Característica: Tiene constructor adicional sin parámetros

class Boton : public Componente {
public:
    // Constructor sin parámetros (para inicializaciones especiales)
    Boton() : Componente(0, "Boton") {}
    
    // Constructor con parámetros (el normal)
    Boton(int p, String n = "Boton") : Componente(p, n) {}
    
    void iniciar() override {}              // Configura pin con INPUT_PULLUP
    void escribir(int valor) override {}    // No aplica para botones
    int leer() override { return HIGH; }    // Retorna estado (HIGH/LOW)
};

// CLASE: PantallaLCD - Interfaz visual para el usuario
// ----------------------------------------------------------------------------
// Hereda de: Componente
// Propósito: Mostrar información del sistema al usuario
// Nota: No usa pin físico directo (usa I2C), por eso pin=0

class PantallaLCD : public Componente {
public:
    PantallaLCD(String n = "LCD") : Componente(0, n) {}
    
    void iniciar() override {}        // Inicializa comunicación I2C
    void escribir(int valor) override {}  // Escribe texto o comandos
    int leer() override { return 0; }     // Podría leer estado de la pantalla
};

// ============================================================================
// SECCIÓN 4: SISTEMA DE MODOS DE OPERACIÓN
// ============================================================================

// CLASE: SistemaModos - Gestor de modos de iluminación
// ----------------------------------------------------------------------------
// No hereda de Componente: es una clase independiente
// Propósito: Gestionar los 6 modos de operación descritos en la documentación
// Responsabilidad: Decidir cómo se comportan las luces según el modo activo

class SistemaModos {
public:
    SistemaModos() {}  // Constructor simple
    
    // Métodos de consulta y control (implementaciones simuladas)
    String getModoActual() { return "NOCHE"; }      // Devuelve modo actual
    void cambiarModo(int nuevoModo) {}              // Cambia a modo específico
    void cambiarModoAuto() {}                       // Activa modo automático
    bool isAutoActivo() { return true; }            // Verifica si auto está activo
    
    // Métodos para determinar comportamiento según modo
    int getBrilloPorModo(int valorLDR) { return 0; }  // Calcula brillo adecuado
    bool esModoManual() { return false; }             // ¿Está en modo manual?
    bool esModoAuto() { return false; }               // ¿Está en modo automático?
    bool esModoFiesta() { return false; }             // ¿Está en modo fiesta?
};

// ============================================================================
// SECCIÓN 5: DECLARACIÓN DE OBJETOS GLOBALES
// ============================================================================

// ARREGLOS PARA ALMACENAR COMPONENTES:
// ------------------------------------
LED* leds[8];                          // 8 LEDs para diferentes habitaciones
SensorLDR ldr(34, "Sensor Luz");       // Sensor en pin 34
Potenciometro pot(35, "Pot Modos");    // Potenciómetro en pin 35
PantallaLCD lcd("Pantalla LCD");       // Pantalla (comunicación I2C)
Boton* botones[9];                     // 9 botones para control
SistemaModos modos;                    // Gestor de modos

// ARREGLO POLIMÓRFICO: puede contener CUALQUIER tipo de Componente
Componente* componentes[30];           // Máximo 30 componentes
int numComponentes = 0;                // Contador de componentes registrados

// ============================================================================
// SECCIÓN 6: FUNCIONES PRINCIPALES DE ARDUINO/ESP32
// ============================================================================

// FUNCIÓN: setup() - Configuración inicial
// ----------------------------------------------------------------------------
// Se ejecuta UNA SOLA VEZ al iniciar el sistema
// Responsabilidad: Inicializar hardware, crear objetos, configurar pines

void setup() {
    Serial.begin(115200);  // Inicia comunicación serial para depuración
    
    // Mensaje inicial de identificación
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 1: Estructura base de clases");
    
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
    
    // Informe de estructura creada
    Serial.println("\nEstructura de clases creada:");
    Serial.println("- Clase base: Componente");
    Serial.println("- Clases hijas: LED, SensorLDR, Potenciometro, Boton, PantallaLCD");
    Serial.println("- Métodos: iniciar(), escribir(), leer()");
    Serial.println("===================================\n");
}

// FUNCIÓN: loop() - Bucle principal
// ----------------------------------------------------------------------------
// Se ejecuta REPETIDAMENTE después de setup()
// Responsabilidad: Lógica continua del sistema, lectura de sensores, actualización

void loop() {
    Serial.println("Sistema base funcionando...");
    delay(1000);  // Pausa de 1 segundo entre ciclos
}
