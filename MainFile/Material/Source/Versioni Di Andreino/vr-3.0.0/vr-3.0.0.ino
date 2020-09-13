//libreria per i sensori infrarossi
#include <QTRSensors.h>

//distanza massima/minima del sensore a ultrasuoni
#define DISTANZA_MASSIMA 15
#define DISTANZA_MINIMA  3

//per ogni letture del sensore ultrasuoni, fai N. letture del sensore infrarossi
#define NUM_LETTURE_SENSORE_INFRAROSSI_ULTRASUONI 75

#define MOTORE_DESTRO       0
#define MOTORE_SINISTRO     1

#define MOTORE_AVANTI       0
#define MOTORE_FERMO        1
#define MOTORE_INDIETRO     2

//potenza motori
#define POTENZA_ALTA        120
#define POTENZA_MEDIA       80
#define POTENZA_BASSA       70

//potenza motori a velocità massima (una volta stabilizzato)
#define POTENZA_NORMALE     125

//valori dei diversi range
#define POS_TANTO_DESTRA    500
#define POS_DESTRA          2000
#define POS_SINISTRA        3000
#define POS_TANTO_SINISTRA  4500

//numero di letture che l'Arduino deve fare in seguito all'uscita del range al centro
//serve per la velocità modulare
#define NUM_LETTURE 15

//variabile di lettura ultrasuoni-infrarossi
int letture_prima_lettura_ultrasuono = NUM_LETTURE_SENSORE_INFRAROSSI_ULTRASUONI;
uint16_t position;

//variabili direzionali dei motori
uint8_t InPinA[] = {8, 7};
uint8_t InPinB[] = {4, 2};

//variabile per regolare la potenza dei motori
uint8_t InMotorePwm[] = {9, 3};

//variabili per Sensore Infrarossi
QTRSensors qtr;
const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];

//Pin sensori ultrasuoni
const int NumeroSensoriUltrasuoni = 2;
const int TRIG_PIN[NumeroSensoriUltrasuoni] = {6, 11};
const int ECHO_PIN[NumeroSensoriUltrasuoni] = {5, 10};

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
  pinMode(TRIG_PIN[0], OUTPUT);
  pinMode(ECHO_PIN[0], INPUT);
  pinMode(TRIG_PIN[1], OUTPUT);
  pinMode(ECHO_PIN[1], INPUT);

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

//funzione per lettura sensore ultrasuoni e infrarossi
void lettura()
{
  position = qtr.readLineBlack(sensorValues);
  letture_prima_lettura_ultrasuono--;

  if (letture_prima_lettura_ultrasuono == 0)
  {
    //resetta il numero di letture
    letture_prima_lettura_ultrasuono = NUM_LETTURE_SENSORE_INFRAROSSI_ULTRASUONI;
    
    bool fermo = false;
    for (int i; i < NumeroSensoriUltrasuoni; i++)
    {
      long durata;
      // Dare un corto segnale basso per poi dare un segnale alto puro:
      digitalWrite(TRIG_PIN[i], LOW);
      delayMicroseconds(2);
      digitalWrite(TRIG_PIN[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN[i], LOW);
      durata = pulseIn(ECHO_PIN[i], HIGH);
      // Converti il tempo in distanza:
      int distanza = durata / 29.1 / 2 ;
      if (distanza < DISTANZA_MASSIMA && distanza>DISTANZA_MINIMA)
      {
        fermo = true;
      }
    }
    //decellera
    if (fermo == true) 
    {
      muoviMotore(MOTORE_DESTRO, MOTORE_INDIETRO, POTENZA_BASSA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_INDIETRO, POTENZA_BASSA);
      delay(200);
      muoviMotore(MOTORE_DESTRO, MOTORE_FERMO, 0);
      muoviMotore(MOTORE_SINISTRO, MOTORE_FERMO, 0);
      delay(2000);
      fermo = false;
    }
  }
}

//funzione per stabilizzarsi sulla linea nera (deve essere al centro) + velocità modulare
void stabilizzazione()
{
  //variabile che indica il numero di letture necessarie che l'Arduino deve ancora fare prima di ritornare a velocità "normale"
  int letture_necessarie = NUM_LETTURE;

  //quando si uscirà da questo ciclo di istruzioni, vorrà dire che la macchina si è stabilizzata sulla linea nera
  while (letture_necessarie > 0)
  {
    lettura();

    //se la linea nera è a sinitra
    if (position > POS_TANTO_SINISTRA)
    {
      //gira tanto a destra
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_INDIETRO, POTENZA_ALTA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }
    else if (position > POS_SINISTRA && position <= POS_TANTO_SINISTRA)
    {
      //gira a sinistra
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_MEDIA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_BASSA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }

    //se la linea nera è a destra
    else if (position >= POS_TANTO_DESTRA && position <= POS_DESTRA)
    {
      //gira a sinistra
      muoviMotore(MOTORE_DESTRO, MOTORE_AVANTI, POTENZA_BASSA);
      muoviMotore(MOTORE_SINISTRO, MOTORE_AVANTI, POTENZA_MEDIA);

      //resetta il numero di letture necessarie
      letture_necessarie = NUM_LETTURE;
    }
    else if (position < POS_TANTO_DESTRA)
    {
      //gira tanto a sinistra
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
  lettura();

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
