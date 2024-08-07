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
#define VERSION "0.4.2"
#define AUTHOR "Christophe BOBILLE - www.locoduino.org"

#include <Arduino.h>

const uint8_t nbSensors = 16; // Choisir le nombre d'entrées souhaitées

// pins
const byte dataInPin = 0;   // entrée des données depuis un autre Arduino dans la chaîne S88 pin = 0
const byte dataOutPin = 1;  // sortie des données vers un autre Arduino dans la chaîne ou vers la centrale pin = 1
const byte clockS88pin = 2; // horloge du bus S88 pin = 2
const byte PSS88pin = 3;    // signal PS du bus S88 pin = 3
const byte beginPin = 4;    // première broche utilisée pour les capteurs
const byte endPin8 = 12;    // dernière broche pour  8 capteurs
const byte endPin16 = 19;   // dernière broche pour 16 capteurs
byte endPin;

/*--------------------------------------------------------------------------------------------------------------------*/

uint16_t clockCounter;    // compteur de tops horloge
/* ---- Ne décommenter cette ligne que dans le cas où vous utilisez une ECOS -------
uint32_t loopCounter = 0; // reset proper à l’ECOS
----------------------------------------------------------------------------------*/
uint16_t sensors;         // tampon de 16 bits pour les capteurs
uint16_t data = 0xFFFF;   // le registre à décalage

// routine d’interruption du signal PS
// (déclenchement d’un nouveau cycle d’horloge)
void PS()
{
  clockCounter = 0; // on remet le compteur à zéro
  data = sensors;   // on vide le tampon des capteurs dans le registre à décalage
  sensors = 0;      // on remet à zéro le tampon des capteurs
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
  if (nbSensors == 8) // MAJ des broches concernées
    endPin = endPin8;
  else
    endPin = endPin16;

  pinMode(dataInPin, INPUT_PULLUP);                                   // pin 0 = entrée des données depuis un autre Arduino
  pinMode(dataOutPin, OUTPUT);                                        // pin 1 = sortie des données vers la centrale ou vers un autre Arduino dans le chaînage S88
  pinMode(clockS88pin, INPUT_PULLUP);                                 // init de la broche pour l’horloge
  attachInterrupt(digitalPinToInterrupt(clockS88pin), clock, RISING); // horloge sur int 0 sur la broche 2
  pinMode(PSS88pin, INPUT_PULLUP);                                    // init de la broche du signal PS
  attachInterrupt(digitalPinToInterrupt(PSS88pin), PS, RISING);       // PS sur int1 sur la broche 3
  pinMode(dataInPin, INPUT_PULLUP);                                   // pin 0 = entrée des données depuis un autre Arduino
  pinMode(dataOutPin, OUTPUT);                                        // pin 1 = sortie des données vers la centrale ou vers un autre Arduino dans le chaînage S88
  for (int i = beginPin; i < endPin; i++)
    pinMode(i, INPUT_PULLUP); // init des broches des capteurs
}

void loop()
{

  /* ---- Ne décommenter ces lignes que dans le cas où vous utilisez une ECOS -------

    if (loopCounter == 20) {
      bitSet(sensors, 0);            // reset des tampons des capteurs pour l’ECOS
    }
  ----------------------------------------------------------------------------------*/

  for (byte i = 0; i < nbSensors; i++)
  { // MAJ des capteurs
    if (!digitalRead(beginPin + i))
      sensors |= 1 << i;
    else
      sensors &= ~(1 << i);
  }
}
