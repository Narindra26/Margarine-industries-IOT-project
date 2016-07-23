/*
   Title: BOILER CONTROL MARGARINE INDUSTRIES
   Description: Control du boiler par internet
   Autor:Narindra RATSIMBA
   Creation:30 March 2016
   Update: 8 April 2016 (Comment and boiler control)

*/

#include <SoftwareSerial.h>
#include <OneWire.h>

//#define DEBUG 1
//Declaration du capteur de temperature DHT11
//#include "DHT.h"
//#define DHTPIN 2 //
//#define DHTTYPE DHT22
//DHT dht(DHTPIN, DHTTYPE);

//Declaration du capteur de temperature DS18B20
OneWire  ds(A3);

//Variables pour les commandes
String message;
String cmd;
String getStr;
int commandeOutput;

//Etat initiale du boiler
int initState;

//--------------Wifi_Configuration-----------------------------
String NomduReseauWifi = "HotRoom"; //
String MotDePasse      = "milmonitoring"; //
//--------------------------------------------------------------

//---------------SWITCH DEBOUNCING------------------------------
long debouncing_time = 1000; //Debouncing en Milliseconds
volatile unsigned long last_micros;
//--------------------------------------------------------------



//---------------TIMERS------------------------------------------
//Variables pour le temps
unsigned long tempsActuel = 0;
unsigned long tempsPrecedent1 = 0;// Temps entre les envois des donnees du capteur
unsigned long tempsPrecedent2 = 0;//Temps entre chaque lecture de commande
unsigned long tempsPrecedent3 = 0;
unsigned long tempsPrecedent4 = 0;
unsigned long interval1 = 5;
unsigned long interval2 = 10;
unsigned long interval3 = 15;
unsigned long interval4 = 30;
//----------------------------------------------------------------

// Variables pour les GPIO
//----------------------------------------------------------------------
int RL1_Speed1 = 12;
int RL2_Speed2 = 9;

int RL3_temoinSpeed1 = 8;
int RL4_temoinSpeed2 = 7;

int RL5_manAuto1 = 5;
int RL6_manAuto2 = 6;

int RL7_Reset = 4;

int switchBtn1 = A2;
int switchBtn2 = A1;

//----------------------------------------------------------------------

//Etat du boiler
boolean boiler1State;
boolean boiler2State;

//Variables pour le capteur de temperature
float coldroom1 = 0;
float coldroom2 = 0;
float chillroom1 = 0;
float chillroom2 = 0;
boolean boiler1 = 0;
boolean boiler2 = 0;
boolean hotroomcontrol = 0;
float hotroom = 0;

//Variables pour ds18b20
float celsius, fahrenheit;


//API key de l ecriture des donnees de temperature chez thingspeak.com
String apiKey = "ZKXIN41FPCFTMM9Z";
String apiCommandKey = "RLZQRXSSJ3QXQI5K";
String apiInitKey = "CFZOIRI78646BYCE";


// Creation de l objet software serial pour la liaison serie virtuelle
SoftwareSerial ser(11, 10); // RX, TX


void setup() {

  // Configuration des GPIO.
  pinMode(RL1_Speed1, OUTPUT);
  pinMode(RL2_Speed2, OUTPUT);
  pinMode(RL7_Reset, OUTPUT);
  pinMode(RL3_temoinSpeed1, OUTPUT);
  pinMode(RL4_temoinSpeed2, OUTPUT);

  delay(500);

  digitalWrite(RL1_Speed1, HIGH);
  digitalWrite(RL2_Speed2, HIGH);
  digitalWrite(RL3_temoinSpeed1, HIGH);
  digitalWrite(RL4_temoinSpeed2, HIGH);
  digitalWrite(RL7_Reset, HIGH);

  pinMode(switchBtn1, INPUT_PULLUP);
  pinMode(switchBtn2, INPUT_PULLUP);

  delay(1000);

  pinMode(RL5_manAuto1, OUTPUT);
  pinMode(RL6_manAuto2, OUTPUT);

  delay(500);

  digitalWrite(RL5_manAuto1, LOW);
  digitalWrite(RL6_manAuto2, LOW);

  delay(500);

  // Debut de la liaison serie
  Serial.begin(9600);
  // Debut de la liaison serie virtuel
  ser.begin(9600);
  delay(500);

  Serial.println("...By Narindra Ratsimba");
  //-------------------INITIALISTATION WIFI----------------------------
  //initser();
  //-------------------********************----------------------------
  // Debut du capteur de temperature
  //dht.begin();

  // reset du module wifi ESP8266
  ser.println("AT+RST");
  connection();
  initialisation();
}



