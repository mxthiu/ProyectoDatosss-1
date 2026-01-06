// ============================================================================
// SECCIÓN 1: CONFIGURACIONES DE HARDWARE Y CONSTANTES
// ============================================================================

#include <Wire.h>  // Biblioteca para comunicación I2C (necesaria para LCD)

// Configuración del hardware - valores fijos del sistema
#define LCD_ADDR 0x27      // Dirección I2C estándar para pantallas LCD
#define SDA_PIN 17         // Pin de datos I2C (Serial Data) para ESP32
#define SCL_PIN 16         // Pin de reloj I2C (Serial Clock) para ESP32
#define UMBRAL_LDR 400     // Valor límite para sensor de luz (<= 400 = oscuro)

// ============================================================================
// SECCIÓN 2: CLASE BASE PRINCIPAL - COMPONENTE (ABSTRACTA)
// ============================================================================

/**
 * Clase base abstracta Componente
 * Define la interfaz común que todos los componentes electrónicos deben implementar
 * Implementa polimorfismo mediante métodos virtuales puros
 */
class Componente {
protected:
    int pin;        // Número de pin GPIO donde está conectado el componente
    String nombre;  // Nombre descriptivo del componente
    
public:
    // Constructores
    Componente(int p, String n) : pin(p), nombre(n) {}  // Constructor principal
    Componente() : pin(0), nombre("") {}                // Constructor por defecto
    virtual ~Componente() {}                            // Destructor virtual para herencia
    
    // MÉTODOS VIRTUALES PUROS (requisito de polimorfismo)
    virtual void iniciar() = 0;      // Inicializa el componente
    virtual void escribir(int valor) = 0;  // Envía datos al componente
    virtual int leer() = 0;          // Lee datos del componente
    
    // Métodos de acceso (getters) - encapsulamiento
    String getNombre() { return nombre; }  // Obtiene nombre del componente
    int getPin() { return pin; }           // Obtiene pin del componente
};

// ============================================================================
// SECCIÓN 3: CLASES DERIVADAS - IMPLEMENTACIONES CONCRETAS
// ============================================================================

/**
 * Clase LED - Control de diodos emisores de luz
 * Hereda de Componente y sobrescribe sus métodos
 * Implementa control de brillo PWM (0-255)
 */
class LED : public Componente {
private:
    int brillo;     // Valor actual de brillo (0-255)
    bool encendido; // Estado actual (true = encendido, false = apagado)
    
public:
    // Constructor con valores por defecto
    LED(int p, String n = "LED") : Componente(p, n), brillo(0), encendido(false) {}
    
    // Métodos sobrescritos (polimorfismo)
    void iniciar() override {
        pinMode(pin, OUTPUT);     // Configura pin como salida
        digitalWrite(pin, LOW);   // Asegura LED apagado al inicio
    }
    
    void escribir(int valor) override {
        brillo = constrain(valor, 0, 255);  // Limita valor entre 0-255
        analogWrite(pin, brillo);           // Aplica brillo PWM
        encendido = (brillo > 0);           // Actualiza estado
    }
    
    int leer() override {
        return brillo;  // Devuelve brillo actual
    }
    
    // Métodos propios específicos para LEDs
    void encender() { 
        escribir(255);      // Brillo máximo
        encendido = true;   // Actualiza estado
    }
    
    void apagar() { 
        escribir(0);        // Brillo cero
        encendido = false;  // Actualiza estado
    }
    
    void alternar() {
        // Cambia estado: si está encendido → apagar, si está apagado → encender
        encendido ? apagar() : encender();
    }
    
    bool estaEncendido() { return encendido; }      // Consulta estado
    void setBrillo(int valor) { escribir(valor); }  // Establece brillo específico
};

/**
 * Clase SensorLDR - Sensor de luz ambiente
 * Hereda de Componente, lee valores analógicos del fotoresistor
 */
class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);  // Configura pin como entrada analógica
    }
    
    void escribir(int valor) override {
        // Los sensores no escriben, método vacío pero requerido por la interfaz
    }
    
    int leer() override {
        return analogRead(pin);  // Lee valor analógico (0-4095 en ESP32)
    }
    
    // Método propio: determina si hay oscuridad según umbral
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;  // Valor bajo = más luz, valor alto = más oscuro
    }
};

/**
 * Clase Potenciometro - Control de selección de modos
 * Hereda de Componente, convierte posición física en modo digital
 */
