//libreria per i sensori infrarossi
#include <QTRSensors.h>


#define MOTORE_DESTRO       0
#define MOTORE_SINISTRO     1

#define MOTORE_AVANTI       0
#define MOTORE_FERMO        1
#define MOTORE_INDIETRO     2

//potenza motori
#define POTENZA_ALTA        120
#define POTENZA_MEDIA       75
#define POTENZA_BASSA       60

//potenza motori a velocità massima (una volta stabilizzato)
#define POTENZA_NORMALE     125

//valori dei diversi range
#define POS_TANTO_DESTRA    1000
#define POS_DESTRA          2300
#define POS_SINISTRA        2700
#define POS_TANTO_SINISTRA  4000

//numero di letture che l'Arduino deve fare in seguito all'uscita del range al centro
//serve per la velocità modulare
#define NUM_LETTURE 15

//variabili direzionali dei motori
uint8_t InPinA[] = {8, 7};
uint8_t InPinB[] = {4, 2};

//variabile per regolare la potenza dei motori
uint8_t InMotorePwm[] = {9, 3};

QTRSensors qtr;

//variabili che indica numero di sensori e il loro valore
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

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
  qtr.setSensorPins((const uint8_t[]) {
    A0, A1, A2, A3, A4, A5
  }, SensorCount);

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

//funzione per stabilizzarsi sulla linea nera (deve essere al centro) + velocità modulare
void stabilizzazione()
{
  //variabile che indica il numero di letture necessarie che l'Arduino deve ancora fare prima di ritornare a velocità "normale"
  int letture_necessarie = NUM_LETTURE;

  //quando si uscirà da questo ciclo di istruzioni, vorrà dire che la macchina si è stabilizzata sulla linea nera 
  while (letture_necessarie > 0)
  {
    //variabile che indica la posizione della linea nera, sapendo i valori letti dai 6 sensori
    uint16_t position = qtr.readLineBlack(sensorValues);

    //se la linea nera è a sinitra
    if (position > POS_TANTO_SINISTRA)
    {
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_INDIETRO, POTENZA_ALTA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }
    else if (position > POS_SINISTRA && position <= POS_TANTO_SINISTRA)
    {
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_BASSA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }

    //se la linea nera è a destra
    else if (position >= POS_TANTO_DESTRA && position <= POS_DESTRA)
    {
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_BASSA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }
    else if (position < POS_TANTO_DESTRA)
    {
      muoviMotore(MOTORE_DESTRO, MOTORE_INDIETRO, POTENZA_ALTA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }

    //se linea nera è al centro
    if (position >= POS_DESTRA && position <= POS_SINISTRA)
    {
      //aumenta la potenza fino a che l'Arduino è sulla linea nera (al centro), 
      //e diminuisce il numero di letture che deve fare, quest'ultima è in funzione alla velocità dei 2 motori
      //più passa tempo, più letture farà l'Arduino, più veloce sarà
      //se esce dal centro il programma resetterà il numero di letture necessarie in modo che ad ogni uscita dal centro
      //l'Arduino abbia una velocità modulare
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, (POTENZA_NORMALE - letture_necessarie * 2));
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, (POTENZA_NORMALE - letture_necessarie * 2));
      letture_necessarie--;
    }
  }
}

void loop()
{
  //variabile che indica la posizione della linea nera, sapendo i valori letti dai 6 sensori
  uint16_t position = qtr.readLineBlack(sensorValues);

  //se linea nera è al centro
  if (position >= POS_DESTRA && position <= POS_SINISTRA)
  {
    muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_NORMALE);
    muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_NORMALE);
  }
  else
  {
    stabilizzazione();
  }
}
