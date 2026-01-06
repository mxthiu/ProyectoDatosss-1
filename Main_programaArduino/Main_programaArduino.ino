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
        encendido ? apagar() : encender();  // Cambia estado
    }
    
    bool estaEncendido() { return encendido; }      // Consulta estado
    void setBrillo(int valor) { escribir(valor); }  // Establece brillo específico
};

class SensorLDR : public Componente {
public:
    SensorLDR(int p, String n = "LDR") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);  // Configura pin como entrada analógica
    }
    
    void escribir(int valor) override {}  // Los sensores no escriben
    
    int leer() override {
        return analogRead(pin);  // Lee valor analógico (0-4095 en ESP32)
    }
    
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;  // Valor bajo = más luz, valor alto = más oscuro
    }
};

class Potenciometro : public Componente {
public:
    Potenciometro(int p, String n = "Pot") : Componente(p, n) {}
    
    void iniciar() override {
        pinMode(pin, INPUT);  // Configura pin como entrada analógica
    }
    
    void escribir(int valor) override {}  // Los potenciómetros no escriben
    
    int leer() override {
        return analogRead(pin);  // Lee posición actual (0-4095)
    }
    
    int getModo() {
        int valor = leer();
        int modo = map(valor, 0, 4095, 0, 6);  // Mapea a 6 modos
        return constrain(modo, 0, 5);          // Limita a rango válido
    }
};

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
    
    void escribir(int valor) override {}  // Los botones no escriben
    
    int leer() override {
        if (pin == 0) return HIGH;  // Si no tiene pin configurado, retorna HIGH
        return digitalRead(pin);     // Lee estado digital
    }
    
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
    
    void escribir(int valor) override {}  // La pantalla no recibe valores numéricos directos
    
    int leer() override { 
        return 0;  // La pantalla no envía datos de lectura
    }
    
    void mostrarInfo(int valorLDR, String modo, int ledsEncendidos) {
        if (!lcdInicializado) return;  // Verifica inicialización
        
        enviarComando(0x01);  // Limpiar display
        delay(2);
        
        // Línea 1: "LDR:345  LED:3/8" (2 espacios entre secciones)
        String linea1 = "LDR:" + String(valorLDR) + "  LED:" + String(ledsEncendidos) + "/8";
        escribirLinea(linea1, 0, 0);
        
        // Línea 2: Modo de operación actual
        String linea2 = "Modo " + modo;
        escribirLinea(linea2, 0, 1);
    }
    
private:
    void enviarByte(uint8_t valor, uint8_t modo) {
        uint8_t alta = (valor & 0xF0) | 0x08 | modo;
        uint8_t baja = ((valor << 4) & 0xF0) | 0x08 | modo;
        
        Wire.beginTransmission(LCD_ADDR);
        Wire.write(alta); Wire.write(alta | 0x04); Wire.write(alta);
        Wire.write(baja); Wire.write(baja | 0x04); Wire.write(baja);
        Wire.endTransmission();
        delayMicroseconds(100);  // Pausa para procesamiento
    }
    
    void enviarComando(uint8_t cmd) {
        enviarByte(cmd, 0);  // Envía como comando (modo = 0)
    }
    
    void escribirLinea(String texto, uint8_t col, uint8_t fila) {
        uint8_t direccion = (fila == 0) ? 0x80 : 0xC0;
        direccion += col;
        enviarComando(direccion);  // Posiciona cursor
        
        // Escribe cada carácter (máximo 16 por línea)
        for (int i = 0; i < texto.length() && i < 16; i++) {
            enviarByte(texto.charAt(i), 1);  // Envía como dato (modo = 1)
        }
    }
};

// ============================================================================
// SECCIÓN 4: SISTEMA DE MODOS DE OPERACIÓN CON LÓGICA OR
// ============================================================================

/**
 * Clase SistemaModos - Gestor central de modos de iluminación
 * VERSIÓN MEJORADA: Implementa lógica OR en modo AUTO
 * (LDR oscuro O botón forzado) → LEDs encendidos
 */
class SistemaModos {
private:
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;
    bool autoActivo;
    
    // ============================================================================
    // NUEVA FUNCIONALIDAD: BANDERA PARA FORZAR ENCENDIDO
    // ============================================================================
    bool forzarEncendido;  // ⭐ NUEVO: cuando es true, LEDs se encienden sin importar LDR
    
    unsigned long ultimoCambioFiesta;
    int ledFiestaActual;
    bool estadoAutoAnterior;
    unsigned long ultimoCambioLED;
    
public:
    // Constructor actualizado con nueva variable
    SistemaModos() : modoActual(0), autoActivo(true), 
                     forzarEncendido(false),  // ⭐ NUEVO: Inicializado en false
                     ultimoCambioFiesta(0), ledFiestaActual(0), 
                     estadoAutoAnterior(false), ultimoCambioLED(0) {}
    
    String getModoActual() {
        return String(nombresModos[modoActual]);
    }
    
