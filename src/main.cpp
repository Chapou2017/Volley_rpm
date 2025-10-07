#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Broches ESP32 (modifiable selon ton câblage)
#define INTERRUPT_PIN1 13  // GPIO13
#define INTERRUPT_PIN2 12  // GPIO12
#define RPWM 15            // GPIO15
#define LPWM 14            // GPIO14
#define BUTTON 0           // GPIO0

// PWM channels
#define RPWM_CHANNEL 0
#define LPWM_CHANNEL 1

int count1 = 0;
int count2 = 0;
unsigned long previousMillis;
unsigned long currentMillis;
int RPM1 = 0;
int RPM2 = 0;

const int V_MAX = 120; // km/h
int V_input = 0;

const int MAX_RPM = 3350;
int rpm_input = 0;
int pwm_value = 0;

volatile bool motorState = false;
volatile bool motorToggleRequested = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void IRAM_ATTR countPassage1() {
  count1++;
}

void IRAM_ATTR countPassage2() {
  count2++;
}

void IRAM_ATTR toggleMotor() {
  motorToggleRequested = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void setup() {
  Serial.begin(115200);
  delay(1000);

  // PWM setup
  ledcSetup(RPWM_CHANNEL, 25000, 8); // 25kHz, 8-bit resolution
  ledcAttachPin(RPWM, RPWM_CHANNEL);
  ledcSetup(LPWM_CHANNEL, 25000, 8);
  ledcAttachPin(LPWM, LPWM_CHANNEL);

  pinMode(INTERRUPT_PIN1, INPUT_PULLUP);
  pinMode(INTERRUPT_PIN2, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);

  previousMillis = millis();

  attachInterrupt(INTERRUPT_PIN1, countPassage1, RISING);
  attachInterrupt(INTERRUPT_PIN2, countPassage2, RISING);
  attachInterrupt(BUTTON, toggleMotor, FALLING);

  lcd.init();
  lcd.backlight();

  delay(2000);

  Serial.println("Système prêt.");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void loop() {
  // Gestion du bouton via interruption
  if (motorToggleRequested) {
    motorToggleRequested = false;
    motorState = !motorState;

    lcd.clear();
    lcd.setCursor(0, 0);
    if (motorState) {
      Serial.println("Démarrage moteur");
      lcd.print("Demarrage moteur");
    } else {
      Serial.println("Arrêt moteur");
      lcd.print("Arret moteur");
    }
    delay(2000);
    lcd.clear();
  }

  // Lecture de la consigne via Serial
  if (Serial.available() > 0) {
    V_input = Serial.parseInt();
    V_input = constrain(V_input, 0, V_MAX);

    rpm_input = (V_input * 1000 / 60) / (0.254 * 3.14159);
    pwm_value = map(rpm_input, 0, MAX_RPM, 0, 255);

    Serial.print("Consigne: ");
    Serial.print(V_input);
    Serial.println(" km/h");
    Serial.print("RPM cible: ");
    Serial.println(rpm_input);
    Serial.print("PWM: ");
    Serial.println(pwm_value);
  }

  // Calcul des RPM toutes les 2 secondes
  currentMillis = millis();
  if (currentMillis - previousMillis >= 2000) {
    detachInterrupt(INTERRUPT_PIN1);
    detachInterrupt(INTERRUPT_PIN2);

    RPM1 = count1 * 30;
    RPM2 = count2 * 30;
    count1 = 0;
    count2 = 0;
    previousMillis = currentMillis;

    attachInterrupt(INTERRUPT_PIN1, countPassage1, RISING);
    attachInterrupt(INTERRUPT_PIN2, countPassage2, RISING);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vmot1 ");
    lcd.setCursor(8, 0);
    lcd.print(RPM1);
    lcd.setCursor(13, 0);
    lcd.print("rpm");

    lcd.setCursor(0, 1);
    lcd.print("Vmot2 ");
    lcd.setCursor(8, 1);
    lcd.print(RPM2);
    lcd.setCursor(13, 1);
    lcd.print("rpm");
  }

  // Commande du moteur
  if (motorState) {
    ledcWrite(RPWM_CHANNEL, pwm_value);
    ledcWrite(LPWM_CHANNEL, 0);
  } else {
    ledcWrite(RPWM_CHANNEL, 0);
    ledcWrite(LPWM_CHANNEL, 0);
  }
}
