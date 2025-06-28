#include <Keypad.h>
#include <LiquidCrystal.h>

// Pines para LCD
int rs = 12;
int e = 11;
int d4 = 5;
int d5 = 4;
int d6 = 3;
int d7 = 2;
LiquidCrystal lcd(rs, e, d4, d5, d6, d7);

// Teclado matricial
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {A0, A1, A2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Sensores y buzzer
const int pirEnterPin = A3;
const int pirExitPin = A4;
const int buzzerPin = 13;

// Contadores
int enterCount = 0;
int exitCount = 0;
int totalAforo = 0;
int contadorMovimientos = 0;

// Control de movimiento
bool movimientoDetectadoEnter = false;
bool movimientoDetectadoExit = false;
unsigned long ultimoTiempoMovimientoEnter = 0;
unsigned long ultimoTiempoMovimientoExit = 0;

//Clave para reiniciar el usuario
String claveIngresada = "";
const String claveCorrecta = "071025";
bool modoClave = false;

// Envío serial periódico
unsigned long ultimoEnvioSerial = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Ingrese aforo:");
  pinMode(pirEnterPin, INPUT);
  pinMode(pirExitPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    // Si estamos en modo ingreso de clave
    if (modoClave) {
      if (isdigit(key)) {
        claveIngresada += key;
        lcd.setCursor(0, 1);
        lcd.print("Clave: ");
        lcd.print(claveIngresada);
      } else if (key == '#') {
        if (claveIngresada == claveCorrecta) {
          lcd.clear();
          lcd.print("Reiniciando...");
          delay(1000);
          enterCount = 0;
          exitCount = 0;
          contadorMovimientos = 0;
          totalAforo = 0;
          lcd.clear();
          lcd.print("Ingrese aforo:");
        } else {
          lcd.clear();
          lcd.print("Clave incorrecta");
          delay(2000);
          lcd.clear();
          lcd.print("Ingrese aforo:");
        }
        claveIngresada = "";
        modoClave = false;
      }
      return;  // Salir de loop() para no ejecutar nada más si estamos en modo clave
    }

    // Activar modo clave con '*'
    if (key == '*') {
      modoClave = true;
      claveIngresada = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ingrese su clave");
      lcd.setCursor(0, 1);
      lcd.print("Clave: ");
      return;
    }
    if (isdigit(key)) {
      totalAforo = totalAforo * 10 + (key - '0');
      lcd.setCursor(0, 1);
      lcd.print(totalAforo);
    } else if (key == '#') {
      lcd.clear();
      lcd.print("Aforo: ");
      lcd.setCursor(7, 0);
      lcd.print(totalAforo);
      lcd.setCursor(0, 1);
      lcd.print("In: 0  Out: 0");
      enterCount = 0;
      exitCount = 0;
      delay(4000);

      // Bucle principal del sistema
      while (true) {
        // PIR entrada
        int pirValueEnter = analogRead(pirEnterPin);
        int pirDigitalValueEnter = (pirValueEnter > 20) ? 1 : 0;
        if (pirDigitalValueEnter == 1 && !movimientoDetectadoEnter) {
          enterCount++;
          lcd.setCursor(4, 1);
          lcd.print(enterCount);
          Serial.println("Movimiento detectado en entrada");
          contadorMovimientos++;
          movimientoDetectadoEnter = true;
          ultimoTiempoMovimientoEnter = millis();
          tone(buzzerPin, 1000);
          delay(500);
          noTone(buzzerPin);
        }

        // PIR salida
        int pirValueExit = analogRead(pirExitPin);
        int pirDigitalValueExit = (pirValueExit > 20) ? 1 : 0;
        if (pirDigitalValueExit == 1 && !movimientoDetectadoExit) {
          exitCount++;
          lcd.setCursor(12, 1);
          lcd.print(exitCount);
          Serial.println("Movimiento detectado en salida");
          contadorMovimientos++;
          movimientoDetectadoExit = true;
          ultimoTiempoMovimientoExit = millis();
        }

        // Reset de detección después de 5 segundos
        if (movimientoDetectadoEnter && millis() - ultimoTiempoMovimientoEnter >= 5000) {
          movimientoDetectadoEnter = false;
        }
        if (movimientoDetectadoExit && millis() - ultimoTiempoMovimientoExit >= 5000) {
          movimientoDetectadoExit = false;
        }

        // Envío periódico por serial cada 1s
        if (millis() - ultimoEnvioSerial >= 1000) {
          Serial.print("IN:");
          Serial.print(enterCount);
          Serial.print(",OUT:");
          Serial.print(exitCount);
          Serial.print(",AFORO:");
          Serial.println(totalAforo);
          ultimoEnvioSerial = millis();
        }

        // Verificar si se alcanzó el aforo
        if (totalAforo > 0 && (enterCount - exitCount >= totalAforo)) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Aforo Max Permitido");
          tone(buzzerPin, 1000);
          delay(1000);
          noTone(buzzerPin);

          while (true) {
            tone(buzzerPin, 1000);
            delay(500);
            noTone(buzzerPin);
            delay(500);
          }
        }
      }
    }
  }
}