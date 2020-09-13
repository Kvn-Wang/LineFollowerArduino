//libreria per i sensori infrarossi
#include <QTRSensors.h>

#define MOTORE_DESTRO       0
#define MOTORE_SINISTRO     1

#define MOTORE_AVANTI       0
#define MOTORE_FERMO        1
#define MOTORE_INDIETRO     2

//valori dei diversi range
#define DESTRA_ESTERNO 1000
#define DESTRA_INTERNO 1500
#define DESTRA_CENTRO_ESTERNO 2300
#define SINISTRA_CENTRO_ESTERNO 2700
#define SINISTRA_INTERNO 3500
#define SINISTRA_ESTERNO 4000

//potenza motori
#define POTENZA_MASSIMA 65
#define POTENZA_ALTA 60
#define POTENZA_MEDIA 50
#define POTENZA_BASSA 40

//variabili direzionali dei motori
uint8_t InPinA[] = {9, 5};
uint8_t InPinB[] = {10, 6};

//variabile per regolare la potenza dei motori
uint8_t InMotorePwm[] = {11, 3 };

QTRSensors qtr;

//variabili che indica numero di sensori e il loro valore
const uint8_t NumeroSensori = 6;
uint16_t ValoriSensori[NumeroSensori];

void setup()
{
  //configurazione dei PIN di INPUT-OUTPUT
  pinMode(InPinA[0], OUTPUT);
  pinMode(InPinA[1], OUTPUT);
  pinMode(InPinB[0], OUTPUT);
  pinMode(InPinB[1], OUTPUT);
  pinMode(InMotorePwm[0], OUTPUT);
  pinMode(InMotorePwm[1], OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Configurazione dei sensori a infrarossi
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5}, NumeroSensori);

  //accendi il led Built-in dell'Arduino per segnalare l'inizio della calibrazione
  digitalWrite(LED_BUILTIN, HIGH); 
  //calibrazione dei sensori ( circa 12s )
  for (uint16_t i = 0; i < 400; i++)
  {
    qtr.calibrate();
  }
  //spegni il led per segnalare la fine della calibrazione
  digitalWrite(LED_BUILTIN, LOW);
}

//funzione per selezionare,dirigere e regolare la potenza del motore
void muoviMotore(uint8_t motore, uint8_t direzione, uint8_t pwm)
{
  if(direzione <= 1)
  {
    digitalWrite(InPinA[motore], LOW);
  }
  else
  {         
    digitalWrite(InPinA[motore], HIGH);
  }
  if(direzione >=1 && direzione <=2)
  {
    digitalWrite(InPinB[motore], LOW);
  }
  else
  {
    digitalWrite(InPinB[motore], HIGH);
  }
  analogWrite(InMotorePwm[motore], ((255/100)*pwm));
}

void loop()
{
  //variabile che indica la posizione della linea nera, sapendo i valori letti dai 6 sensori
  uint16_t position = qtr.readLineBlack(ValoriSensori);
  
  //se la linea nera è a sinitra
  if(position>SINISTRA_ESTERNO)
  { 
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_INDIETRO, POTENZA_ALTA);
  }
  else if(position>=SINISTRA_INTERNO && position<=SINISTRA_ESTERNO)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MASSIMA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }
  else if(position>SINISTRA_CENTRO_ESTERNO && position<SINISTRA_INTERNO)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_ALTA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }

  //se linea nera è al centro
  else if(position>=DESTRA_CENTRO_ESTERNO && position<=SINISTRA_CENTRO_ESTERNO)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }
  
  //se la linea nera è a destra
  else if(position>DESTRA_INTERNO && position<DESTRA_CENTRO_ESTERNO)
  { 
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_ALTA);
  }
  else if(position>=DESTRA_ESTERNO && position<=DESTRA_INTERNO)
  { 
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MASSIMA);
  }
  else if(position<DESTRA_ESTERNO)
  { 
    muoviMotore(MOTORE_DESTRO, MOTORE_INDIETRO, POTENZA_ALTA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }
}