void loop()
{
  temperature(); //Lecture des donnees des capteurs
  //debug();

  //Temps depuis l allumage du systeme
  tempsActuel = millis();

  // Si le temps d interval entre chaque envoi de donnee est atteint ou depasse alors on envoie les donnees
  if (tempsActuel - tempsPrecedent1 >= interval1)
  {
#ifdef DEBUG
    Serial.print("temps 1: ");
    Serial.println(tempsActuel - tempsPrecedent1 * 100);
#endif
    tempsPrecedent1 = tempsActuel;
    manuel();
  }

  // Si le temps d interval entre chaque lecture de la valeur de l etat des GPIO est atteint ou depasse alors on lit les etats des GPIO
  if (tempsActuel - tempsPrecedent2 >= interval2 * 1000)
  {
#ifdef DEBUG
    Serial.print("temps 2: ");
    Serial.println(tempsActuel - tempsPrecedent2);
#endif

    tempsPrecedent2 = tempsActuel;
    commande();
    manuel();
  }

  // Si le temps d interval entre chaque lecture de la valeur de l etat des GPIO est atteint ou depasse alors on lit les etats des GPIO
  if (tempsActuel - tempsPrecedent3 >= interval3 * 1000)
  {
#ifdef DEBUG
    Serial.print("temps 3: ");
    Serial.println(tempsActuel - tempsPrecedent3);
#endif

    tempsPrecedent3 = tempsActuel;
    thingspeak();
  }

  /*
    if (tempsActuel - tempsPrecedent4 >= interval4)
    {
    #ifdef DEBUG
    Serial.print("temps 4: ");
    Serial.println(tempsActuel - tempsPrecedent4);
    #endif

    tempsPrecedent4 = tempsActuel;
    //ds18b20();; //
    }
  */

}

/****************************************************************/
/*                Fonction qui initialise l'ESP8266             */
/****************************************************************/
void initser()
{
#ifdef DEBUG
  Serial.println("**********************************************************");
  Serial.println("**************** DEBUT DE L'INITIALISATION ***************");
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+RST");
  recoitDuser(2000);
#ifdef DEBUG
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+CWMODE=3");
  recoitDuser(5000);
#ifdef DEBUG
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+CWJAP=\"" + NomduReseauWifi + "\",\"" + MotDePasse + "\"");
  recoitDuser(10000);
#ifdef DEBUG
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+CIFSR");
  recoitDuser(1000);
#ifdef DEBUG
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+CIPMUX=1");
  recoitDuser(1000);
#ifdef DEBUG
  Serial.println("**********************************************************");
#endif
  envoieAuser("AT+CIPSERVER=1,80");
  recoitDuser(1000);
#ifdef DEBUG
  Serial.println("**********************************************************");
  Serial.println("***************** INITIALISATION TERMINEE ****************");
  Serial.println("**********************************************************");
  Serial.println("");
#endif
}

/****************************************************************/
/*        Fonction qui envoie une commande à l'ser          */
/****************************************************************/
void envoieAuser(String commande)
{
  ser.println(commande);
}
/****************************************************************/
/*Fonction qui lit et affiche les messages envoyés par l'ser*/
/****************************************************************/
void recoitDuser(const int timeout)
{
  String reponse = "";
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (ser.available())
    {
      char c = ser.read();
      reponse += c;
    }
  }

#ifdef DEBUG
  Serial.print(reponse);
#endif

}
//---------------------------------------------------------------

