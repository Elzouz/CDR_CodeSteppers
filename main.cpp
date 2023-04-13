#include <Arduino.h>
#include <cmath>

#define DIR_RIGHT 0
#define STEP_RIGHT 1
#define DIR_LEFT 5
#define STEP_LEFT 6

int microsteps[5][4] = {{0, 0, 0, 1}, {1, 0, 0, 2}, {0, 1, 0, 4}, {1, 1, 0, 8}, {1, 1, 1, 16} };

#define uint unsigned int
#define MS_MODE 3 //selectionne mode de microsteps

//MS1, MS2, MS3 pins pour steppers gauche et droite
#define MS1L 9
#define MS2L 8
#define MS3L 7

#define MS1R 4
#define MS2R 3
#define MS3R 2

//constantes robot (cm) : 
#define wheelsDiam 6.96
#define entrax 16.15 //sera utile pour la rotation

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(STEP_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(STEP_LEFT, OUTPUT);

  pinMode(MS1L, OUTPUT);
  pinMode(MS2L, OUTPUT);
  pinMode(MS3L, OUTPUT);
  pinMode(MS1R, OUTPUT);
  pinMode(MS2R, OUTPUT);
  pinMode(MS3R, OUTPUT);

  digitalWrite(MS1L, microsteps[MS_MODE][0]);
  digitalWrite(MS2L, microsteps[MS_MODE][1]);
  digitalWrite(MS3L, microsteps[MS_MODE][2]);
  digitalWrite(MS1R, microsteps[MS_MODE][0]);
  digitalWrite(MS2R, microsteps[MS_MODE][1]);
  digitalWrite(MS3R, microsteps[MS_MODE][2]);
}

#define DELAY_MIN 1000
#define DELAY_MAX 3000
#define DELAY_CHANGE 100
#define DELAY_CHANGE_COUNT 5

//Distance calculations
const double tour = M_PI*wheelsDiam; //distance pour 1 tour de roue (cm)
int stepsTour = 200*microsteps[MS_MODE][3]; //nombre de steps pour 1 tour de roue
double stepsCm = stepsTour/tour; //nombre de steps pour 1 cm


void Avancer(uint ticks) {
  digitalWrite(DIR_RIGHT, LOW);
  digitalWrite(DIR_LEFT, HIGH);

  int currentDelay = DELAY_MAX;
  int count = 0;

  //Acceleration ou deceleration progressive du robot
  bool acc = true;
  bool decc = false;

  int ticksToDecc = (DELAY_MAX - DELAY_MIN) / DELAY_CHANGE * DELAY_CHANGE_COUNT;
  ticksToDecc = min(ticksToDecc, (int)(ticks / 2));


  for(int i = 0; i < ticks * microsteps[MS_MODE][3]; i++) {
    if(++count == DELAY_CHANGE_COUNT) {
      count = 0;
      if(acc) {
        currentDelay -= DELAY_CHANGE;
        if(currentDelay <= DELAY_MIN) {
         acc = false;
        }
      } 
      else if(decc){
        currentDelay += DELAY_CHANGE;
        if(currentDelay >= DELAY_MAX) {
          decc = false;
        }
      }
    }

    if(i == ticks - ticksToDecc) {
      decc = true;
      acc = false;
    }

    digitalWrite(STEP_RIGHT, HIGH);
    digitalWrite(STEP_LEFT, HIGH);
    delayMicroseconds(currentDelay);
    digitalWrite(STEP_RIGHT, LOW);
    digitalWrite(STEP_LEFT, LOW);
    delayMicroseconds(currentDelay);
  }
}


void Rotate(double angle, char direction)
{
  double stepsAngle = (microsteps[MS_MODE][3]*entrax*M_PI)/(4*tour);
  for(int i = 0; i < stepsAngle * microsteps[MS_MODE][3]; i++) {
    if(direction == 'D') //Roue Gauche doit avancer et droite reculer pour tourner à droite
    {
      digitalWrite(DIR_RIGHT, HIGH);
      digitalWrite(DIR_LEFT, HIGH);      
    }
    if(direction == 'G') //inverse de droite
    {
      digitalWrite(DIR_RIGHT, LOW);
      digitalWrite(DIR_LEFT, LOW);
    }  
    digitalWrite(STEP_RIGHT, HIGH);
    digitalWrite(STEP_LEFT, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_RIGHT, LOW);
    digitalWrite(STEP_LEFT, LOW);
    delayMicroseconds(1000);
  }
}

int posInitx = 0;
int posInity = 0;
int angleInit = 0;

void GoToPoint(int x, int y)
{
  //sauvegarde de l'ancienne position :

  //calcul nombre de pas à faire pour atteindre le point :
  double distance = sqrt((x-posInitx)*(x-posInitx) + (y-posInity)*(y-posInity));
  int stepsDistance = distance*stepsCm;
  double angle = atan2(y-posInity, x-posInitx) - angleInit;
  char direction = 'D';
  if(angle > 180){
    angle = 360 - angle;
    direction = 'G';
  }
  Rotate(angle, direction);
  Avancer(stepsDistance);

  //sauvegarde de la nouvelle position après l'avoir atteint :
  int posx = posInitx + x;
  int posy = posInity + y;
  posInitx = posx;
  posInity = posy;
  angleInit = angle;
}

void loop()
{
  GoToPoint(0, 10);
  GoToPoint(10, 10);
  GoToPoint(20, 20);
  GoToPoint(0, 0);
}


