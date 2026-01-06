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
    }
    
    void escribir(int valor) override {}
    
    int leer() override {
        return analogRead(pin);
    }
    
    bool esOscuro() {
        return leer() <= UMBRAL_LDR;
    }
};

// ================= CLASE POTENCIÓMETRO (COMPLETA) =================
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

// ================= CLASE BOTÓN (COMPLETA) =================
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

// ================= CLASE LCD (COMPLETA) =================
class PantallaLCD : public Componente {
private:
    bool lcdInicializado;
    
public:
    PantallaLCD(String n = "LCD") : Componente(0, n), lcdInicializado(false) {}
    
    void iniciar() override {
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(100);
        
        // Inicialización estándar de LCD 16x2
        enviarComando(0x03);
        delay(5);
        enviarComando(0x03);
        delay(1);
        enviarComando(0x03);
        delay(10);
        
        enviarComando(0x02);
        delay(1);
        
        enviarComando(0x28);
        enviarComando(0x0C);
        enviarComando(0x06);
        enviarComando(0x01);
        delay(2);
        
        lcdInicializado = true;
        Serial.println("LCD inicializado correctamente");
    }
    
    void escribir(int valor) override {}
    
    int leer() override { return 0; }
    
    void mostrarInfo(int valorLDR, String modo, int ledsEncendidos) {
        if (!lcdInicializado) return;
        
        // Limpiar display
        enviarComando(0x01);
        delay(2);
        
        // Línea 1: "LDR:345 LED:3/8"
        String linea1 = "LDR:" + String(valorLDR) + "  LED:" + String(ledsEncendidos) + "/8";
        escribirLinea(linea1, 0, 0);
        
        // Línea 2: Modo actual
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
        delayMicroseconds(100);
    }
    
    void enviarComando(uint8_t cmd) {
        enviarByte(cmd, 0);
    }
    
    void escribirLinea(String texto, uint8_t col, uint8_t fila) {
        uint8_t direccion = (fila == 0) ? 0x80 : 0xC0;
        direccion += col;
        enviarComando(direccion);
        
        for (int i = 0; i < texto.length() && i < 16; i++) {
            enviarByte(texto.charAt(i), 1);
        }
    }
};

// ================= SISTEMA DE MODOS (COMPLETO) =================
class SistemaModos {
private:
    const char* nombresModos[6] = {"NOCHE", "LECTURA", "RELAJ", "FIESTA", "AUTO", "MANUAL"};
    int modoActual;
    bool autoActivo;
    unsigned long ultimoCambioFiesta;
    int ledFiestaActual;
    bool estadoAutoAnterior;
    unsigned long ultimoCambioLED;
    
public:
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
        modoActual = 4; // Modo AUTO
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
            case 4: // AUTOMÁTICO
            {
                bool oscuroAhora = (valorLDR <= UMBRAL_LDR);
                
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
    Serial.println("Commit 6: Pantalla LCD implementada");
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
    
    // 2. INICIALIZAR BOTONES
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
    
    // 4. INICIALIZAR TODOS LOS COMPONENTES
    Serial.println("\n=== INICIALIZANDO COMPONENTES ===");
    for (int i = 0; i < numComponentes; i++) {
        componentes[i]->iniciar();
        Serial.print("- ");
        Serial.print(componentes[i]->getNombre());
        Serial.print(" en GPIO");
        Serial.println(componentes[i]->getPin());
    }
    
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
    
    Serial.println("\n=== MODOS IMPLEMENTADOS ===");
    Serial.println("NOCHE: 20% (51)");
    Serial.println("LECTURA: 40% (102)");
    Serial.println("RELAJ: 30% (76)");
    Serial.println("FIESTA: Animación secuencial");
    Serial.println("AUTO: Control por LDR (umbral 400)");
    Serial.println("MANUAL: Control por botones");
    
    // 5. APAGAR TODOS LOS LEDs INICIALMENTE
    for (int i = 0; i < 8; i++) {
        leds[i]->apagar();
    }
    
    // 6. MOSTRAR MENSAJE INICIAL EN LCD
    lcd.mostrarInfo(0, "INICIO", 0);
    
    Serial.println("\n=== SISTEMA LISTO ===");
    Serial.println("LCD mostrando información del sistema");
    Serial.println("===================================\n");
}

// ================= LOOP PRINCIPAL =================
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
    
    // 5. APLICAR MODO ACTUAL A LEDs
    if (!modos.esModoManual()) {
        if (modos.esModoFiesta()) {
            // Modo FIESTA (animación)
            modos.manejarModoFiesta(leds, 8);
        } else {
            // Otros modos (NOCHE, LECTURA, RELAJ, AUTO)
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
    
    // 7. MOSTRAR INFORMACIÓN EN LCD
    lcd.mostrarInfo(valorLDR, modos.getModoActual(), ledsOn);
    
    // 8. LOG EN SERIAL
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
    
    delay(50);
}
