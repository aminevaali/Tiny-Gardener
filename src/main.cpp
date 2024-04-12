/************************************************************************************
 *  Created By: Amin Vali
 **********************************************************************************/
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <arduino-timer.h>

#define SECOND 1000
#define MINUTE 60000

#define VALVE1 5
#define VALVE2 6
#define DIRT_HUMIDITY_PIN A1
#define DRYCOUNTERLIMIT 7

#define rxPin 3 // Arduino rx = pin3 ==> connected to tx pin of GSM module
#define txPin 2 // Arduino tx = pin2 ==> connected to rx pin of GSM module
#define gsmResetPin 4 // connected to rst pin of GSM module

#define TOTAL_PHONE_NUMS 3
#define PHONE_NUM_LENGTH 13


//#### watering state variables ####
unsigned short dryCounter = 0;
const unsigned long wateringTime1 = 20UL * MINUTE; // 0.5 hour by milliseconds
const unsigned long wateringTime2 = 25UL * MINUTE;
const int HUMIDITY_TO_WATERING = 768; // 750% dryness = 20% humidity
bool automaticWatering = true;
bool chainValves = true;
//#### end watering state variables ####

char phoneNumbers[TOTAL_PHONE_NUMS][PHONE_NUM_LENGTH + 1]; // This variable saves allowed phone numbers which is read from EEPROM
SoftwareSerial sim800(rxPin,txPin); // sim800 variable is used to interact with sim800 module
auto timer = timer_create_default(); // making timer object via arduino-timer library to schedule tasks
String smsStatus = "",senderNumber = "",receivedDate = "",msg = ""; // variables to save the strings fetched from sim800 module


// #### function prototypes ####
void afterWatering1(void *args);
inline void afterWatering2(void *args);
void wateringSetup();
void openValve1();
void openValve2();
inline void closeValve1();
inline void closeValve2();
void parseData(String buff);
void extractSmsCMT(String buff);
void extractSmsCMGR(String buff);
void doAction(String phoneNumber);
void Reply(String text, String Phone);
void sendStatus(String Phone);
void readPhoneNumsFromEEPROM(int addrOffset, char outputPhoneNo[PHONE_NUM_LENGTH + 1]);
boolean validatePhoneNo(const char *number);
void resetGsm();
// #### end of function prototypes ####


/*******************************************************************************
 * setup function
 ******************************************************************************/
void setup() {

  Serial.begin(9600); // Arduino SoftwareSerial initialization
  sim800.begin(9600); // Sim800 SoftwareSerial initialization
  Serial.println("Tiny Gardener Initializing...");

  wateringSetup(); // determining pinModes of valves and soil humidity sensor
  pinMode(gsmResetPin, OUTPUT); // determining gsmResetPin as output
  digitalWrite(gsmResetPin, HIGH); // disabling reset pin by making it HIGH value (it's active-low)

  //#### Reading allowed phone numbers from EEPROM ####
  Serial.println("List of Registered Phone Numbers");
  for (int i = 0; i < TOTAL_PHONE_NUMS; i++){
    readPhoneNumsFromEEPROM(i * 13, phoneNumbers[i]);
    Serial.println(phoneNumbers[i]);
  }
  //#### End Reading allowed phone numbers from EEPROM ####

  resetGsm(); // Reseting the GSM module everytime that Arduino turns on
}


/*******************************************************************************
 * Loop Function
 ******************************************************************************/
void loop() {
  timer.tick();

  int humidity = analogRead(DIRT_HUMIDITY_PIN);
  if(humidity >= HUMIDITY_TO_WATERING){
    if(dryCounter < DRYCOUNTERLIMIT){
      dryCounter++;
    }
  }else{
    dryCounter = 0;
  }
  if(digitalRead(VALVE1) == 1 && digitalRead(VALVE2) == 1 && automaticWatering && dryCounter == DRYCOUNTERLIMIT){
    openValve1();
  }

  while(sim800.available()){
    parseData(sim800.readString());
  }

  while(Serial.available())  {
    sim800.println(Serial.readString());
  }

} //main loop ends


void afterWatering1(void *args){
  closeValve1();
  if(chainValves){
    openValve2();
    Reply("v2 opnd aftr v1", phoneNumbers[0]);
  }

  int humidity = analogRead(DIRT_HUMIDITY_PIN);
  if(humidity >= HUMIDITY_TO_WATERING){
    automaticWatering = false;
    Reply("snsr sspndd. err", phoneNumbers[0]);
  }
}

inline void afterWatering2(void *args){
  closeValve2();
}

/****
 * wateringSetup function
 * determining pinModes of valves and soil humidity sensor
****/
void wateringSetup(){
  pinMode(VALVE1, OUTPUT);
  pinMode(VALVE2, OUTPUT);
  pinMode(DIRT_HUMIDITY_PIN, INPUT);
  closeValve1();
  closeValve2();
}

void openValve1(){
  digitalWrite(VALVE1, LOW);
  timer.in(wateringTime1, afterWatering1);
}

void openValve2(){
  digitalWrite(VALVE2, LOW);
  timer.in(wateringTime2, afterWatering2);
}

inline void closeValve1(){
  digitalWrite(VALVE1, HIGH);
}

inline void closeValve2(){
  digitalWrite(VALVE2, HIGH);
}

/*******************************************************************************
 * parseData function:
 * this function parse the incomming command such as CMTI or CMGR etc.
 * if the sms is received. then this function read that sms and then pass 
 * that sms to "extractSms" function. Then "extractSms" function divide the
 * sms into parts. such as sender_phone, sms_body, received_date etc.
 ******************************************************************************/
