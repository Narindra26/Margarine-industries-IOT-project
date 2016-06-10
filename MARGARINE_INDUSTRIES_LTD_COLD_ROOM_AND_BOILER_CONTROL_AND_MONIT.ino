/*
   Title: MARGARINE INDUSTRIES LTD BOILER CONTROL AND TEMPERATURE MONITORING OF COLD ROOM
   Description: THIS PROGRAM MONITOR THE TEMPERATURE OF COLD ROOM AND CONTROL AN INDUSTRIAL BOILER
   Autor:Narindra RATSIMBA
   Creation:30 March 2016
   Update: 8 April 2016 (Comment and boiler control)
*/

#include <SoftwareSerial.h>

//Declaration du capteur de temperature
#include "DHT.h"
#define DHTPIN 2 //
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Variables pour les commandes
String message;
String cmd;
String getStr;
int commandeOutput;

//Variables pour le temps
long tempsActuel = 0;
long tempsPrecedent1 = 0;// Temps entre les envois des donnees du capteur
long tempsPrecedent2 = 0;//Temps entre chaque lecture de commande
int interval1 = 16000;
int interval2 = 1000;


// Variables pour les GPIO
int pin13 = 3;
int pin12 = 5;

//Variables pour le capteur de temperature
float h;
float t;
float f;
boolean pinStatus13;
boolean pinStatus12;


//API key de l ecriture des donnees de temperature chez thingspeak.com
String apiKey = "F99TC24D7JYQA1LL";


// Creation de l objet software serial pour la liaison serie virtuelle
SoftwareSerial ser(11, 10); // RX, TX


void setup() {
  // Configuration des GPIO.
  pinMode(pin13, OUTPUT);
  // Debut de la liaison serie
  Serial.begin(9600);
  // Debut de la liaison serie virtuel
  ser.begin(9600);
  // Debut du capteur de temperature
  dht.begin();
  // reset du module wifi ESP8266
  ser.println("AT+RST");
}



void loop()
{

  temperature(); //Lecture des donnees des capteurs
  //Temps depuis l allumage du systeme
  tempsActuel = millis();

  // Si le temps d interval entre chaque envoi de donnee est atteint ou depasse alors on envoie les donnees
  if (tempsActuel - tempsPrecedent1 >= interval1)
  {
    tempsPrecedent1 = tempsActuel;
    esp_8266(); //Envoi des donnees des capteurs
  }

  // Si le temps d interval entre chaque lecture de la valeur de l etat des GPIO est atteint ou depasse alors on lit les etats des GPIO
  if (tempsActuel - tempsPrecedent2 >= interval2)
  {
    tempsPrecedent2 = tempsActuel;
    commande(); //
  }
}


//Fonction de lecture des valeurs du capteur
void temperature()
{
  h = dht.readHumidity();

  t = dht.readTemperature();

  f = dht.readTemperature(true);

  pinStatus13 = digitalRead(pin13);
}


//Fonction pour l'envoi des donnees des capteurs
void esp_8266()
{
  Serial.print("Humidity %: ");
  Serial.println(h);
  Serial.print("Temperature C: ");
  Serial.println(t);
  Serial.print("Temperature F: ");
  Serial.println(f);
  Serial.print("BOILER: ");
  Serial.println(pinStatus13);

  // Debut de la connection TCP
  cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);

  if (ser.find("Error")) {
    Serial.println("ERREUR DE CONNECTION AVEC THINGSPEAK.COM");
    return;
  }

  // Preparation de l envoi de la requete GET
  getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr += "&field1=";
  getStr += String(h);
  getStr += "&field2=";
  getStr += String(t);
  getStr += "&field3=";
  getStr += String(f);
  getStr += "&field4=";
  getStr += String(pinStatus13);
  getStr += "\r\n\r\n";


  // Envoi de la longueur des donnees
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);
  if (ser.find(">")) {
    ser.print(getStr);
    ser.println("AT+CIPCLOSE");//Fermeture de la connection
    Serial.println("FIN DE L ENVOI DE DONNEES");
  }

  else {
    ser.println("AT+CIPCLOSE");
    Serial.println("FERMETURE DE LA CONNECTION AVEC THINGSPEAK.COM");
  }
}

void commande()
{
  // Debut de la connection TCP pour l ecoute des commandes
  cmd  = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);

  if (ser.find("Error"))
  {
    Serial.println("ERREUR DE CONNECTION AVEC APP");
    return;
  }
  // Preparation de l envoi de la requete GET
  getStr = "GET /talkbacks/7787/commands/execute?api_key=T7K74EZWOWNQ81GX";
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
      Serial.println("COMMAND FOUND");

      String line = ser.readStringUntil('\r');
      message = (String)line;

      if (message.substring(6, 13) == "13HIGH*")
      {
        Serial.println("Pin 13 HIGH");
        commandeOutput = 1;
      }

      else if (message.substring(6, 12) == "13LOW*")
      {
        Serial.println("Pin 13 LOW");
        commandeOutput = 2;
      }

      else if (message.substring(6, 13) == "12HIGH*")
      {
        Serial.println("Pin 12 HIGH");
        commandeOutput = 3;
      }

      else if (message.substring(6, 12) == "12LOW*")
      {
        Serial.println("Pin 12 LOW");
        commandeOutput = 4;
      }

      switch (commandeOutput)
      {
        case 1:
          digitalWrite(pin13, HIGH);
          Serial.println("13 ON");
          break;
        case 2:
          digitalWrite(pin13, LOW);
          Serial.println("13 OFF");
          break;
        case 3:
          digitalWrite(pin12, HIGH);
          Serial.println("12 ON");
          break;
        case 4:
          digitalWrite(pin12, LOW);
          Serial.println("12 OFF");
          break;
        default:
          Serial.println("NO OUTPUT COMMAND FOUND");
          break;
      }

    }
    else {
      ser.println("AT+CIPCLOSE");
      Serial.println("NO OUTPUT COMMAND FOUND, CLOSING CONNECTION WITH CONNECTION WITH TALKBACK");
    }
  }
}