    void cambiarModo(int nuevoModo) {
        if (nuevoModo >= 0 && nuevoModo <= 5) {
            modoActual = nuevoModo;
        }
    }
    
    void cambiarModoAuto() {
        modoActual = 4;  // Modo AUTO
        autoActivo = true;
        Serial.println("Atajo P4: Modo AUTO activado");
    }
    
    void toggleAuto() {
        autoActivo = !autoActivo;
        Serial.print("Auto: ");
        Serial.println(autoActivo ? "ACTIVADO" : "DESACTIVADO");
    }
    
    // ============================================================================
    // NUEVO MÉTODO: ALTERNAR ESTADO DE FORZADO DE ENCENDIDO
    // ============================================================================
    void toggleForzarEncendido() {
        forzarEncendido = !forzarEncendido;  // ⭐ Alterna entre true/false
        Serial.print("Forzar encendido: ");
        Serial.println(forzarEncendido ? "ACTIVADO" : "DESACTIVADO");
    }
    
    bool isAutoActivo() {
        return autoActivo;
    }
    
    // ============================================================================
    // NUEVO MÉTODO: CONSULTAR ESTADO DE FORZADO
    // ============================================================================
    bool isForzarEncendido() {
        return forzarEncendido;  // ⭐ Getter para consultar estado
    }
    
    // ============================================================================
    // MÉTODO MEJORADO: IMPLEMENTA LÓGICA OR EN MODO AUTO
    // ============================================================================
    int getBrilloPorModo(int valorLDR) {
        switch(modoActual) {
            case 0: // NOCHE - 20%
                return 51;
            case 1: // LECTURA - 40%
                return 102;
            case 2: // RELAJACIÓN - 30%
                return 76;
            case 3: // FIESTA
                return 0;
            case 4: // AUTOMÁTICO - CON LÓGICA OR (⭐ NUEVA IMPLEMENTACIÓN)
            {
                // ⭐ LÓGICA OR: (LDR oscuro) O (botón forzado)
                bool oscuroAhora = (valorLDR <= UMBRAL_LDR);
                bool debeEncender = oscuroAhora || forzarEncendido;  // ← OPERADOR OR
                
                // Detección de cambio con debounce
                if (debeEncender != estadoAutoAnterior && (millis() - ultimoCambioLED > 1000)) {
                    estadoAutoAnterior = debeEncender;
                    ultimoCambioLED = millis();
                    
                    // ⭐ LOG MEJORADO: Muestra ambos factores de decisión
                    Serial.print("AUTO [LDR:");
                    Serial.print(valorLDR);
                    Serial.print(" (");
                    Serial.print(oscuroAhora ? "OSCURO" : "CLARO");
                    Serial.print(") | Forzar:");
                    Serial.print(forzarEncendido ? "SI" : "NO");
                    Serial.print("] -> LEDs ");
                    Serial.println(debeEncender ? "ON" : "OFF");
                }
                
                // ⭐ RETORNA 255 si CUALQUIERA de las condiciones es verdadera
                return debeEncender ? 255 : 0;
            }
            case 5: // MANUAL
                return -1;
            default:
                return 100;
        }
    }
    