void parseData(String buff){
  Serial.print(buff);
  Serial.println();
  if(buff.indexOf("RING") != -1){
    delay(2000);
    resetGsm();
  }

  unsigned int len, index;

  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  
  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    
    if(cmd == "+CMTI"){
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      //get the message stored at memory location "temp"
      sim800.println(temp); 
    }
    else if(cmd == "+CMT" || cmd == "+CMGR"){
      if(cmd == "+CMT")
        extractSmsCMT(buff);
      else //if(cmd == "+CMGR")
        extractSmsCMGR(buff);
      //----------------------------------------------------------------------------
      if(validatePhoneNo(senderNumber.c_str())){
        doAction(senderNumber);
        //delete all sms
        sim800.println("AT+CMGD=1,4");
        delay(1000);
        sim800.println("AT+CMGDA= \"DEL ALL\"");
        delay(1000);
      }
      //----------------------------------------------------------------------------
    }
    // else if(cmd == "+CMGR"){
    // }
  }
  else{
  //The result of AT Command is "OK"
  }
}

/*******************************************************************************
 * extractSms function:
 * This function divide the sms into parts. such as sender_phone, sms_body, 
 * received_date etc.
 ******************************************************************************/
void extractSmsCMT(String buff){
   unsigned int index;
   
    index = buff.indexOf(",");
    senderNumber = buff.substring(1, index-1); 
    buff.remove(0, index+5);

    Serial.println(buff);
    
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();
}

void extractSmsCMGR(String buff){
  unsigned int index;
   
  index = buff.indexOf(",");
  smsStatus = buff.substring(1, index-1); 
  buff.remove(0, index+2);
  
  senderNumber = buff.substring(0, 13);
  buff.remove(0,19);
  
  receivedDate = buff.substring(0, 20);
  buff.remove(0,buff.indexOf("\r"));
  buff.trim();
  
  index =buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
}

/*******************************************************************************
 * doAction function:
 * Performs action according to the received sms
 ******************************************************************************/
void doAction(String phoneNumber){
  if(msg == "callme"){
    sim800.println("ATD" + phoneNumber + ";");
  }else if(msg == "ping"){
    Reply("pong", phoneNumber);
  }else if(msg == "rstgsm"){
    resetGsm();
  }else if(msg == "v1 on"){
    openValve1();
    Reply("valve1 opened", phoneNumber);
  }else if(msg == "v1 off"){
    closeValve1();
    Reply("valve1 closed", phoneNumber);
  }else if(msg == "v2 on"){
    openValve2();
    Reply("valve2 opened", phoneNumber);
  }else if(msg == "v2 off"){
    closeValve2();
    Reply("valve2 closed", phoneNumber);
  }else if(msg == "status"){
    sendStatus(phoneNumber);
  }else if(msg == "ch tog"){ // chainValves toggle
    chainValves = !chainValves;
    Reply(chainValves?"ch on":"ch off", phoneNumber);
  }else if(msg == "auto tog"){ // automatic watering toggle
    automaticWatering = !automaticWatering;
    Reply(automaticWatering?"aut on":"aut off", phoneNumber);
  }

  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
}

/*******************************************************************************
 * Reply function
 * Send an sms
 ******************************************************************************/
void Reply(String text, String Phone)
{
    delay(3000);
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\""+Phone+"\"\r");
    delay(1000);
    sim800.print(text);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");
}

void sendStatus(String Phone){
  delay(3000);
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\""+Phone+"\"\r");
  delay(1000);

  sim800.print("valve1: ");
  sim800.println(digitalRead(VALVE1)? "closed" : "open");

  sim800.print("valve2: ");
  sim800.println(digitalRead(VALVE2)? "closed" : "open");

  sim800.print("humidity: ");
  int dryness = analogRead(DIRT_HUMIDITY_PIN) * 100UL / 1023UL;
  sim800.print(100 - dryness);
  sim800.println("%");

  sim800.print("valves chain: ");
  sim800.println(chainValves? "yes" : "no");

  sim800.print("auto watering: ");
  sim800.print(automaticWatering? "yes" : "no");

  delay(100);
  sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}

/*******************************************************************************
 * readPhoneNumsFromEEPROM function:
 * Store phone numbers in EEPROM
 ******************************************************************************/
void readPhoneNumsFromEEPROM(int addrOffset, char outputPhoneNo[PHONE_NUM_LENGTH + 1])
{
  for (int i = 0; i < PHONE_NUM_LENGTH; i++)
  {
    outputPhoneNo[i] = EEPROM.read(addrOffset + i);
  }
  outputPhoneNo[PHONE_NUM_LENGTH] = '\0';
}

/*******************************************************************************
 * validatePhoneNo function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
boolean validatePhoneNo(const char *number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < TOTAL_PHONE_NUMS; i++){
    if(strcmp(phoneNumbers[i], number) == 0){
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}

void resetGsm(){
  digitalWrite(gsmResetPin, LOW);
  delay(1000);
  digitalWrite(gsmResetPin, HIGH);
  delay(30 * SECOND);

  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);
  //delete all sms
  sim800.println("AT+CMGD=1,4");
  delay(1000);
  sim800.println("AT+CMGDA= \"DEL ALL\"");
  delay(1000);
  Reply("Gsm reseted successfully", phoneNumbers[0]);
}