//Fonction de lecture des valeurs du capteur
void temperature()
{

  boiler1State = digitalRead(RL1_Speed1);
  boiler2State = digitalRead(RL2_Speed2);
  int boilerVitesse;
  if (boiler1State == HIGH && boiler2State == HIGH)
  {
    boilerVitesse = 0;
  }
  else if (boiler1State == LOW && boiler2State == HIGH)
  {
    boilerVitesse = 1;
  }
  else if (boiler1State == LOW && boiler2State == LOW)
  {
    boilerVitesse = 2;
  }

  coldroom1 = boilerVitesse;

  coldroom2 = boilerVitesse;

  //chillroom1 = celsius;

  //chillroom2 = celsius;

  //boiler1 = digitalRead(RL1_Speed1);

  // boiler2 = digitalRead(RL2_Speed2);

  //hotroomcontrol = 0;

  //hotroom = celsius;
}

void debug()
{
  Serial.print("Coldroom1: ");
  Serial.println(coldroom1);
  Serial.print("Coldroom2: ");
  Serial.println(coldroom2);
  Serial.print("Chillroom1: ");
  Serial.println(chillroom1);
  Serial.print("Chillroom2: ");
  Serial.println(chillroom2);
  Serial.print("Boiler1: ");
  Serial.println(boiler1);
  Serial.print("Boiler2: ");
  Serial.println(boiler2);
  Serial.print("Hot room state: ");
  Serial.println(hotroomcontrol);
  Serial.print("Hot room temperature: ");
  Serial.println(hotroom);

}

//----------------------------COMMANDE LOCALE-----------------------------------------
//Commande locale
void manuel()
{
  int switchBtn1State = digitalRead(switchBtn1);
  int switchBtn2State = digitalRead(switchBtn2);
  boiler1State = digitalRead(RL1_Speed1);
  boiler2State = digitalRead(RL2_Speed2);


#ifdef DEBUG
  Serial.print("SWITCH1: ");
  Serial.println(switchBtn1State);
  Serial.print("SWITCH2: ");
  Serial.println(switchBtn2State);
#endif

  if ((long)(micros() - last_micros) >= debouncing_time * 1000)
  {
    if (switchBtn1State == 0 )
    {
      if (boiler1State == 1 && boiler2State == 1)
      {
        digitalWrite(RL2_Speed2, HIGH);//Eteindre RL2
        delay(1000);
        digitalWrite(RL1_Speed1, LOW);//Vitesse 1
        delay(1500);
        digitalWrite(RL7_Reset, LOW); //reset on
        delay(1000);
        digitalWrite(RL7_Reset, HIGH); //reset off
        delay(1000);
        digitalWrite(RL3_temoinSpeed1, LOW);


#ifdef DEBUG
        Serial.println("BOILER SPEED 1");
#endif
      }

      else if (boiler1State == 0)
      {
        if (boiler2State == 1)
        {
          digitalWrite(RL2_Speed2, HIGH); //Eteindre RL2
          digitalWrite(RL4_temoinSpeed2, HIGH);

          delay(1000);
          digitalWrite(RL1_Speed1, HIGH); //Eteindre RL1
          digitalWrite(RL3_temoinSpeed1, HIGH);
#ifdef DEBUG
          Serial.println("BOILER OFF");
#endif
        }
        else if (boiler2State == 0)
        {
          digitalWrite(RL2_Speed2, HIGH); //Eteindre RL2
          digitalWrite(RL4_temoinSpeed2, HIGH);
          delay(1000);
          digitalWrite(RL1_Speed1, LOW); ////Vitesse 1
          digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
          Serial.println("BOILER SPEED 2 OFF");
          Serial.println("BOILER SPEED 1 ON");
#endif
        }

      }

    }


    if (switchBtn2State == 0)
    {

      if (boiler2State == 1 && boiler1State == 1)
      {
        digitalWrite(RL1_Speed1, LOW); //Vitesse 1
        delay(1000);
        digitalWrite(RL7_Reset, LOW); //reset on
        delay(1000);
        digitalWrite(RL7_Reset, HIGH); //reset off
#ifdef DEBUG
        Serial.println("BOILER SPEED 1 ON... Waiting for speed 2");
#endif
        digitalWrite(RL3_temoinSpeed1, LOW);
        delay(1000);
        digitalWrite(RL2_Speed2, LOW); // Vitesse 2
#ifdef DEBUG
        Serial.println("BOILER SPEED 2 ON");
#endif
        digitalWrite(RL4_temoinSpeed2, LOW);
      }

      else if (boiler2State == 1 && boiler1State == 0)
      {
        digitalWrite(RL1_Speed1, LOW); //Vitesse 1
        digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
        Serial.println("BOILER SPEED 1 ON... Waiting for speed 2");
#endif
        delay(1000);
        digitalWrite(RL2_Speed2, LOW); //Vitesse 2
#ifdef DEBUG
        Serial.println("BOILER SPEED 2 ON");
#endif
        digitalWrite(RL4_temoinSpeed2, LOW);
      }

      else if (boiler2State == 0)
      {
        digitalWrite(RL2_Speed2, HIGH); // Eteindre vitesse 2
        digitalWrite(RL4_temoinSpeed2, HIGH);
#ifdef DEBUG
        Serial.println("BOILER SPEED 2 OFF... Waiting for speed 1");
#endif
        delay(1000);
        digitalWrite(RL1_Speed1, HIGH); // Eteindre vitesse 1
        digitalWrite(RL3_temoinSpeed1, HIGH);
#ifdef DEBUG
        Serial.println("BOILER SPEED 1 OFF");
#endif
      }
    }

    last_micros = micros();
  }
}
//----------------------------FIN DE LA COMMANDE LOCALE-----------------------------------------


