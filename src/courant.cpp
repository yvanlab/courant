
#include <Arduino.h>

#include "ElectricLine.h"
#include "LuxManager.h"
#include <thingSpeakManager.h>
#include <WifiManagerV2.h>
#include "SettingManager.h"
#include <myTimer.h>
#include <ADS1115.h>
#include <DHT.h>

#include <DHTManagerV2.h>
/*#include "DHTTemperature.h"
#include "DHTHumidity.h"*/

#define MODULE_NAME CURRENT_NAME
#define MODULE_MDNS CURRENT_MDNS
#define MODULE_MDNS_AP CURRENT_MDNS_AP
#define MODULE_IP   CURRENT_IP



#define LOG_LABEL     5 //"log"
//#define WATT_LABEL "watt"
#define CURRENT_LINE_1_LABEL  1 //"current"
#define CURRENT_LINE_2_LABEL  2 //"KWH"
#define CURRENT_LINE_3_LABEL  3
#define KWH_LINE_1_LABEL      4
#define KWH_LINE_2_LABEL      5
#define LUX_LABEL             6
#define HUM_LABEL             7 //"HUM"
#define TEMP_LABEL            8 //"TEMP"


ADS1115 adc0(ADS1115_DEFAULT_ADDRESS);
//DHT dht;

uint8_t  pinLed = D4;
uint8_t  pinDHT = D5;

SettingManager smManager(pinLed);
ElectricLine elecLine1(pinLed,&adc0,ADS1115_MUX_P0_NG);
ElectricLine elecLine2(pinLed,&adc0,ADS1115_MUX_P1_NG);
ElectricLine elecLine3(pinLed,&adc0,ADS1115_MUX_P2_NG);
LuxManager   lux(pinLed,&adc0,ADS1115_MUX_P3_NG);
DHTManagerV2 dht(pinLed,pinDHT);

WifiManager wfManager(pinLed,&smManager);
thingSpeakManager sfManager(pinLed);


#ifdef MCPOC_TELNET
RemoteDebug             Debug;
#endif

#ifdef MCPOC_TELNET // Not in PRODUCTION
void processCmdRemoteDebug() {
    String lastCmd = Debug.getLastCommand();
    if (lastCmd == "l1") {
      DEBUGLOG(elecLine1.getValue());
    } else if (lastCmd == "l2") {
        DEBUGLOG(elecLine2.getValue());
    } else if (lastCmd == "l3") {
      DEBUGLOG(elecLine2.getValue());
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
                      /*temp.log(JSON_TEXT)  + ","+hum.log(JSON_TEXT)*/ dht.log(JSON_TEXT)  + "," + wfManager.getHourManager()->log(JSON_TEXT) + ","+
                      elecLine1.log(JSON_TEXT) + ","+ elecLine2.log(JSON_TEXT) + ","+ elecLine3.log(JSON_TEXT) + ","  +
                      lux.log(JSON_TEXT) + "," +
                      sfManager.log(JSON_TEXT)+"],";
    tt += "\"courant1\":{" + elecLine1.toString(JSON_TEXT) + "},";
    tt += "\"courant2\":{" + elecLine2.toString(JSON_TEXT) + "},";
    tt += "\"courant3\":{" + elecLine3.toString(JSON_TEXT) + "},";
    tt += "\"lux\":{" + lux.toString(JSON_TEXT) + "},";
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
      <title>Summary page</title>\
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
  message += "<p>Courant  line 2\t: " + elecLine2.toString(STD_TEXT) + "</p>";
  message += "<p>Courant line 3\t: " + elecLine3.toString(STD_TEXT) + "</p>";
  message += "<p>Luminosite \t: " + lux.toString(STD_TEXT) + "</p>";
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
    String helpCmd = "l1\n\rl2\n\rl3\n\rrestart\n\rreset\n\r";
    Debug.setHelpProjectsCmds(helpCmd);
    Debug.setCallBackProjectCmds(&processCmdRemoteDebug);
  #endif

  #ifdef MCPOC_MOCK
  randomSeed(analogRead(0));
  #endif

  mtTimer.begin(timerFrequence);
  mtTimer.setCustomMS(3000); //10s

  #ifdef MCPOC_TEST
  Serial.println("Testing device connections...");
  Serial.println(adc0.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");
  adc0.showConfigRegister();
  Serial.print("HighThreshold="); Serial.println(adc0.getHighThreshold(),BIN);
  Serial.print("LowThreshold="); Serial.println(adc0.getLowThreshold(),BIN);
  #endif
  adc0.initialize(); // initialize ADS1115 16 bit A/D chip
  // We're going to do single shot sampling
  adc0.setMode(ADS1115_MODE_SINGLESHOT);
  // Slow things down so that we can see that the "poll for conversion" code works
  adc0.setRate(ADS1115_RATE_860);
  // Set the gain (PGA) +/- 6.144V
  // Note that any analog input must be higher than –0.3V and less than VDD +0.3
  adc0.setGain(ADS1115_PGA_4P096);
  //adc0.setConversionReadyPinMode();

  //dht.setup(pinDHT,DHT::AUTO_DETECT);
  //ESP.wdtDisable();
}


void loop ( void ) {
	wfManager.handleClient();
  if (mtTimer.isCustomPeriod()) {
     //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("readCapteurs");
      elecLine1.mesure();
      elecLine2.mesure();
      elecLine3.mesure();
      lux.mesure();
      dht.mesure();
      /*temp.mesure();
      hum.mesure();*/
      //DEBUGLOG(getJson());
  }
  if (mtTimer.is1MNPeriod()) {
      //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("send to IoT");
      elecLine1.getValue();
      elecLine2.getValue();
      elecLine3.getValue();
      lux.getValue();
      dht.getHumiditySensor()->getValue();
      dht.getTemperatureSensor()->getValue();
      sfManager.addVariable(CURRENT_LINE_1_LABEL, String(elecLine1.getValue()));
      sfManager.addVariable(CURRENT_LINE_2_LABEL, String(elecLine2.getValue()));
      sfManager.addVariable(CURRENT_LINE_3_LABEL, String(elecLine3.getValue()));
      sfManager.addVariable(KWH_LINE_1_LABEL, String(elecLine1.getKWattHour()));
      sfManager.addVariable(KWH_LINE_2_LABEL, String(elecLine2.getKWattHour()));
      sfManager.addVariable(LUX_LABEL, String(lux.getValue()));
      sfManager.addVariable(HUM_LABEL, String(dht.getHumiditySensor()->getValue()));
      sfManager.addVariable(TEMP_LABEL, String(dht.getTemperatureSensor()->getValue()));
      sfManager.sendIoT( smManager.m_privateKey, smManager.m_publicKey);
      DEBUGLOG(getJson());
  }

  if (mtTimer.is5MNPeriod()) {
    if (wfManager.getHourManager()->isNextDay()) {
       // clear max/min
       elecLine1.clear();
       elecLine2.clear();
       elecLine3.clear();
       lux.clear();
       dht.clear();
     }
    if (!WiFi.isConnected()) {
      ESP.restart();
    }
  }

  mtTimer.clearPeriod();
}
