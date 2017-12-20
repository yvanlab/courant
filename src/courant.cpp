
#include <Arduino.h>

#include "ElectricLine.h"
#include <thingSpeakManager.h>
#include <WifiManagerV2.h>
#include "SettingManager.h"
#include <myTimer.h>
#include <ADS1115.h>

#include <DHTManager.h>

//Channel ID : 349755
//key : 80BASNDM7S5TMHIG

#define MODULE_NAME CURRENT_NAME
#define MODULE_MDNS CURRENT_MDNS
#define MODULE_MDNS_AP CURRENT_MDNS_AP
#define MODULE_IP   CURRENT_IP



#define LOG_LABEL     5 //"log"
//#define WATT_LABEL "watt"
#define CURRENT_LINE_1_LABEL 1 //"current"
#define CURRENT_LINE_2_LABEL 2 //"KWH"
#define CURRENT_LINE_3_LABEL 3
#define KWH_LINE_1_LABEL 4
#define HUM_LABEL     5 //"HUM"
#define TEMP_LABEL    6 //"TEMP"


ADS1115 adc0(ADS1115_DEFAULT_ADDRESS);


const unsigned char  pinLed = D4;
const unsigned char  pinCurrent = A0;
//const unsigned long timerFrequence = 6000;//ms
//const unsigned long maxNbreMesure = 60000/timerFrequence; // send KPI every minute



SettingManager smManager(pinLed);
ElectricLine elecLine1(pinLed,&adc0,0);
ElectricLine elecLine2(pinCurrent,&adc0,1);
ElectricLine elecLine3(pinCurrent,&adc0,2);

WifiManager wfManager(pinLed,&smManager);
thingSpeakManager sfManager(pinLed);
DHTManager dhtManager(D2,pinLed);

#ifdef MCPOC_TELNET
RemoteDebug             Debug;
#endif

#ifdef MCPOC_TELNET // Not in PRODUCTION
void processCmdRemoteDebug() {
    String lastCmd = Debug.getLastCommand();
    if (lastCmd == "l1") {
      DEBUGLOG(elecLine1.readCurrent());
    } else if (lastCmd == "l2") {
        DEBUGLOG(elecLine2.readCurrent());
    } else if (lastCmd == "l3") {
      DEBUGLOG(elecLine2.readCurrent());
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
    tt += "\"uptime\":\"" + NTP.getUptimeString() +"\"," ;
    tt += "\"build_date\":\""+ String(__DATE__" " __TIME__)  +"\"},";
    tt += "\"LOG\":["+wfManager.log(JSON_TEXT)  + "," +
                      dhtManager.log(JSON_TEXT)  + "," + wfManager.getHourManager()->log(JSON_TEXT) + ","+
                      elecLine1.log(JSON_TEXT) + ","+ elecLine2.log(JSON_TEXT) + ","+ elecLine3.log(JSON_TEXT) + ","  +
                      sfManager.log(JSON_TEXT)+"],";
    tt += "\"courant\":{" + elecLine1.toString(JSON_TEXT) + "},";
    tt += "\"courant2\":{" + elecLine2.toString(JSON_TEXT) + "},";
    tt += "\"courant3\":{" + elecLine3.toString(JSON_TEXT) + "},";
    tt += "\"dht\":{"+ dhtManager.toString(JSON_TEXT) + "},";
    //tt += "\"datetime\":{\"date\":\""+NTP.getDateStr()+"\",\"time\":\""+NTP.getTimeStr()+"\"}}";
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
  message += "<p>" + elecLine1.toString(STD_TEXT) + "</p>";
  message += "<p>" + dhtManager.toString(STD_TEXT) + "</p>";
  message += "<h2>Log data</h2>\
  		<TABLE border=2 cellpadding=10 log>";
  message += "<TR><TD>"+smManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+elecLine1.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+wfManager.getHourManager()->log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+sfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+wfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+dhtManager.log(STD_TEXT)+"</TD></TR>";
  message += "</TABLE>\
  		        <h2>Links</h2>";
  message += "<A HREF=\""+WiFi.localIP().toString()+ "\">cette page</A></br>";
  message += "<A HREF=\"https://thingspeak.com/channels/"+ String(smManager.m_privateKey) +"\">thingspeak</A></br>\
              <h2>Commands</h2>\
              <ul><li>/clear => erase credentials</li>\
                  <li>/credential => display credential</li>\
                  <li>/restart => restart module</li>\
                  <li>/status => Json details</li>\
                  <li>/whatever => display summary</li></ul>";
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
  mtTimer.setCustomMS(10000); //10s
}


void loop ( void ) {
	wfManager.handleClient();
  if (mtTimer.isCustomPeriod()) {
      //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("readCapteurs");
      elecLine1.measureCurrent();
      elecLine2.measureCurrent();
      elecLine3.measureCurrent();
      //DEBUGLOG(getJson());
  }
  if (mtTimer.is1MNPeriod()) {
      //DEBUGLOG(ESP_PLATFORM );
      DEBUGLOG("send to IoT");
      sfManager.addVariable(CURRENT_LINE_1_LABEL, String(elecLine1.getCourant()));
      sfManager.addVariable(CURRENT_LINE_2_LABEL, String(elecLine2.getCourant()));
      sfManager.addVariable(CURRENT_LINE_3_LABEL, String(elecLine3.getCourant()));
      sfManager.addVariable(HUM_LABEL, String(dhtManager.getHumidity()));
      sfManager.addVariable(TEMP_LABEL, String(dhtManager.getTemperature()));
      sfManager.sendIoT( smManager.m_privateKey, smManager.m_publicKey);
      DEBUGLOG(getJson());
  }

  if (mtTimer.is5MNPeriod()) {
    if (!WiFi.isConnected()) {
      ESP.restart();
    }
  }


    mtTimer.clearPeriod();
}
