#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "img.h"
#define sck 1 /* 屏幕 */
#define sda 3

String  keys  = "32087ff2b8704262a4c4471be430f8b1";  // 接口地址：https://console.heweather.com/app/index
String  dq  = "101210106"; //填入城市编号  获取编号 https://where.heweather.com/index.html

String weekDays[7]={"星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
//Month names
String months[12]={"一月", "二月", "三月", "四月","五月", "六月", "七月", "八月", "九月", "十月", "十一月", "十二月"};

int             icon    = 100;
String          temp    = "NaN";
String          feelsLike   = "NaN";
String          text    = "NaN";
String          pm2p5   ="NaN";

unsigned long   previousMillis = 0;
const long      interval =1800000;      /* 半小时更新天气  */

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2( U8G2_R0, /* clock=*/ sck, /* data=*/ sda, /* reset=*/ U8X8_PIN_NONE );
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"ntp1.aliyun.com",60*60*8,30*60*1000);

bool autoConfig()
{
  WiFi.begin();
  int count=0;
  for (int i = 0; i < 20; i++)
  {
    count++;
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED)
    {
      displayWifiInfo();
      WiFi.printDiag(Serial);
      return true;
    }
    else
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_unifont_t_chinese2);
      u8g2.setCursor(0, 14);
      u8g2.print("Begin Connecting...");
      u8g2.setCursor(40, 30);
      u8g2.print("(Times)");
      u8g2.setCursor(58, 47);
      u8g2.print(count);
      u8g2.sendBuffer();    
      Serial.println(wstatus);
      delay(1000);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
}

void smartConfig()
{
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setCursor(0, 14);
  u8g2.print("Wait EspTouch...");
  u8g2.sendBuffer();  
  while (1)
  {
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      WiFi.setAutoConnect(true);  // 设置自动连接
      displayWifiInfo();
      break;
    }
    delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
  }
}

void displayWifiInfo()
{
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_unifont_t_chinese2);
  //u8g2.setCursor(0, 14);
  //u8g2.print("Connected!");  
  //u8g2.setCursor(0, 30);
  //u8g2.print(WiFi.SSID().c_str());
  //u8g2.setCursor(0, 47);
  //u8g2.print(WiFi.psk().c_str());
  u8g2.drawXBMP(0,0,128,64,logo);
  u8g2.sendBuffer();
}

void setup()
{
  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();
  timeClient.begin();
     
  if (!autoConfig())
  {
    Serial.println("Start module");
    smartConfig();
  }
  delay(1000);
  gethttpandshow();
}

void loop()
{
  showinfo();
  delay(1000);
  if(WiFi.status()== WL_CONNECTED)
  {
    unsigned long currentMillis = millis();
    if ( currentMillis - previousMillis >= interval )
    {
      previousMillis = currentMillis;
      gethttpandshow();
    }
  }
}

void showinfo()
{
    u8g2.clearBuffer();
    iconlib();
    u8g2.setFont( u8g2_font_unifont_t_chinese2 );
    u8g2.setFontDirection( 0 );    
    u8g2.setCursor(40,15);
    u8g2.print("TEMP:");
    u8g2.setCursor(85,15);
    u8g2.print(temp);//温度
    u8g2.drawXBMP(105,2,12,15,iocn_ssd);
    
    u8g2.setCursor(40,30);
    u8g2.print("PM2.5:");  
    u8g2.setCursor(90,30);
    u8g2.print(pm2p5);//pm2.5浓度

    //可以根据自己的喜好显示星期和月份
    timeClient.update();
    //获取时间戳
    unsigned long epochTime = timeClient.getEpochTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);
    //转化为当前时间
    String formattedTime = timeClient.getFormattedTime();
    Serial.print("Formatted Time: ");
    Serial.println(formattedTime);
    int currentHour = timeClient.getHours();
    Serial.print("Hour: ");
    Serial.println(currentHour);
    int currentMinute = timeClient.getMinutes();
    Serial.print("Minutes: ");
    Serial.println(currentMinute);
    int currentSecond = timeClient.getSeconds();
    Serial.print("Seconds: ");
    Serial.println(currentSecond);
    String weekDay = weekDays[timeClient.getDay()];
    Serial.print("Week Day: ");
    Serial.println(weekDay);

    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    Serial.print("Month day: ");
    Serial.println(monthDay);
    int currentMonth = ptm->tm_mon+1;
    Serial.print("Month: ");
    Serial.println(currentMonth);
    String currentMonthName = months[currentMonth-1];
    Serial.print("Month name: ");
    Serial.println(currentMonthName);
    int currentYear = ptm->tm_year+1900;
    Serial.print("Year: ");
    Serial.println(currentYear);
    
    String currentDate = String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay);
    Serial.print("Current date: ");
    Serial.println(currentDate); 
    u8g2.drawLine(0, 40, 125, 40);
    u8g2.setFont(u8g2_font_ncenB14_tr);  
    u8g2.setFontDirection(0);
    u8g2.setCursor(0,60);
    u8g2.print(formattedTime);
    u8g2.setFont(u8g2_font_6x10_tf);  
    u8g2.setFontDirection(0);
    u8g2.setCursor(80,60);
    u8g2.print(currentDate);    
    u8g2.sendBuffer();       
}