//--------------------------------COMMANDE VIA INTERNET----------------------------------------
void commande()
{
  connection();

  // Preparation de l envoi de la requete GET
  getStr = "GET /talkbacks/8543/commands/execute?api_key=";
  getStr += apiCommandKey;
  getStr += "\r\n";

  // Envoi de la longueur des donnees
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

  if (ser.find(">"))
  {
    ser.print(getStr);
    if (ser.find("+"))
    {
#ifdef DEBUG
      Serial.println("COMMAND FOUND");
#endif

      String line = ser.readStringUntil('\r');
      message = (String)line;

      if (message.substring(6, 13) == "13HIGH*")
      {
#ifdef DEBUG
        Serial.println("1st speed on");
#endif
        commandeOutput = 1;
      }

      else if (message.substring(6, 12) == "13LOW*")
      {
#ifdef DEBUG
        Serial.println("1st speed off");
#endif
        commandeOutput = 2;
      }

      else if (message.substring(6, 13) == "12HIGH*")
      {
#ifdef DEBUG
        Serial.println("2nd speed on");
#endif
        commandeOutput = 3;
      }

      else if (message.substring(6, 12) == "12LOW*")
      {
#ifdef DEBUG
        Serial.println("2nd speed off");
#endif
        commandeOutput = 4;
      }

      boiler1State = digitalRead(RL1_Speed1);
      boiler2State = digitalRead(RL2_Speed2);

      switch (commandeOutput)
      {
        case 1:
          if (boiler1State == 1 && boiler2State == 1)
          {
            digitalWrite(RL2_Speed2, HIGH);//Eteindre RL2
            delay(1000);
            digitalWrite(RL1_Speed1, LOW);//Vitesse 1
            delay(1500);
            digitalWrite(RL7_Reset, LOW); //reset on
            delay(1000);
            digitalWrite(RL7_Reset, HIGH); //reset off
            delay(1000);
            digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
            Serial.println("BOILER SPEED 1");
#endif
          }

          else if (boiler2State == 0 && boiler1State == 0)
          {
            digitalWrite(RL2_Speed2, HIGH); //Eteindre RL2
            digitalWrite(RL4_temoinSpeed2, HIGH);
            delay(1000);
            digitalWrite(RL1_Speed1, LOW); ////Vitesse 1
            digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
            Serial.println("BOILER SPEED 2 OFF");
            Serial.println("BOILER SPEED 1 ON");
#endif
          }
          break;

        case 2:
          digitalWrite(RL2_Speed2, HIGH); // Eteindre vitesse 2
          digitalWrite(RL4_temoinSpeed2, HIGH);
#ifdef DEBUG
          Serial.println("BOILER SPEED 2 OFF... Waiting for speed 1");
#endif
          delay(1000);
          digitalWrite(RL1_Speed1, HIGH); // Eteindre vitesse 1
          digitalWrite(RL3_temoinSpeed1, HIGH);
#ifdef DEBUG
          Serial.println("BOILER SPEED 1 OFF");
#endif
          break;

        case 3:
          if (boiler1State == 1 && boiler2State == 1)
          {
            digitalWrite(RL1_Speed1, LOW); //Vitesse 1
            delay(1000);
            digitalWrite(RL7_Reset, LOW); //reset on
            delay(1000);
            digitalWrite(RL7_Reset, HIGH); //reset off
#ifdef DEBUG
            Serial.println("BOILER SPEED 1 ON... Waiting for speed 2");
#endif
            digitalWrite(RL3_temoinSpeed1, LOW);
            delay(1000);
            digitalWrite(RL2_Speed2, LOW); // Vitesse 2
#ifdef DEBUG
            Serial.println("BOILER SPEED 2 ON");
#endif
            digitalWrite(RL4_temoinSpeed2, LOW);
          }
          else if (boiler1State == 0 && boiler2State == 1 )
          {
            digitalWrite(RL1_Speed1, LOW); //Vitesse 1
            digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
            Serial.println("BOILER SPEED 1 ON... Waiting for speed 2");
#endif
            delay(1000);
            digitalWrite(RL2_Speed2, LOW); //Vitesse 2
#ifdef DEBUG
            Serial.println("BOILER SPEED 2 ON");
#endif
            digitalWrite(RL4_temoinSpeed2, LOW);
          }
          break;

        case 4:

          digitalWrite(RL2_Speed2, HIGH); // Eteindre vitesse 2
          digitalWrite(RL4_temoinSpeed2, HIGH);
#ifdef DEBUG
          Serial.println("BOILER SPEED 2 OFF... Waiting for speed 1");
#endif
          delay(1000);
          digitalWrite(RL1_Speed1, HIGH); // Eteindre vitesse 1
          digitalWrite(RL3_temoinSpeed1, HIGH);
#ifdef DEBUG
          Serial.println("BOILER SPEED 1 OFF");
#endif

          break;
        default:
#ifdef DEBUG
          Serial.println("NO OUTPUT COMMAND FOUND");
#endif
          break;
      }

    }
    else {
      ser.println("AT+CIPCLOSE");
#ifdef DEBUG
      Serial.println("FERMETURE DE LA CONNECTION AVEC TALKBACK");
      Serial.println("...RETENTATIVE DE CONNECTION AVEC TALKBACK");
#endif
    }


  }
}
//-------------------------FIN DE LA COMMANDE VIA INTERNET----------------------------------------