class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);  // Configura pin como entrada analógica
    }
    
    void escribir(int valor) override {
        // Los potenciómetros no escriben, método vacío
    }
    
    int leer() override {
        return analogRead(pin);  // Lee posición actual (0-4095)
    }
    
    // Método propio: convierte valor analógico a modo (0-5)
    int getModo() {
        int valor = leer();
        int modo = map(valor, 0, 4095, 0, 6);  // Mapea a 6 modos
        return constrain(modo, 0, 5);          // Limita a rango válido
    }
};

/**
 * Clase Boton - Entrada digital con debounce
 * Hereda de Componente, implementa detección de pulsaciones sin rebotes
 */
class Boton : public Componente {
private:
    bool estadoAnterior;        // Estado previo para detección de flanco
    unsigned long ultimoDebounce; // Tiempo del último cambio (para debounce)
    
public:
    // Constructores
    Boton() : Componente(0, "Boton"), estadoAnterior(HIGH), ultimoDebounce(0) {}
    Boton(int p, String n = "Boton") : Componente(p, n), estadoAnterior(HIGH), ultimoDebounce(0) {}
    
    void iniciar() override {
        if (pin != 0) {
            pinMode(pin, INPUT_PULLUP);  // Configura con resistencia pull-up interna
        }
    }
    
    void escribir(int valor) override {
        // Los botones no escriben, método vacío
    }
    
    int leer() override {
        if (pin == 0) return HIGH;  // Si no tiene pin configurado, retorna HIGH
        return digitalRead(pin);     // Lee estado digital
    }
    
    // Método propio: detecta pulsación con debounce
    bool presionado() {
        if (pin == 0) return false;  // Verifica pin válido
        
        bool actual = digitalRead(pin);
        // Detección de flanco descendente (HIGH → LOW)
        bool presion = (estadoAnterior == HIGH && actual == LOW);
        
        // Debounce: solo acepta pulsación si ha pasado tiempo suficiente
        if (presion && (millis() - ultimoDebounce > 50)) {
            estadoAnterior = actual;
            ultimoDebounce = millis();
            return true;
        }
        
        estadoAnterior = actual;
        return false;
    }
};

/**
 * Clase PantallaLCD - Interfaz visual I2C
 * Hereda de Componente, maneja comunicación con pantalla LCD 16x2
 */
class PantallaLCD : public Componente {
private:
    bool lcdInicializado;  // Bandera de estado de inicialización
    
public:
    PantallaLCD(String n = "LCD") : Componente(0, n), lcdInicializado(false) {}
    
    void iniciar() override {
        Wire.begin(SDA_PIN, SCL_PIN);  // Inicia comunicación I2C
        delay(100);  // Espera estabilización
        
        // Secuencia de inicialización estándar para LCD HD44780
        enviarComando(0x03); delay(5);
        enviarComando(0x03); delay(1);
        enviarComando(0x03); delay(10);
        
        enviarComando(0x02); delay(1);    // Modo 4 bits
        enviarComando(0x28);              // 2 líneas, matriz 5x8
        enviarComando(0x0C);              // Display ON, cursor OFF
        enviarComando(0x06);              // Incremento automático de cursor
        enviarComando(0x01); delay(2);    // Limpiar display
        
        lcdInicializado = true;  // Marca como inicializado
    }
    
    void escribir(int valor) override {
        // La pantalla no recibe valores numéricos directos
    }
    
    int leer() override { 
        return 0;  // La pantalla no envía datos de lectura
    }
    
    /**
     * Método propio: muestra información del sistema
     * @param valorLDR Valor actual del sensor de luz
     * @param modo Modo de operación actual
     * @param ledsEncendidos Cantidad de LEDs activos
     */
    void mostrarInfo(int valorLDR, String modo, int ledsEncendidos) {
        if (!lcdInicializado) return;  // Verifica inicialización
        
        // Limpiar display antes de escribir
        enviarComando(0x01);
        delay(2);
        
        // Línea 1: Información de sensores y LEDs
        // Formato: "LDR:345  LED:3/8" (2 espacios entre secciones)
        String linea1 = "LDR:" + String(valorLDR) + "  LED:" + String(ledsEncendidos) + "/8";
        escribirLinea(linea1, 0, 0);
        
        // Línea 2: Modo de operación actual
        String linea2 = "Modo " + modo;
        escribirLinea(linea2, 0, 1);
    }
    
private:
    // Métodos privados para manejo de bajo nivel del LCD
    