void gethttpandshow()
{
  weather();
  delay(1000);
  air();
  delay(1000);
}

void weather()
{
String line;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
  if (https.begin(*client, "https://devapi.heweather.net/v7/weather/now?gzip=n&location="+dq+"&key="+keys))
  {  
      int httpCode = https.GET();  
      if (httpCode > 0) {  
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {  
          line = https.getString();
          DynamicJsonBuffer jsonBuffer(1024);
          JsonObject& res_json = jsonBuffer.parseObject(line);
          
          int r1=res_json["now"]["icon"];//天气图标
          String r2=res_json["now"]["text"];//天气文字
          String r3=res_json["now"]["temp"];//温度
          String r4=res_json["now"]["feelsLike"];//体感温度
          jsonBuffer.clear();
          icon=r1;
          text=r2;
          temp=r3;
          feelsLike=r4;
        }  
      } 
      https.end();  
  } 
  else 
  {  
      Serial.printf("[HTTPS]请求链接失败\n");  
  } 
}

void air()
{
  String line;
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();  
  HTTPClient https;  
  if (https.begin(*client, "https://devapi.heweather.net/v7/air/now?gzip=n&location="+dq+"&key="+keys))
  {  
      int httpCode = https.GET();  
      if (httpCode > 0) {  
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {  
          line = https.getString();
          DynamicJsonBuffer jsonBuffer(1024);
          JsonObject& res_json = jsonBuffer.parseObject(line);
          String r1=res_json["now"]["pm2p5"];
          pm2p5=r1;
          jsonBuffer.clear();
        }  
      }
      https.end();  
  } 
  else 
  {  
      Serial.printf("[HTTPS]请求链接失败\n");  
  }
}

void iconlib(){
  if (icon==100||icon==150){//晴
      u8g2.drawXBMP(0,0,40,40,iocn_100);
  }else if (icon==102||icon==101){
      u8g2.drawXBMP(0,0,40,40,iocn_102);//云
  }else  if (icon==103||icon==153){
      u8g2.drawXBMP(0,0,40,40,iocn_103);//晴间多云
  }else if (icon==104||icon==154){
      u8g2.drawXBMP(0,0,40,40,iocn_104);//阴
  }else if (icon>=300&&icon<=301){
      u8g2.drawXBMP(0,0,40,40,iocn_301);//阵雨
  }else if (icon>=302&&icon<=303){
      u8g2.drawXBMP(0,0,40,40,iocn_302);//雷阵雨
  }else if (icon==304){
      u8g2.drawXBMP(0,0,40,40,iocn_304);//冰雹
  }else if (icon==399||icon==314||icon==305||icon==306||icon==307||icon==315||icon==350||icon==351){
      u8g2.drawXBMP(0,0,40,40,iocn_307);//雨
  }else if ((icon>=308&&icon<=313)||(icon>=316&&icon<=318)){
      u8g2.drawXBMP(0,0,40,40,iocn_310);//暴雨
  }else if ((icon>=402&&icon<=406)||icon==409||icon==410||icon==400||icon==401||icon==408||icon==499||icon==456){
      u8g2.drawXBMP(0,0,40,40,iocn_401);//雪
  }else if (icon==407||icon==457){
      u8g2.drawXBMP(0,0,40,40,iocn_407);//阵雪
  }else if (icon>=500&&icon<=502){
      u8g2.drawXBMP(0,0,40,40,iocn_500);//雾霾
  }else if (icon>=503&&icon<=508){
      u8g2.drawXBMP(0,0,40,40,iocn_503);//沙尘暴
  }else if (icon>=509&&icon<=515){
      u8g2.drawXBMP(0,0,40,40,iocn_509);//不适宜生存
  }else{      
      u8g2.drawXBMP(0,0,40,40,iocn_999);//未知
  }
}
