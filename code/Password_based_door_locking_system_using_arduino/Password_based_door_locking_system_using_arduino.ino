#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>
#include "DHT.h"

#define DHTPIN 10
#define DHTTYPE DHT11
#define MOTORPIN 12
float TEMP_HIGH = 30;
// float HUM_HIGH = 70;
Servo myservo;
int pos = 0;                               // position of servo motor
LiquidCrystal lcd(A4, A5, A3, A2, A1, 11); // creation objet
const byte rows = 4;
const byte cols = 3;
DHT dht(DHTPIN, DHTTYPE); // creation d'objet smito dht
int seuilLuminosite = 400;
const int ldrPin = A0;     // Capteur de lumière (LDR) sur la broche analogique A0
const int ledPin_lum = 13; // LED (éclairage) sur la broche numérique 13

unsigned long lastTempCheck = 0;
const unsigned long TEMP_INTERVAL = 2000;
unsigned long lastLightCheck = 0;
const unsigned long LIGHT_INTERVAL = 1000;
float temp = 0; 
float hum = 0;

void handleTemperature()
{
  if (millis() - lastTempCheck >= TEMP_INTERVAL)
  {
    lastTempCheck = millis();

    temp = dht.readTemperature();
    hum  = dht.readHumidity();

    if (!isnan(temp))
    {
      if (temp >= TEMP_HIGH)
        digitalWrite(MOTORPIN, HIGH);
      else
        digitalWrite(MOTORPIN, LOW);
        displayTempHum();
    }
  }
}

void handleLight()
{
  if (millis() - lastLightCheck >= LIGHT_INTERVAL)
  {
    lastLightCheck = millis();

    int valeurLDR = analogRead(ldrPin);

    if (valeurLDR < seuilLuminosite)
      digitalWrite(ledPin_lum, HIGH);
    else
      digitalWrite(ledPin_lum, LOW);
  }
}

void displayTempHum()
{
  lcd.setCursor(0, 3);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print((char)223);
  lcd.print("C  Hum: ");
  lcd.print(hum, 0);
  lcd.print("%   ");
}


char enteredCode[5];
int index = 0;
struct Student
{
  const char *code;
  const char *name;
  bool present;
};

Student students[] = {
    {"1234", "ABDO", false},
    {"2345", "HOUDA", false},
    {"3456", "ALI", false}};

const int studentsCount = sizeof(students) / sizeof(students[0]);

const char *staffCodes[] = {
    "9999",
    "8888"};

const int staffCount = sizeof(staffCodes) / sizeof(staffCodes[0]);

char key[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[rows] = {2, 3, 4, 5};
byte colPins[cols] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(key), rowPins, colPins, rows, cols); // creation objet
bool idleScreenShown = false;

// Cette fonction initialise tous les composants du système :
// communication série, écran LCD, capteurs, moteur servo et sorties.
void setup()
{
  Serial.begin(9600);
  Serial.println(F("SYSTEM_READY"));
  myservo.attach(9); // Servo motor connection
  lcd.begin(20, 4);
  dht.begin();
  pinMode(MOTORPIN, OUTPUT);
  pinMode(ledPin_lum, OUTPUT);
  displayscreen();
  
}

// Cette fonction s'exécute en boucle :
// - lit la lumière et contrôle la LED
// - lit la température et contrôle le moteur
// - lit le clavier
// - vérifie le code
// - gère l'accès et la présence
void loop()
{
  handleTemperature();
  handleLight();

  char key = keypad.getKey();
  if (key == NO_KEY)
  {
    if (!idleScreenShown)
    {
      clearMainArea();
      displayscreen();
      idleScreenShown = true;
    }
    return;
  }

  if (key == '*')
  {
    index = 0;
    clearMainArea();
    displayscreen();
    return;
  }

  if (index < 4 && key >= '0' && key <= '9')
  {
    if (index == 0)
    {
      clearMainArea();
      lcd.setCursor(0, 0);
      lcd.print(F("PASSWORD:"));
    }

    enteredCode[index++] = key;
    lcd.setCursor(7 + index - 1, 1);
    lcd.print("*");

    if (index == 4)
    {
      enteredCode[4] = '\0';
      ///////////////////////////////////////////////////////////////
      int studentIndex = findStudent(enteredCode);

      if (studentIndex != -1)
      {
        students[studentIndex].present = true;
        welcomeStudent(students[studentIndex].name);
        unlockdoor();
      }
      else if (isStaff(enteredCode))
      {
        unlockdoor();
      }
      else
      {
        incorrect();
      }

      memset(enteredCode, 0, sizeof(enteredCode));
      index = 0;
      lcd.clear();
      displayscreen();
    }
  }
}

