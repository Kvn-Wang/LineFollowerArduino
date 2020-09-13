//libreria per i sensori infrarossi
#include <QTRSensors.h>

#define MOTORE_DESTRO       0
#define MOTORE_SINISTRO     1

#define MOTORE_AVANTI       0
#define MOTORE_FERMO        1
#define MOTORE_INDIETRO     2

//velocita dei motori
#define POTENZA_ALTA        100
#define POTENZA_MEDIA       80
#define POTENZA_BASSA       70

//valori dei diversi range
#define POS_TANTO_DESTRA    750
#define POS_DESTRA          2000
#define POS_SINISTRA        3000
#define POS_TANTO_SINISTRA  4250

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
  if (direzione <= 1)
  {
    digitalWrite(InPinA[motore], LOW);
  }
  else
  {
    digitalWrite(InPinA[motore], HIGH);
  }
  if (direzione >= 1 && direzione <= 2)
  {
    digitalWrite(InPinB[motore], LOW);
  }
  else
  {
    digitalWrite(InPinB[motore], HIGH);
  }
  analogWrite(InMotorePwm[motore], ((255 / 100)*pwm));
}

void loop()
{
  //variabile che indica la posizione della linea nera, sapendo i valori letti dai 6 sensori
  uint16_t position = qtr.readLineBlack(ValoriSensori);
  
  //se la linea nera è a sinitra
  if (position > POS_TANTO_SINISTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_BASSA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_INDIETRO, POTENZA_ALTA);
  }
  else if (position > POS_SINISTRA && position <= POS_TANTO_SINISTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_BASSA);
  }

  //se linea nera è al centro
  else if (position >= POS_DESTRA && position <= POS_SINISTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }

  //se la linea nera è a destra
  else if (position >= POS_TANTO_DESTRA && position <= POS_DESTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_BASSA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);
  }
  else if (position < POS_TANTO_DESTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_INDIETRO, POTENZA_ALTA);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_BASSA);
  }

}