    /**
     * Envía un byte al LCD (comando o dato)
     * @param valor Byte a enviar
     * @param modo 0 para comando, 1 para dato
     */
    void enviarByte(uint8_t valor, uint8_t modo) {
        uint8_t alta = (valor & 0xF0) | 0x08 | modo;   // 4 bits altos + enable + modo
        uint8_t baja = ((valor << 4) & 0xF0) | 0x08 | modo;  // 4 bits bajos + enable + modo
        
        Wire.beginTransmission(LCD_ADDR);
        // Envía 3 veces cada nibble con pulso en enable
        Wire.write(alta); Wire.write(alta | 0x04); Wire.write(alta);
        Wire.write(baja); Wire.write(baja | 0x04); Wire.write(baja);
        Wire.endTransmission();
        delayMicroseconds(100);  // Pausa para procesamiento
    }
    
    void enviarComando(uint8_t cmd) {
        enviarByte(cmd, 0);  // Envía como comando (modo = 0)
    }
    
    /**
     * Escribe una línea de texto en posición específica
     * @param texto Cadena a mostrar
     * @param col Columna inicial (0-15)
     * @param fila Fila (0-1)
     */
    void escribirLinea(String texto, uint8_t col, uint8_t fila) {
        // Calcula dirección DDRAM según fila
        uint8_t direccion = (fila == 0) ? 0x80 : 0xC0;
        direccion += col;  // Ajusta por columna
        enviarComando(direccion);  // Posiciona cursor
        
        // Escribe cada carácter (máximo 16 por línea)
        for (int i = 0; i < texto.length() && i < 16; i++) {
            enviarByte(texto.charAt(i), 1);  // Envía como dato (modo = 1)
        }
    }
};

// ============================================================================
// SECCIÓN 4: SISTEMA DE MODOS DE OPERACIÓN
// ============================================================================

/**
 * Clase SistemaModos - Gestor central de modos de iluminación
 * No hereda de Componente, es una clase independiente para lógica de negocio
 */
class SistemaModos {
private:
    // Nombres descriptivos de los 6 modos disponibles
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    
    // Estado interno del sistema
    int modoActual;                     // Índice del modo actual (0-5)
    bool autoActivo;                    // Estado del submodo AUTO (true/false)
    unsigned long ultimoCambioFiesta;   // Temporizador para animación FIESTA
    int ledFiestaActual;                // Índice del LED activo en modo FIESTA
    bool estadoAutoAnterior;            // Estado previo para detección de cambios
    unsigned long ultimoCambioLED;      // Temporizador para cambios en AUTO
    
public:
    // Constructor - inicializa todas las variables en estado seguro
    SistemaModos() : modoActual(0), autoActivo(true), ultimoCambioFiesta(0), 
                     ledFiestaActual(0), estadoAutoAnterior(false), ultimoCambioLED(0) {}
    
    // Métodos de acceso al estado
    String getModoActual() {
        return String(nombresModos[modoActual]);  // Convierte a String
    }
    
    void cambiarModo(int nuevoModo) {
        // Valida rango antes de asignar
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
        }
    }
    
    // Métodos específicos para control de modos
    void cambiarModoAuto() {
        modoActual = 4;  // Índice 4 corresponde a "AUTO"
        autoActivo = true;  // Activa automáticamente el submodo
        Serial.println("Atajo P4: Modo AUTO activado");
    }
    
    void toggleAuto() {
        autoActivo = !autoActivo;  // Alterna estado
        Serial.print("Auto: ");
        Serial.println(autoActivo ? "ACTIVADO" : "DESACTIVADO");
    }
    
    bool isAutoActivo() {
        return autoActivo;
    }
    
    /**
     * Calcula brillo según modo actual y condiciones de luz
     * @param valorLDR Valor actual del sensor de luz
     * @return Nivel de brillo (0-255) o -1 para control manual
     */
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE - Brillo bajo (20% = 51)
                return 51;
                
            case 1: // LECTURA - Brillo medio-alto (40% = 102)
                return 102;
                
            case 2: // RELAJACIÓN - Brillo medio (30% = 76)
                return 76;
                
            case 3: // FIESTA - Controlado por animación
                return 0;  // Se maneja externamente
                
            case 4: // AUTOMÁTICO - Control inteligente por LDR
            {
                // Determina si está oscuro según umbral
                bool oscuroAhora = (valorLDR <= UMBRAL_LDR);
                
                // Detección de cambio con debounce de 1 segundo
                if (oscuroAhora != estadoAutoAnterior && (millis() - ultimoCambioLED > 1000)) {
                    estadoAutoAnterior = oscuroAhora;
                    ultimoCambioLED = millis();
                    // Log informativo
                    Serial.print("AUTO (");
                    Serial.print(valorLDR);
                    Serial.print(" <= 400): ");
                    Serial.println(oscuroAhora ? "OSCURO -> LEDs ON" : "CLARO -> LEDs OFF");
                }
                
                // Retorna brillo máximo si oscuro, cero si claro
                return oscuroAhora ? 255 : 0;
            }
                
            case 5: // MANUAL - Control total por usuario
                return -1;  // Valor especial que indica control manual
                
            default: // Caso de seguridad
                return 100;
        }
    }
    
    /**
     * Maneja animación del modo FIESTA
     * Secuencia cíclica que enciende un LED a la vez
     * @param leds Array de punteros a objetos LED
     * @param totalLEDs Cantidad de LEDs en el array
     */
    void manejarModoFiesta(LED** leds, int totalLEDs) {
        // Cambia de LED cada 200ms para efecto de animación
        if (millis() - ultimoCambioFiesta > 200) {
            // Apagar todos los LEDs primero
            for (int i = 0; i < totalLEDs; i++) {
                leds[i]->apagar();
            }
            
            // Encender solo el LED actual de la secuencia
            leds[ledFiestaActual]->encender();
            
            // Avanza al siguiente LED (cíclico)
            ledFiestaActual = (ledFiestaActual + 1) % totalLEDs;
            ultimoCambioFiesta = millis();  // Reinicia temporizador
        }
    }
    
    // Métodos de consulta rápida de modo actual
    bool esModoFiesta() { return modoActual == 3; }
    bool esModoManual() { return modoActual == 5; }
    bool esModoAuto() { return modoActual == 4; }
};

