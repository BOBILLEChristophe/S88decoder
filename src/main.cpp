/*--------------------------------------------------------------------------------------------------------------------

Programme destiné à réaliser un décodeur S88 à partir d’un Arduino UNO, NANO ou MEGA

Ce décodeur peut-être programmé pour 8 ou 16 entrées. Le choix est à faire
en début de programme ligne 5 : const uint8_t nbSensors = 16;

Le schéma de câblage est disponible ici :

Il est possible de relier plusieurs décodeurs entre eux en reliant la dataOutPin du précédent avec la dataInPin de celui-ci

Ce programme est basé sur une réalisation de Jean_Pierre CLAUDE disponible ici : https://www.locoduino.org/spip.php?article138

Il a été testé avec le logiciel Rocrail®.

Pour une utilisation avec une centrale ECOS®, veillez à décommander les lignes concernée.

--------------------------------------------------------------------------------------------------------------------*/

#define PROJECT "S88 decoder"
#define VERSION "0.4.3"
#define AUTHOR "Christophe BOBILLE - www.locoduino.org"

#include <Arduino.h>

const uint8_t nbSensors = 16; // Choisir le nombre d'entrées souhaitées

// pins
const byte dataInPin = 0;   // entrée des données depuis un autre Arduino dans la chaîne S88 pin = 0
const byte dataOutPin = 1;  // sortie des données vers un autre Arduino dans la chaîne ou vers la centrale pin = 1
const byte clockS88pin = 2; // horloge du bus S88 pin = 2
const byte PSS88pin = 3;    // signal PS du bus S88 pin = 3

#if defined(ARDUINO_AVR_NANO)
const byte sensorInPin[] = {4, 5, 6, 7, 8 ,9 ,10, 11, 12, 14, 15, 16, 17, 18, 19, 13 };
#elif defined(ARDUINO_AVR_UNO)
const byte sensorInPin[] = {4, 5, 6, 7, 8 ,9 ,10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

volatile uint16_t clockCounter;    // compteur de tops horloge

/* ---- Ne décommenter cette ligne que dans le cas où vous utilisez une ECOS -------
volatile uint32_t loopCounter = 0; // reset proper à l’ECOS
----------------------------------------------------------------------------------*/
volatile uint16_t sensorsBuffer;         // tampon de 16 bits pour les capteurs
volatile uint16_t data = 0xFFFF;   // le registre à décalage

// routine d’interruption du signal PS
// (déclenchement d’un nouveau cycle d’horloge)
void PS()
{
  clockCounter = 0; // on remet le compteur à zéro
  noInterrupts();   // Désactiver les interruptions pour manipuler `sensors`
  data = sensorsBuffer;   // on vide le tampon des capteurs dans le registre à décalage
  sensorsBuffer = 0;      // on remet à zéro le tampon des capteurs
  interrupts();     // Réactiver les interruptions

  /* ---- Ne décommenter cette ligne que dans le cas où vous utilisez une ECOS -------
  loopCounter++; // Pour l'ECOS, on incrémente le nombre de top d’horloge
  ----------------------------------------------------------------------------------*/
}

// routine d’interruption de l’horloge S88
void clock()
{
  digitalWrite(dataOutPin, bitRead(data, clockCounter)); // on décale 1 bit en sortie
  delayMicroseconds(16);                                 // délai pour le décalage
  bitWrite(data, clockCounter, digitalRead(dataInPin));  // on décale 1 bit en entrée
  clockCounter = (clockCounter + 1) % nbSensors;         // modulo le nombre de capteurs (8 ou 16)
}

void setup()
{
  pinMode(dataInPin, INPUT_PULLUP);                                   // pin 0 = entrée des données depuis un autre Arduino
  pinMode(dataOutPin, OUTPUT);                                        // pin 1 = sortie des données vers la centrale ou vers un autre Arduino dans le chaînage S88
  pinMode(clockS88pin, INPUT_PULLUP);                                 // init de la broche pour l’horloge
  pinMode(PSS88pin, INPUT_PULLUP);                                    // init de la broche du signal PS
  attachInterrupt(digitalPinToInterrupt(clockS88pin), clock, RISING); // horloge sur int 0 sur la broche 2
  attachInterrupt(digitalPinToInterrupt(PSS88pin), PS, RISING);       // PS sur int1 sur la broche 3
  for (int i = 0; i < nbSensors; i++)
    pinMode(sensorInPin[i], INPUT_PULLUP); // init des broches des capteurs
}

void loop()
{

  /* ---- Ne décommenter ces lignes que dans le cas où vous utilisez une ECOS -------
    if (loopCounter == 20) {
      bitSet(sensors, 0);            // reset des tampons des capteurs pour l’ECOS
      loopCounter = 0;
    }
  ----------------------------------------------------------------------------------*/

  for (byte i = 0; i < nbSensors; i++)
  { // MAJ des capteurs
    if (!digitalRead(sensorInPin[i]))
      sensorsBuffer |= 1 << i;
    else
      sensorsBuffer &= ~(1 << i);
  }
}