// Cette fonction cherche le code de l'étudiant dans la liste.
// Elle retourne l'indice de l'étudiant si trouvé, sinon -1.
int findStudent(const char *code)
{
  for (int i = 0; i < studentsCount; i++)
  {
    if (strcmp(code, students[i].code) == 0)
      return i;
  }
  return -1;
}

// Cette fonction vérifie si le code appartient au staff.
bool isStaff(const char *code)
{
  for (int i = 0; i < staffCount; i++)
  {
    if (strcmp(code, staffCodes[i]) == 0)
      return true;
  }
  return false;
}

void clearMainArea()
{
  for (int i = 0; i < 3; i++)
  {
    lcd.setCursor(0, i);
    lcd.print("                    "); // 20 spaces
  }
}


// Cette fonction affiche le nom de l'étudiant sur l'écran LCD
// et envoie sa présence vers le PC par le port série.
void welcomeStudent(const char *name)
{
  clearMainArea();
  lcd.setCursor(6, 0);
  lcd.print(F("BIENVENUE"));
  lcd.setCursor(6, 1);
  lcd.print(name);
  Serial.print(name);
  Serial.println(",PRESENT");
  delay(2000);
}

// Cette fonction ouvre la porte avec le moteur servo,
// attend quelques secondes, puis la referme automatiquement.

void unlockdoor()
{
  delay(500);

  clearMainArea(); 

  lcd.setCursor(3, 0);
  lcd.print(F("ACCES AUTORISE"));

  lcd.setCursor(3, 1);
  lcd.print(F("PORTE  OUVERTE"));

  for (pos = 180; pos >= 0; pos -= 5)
  {
    myservo.write(pos);
    delay(10);
  }

  delay(2000);

  counterbeep(); 

  delay(500);

  for (pos = 0; pos <= 180; pos += 5)
  {
    myservo.write(pos);
    delay(15);
  }

  delay(500);

  clearMainArea();
  displayscreen();
}


// Cette fonction affiche un message d'erreur
// quand le code entré est incorrect.
void incorrect()
{
  clearMainArea();

  lcd.setCursor(3, 0);
  lcd.print(F("CODE INCORRECT"));

  lcd.setCursor(3, 1);
  lcd.print(F("Acces   refuse"));

  Serial.println(F("CODE INCORRECT"));

  delay(1500);

  clearMainArea();
  displayscreen();
}


// Cette fonction efface le contenu de l'écran LCD.
void clearscreen()
{
  lcd.setCursor(0, 0);
  lcd.println(" ");
  lcd.setCursor(0, 1);
  lcd.println(" ");
}

// Cette fonction affiche le message principal
// pour demander à l'utilisateur d'entrer le code.
void displayscreen()
{
  lcd.setCursor(0, 0);
  lcd.print("   Systeme d'acces  ");
  lcd.setCursor(0, 1);
  lcd.print("   Entrez  le code   ");
}

//--------------Function 5 - Count down------------------//
void counterbeep()
{
  for (int i = 5; i >= 1; i--)
  {
    clearMainArea();

    lcd.setCursor(2, 0);
    lcd.print(F("ENTREZ RAPIDEMENT"));

    lcd.setCursor(9, 1);
    lcd.print(i);

    delay(700);
  }

  clearMainArea();

  lcd.setCursor(4, 1);
  lcd.print(F("FERMETURE..."));

  delay(1000);

  clearMainArea();

  lcd.setCursor(4, 1);
  lcd.print(F("PORTE FERMEE"));

  delay(800);
}

