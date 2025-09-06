#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>



#define INTERRUPT_PIN1 D7  // Broche D7 (GPIO13)
#define INTERRUPT_PIN2 D6  // Broche D6 (GPIO12)

#define RPWM D8  // Broche 8 (GPIO15)
#define LPWM D3  // Broche 5 (GPIO14)

#define button D5  //bouton start stop, broche D3 (GPIO0)

int count1 = 0;  // Compteur de passages capteur 1
int count2 = 0;  // Compteur de passages capteur 2
unsigned long previousMillis;
unsigned long currentMillis;
int RPM1 = 0;
int RPM2 = 0;

const int V_MAX = 120; // en km/h
int V_input = 0;

const int MAX_RPM = 3350; 
int rpm_input = 0;

int pwm_value = 0;

bool motorState = false; // flag pour l'état du moteur

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27(Cooperate with 3 short circuit caps) for a 16 chars and 2 line display

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ICACHE_RAM_ATTR countPassage1() {
  count1=count1+1;
}

void ICACHE_RAM_ATTR countPassage2() {
  count2=count2+1;
}

void ICACHE_RAM_ATTR toggleMotor() {
    motorState = !motorState;
    if (motorState == true) {
      Serial.println("démarrage moteur");
      lcd.clear();
      lcd.setCursor(0,0); // positionne le curseur à la colonne 1 et à la ligne 1  
      lcd.print("Demarrage moteur");
    }
    else {
      Serial.println("arret moteur");
      lcd.clear();
      lcd.setCursor(0,0); // positionne le curseur à la colonne 1 et à la ligne 1  
      lcd.print("Arret moteur");
    }
    delay(2000);
    lcd.clear();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void setup() {
  Serial.begin(9600);
  delay(1000);
  analogWriteFreq(25000); //ajustement de la fréquence de PWM pour éviter du bruit audible
  pinMode(INTERRUPT_PIN1, INPUT_PULLUP);
  pinMode(INTERRUPT_PIN2, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(button, INPUT_PULLUP);
 
  motorState = false;
  
  previousMillis = millis();
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN1), countPassage1, RISING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN2), countPassage2, RISING);
  attachInterrupt(digitalPinToInterrupt(button), toggleMotor, FALLING);
  
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED on (HIGH is the voltage level)
  delay(5000);                        // wait for 5 seconds
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  
  lcd.init();       //setup affichage lcd
  lcd.backlight();  //setup affichage lcd

  delay(2000);

  Serial.println("Consigne de vitesse: ");
  Serial.println("motorState ");
  Serial.println(motorState);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void loop() {
  if (Serial.available() >0) {
    V_input = Serial.parseInt();
    V_input = constrain(V_input, 0, V_MAX);

    rpm_input = (V_input*1000/60)/(0.254*3.14159);
    
    //rpm_input = Serial.parseInt();
    //rpm_input = constrain(rpm_input, 0, MAX_RPM);
    pwm_value = map(rpm_input, 0, MAX_RPM, 0, 255);

    //analogWrite(RPWM, pwm_value);
    //analogWrite(LPWM, 0);

    Serial.print(V_input);
    Serial.println(" km/h");
    Serial.print(rpm_input);
    Serial.println(" tr/min");
    Serial.print("Valeur de PWM: ");
    Serial.println(pwm_value);
  }

  currentMillis = millis();
    
  if (currentMillis - previousMillis >= 2000) { // toutes les 2 secondes
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN1));
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN2));
    // Calcul du régime en tours par minute
    RPM1 = (count1 * 30.0); // car count sur 2 secondes, à modifier si intervalle différent
    RPM2 = (count2 * 30.0); // car count sur 2 secondes, à modifier si intervalle différent
    count1 = 0; // réinitialiser le compteur
    count2 = 0; // réinitialiser le compteur
    previousMillis = currentMillis;
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN1), countPassage1, RISING);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN2), countPassage2, RISING);
    
    lcd.clear();
    lcd.setCursor(0,0); // positionne le curseur à la colonne 1 et à la ligne 1  
    lcd.print("Vmot1   ");
    lcd.setCursor(8,0); // positionne le curseur à la colonne 8 et à la ligne 1 
    lcd.print(RPM1);
    lcd.setCursor(13,0); // positionne le curseur à la colonne 13 et à la ligne 1
    lcd.print("rpm");
    lcd.setCursor(0,1); // positionne le curseur à la colonne 1 et à la ligne 2  
    lcd.print("Vmot2   ");
    lcd.setCursor(8,1); // positionne le curseur à la colonne 8 et à la ligne 2 
    lcd.print(RPM2);
    lcd.setCursor(13,1); // positionne le curseur à la colonne 13 et à la ligne 2
    lcd.print("rpm");
    
  }
  
  if (motorState == true) {
      analogWrite(RPWM, pwm_value);
      analogWrite(LPWM, 0);
  }
  else {
      analogWrite(RPWM, 0);
      analogWrite(LPWM, 0);
  }
}