// ============================================================================
// SECCIÓN 5: DECLARACIÓN DE OBJETOS GLOBALES
// ============================================================================

// Instancias de componentes (representan hardware real)
LED* leds[8];                           // Array de 8 LEDs para diferentes espacios
SensorLDR ldr(34, "Sensor Luz");        // Sensor de luz en GPIO34
Potenciometro pot(35, "Pot Modos");     // Selector de modos en GPIO35
PantallaLCD lcd("Pantalla LCD");        // Display para información

// Sistema de control
Boton* botones[9];                      // 9 botones (1 auto + 8 manuales)
SistemaModos modos;                     // Gestor central de modos

// Array polimórfico para manejo unificado de componentes
Componente* componentes[30];            // Puede contener CUALQUIER tipo de Componente
int numComponentes = 0;                 // Contador de componentes agregados

// ============================================================================
// SECCIÓN 6: CONFIGURACIÓN INICIAL (SETUP)
// ============================================================================

void setup() {
    // Inicializa comunicación serial para monitoreo
    Serial.begin(115200);
    
    // Encabezado informativo del sistema
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Commit 7: Ajustes finales completos");
    Serial.println("Umbral LDR: <= 400 = OSCURO (LEDs ON)");
    
    // ------------------------------------------------------------------------
    // 1. CREACIÓN E INICIALIZACIÓN DE LEDs
    // ------------------------------------------------------------------------
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    // Estado inicial: todos los LEDs apagados
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    // ------------------------------------------------------------------------
    // 2. CREACIÓN E INICIALIZACIÓN DE BOTONES
    // ------------------------------------------------------------------------
    botones[0] = new Boton(4, "P4 AtajoAuto");    // Botón especial para atajo AUTO
    botones[1] = new Boton(26, "Boton C1");       // Cuarto 1
    botones[2] = new Boton(27, "Boton C2");       // Cuarto 2
    botones[3] = new Boton(14, "Boton C3");       // Cuarto 3
    botones[4] = new Boton(2, "Boton Sala");      // Sala
    botones[5] = new Boton(12, "Boton Cocina");   // Cocina
    botones[6] = new Boton(25, "Boton P.Del");    // Patio Delantero
    botones[7] = new Boton(33, "Boton P.Tras");   // Patio Trasero
    botones[8] = new Boton(32, "Boton P.Int");    // Patio Interior
    
    // ------------------------------------------------------------------------
    // 3. CONFIGURACIÓN DEL ARRAY POLIMÓRFICO
    // ------------------------------------------------------------------------
    // Agrega LEDs al array (demostración de polimorfismo)
    for (int i = 0; i < 8; i++) {
        componentes[numComponentes++] = leds[i];
    }
    
    // Agrega sensores y pantalla
    componentes[numComponentes++] = &ldr;   // Sensor es instancia estática
    componentes[numComponentes++] = &pot;
    componentes[numComponentes++] = &lcd;
    
    // Agrega botones
    for (int i = 0; i < 9; i++) {
        componentes[numComponentes++] = botones[i];
    }
    
    // ------------------------------------------------------------------------
    // 4. INICIALIZACIÓN POLIMÓRFICA DE TODOS LOS COMPONENTES
    // ------------------------------------------------------------------------
    Serial.println("\n=== INICIALIZANDO COMPONENTES ===");
    for (int i = 0; i < numComponentes; i++) {
        componentes[i]->iniciar();  // ⭐ POLIMORFISMO: llama al iniciar() correcto
        Serial.print("- ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" en GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
    // ------------------------------------------------------------------------
    // 5. INFORMACIÓN DE CONFIGURACIÓN DE BOTONES
    // ------------------------------------------------------------------------
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
    
    // ------------------------------------------------------------------------
    // 6. DESCRIPCIÓN DE MODOS DE OPERACIÓN
    // ------------------------------------------------------------------------
    Serial.println("\n=== MODOS CON BRILLOS ===");
    Serial.println("NOCHE: 20% (51) - Iluminación nocturna suave");
    Serial.println("LECTURA: 40% (102) - Iluminación para actividades");
    Serial.println("RELAJ: 30% (76) - Ambiente relajado");
    Serial.println("AUTO: Umbral 400 - Control automático por luz ambiente");
    Serial.println("MANUAL: Todos apagados al inicio - Control total por usuario");
    Serial.println("===================================\n");
}

// ============================================================================
// SECCIÓN 7: BUCLE PRINCIPAL (LOOP)
// ============================================================================

void loop() {
    // ------------------------------------------------------------------------
    // 1. LECTURA DE SENSORES - Entrada del sistema
    // ------------------------------------------------------------------------
    int valorLDR = ldr.leer();           // Valor crudo del sensor (0-4095)
    bool estaOscuro = ldr.esOscuro();    // Interpretación booleana (<= 400)
    
    // ------------------------------------------------------------------------
    // 2. ACTUALIZACIÓN DE MODO - Control por potenciómetro
    // ------------------------------------------------------------------------
    int modoPot = pot.getModo();         // Lee posición física (0-5)
    modos.cambiarModo(modoPot);          // Actualiza modo del sistema
    
    // ------------------------------------------------------------------------
    // 3. DETECCIÓN DE BOTONES - Entrada del usuario
    // ------------------------------------------------------------------------
    // Botón P4: atajo directo al modo AUTO
    if (botones[0]->presionado()) {
        modos.cambiarModoAuto();
    }
    
    // ------------------------------------------------------------------------
    // 4. MODO MANUAL - Control individual de LEDs
    // ------------------------------------------------------------------------
    if (modos.esModoManual()) {
        // Escanea los 8 botones de control manual
        for (int i = 1; i <= 8; i++) {
            if (botones[i]->presionado()) {
                leds[i-1]->alternar();  // Alterna estado del LED correspondiente
                
                // Log informativo
                Serial.print("MANUAL - LED ");
                Serial.print(i);
                Serial.print(" (");
                Serial.print(leds[i-1]->getNombre());
                Serial.print("): ");
                Serial.println(leds[i-1]->estaEncendido() ? "ON" : "OFF");
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 5. APLICACIÓN DE MODO ACTUAL - Lógica central del sistema
    // ------------------------------------------------------------------------
    if (!modos.esModoManual()) {  // Si NO está en modo manual
        if (modos.esModoFiesta()) {
            // MODO FIESTA: animación secuencial
            modos.manejarModoFiesta(leds, 8);
        } else {
            // MODOS NOCHE, LECTURA, RELAJ, AUTO
            int brillo = modos.getBrilloPorModo(valorLDR);
            
            if (brillo >= 0) {  // -1 indica modo manual
                // Aplica brillo si es modo AUTO o si auto está activado en otros modos
                if (modos.esModoAuto() || modos.isAutoActivo()) {
                    for (int i = 0; i < 8; i++) {
                        leds[i]->setBrillo(brillo);  // Aplica mismo brillo a todos los LEDs
                    }
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 6. MONITOREO DEL SISTEMA - Conteo de LEDs activos
    // ------------------------------------------------------------------------
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // ------------------------------------------------------------------------
    // 7. ACTUALIZACIÓN DE INTERFAZ - Pantalla LCD
    // ------------------------------------------------------------------------
    lcd.mostrarInfo(valorLDR, modos.getModoActual(), ledsOn);
    
    // ------------------------------------------------------------------------
    // 8. REGISTRO Y MONITOREO - Salida serial (cada segundo)
    // ------------------------------------------------------------------------
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
        
        ultimoLog = millis();  // Reinicia temporizador
    }
    
    // ------------------------------------------------------------------------
    // 9. CONTROL DE TIEMPO - Estabilidad del sistema
    // ------------------------------------------------------------------------
    delay(50);  // Pausa para evitar sobrecarga y permitir respuesta
}
