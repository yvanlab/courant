
#include <Arduino.h>

#include "ElectricLineA0.h"
#include "EauCapteur.h"

#include "SettingManager.h"
#include <myTimer.h>
#include <DHT.h>

#include <DHTManagerV2.h>

#include <WifiManagerV2.h>
#include <grovestreamsManager.h>
#include <thingSpeakManager.h>
#define MODULE_NAME CURRENT_GARAGE_NAME
#define MODULE_MDNS CURRENT_GARAGE_MDNS
#define MODULE_MDNS_AP CURRENT_GARAGE_MDNS_AP
#define MODULE_IP   CURRENT_GARAGE_IP



#define LOG_LABEL     5 //"log"
//#define WATT_LABEL "watt"
#define CURRENT_LINE_LABEL  1 //"current"
#define KWH_LINE_LABEL      2
#define HUM_LABEL           3 //"HUM"
#define TEMP_LABEL          4 //"TEMP"
#define EAU_LITRE           5 //"TEMP"


//ADS1115 adc0(ADS1115_DEFAULT_ADDRESS);
//DHT dht;

uint8_t  pinLed = D4;
uint8_t  pinDHT = D5;
uint8_t  pinFlowMeter = D2;
const unsigned char  pinCurrent = A0;

SettingManager smManager(pinLed);
ElectricLineA0 elecLine1(pinLed);
DHTManagerV2 dht(pinLed,pinDHT);
//EauCapteur flowMeter(pinLed, pinFlowMeter);

WifiManager wfManager(pinLed,&smManager);
thingSpeakManager sfManager(pinLed);
grovestreamsManager grovesMgt(pinLed);

#ifdef MCPOC_TELNET
RemoteDebug             Debug;
#endif

#ifdef MCPOC_TELNET // Not in PRODUCTION
void processCmdRemoteDebug() {
    String lastCmd = Debug.getLastCommand();
    if (lastCmd == "l1") {
      DEBUGLOG(elecLine1.getValue());
    } else if (lastCmd == "restart") {
        ESP.restart();
    } else if (lastCmd == "reset") {
      WiFi.disconnect();
    }
}
#endif


String getJson()
{
  String tt("{\"module\":{");
    tt += "\"nom\":\"" + String(MODULE_NAME) +"\"," ;
    tt += "\"version\":\"" + String(CURRENT_VERSION) +"\"," ;
    tt += "\"status\":\"" + String(STATUS_PERIPHERIC_WORKING) +"\"," ;
    tt += "\"uptime\":\"" + NTP.getUptimeString() +"\"," ;
    tt += "\"build_date\":\""+ String(__DATE__" " __TIME__)  +"\"},";
    tt += "\"LOG\":["+wfManager.log(JSON_TEXT)  + "," +
                      dht.log(JSON_TEXT)  + "," + wfManager.getHourManager()->log(JSON_TEXT) + ","+
                      //flowMeter.log(JSON_TEXT) + ","  +
                      elecLine1.log(JSON_TEXT) + ","  +
                      grovesMgt.log(JSON_TEXT) + "," +
                      sfManager.log(JSON_TEXT)+"],";
    tt += "\"courant1\":{" + elecLine1.toString(JSON_TEXT) + "},";
     //tt += "\"eau\":{flowMeter.toString(JSON_TEXT)},";
    tt += dht.toString(JSON_TEXT)+",";

    tt += "\"datetime\":{" + wfManager.getHourManager()->toDTString(JSON_TEXT) + "}}";
    return tt;
}


void dataJson() {
	digitalWrite ( pinLed, LOW );
  wfManager.getServer()->send ( 200, "text/json", getJson() );
  digitalWrite ( pinLed, HIGH );
}



