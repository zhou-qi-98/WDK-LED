#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <arduino_homekit_server.h>
#include <Ticker.h>
#include "ButtonDebounce.h"
#include "ButtonHandler.h"

#ifndef STASSID
#define STASSID "WDK_LED"
#define STAPSK  "12345678"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;
ESP8266WebServer server(80);//服务器端口80

/*str字符串存放的是web配网的页面html代码*/
String str = "<!DOCTYPE html><html>  <head>    <meta charset=\"UTF-8\">    <meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0\">   <meta http-equiv=\"X-UA-Compatible\"content=\"ie=edge\">    <title>杭电信工周加浪</title>  <style type=\"text/css\"> body  {   text-align: center;   padding-bottom: h px; } input {   width: 235px;   height: 38px;   border-radius: 10px;    margin-top: 16px;   margin-bottom: 20px;    border: 1px solid #ccc;   background-color: transparent;    transition: border-color ease-in-out 3s,box-shadow ease-in-out 3s;      transition: border-radius 2s;     -webkit-transition: border-radius 2s;   } .cc {   text-align: center;   padding-top: 20px;  } #juli20 {   padding-top: 20px;  }   .set{   width: 310px;   background:#9B30FF;   color: #FFFFFF;   font-size: 17px;  } .shuoming{    text-align: center;   color: #7A7A7A;   z-index: 9999;    position: fixed;    bottom: 40px;   width: 100%;    height: h px; } a {   text-decoration:none;   color: #9B30FF; } .lanse{   color: #000;  }</style>   </head> <body>    <h2 class=\"juli20\">WEB配网</h2>   <p>联系方式：18094716852</p>      <form class=\"cc\" name=\"my\">     <font class=\"lanse\">WiFi名称：</font>          <input type=\"text\" name=\"s\" placeholder=\"请输入您WiFi的名称\" id=\"aa\">      <br>      <font class=\"lanse\">WiFi密码：</font>            <input type=\"password\" name=\"p\" placeholder=\"请输入您WiFi的密码\" id=\"bb\">      <br>      <input class=\"set\" type=\"submit\" value=\"连接\" onclick=\"wifi()\">         </form>   <div class=\"shuoming\">      <p>Copyright &nbsp © &nbsp 2020 &nbsp by &nbsp<a href=\"http://xwx.7yunkj.com\">网址</a></p>       </div>    <script language=\"javascript\">            function wifi()     {       var ssid = aa.value;        var password = bb.value;        var xmlhttp=new XMLHttpRequest();       xmlhttp.open(\"GET\",\"/HandleVal?ssid=\"+ssid+\"&password=\"+password,true);       xmlhttp.send();         alert(\"用户名为：\"+ssid+' '+\"密码为：\"+password);      }   </script> </body></html>";

#define PIN_LED 16//D0
#define PIN_MEG 2//D4

#define SIMPLE_INFO(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

/*****************************************************
 * 函数名称：handleRoot()
 * 函数说明：客户端请求回调函数
 * 参数说明：无
******************************************************/
void handleRoot() {
  server.send(200, "text/html", str);
}
/*****************************************************
 * 函数名称：HandleVal()
 * 函数说明：对客户端请求返回值处理
 * 参数说明：无
******************************************************/
void HandleVal()
{
    String wifis = server.arg("ssid"); //从JavaScript发送的数据中找ssid的值
    String wifip = server.arg("password"); //从JavaScript发送的数据中找password的值
    Serial.println(wifis); Serial.println(wifip);
    WiFi.begin(wifis,wifip);
}
/*****************************************************
 * 函数名称：handleNotFound()
 * 函数说明：响应失败函数
 * 参数说明：无
******************************************************/
void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}
/*****************************************************
 * 函数名称：autoConfig()
 * 函数说明：自动连接WiFi函数
 * 参数说明：无
 * 返回值说明:true：连接成功 false：连接失败
******************************************************/
bool autoConfig()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("AutoConfig Waiting......");
  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;
      //break;
    }
    else
    {
      digitalWrite(PIN_MEG, 1);
      Serial.print(".");
      delay(800);
      //digitalWrite(PIN_MEG, 0);
      analogWrite(PIN_MEG,980);
      delay(10);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
  //WiFi.printDiag(Serial);
}
/*****************************************************
 * 函数名称：htmlConfig()
 * 函数说明：web配置WiFi函数
 * 参数说明：无
******************************************************/
void htmlConfig()
{
    WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    WiFi.softAP(ssid, password);
    Serial.println("AP设置完成");
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  
    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS responder started");
    }
  
    server.on("/", handleRoot);
    server.on("/HandleVal", HTTP_GET, HandleVal);
    server.onNotFound(handleNotFound);//请求失败回调函数
  
    server.begin();//开启服务器
    Serial.println("HTTP server started");
    while(1)
    {   digitalWrite(PIN_MEG, 1);
        delay(100);
        digitalWrite(PIN_MEG, 0);
        delay(100);
        //Serial.println("HtmlCESHI");
        server.handleClient();
        MDNS.update();  
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("HtmlConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            Serial.println("HTML连接成功");
            break;
        }
    }  
}



