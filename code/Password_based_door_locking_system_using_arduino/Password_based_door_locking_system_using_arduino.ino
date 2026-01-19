#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>
#include "DHT.h"

#define DHTPIN 10
#define DHTTYPE DHT11
#define LEDPIN 12
float TEMP_HIGH = 30;
float HUM_HIGH = 70;
Servo myservo;
int pos=0; // position of servo motor
LiquidCrystal lcd(A4, A5, A3, A2, A1, 11);
const byte rows=4;
const byte cols=3;
DHT dht(DHTPIN, DHTTYPE);
int seuilLuminosite = 400; 
const int ldrPin = A0;  // Capteur de lumière (LDR) sur la broche analogique A0
const int ledPin_lum = 13;  // LED (éclairage) sur la broche numérique 13


char enteredCode[5];
int index = 0;
struct Student {
  const char* code;
  const char* name;
  bool present;
};

Student students[] = {
  {"1234", "ABDO", false},
  {"2345", "HOUDA", false},
  {"3456", "ALI", false}
};

const int studentsCount = sizeof(students) / sizeof(students[0]);

const char* staffCodes[] = {
  "9999",
  "8888"
};

const int staffCount = sizeof(staffCodes) / sizeof(staffCodes[0]);



 
char key[rows][cols]={
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};



byte rowPins[rows]={2,3, 4, 5};
byte colPins[cols]={6,7,8};
Keypad keypad= Keypad(makeKeymap(key),rowPins,colPins,rows,cols);
char* password="1234";
int currentposition=0;
bool idleScreenShown = false;

 
 
 
void setup()
{
  Serial.begin(9600);
  Serial.println("SYSTEM_READY");
  displayscreen();
  myservo.attach(9); //Servo motor connection
  lcd.begin(16,2);
  dht.begin();
  pinMode(LEDPIN, OUTPUT);
  pinMode(ledPin_lum, OUTPUT);
 
}

void loop()
{
  ////////////////////////////////////////////////////////////////////////////////////////
  // 1. Lecture de la lumière ambiante
  int valeurLDR = analogRead(ldrPin);

  // 2. Logique d'allumage
  // Si la valeur est basse, cela signifie qu'il fait sombre
  if (valeurLDR < seuilLuminosite) {
    digitalWrite(ledPin_lum, HIGH); // Allume la LED
  } 
  else {
    digitalWrite(ledPin_lum, LOW);  // Éteint la LED
  }
  ////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////// TACHE 1 ////////////////////////////////////////////////
  float RH = dht.readHumidity();
  float Temp = dht.readTemperature();
  if (isnan(RH) || isnan(Temp)) {
    Serial.println("DHT Error");
    delay(2000);
    return;
  }
  if (Temp >= TEMP_HIGH || RH >= HUM_HIGH) {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW);
  }
  ///////////////////////////////////////////////////////////////////////////////////////
  
  char key = keypad.getKey();
  if (key == NO_KEY)
  {
      if (!idleScreenShown)
      {
          lcd.clear();
          displayscreen();
          idleScreenShown = true;
      }
      return;
  }

  if (key == '*')
  {
      index = 0;
      lcd.clear();
      displayscreen();
      return;
  }

  if (index < 4 && key >= '0' && key <= '9')
  {
    if (index == 0) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("PASSWORD:");
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


int findStudent(const char* code)
{
  for (int i = 0; i < studentsCount; i++)
  {
    if (strcmp(code, students[i].code) == 0)
      return i;
  }
  return -1;
}

bool isStaff(const char* code)
{
  for (int i = 0; i < staffCount; i++)
  {
    if (strcmp(code, staffCodes[i]) == 0)
      return true;
  }
  return false;
}

void welcomeStudent(const char* name)
{
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("WELCOME");
  lcd.setCursor(5,1);
  lcd.print(name);
  Serial.print(name);
  Serial.println(",PRESENT");
  delay(2000);
}

/////////////////////////////////////////////////////////////////////////
 
//------------------ Function 1- OPEN THE DOOR--------------//
 
void unlockdoor() {
  delay(900);
  
  lcd.setCursor(0,0);
  lcd.println(" ");
  lcd.setCursor(1,0);
  lcd.print("Access Granted");
  lcd.setCursor(4,1);
  lcd.println("WELCOME!!");
  lcd.setCursor(15,1);
  lcd.println(" ");
  lcd.setCursor(16,1);
  lcd.println(" ");
  lcd.setCursor(14,1);
  lcd.println(" ");
  lcd.setCursor(13,1);
  lcd.println(" ");
  for(pos = 180; pos>=0; pos-=5) // open the door
  {
    myservo.write(pos); 
    delay(5); 
  }
  delay(2000);
  delay(1000);
  counterbeep();
  delay(1000); 
  for(pos = 0; pos <= 180; pos +=5) // close the door
  { // in steps of 1 degree
    myservo.write(pos); 
    delay(15);
    currentposition=0;
    lcd.clear();
    displayscreen();
  }
}
 
//--------------------Function 2- Wrong code--------------//
 
void incorrect() {
  delay(500);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("CODE");
  lcd.setCursor(6,0);
  lcd.print("INCORRECT");
  lcd.setCursor(15,1);
  lcd.println(" ");
  lcd.setCursor(4,1);
  lcd.println("GET AWAY!!!");
  lcd.setCursor(13,1);
  lcd.println(" ");
  Serial.println("CODE INCORRECT YOU ARE UNAUTHORIZED");
  delay(3000);
  lcd.clear();
  displayscreen();
}
//-------Function 3 - CLEAR THE SCREEN--------------------/
void clearscreen() {
  lcd.setCursor(0,0);
  lcd.println(" ");
  lcd.setCursor(0,1);
  lcd.println(" ");
}

//------------Function 4 - DISPLAY FUNCTION--------------------//
void displayscreen()
{ 
  lcd.setCursor(0,0);
  lcd.println("*ENTER THE CODE*");
  lcd.setCursor(1 ,1);
  lcd.println("TO OPEN DOOR!!");
}
//--------------Function 5 - Count down------------------//
void counterbeep() {
  delay(1200);
  
  
  lcd.clear();
  
  lcd.setCursor(2,15);
  lcd.println(" ");
  lcd.setCursor(2,14);
  lcd.println(" ");
  lcd.setCursor(2,0);
  delay(200);
  lcd.println("GET IN WITHIN:::");
  
  lcd.setCursor(4,1);
  lcd.print("5");
  delay(200);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  delay(1000);
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  lcd.setCursor(4,1); //2
  lcd.print("4");
  delay(100);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  delay(1000);

  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  lcd.setCursor(4,1); 
  lcd.print("3");
  delay(100);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  delay(1000);

  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  lcd.setCursor(4,1); 
  lcd.print("2");
  delay(100);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN:");
  delay(1000);

  lcd.setCursor(4,1);
  lcd.print("1");
  delay(100);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.println("GET IN WITHIN::");

  delay(1000);
  delay(40);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("RE-LOCKING");
  delay(500);
  lcd.setCursor(12,0);
  lcd.print(".");
  delay(500);
  lcd.setCursor(13,0);
  lcd.print(".");
  delay(500);
  lcd.setCursor(14,0);
  lcd.print(".");
  delay(400);
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("LOCKED!");
  delay(440);
}


void openDoorFromBlynk()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remote Access");

  for(pos = 180; pos>=0; pos-=5)
  {
    myservo.write(pos);
    delay(5);
  }

  delay(3000);

  for(pos = 0; pos<=180; pos+=5)
  {
    myservo.write(pos);
    delay(15);
  }

  lcd.clear();
  displayscreen();
}