void dataPage() {
	digitalWrite ( pinLed, LOW );

  String message =
  "<html>\
    <head>\
      <meta http-equiv='refresh' content='5'/>\
      <title>Summary page Garage</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
      <h1>Real time data!</h1>";
  message += "<p>" + wfManager.toString(STD_TEXT) + "</p>";
  message += "<p>Date Hour : " + wfManager.getHourManager()->toDTString(STD_TEXT) + "</p>";
  message += "<p>Uptime: " + wfManager.getHourManager()->toUTString() + "</p>";
  message += "<p>Courant line 1 \t: " + elecLine1.toString(STD_TEXT) + "</p>";
  message += "<p>Temperature\t: " + dht.getTemperatureSensor()->toString(STD_TEXT) + "</p>";
  message += "<p>Humidite\t: " + dht.getHumiditySensor()->toString(STD_TEXT) + "</p>";
  //message += "<p>" + dhtManager.toString(STD_TEXT) + "</p>";
  message += "<h2>Log data</h2>\
  		<TABLE border=2 cellpadding=10 log>";
  message += "<TR><TD>"+smManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+elecLine1.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+wfManager.getHourManager()->log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+sfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+wfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+dht.log(STD_TEXT)+"</TD></TR>";
  message += "</TABLE>\
  		        <h2>Links</h2>";
  message += "<A HREF=\""+WiFi.localIP().toString()+ "\">cette page</A></br>";
  message += "<A HREF=\"https://thingspeak.com/channels/"+ String(smManager.m_privateKey) +"\">thingspeak</A></br>\
              <h2>Commands</h2>\
              <ul><li>/clear => erase credentials</li>\
                  <li>/credential\t=> display credential</li>\
                  <li>/restart\t=> restart module</li>\
                  <li>/status\t=> Json details</li>\
                  <li>/whatever\t=> display summary</li></ul>";
  message += "</body></html>";
  wfManager.getServer()->send ( 200, "text/html", message );

  digitalWrite ( pinLed, HIGH );

}





void startWiFiserver() {
  if (wfManager.begin(IPAddress(MODULE_IP),MODULE_NAME, MODULE_MDNS, MODULE_MDNS_AP)==WL_CONNECTED) {
    wfManager.getServer()->on ( "/", dataPage );
    wfManager.getServer()->onNotFound ( dataPage );
  }
  wfManager.getServer()->on ( "/status", dataJson );

  Serial.println(wfManager.toString(STD_TEXT));
}



void setup ( void ) {
  // Iniialise input
  Wire.begin();
  Serial.begin ( 115200 );

  smManager.readData();
  DEBUGLOG("");DEBUGLOG(smManager.toString(STD_TEXT));
  startWiFiserver();

  #ifdef MCPOC_TELNET
    Debug.begin(MODULE_NAME);
    String helpCmd = "l1\n\rrestart\n\rreset\n\r";
    Debug.setHelpProjectsCmds(helpCmd);
    Debug.setCallBackProjectCmds(&processCmdRemoteDebug);
  #endif

  #ifdef MCPOC_MOCK
  randomSeed(analogRead(0));
  #endif

  mtTimer.begin(timerFrequence);
  mtTimer.setCustomMS(3000); //10s

  pinMode ( pinCurrent, INPUT );

}

capteurValue  lgn1,temp,eau;
void loop ( void ) {
	wfManager.handleClient();
  if (mtTimer.isCustomPeriod()) {
     //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("readCapteurs");
      elecLine1.mesure();
      dht.mesure();
      //flowMeter.mesure();
  }
  if (mtTimer.is1MNPeriod()) {
      //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("send to IoT");
      /*elecLine1.getValue();
      dht.getHumiditySensor()->getValue();
      dht.getTemperatureSensor()->getValue();*/
      float fLign = elecLine1.getValue();
      lgn1.mesure(fLign);
      sfManager.addVariable(CURRENT_LINE_LABEL, String(fLign));
      sfManager.addVariable(KWH_LINE_LABEL, String(elecLine1.getKWattHour()));
      sfManager.addVariable(HUM_LABEL, String(dht.getHumiditySensor()->getValue()));

      fLign = dht.getTemperatureSensor()->getValue();
      temp.mesure(fLign);
      sfManager.addVariable(TEMP_LABEL, String(fLign));

      /*fLign = flowMeter.getValue();
      eau.mesure(fLign);
      sfManager.addVariable(EAU_LITRE, String(fLign));*/

      sfManager.sendIoT( smManager.m_privateKey, smManager.m_publicKey);
      DEBUGLOG(getJson());
  }


  if (mtTimer.is1HPeriod()) {
    if (WiFi.isConnected()) {
      temp.set();
      grovesMgt.addVariable(TEMP_GARAGE , String(temp.m_value));
      grovesMgt.sendIoT(TEMP_ID);

      lgn1.set();
      grovesMgt.addVariable(COURANT_GENERAL , String(lgn1.m_value));
      grovesMgt.sendIoT(COURANT_ID);
      
      /*eau.set();
      grovesMgt.addVariable(EAU_JARDIN , String(eau.m_value));
      grovesMgt.sendIoT(COURANT_ID);*/
    }
  }


  if (mtTimer.is5MNPeriod()) {
    if (wfManager.getHourManager()->isNextDay()) {
       // clear max/min
       elecLine1.clear();
       dht.clear();
     }
    if (!WiFi.isConnected()) {
      ESP.restart();
    }
  }

  mtTimer.clearPeriod();
}