void blink_led(int interval, int count) {
	for (int i = 0; i < count; i++) {
		builtinledSetStatus(true);
		delay(interval);
		builtinledSetStatus(false);
		delay(interval);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.setRxBufferSize(32);
	Serial.setDebugOutput(false);

	pinMode(PIN_LED, OUTPUT);
 pinMode(2, OUTPUT);
  bool wifiConfig = autoConfig();
    
  if(wifiConfig == false){
      
      htmlConfig();//HTML配网   
  }
  
	SIMPLE_INFO("");
	SIMPLE_INFO("SketchSize: %d", ESP.getSketchSize());
	SIMPLE_INFO("FreeSketchSpace: %d", ESP.getFreeSketchSpace());
	SIMPLE_INFO("FlashChipSize: %d", ESP.getFlashChipSize());
	SIMPLE_INFO("FlashChipRealSize: %d", ESP.getFlashChipRealSize());
	SIMPLE_INFO("FlashChipSpeed: %d", ESP.getFlashChipSpeed());
	SIMPLE_INFO("SdkVersion: %s", ESP.getSdkVersion());
	SIMPLE_INFO("FullVersion: %s", ESP.getFullVersion().c_str());
	SIMPLE_INFO("CpuFreq: %dMHz", ESP.getCpuFreqMHz());
	SIMPLE_INFO("FreeHeap: %d", ESP.getFreeHeap());
	SIMPLE_INFO("ResetInfo: %s", ESP.getResetInfo().c_str());
	SIMPLE_INFO("ResetReason: %s", ESP.getResetReason().c_str());
	INFO_HEAP();
	homekit_setup();
	INFO_HEAP();
	blink_led(200, 3);
}

void loop() {
	homekit_loop();
}

void builtinledSetStatus(bool on) {
	digitalWrite(PIN_LED, on ? LOW : HIGH);
}

//==============================
// Homekit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" void occupancy_toggle();
extern "C" void led_toggle();
extern "C" void accessory_init();

ButtonDebounce btn(0, INPUT_PULLUP, LOW);
ButtonHandler btnHandler;

void IRAM_ATTR btnInterrupt() {
	btn.update();
}

void homekit_setup() {
	accessory_init();
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

	arduino_homekit_setup(&config);

	btn.setCallback(std::bind(&ButtonHandler::handleChange, &btnHandler,
			std::placeholders::_1));
	btn.setInterrupt(btnInterrupt);
	btnHandler.setIsDownFunction(std::bind(&ButtonDebounce::checkIsDown, &btn));
	btnHandler.setCallback([](button_event e) {
		if (e == BUTTON_EVENT_SINGLECLICK) {
			SIMPLE_INFO("Button Event: SINGLECLICK");
			led_toggle();
		} else if (e == BUTTON_EVENT_DOUBLECLICK) {
			SIMPLE_INFO("Button Event: DOUBLECLICK");
			occupancy_toggle();
		} else if (e == BUTTON_EVENT_LONGCLICK) {
			SIMPLE_INFO("Button Event: LONGCLICK");
			SIMPLE_INFO("Rebooting...");
			homekit_storage_reset();
			ESP.restart(); // or system_restart();
		}
	});
}

void homekit_loop() {
	btnHandler.loop();
	arduino_homekit_loop();
	static uint32_t next_heap_millis = 0;
	uint32_t time = millis();
	if (time > next_heap_millis) {
		SIMPLE_INFO("heap: %d, sockets: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
		next_heap_millis = time + 5000;
	}
}