//-------------------------ENVOI DONNEES VERS INTERNET----------------------------------------
//Fonction pour l'envoi des donnees des capteurs
void thingspeak()
{
  // Debut de la connection TCP
  cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);

  if (ser.find("Error")) {
#ifdef DEBUG
    Serial.println("ERREUR DE CONNECTION AVEC THINGSPEAK.COM");
#endif
    return;
  }

  // Preparation de l envoi de la requete GET
  getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr += "&field1=";
  getStr += String(coldroom1);
  getStr += "&field2=";
  getStr += String(coldroom2);
  //getStr += "&field3=";
  //getStr += String(chillroom1);
  //getStr += "&field4=";
  //getStr += String(chillroom2);
  //getStr += "&field5=";
  //getStr += String(boiler1);
  //getStr += "&field6=";
  //getStr += String(boiler2);
  //getStr += "&field7=";
  //getStr += String(hotroomcontrol);
  //getStr += "&field8=";
  //getStr += String(hotroom);
  getStr += "\r\n\r\n";


  // Envoi de la longueur des donnees
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);
  if (ser.find(">")) {
    ser.print(getStr);
    //ser.println("AT+CIPCLOSE");//Fermeture de la connection
#ifdef DEBUG
    Serial.println("FIN DE L ENVOI DE DONNEES THINGSPEAK");
#endif
  }

  else {
    ser.println("AT+CIPCLOSE");
#ifdef DEBUG
    Serial.println("FERMETURE DE LA CONNECTION AVEC THINGSPEAK.COM");
    Serial.println("RETENTATIVE DE CONNECTION AVEC THINGSPEAK.COM");
#endif
    connection();
    thingspeak();
  }
}
//--------------------FIN DE L ENVOI DONNEES VERS INTERNET----------------------------------------

