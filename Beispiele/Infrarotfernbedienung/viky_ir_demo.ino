//Benötigte Bibliotheken

#include <Otto.h>
Otto Otto;
#include <IRremote.h>
#include <PlayRtttl.hpp>

//

const char data[] = "VARIABLE#";
unsigned long int matrix;

int currentState = 0;
int previousState = 0;


//Wir definieren die PINS

//Für die LED Matrix
#define CLK A1         // Clock Pin
#define CS A2          // Chip Select (CS) Pin
#define DIN A3         // Data In Pin
#define Orientation 0  // 8x8 LED Matrix Ausrichtung  Oben  = 1, Unten = 2, Links = 3, Rechts = 4

//IR-Empfänger
#define IR_pin 7

#define LeftLeg 2    // linkes Bein Pin, servo[0]
#define RightLeg 3   // rechtes Bein Pin, servo[1]
#define LeftFoot 4   // linker Fuß Pin, servo[2]
#define RightFoot 5  // rechter Fuß Pin, servo[3]
#define Buzzer 13    //Buzzer Pin

//Ultraschallsensor
#define Trigger 8
#define Echo 9


/*
Funktion, um ein Infrarotsignal mit dem IR-Empfänger zu empfangen und zu decodieren. 
Der decodierte Infrarotcode wird dann zurückgegeben, sofern die Dekodierung erfolgreich war. 
Andernfalls wird 0 zurückgegeben
*/
long code = 0;
IRrecv ir_rx(IR_pin);
decode_results ir_rx_results;

unsigned long fnc_ir_rx_decode() {
  bool decoded = false;
  if (ir_rx.decode(&ir_rx_results)) {
    decoded = true;
    ir_rx.resume();
  }
  if (decoded)
    return ir_rx_results.value;
  else
    return 0;
}

//Funktion für eine einfache Ultraschall-Distanzmessung

long ultrasound_distance_simple() {
  long duration, distance;     //Es werden zwei Variablen deklariert: "duration" und "distance"
  digitalWrite(Trigger, LOW);  //Trigger-Pin auf LOW, um sicherzustellen, dass der Ultraschallsensor nicht aktiv ist
  delayMicroseconds(2);
  digitalWrite(Trigger, HIGH);  //Trigger-Pin auf HIGH, um den Ultraschallsensor zu aktivieren
  delayMicroseconds(10);
  digitalWrite(Trigger, LOW);      //setzt den Trigger-Pin wieder auf LOW, um den Ultraschallsensor zu deaktivieren und den Ultraschallimpuls zu senden
  duration = pulseIn(Echo, HIGH);  //erfasst die Dauer des Echo-Impulses
  distance = duration / 58;

  /*
Der Wert 58 ist ein empirischer Faktor, der verwendet wird, 
um die Dauer des Echo-Impulses in Zentimeter umzurechnen.
Es basiert auf der Schallgeschwindigkeit und der Tatsache, 
dass der Schall eine Strecke von 2 cm (hin und zurück) 
in etwa 58 Mikrosekunden zurücklegt.
*/
  return distance;
}


void setup() {
  Serial.begin(115200);

  Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true, Buzzer);
  /*
initialisiert die Otto-bibliothek. Es werden die Pins für das linke Bein (LeftLeg), 
das rechte Bein (RightLeg), den linken Fuß (LeftFoot), den rechten Fuß (RightFoot), 
den Piezo-Summer (Buzzer) und der Modus für das Hochladen von Bewegungen (true) übergeben.
*/

  Otto.home();
  pinMode(A0, INPUT);   //konfiguriert den Pin A0 als Eingang. Der Pin kann verwendet werden, um analoge Signale zu lesen.
  pinMode(13, OUTPUT);  //konfiguriert den Pin 13 als Ausgang. Der Pin kann verwendet werden, um ein digitales Signal auszugeben.
  pinMode(8, OUTPUT);
  pinMode(9, INPUT);
  ir_rx.enableIRIn();  //aktiviert den IR-Empfänger, um Infrarotsignale zu empfangen.

  Otto.initMATRIX(DIN, CS, CLK, Orientation);
  Otto.sing(S_happy);
  Otto.clearMouth();
  Otto.writeText("HI", 80);  // begrenzt auf GROSSBUCHSTABEN/ZIFFERN : ; < > = @, MAX.9 Zeichen
  matrix = 0b010010111111111111011110001100;
  Otto.putMouth(matrix, false);
  delay(1 * 1000);
  delay(1 * 1000);
  Otto.clearMouth();
  Otto.swing(1, 1000, 25);
  Otto.playGesture(OttoHappy);
  Otto.swing(4, 1000, 25);
  Otto.matrixIntensity(5);  //the brightness of the LED matrix use values from 0 to 15 only
  ir_rx.enableIRIn();
}