    void manejarModoFiesta(LED** leds, int totalLEDs) {
        if (millis() - ultimoCambioFiesta > 200) {
            for (int i = 0; i < totalLEDs; i++) {
                leds[i]->apagar();
            }
            
            leds[ledFiestaActual]->encender();
            
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

// ⭐ NUEVO NOMBRE: Botón 0 ahora es "P4 ForzarAuto" en vez de "P4 AtajoAuto"
Boton* botones[9];
SistemaModos modos;

Componente* componentes[30];
int numComponentes = 0;

// ============================================================================
// SECCIÓN 6: CONFIGURACIÓN INICIAL (SETUP) - ACTUALIZADO
// ============================================================================

void setup() {
    Serial.begin(115200);
    
    // ⭐ ENCABEZADO MEJORADO: Explica nueva funcionalidad
    Serial.println("=== INICIANDO CASA INTELIGENTE ===");
    Serial.println("Umbral LDR: <= 400 = OSCURO (LEDs ON)");
    Serial.println("Modo AUTO: Lógica OR (LDR oscuro O Botón P4 forzado)");
    
    // 1. INICIALIZAR LEDs
    leds[0] = new LED(21, "CUARTO1");
    leds[1] = new LED(19, "CUARTO2");
    leds[2] = new LED(5, "CUARTO3");
    leds[3] = new LED(15, "SALA");
    leds[4] = new LED(22, "COCINA");
    leds[5] = new LED(23, "PATIO DELANTERO");
    leds[6] = new LED(13, "PATIO TRASERO");
    leds[7] = new LED(18, "PATIO INTERIOR");
    
    // 2. APAGAR TODOS LOS LEDs INICIALMENTE
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    // 3. INICIALIZAR BOTONES
    // ⭐ CAMBIO: Botón P4 ahora tiene nueva funcionalidad
    botones[0] = new Boton(4, "P4 ForzarAuto");  // ← NUEVO NOMBRE REFLEJA FUNCIÓN
    botones[1] = new Boton(26, "Boton C1");
    botones[2] = new Boton(27, "Boton C2");
    botones[3] = new Boton(14, "Boton C3");
    botones[4] = new Boton(2, "Boton Sala");
    botones[5] = new Boton(12, "Boton Cocina");
    botones[6] = new Boton(25, "Boton P.Del");
    botones[7] = new Boton(33, "Boton P.Tras");
    botones[8] = new Boton(32, "Boton P.Int");
    
    // 4. AGREGAR COMPONENTES AL ARRAY POLIMÓRFICO
    for (int i = 0; i < 8; i++) {
        componentes[numComponentes++] = leds[i];
    }
    
    componentes[numComponentes++] = &ldr;
    componentes[numComponentes++] = &pot;
    componentes[numComponentes++] = &lcd;
    
    for (int i = 0; i < 9; i++) {
        componentes[numComponentes++] = botones[i];
    }
    
    // 5. INICIALIZAR COMPONENTES
    Serial.println("\n=== INICIALIZANDO COMPONENTES ===");
    for (int i = 0; i < numComponentes; i++) {
        componentes[i]->iniciar();
        Serial.print("- ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" en GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
    // ⭐ INFORMACIÓN ACTUALIZADA DE BOTONES
    Serial.println("\n=== BOTONES CONFIGURADOS ===");
    Serial.println("P4 (GPIO4): Forzar encendido en modo AUTO");
    Serial.println("GPIO26: Cuarto 1");
    Serial.println("GPIO27: Cuarto 2");
    Serial.println("GPIO14: Cuarto 3");
    Serial.println("GPIO2: Sala");
    Serial.println("GPIO12: Cocina");
    Serial.println("GPIO25: Patio Delantero");
    Serial.println("GPIO33: Patio Trasero");
    Serial.println("GPIO32: Patio Interior");
    
    // ⭐ DESCRIPCIÓN MEJORADA DE MODO AUTO
    Serial.println("\n=== MODOS CON BRILLOS ===");
    Serial.println("NOCHE: 20% (51)");
    Serial.println("LECTURA: 40% (102)");
    Serial.println("RELAJ: 30% (76)");
    Serial.println("AUTO: Umbral 400 + Forzado por P4 (Lógica OR)");  // ← NUEVA DESCRIPCIÓN
    Serial.println("MANUAL: Todos apagados al inicio");
    Serial.println("===================================\n");
}

// ============================================================================
// SECCIÓN 7: BUCLE PRINCIPAL (LOOP) - CON NUEVA LÓGICA
// ============================================================================

void loop() {
    // 1. LEER LDR
    int valorLDR = ldr.leer();
    bool estaOscuro = ldr.esOscuro();
    
    // 2. ACTUALIZAR MODO CON POTENCIÓMETRO
    int modoPot = pot.getModo();
    modos.cambiarModo(modoPot);
    
    // ============================================================================
    // 3. NUEVA LÓGICA: BOTÓN P4 PARA FORZAR ENCENDIDO (LÓGICA OR)
    // ============================================================================
    if (botones[0]->presionado()) {
        if (modos.esModoAuto()) {
            // ⭐ CASO 1: Ya está en modo AUTO → solo alterna forzado
            modos.toggleForzarEncendido();
        } else {
            // ⭐ CASO 2: No está en modo AUTO → cambia a AUTO Y activa forzado
            modos.cambiarModoAuto();
            modos.toggleForzarEncendido();
        }
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
    
    // 5. APLICAR MODO ACTUAL A LEDs
    // ⭐ NOTA: La lógica OR ya está implementada en getBrilloPorModo()
    if (!modos.esModoManual()) {
        if (modos.esModoFiesta()) {
            modos.manejarModoFiesta(leds, 8);
        } else {
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
    
    // 6. CONTAR LEDs ENCENDIDOS
    int ledsOn = 0;
    for (int i = 0; i < 8; i++) {
        if (leds[i]->estaEncendido()) ledsOn++;
    }
    
    // 7. MOSTRAR EN LCD
    lcd.mostrarInfo(valorLDR, modos.getModoActual(), ledsOn);
    
    // ============================================================================
    // 8. LOG EN SERIAL MEJORADO (muestra estado de forzado)
    // ============================================================================
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
        
        // ⭐ NUEVO: Muestra estado de forzado cuando está en modo AUTO
        if (modos.esModoAuto()) {
            Serial.print(" | Forzar: ");
            Serial.print(modos.isForzarEncendido() ? "ON" : "OFF");
        }
        
        Serial.print(" | LEDs: ");
        Serial.print(ledsOn);
        Serial.println("/8");
        
        ultimoLog = millis();
    }
    
    delay(50);
}