//----------------------TEMPERATURE---------------------------------------------------------------
void ds18b20() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];


  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }

  for ( i = 0; i < 8; i++) {

  }

  if (OneWire::crc8(addr, 7) != addr[7]) {

    return;
  }


  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:

      type_s = 0;
      break;
    default:

      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad


  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();

  }


  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

#ifdef DEBUG
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
#endif
}

//--------------FIN----TEMPERATURE---------------------------------------------------------------



//--------------------------------Connection initiale---------------------------------
void connection()
{
  // Debut de la connection TCP pour l ecoute des commandes
  cmd  = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  if (ser.find("Error"))
  {
#ifdef DEBUG
    Serial.println("ERREUR DE CONNECTION AVEC THINGSPEAK");
#endif
    connection();
  }
  else
  {
#ifdef DEBUG
    Serial.println("CONNECTION AVEC THINGSPEAK REUSSIE");
#endif
  }

}
//-----------------------------Fin de la Connection initiale---------------------------------



//-------------------------DERNIER ETAT DU BOILER----------------------------------------
void initialisation()
{
  // Preparation de l envoi de la requete GET
  getStr = "GET /apps/thinghttp/send_request?api_key=";
  getStr += apiInitKey;
  getStr += "\r\n";

  // Envoi de la longueur des donnees
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);
  if (ser.find(">"))
  {
    ser.print(getStr);
    if (ser.find(":"))
    {
      String line = ser.readStringUntil('\n');
      message = line;
      initState = message.toInt();
#ifdef DEBUG
      Serial.print("LAST STATE: ");
      Serial.println(initState);
#endif
    }



#ifdef DEBUG
    Serial.println("INITIALISATION TERMINEE");
#endif
    initialState();
  }
  else
  {
    ser.println("AT+CIPCLOSE");
#ifdef DEBUG
    Serial.println("TENTATIVE D'INITIALISTATION");
    delay(1500);
    connection();
    initialisation();
#endif
  }
}
//--------------------FIN DE L INITIALISTAION.----------------------------------------


//---------------------------------Etat initiale---------------------------------------
void initialState()
{
  switch (initState)
  {
    case 0:
      digitalWrite(RL2_Speed2, HIGH); //Eteindre RL2
      digitalWrite(RL4_temoinSpeed2, HIGH);
      delay(1000);
      digitalWrite(RL1_Speed1, HIGH); ////Eteindre RL 1
      digitalWrite(RL3_temoinSpeed1, HIGH);
#ifdef DEBUG
      Serial.println("BOILER SPEED 2 OFF");
      Serial.println("BOILER SPEED 1 OFF");
#endif
      break;

    case 1:
      digitalWrite(RL2_Speed2, HIGH);//Eteindre RL2
      delay(1000);
      digitalWrite(RL1_Speed1, LOW);//Vitesse 1
      delay(1500);
      digitalWrite(RL7_Reset, LOW); //reset on
      delay(1000);
      digitalWrite(RL7_Reset, HIGH); //reset off
      delay(1000);
      digitalWrite(RL3_temoinSpeed1, LOW);
#ifdef DEBUG
      Serial.println("BOILER SPEED 1");
#endif
      break;

    case 2:
      digitalWrite(RL1_Speed1, LOW); //Vitesse 1
      digitalWrite(RL3_temoinSpeed1, LOW);
      delay(1000);
      digitalWrite(RL7_Reset, LOW); //reset on
      delay(1000);
      digitalWrite(RL7_Reset, HIGH); //reset off
#ifdef DEBUG
      Serial.println("BOILER SPEED 1 ON... Waiting for speed 2");
#endif
      delay(1000);
      digitalWrite(RL2_Speed2, LOW); // Vitesse 2
#ifdef DEBUG
      Serial.println("BOILER SPEED 2 ON");
#endif
      digitalWrite(RL4_temoinSpeed2, LOW);
      break;

    default:
#ifdef DEBUG
      Serial.println("NO OUTPUT COMMAND FOUND");
#endif
      break;
  }
}


//---------------------------Fin de l initialisation des commandes-------------------------------