void loop() {
  code = (unsigned long)fnc_ir_rx_decode();
  //ir_rx.resume();

  //Bewegungen

  if (code == 0x00FF18E7) {
    Serial.println(code);
    Otto.walk(1, 1000, 1);  // Vorwärts
    currentState = 1;
    Serial.println(currentState);
  }

  if (code == 0x00FF4AB5) {
    Serial.println(code);
    Otto.walk(1, 1000, -1);  // Rückwärts
    currentState = 2;
    Serial.println(currentState);
  }

  if (code == 0x00FF5AA5) {
    Serial.println(code);
    Otto.turn(1, 1000, 1);  // Links
    currentState = 4;
    Serial.println(currentState);
  }

  if (code == 0x00FF10EF) {
    Serial.println(code);
    Otto.turn(1, 1000, -1);  // Rechts
    currentState = 3;
    Serial.println(currentState);
  }

  // Wenn Knopf gedrueckt wird, sendet Fernbedienung FFFFFFFF (Wiederholungscode)

  if (code == 0xFFFFFFFF) {
    // Wenn der Knopf gedrückt gehalten wird, führe die vorherige Aktion erneut aus
    if (previousState == 1) {
      Otto.walk(1, 1000, 1);  // Vorwärts
    } else if (previousState == 2) {
      Otto.walk(1, 1000, -1);  // Rückwärts
    } else if (previousState == 3) {
      Otto.turn(1, 1000, -1);  // Rechts
    } else if (previousState == 4) {
      Otto.turn(1, 1000, 1);  // Links
    } else if (previousState == 5) {
      previousState = currentState;
    }
  } else {
    // Aktualisiere den vorherigen Status nur, wenn der Code nicht 0xFFFFFFFF ist
    previousState = currentState;
  }

  //Andere Tasten

  if (code == (0x00FF38C7)) {  //Taste ok
    Otto.home();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FFA25D)) {  //Taste 1
    Otto.updown(1, 1000, 25);
    currentState = 5;
    previousState = currentState;
    Otto.sing(S_superHappy);
    delay(1000);
    ir_rx.enableIRIn();
  }


  if (code == (0x00FF629D)) {  //Taste 2
    Otto.playGesture(OttoWave);
    Otto.moonwalker(4, 1000, 25, 1);
    currentState = 5;
    previousState = currentState;
    ir_rx.enableIRIn();
  }


  if (code == (0x00FFE21D)) {  //Taste 3
    Otto.bend(1, 1000, -1);
    playRtttlBlockingPGM(13, A_Team);
    Otto.playGesture(OttoVictory);
    delay(1000);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FF22DD)) {  //Taste 4
    Otto.swing(1, 1000, 25);
    Otto.playGesture(OttoSuperHappy);
    Otto.updown(2, 1000, 25);
    Otto.tiptoeSwing(5, 1000, 25);
    Otto.playGesture(OttoVictory);
    Otto.jitter(5, 1000, 25);
    Otto.ascendingTurn(5, 1000, 25);
    delay(1000);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FF02FD)) {  //Taste 5 Ultraschallsensor test, beim Fernbedienungsdrücken die Hand vor dem Sensor halten.
    Otto.updown(1, 1000, 25);
    if (ultrasound_distance_simple() < 15) {
      Otto.walk(2, 1000, -1);  //Rueckwaerts laufen
      //Otto.playGesture(OttoSad); laesst ihn umfallen
      Otto.playGesture(OttoAngry);
      Otto.playGesture(OttoFretful);
      ir_rx.enableIRIn();
      currentState = 5;
      previousState = currentState;
    }
  }


  if (code == (0x00FFC23D)) {  //Taste 6
    Otto.playGesture(OttoMagic);
    playRtttlBlockingPGM(13, TakeOnMe);
    Otto.playGesture(OttoLove);
    delay(1000);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FFE01F)) {  //Taste 7
    Otto.updown(1, 1000, 25);
    Otto.initMATRIX(DIN, CS, CLK, Orientation);
    Otto.clearMouth();
    //Otto.writeText ( "HALLO",80); //begrenzt auf GROSSBUCHSTABEN/ZIFFERN : ; < > = @, MAX.9 Zeichen
    matrix = 0b010010111111111111011110001100;  //Herz
    Otto.putMouth(matrix, false);
    delay(2 * 1000);
    delay(1 * 1000);
    Otto.writeText("ICH BIN", 80);  //begrenzt auf GROSSBUCHSTABEN/ZIFFERN : ; < > = @, MAX.9 Zeichen
    Otto.writeText("VIKY", 80);     //begrenzt auf GROSSBUCHSTABEN/ZIFFERN : ; < > = @, MAX.9 Zeichen
    Otto.playGesture(OttoLove);
    delay(1 * 1000);
    Otto.clearMouth();
    Otto.putMouth(happyOpen);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }


  if (code == (0x00FFA857)) {  //Taste 8
    Otto.updown(1, 1000, 25);
    Otto.sing(S_superHappy);
    delay(1000);
    Otto.playGesture(OttoConfused);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FF906F)) {  //Taste 9
    Otto.updown(1, 1000, 25);
    Otto.shakeLeg(1, 1000, -1);
    playRtttlBlockingPGM(13, Indiana);
    Otto.playGesture(OttoFart);
    delay(1000);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (code == (0x00FF9867)) {  //Taste 0
    Otto.playGesture(OttoFail);
    //Otto.playGesture(OttoFart);
    Otto.playGesture(OttoSleeping);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }

  if (digitalRead(A0)) {  //Touchsensor
    Otto.updown(1, 1000, 25);
    Otto.sing(S_superHappy);
    delay(1000);
    ir_rx.enableIRIn();
    currentState = 5;
    previousState = currentState;
  }
}