#include <Dali.h>


const int DALI_TX = 3;
const int DALI_RX_A = 0;
char cmdStr[50];


void help() {
  PN("Enter <ADDR>,<CMD> or another command from list:");
  PN("<ADDR>,<CMD>: Numbers, either: XX or DDD or BBBBBBBB");
  PN("help       - command list");
  PN("on         - broadcast on");
  PN("off        - broadcast off");
  PN("scan       - device short address scan");
  PN("reset      - Redo bus init sequence");
  PN("terse      - Driver mode");
  PN("verbose    - Human mode");
  PN("initialise - perform process of initialisation");
  PN("randomise  - start process of initialisation");
  PN("assignaddr - assign addresses after randomise");
  PN("searchall  - search luminairs in whole range (FFFFFF)");
}

void setup() {

  Serial.begin(57600);
  dali.setupTransmit(DALI_TX);
  dali.setupAnalogReceive(DALI_RX_A);
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, LOW);
  dali.msgMode = true;
  help(); //Show help
  dali.busInit();

  Serial.print(dali.msgMode ? "OK> " : ":");
  memset(cmdStr, 0, sizeof(cmdStr));
}


void sinus () {
  uint8_t lf_1_add = 0;
  uint8_t lf_2_add = 1;
  uint8_t lf_3_add = 2;
  uint8_t lf_1;
  uint8_t lf_2;
  uint8_t lf_3;
  int i;

  while (Serial.available() == 0) {
    for (i = 0; i < 360; i = i + 1) {

      if (Serial.available() != 0) {
        dali.transmit(BROADCAST_C, ON_C);
        break;
      }

      lf_1 = (int) abs(254 * sin(i * 3.14 / 180));
      lf_2 = (int) abs(254 * sin(i * 3.14 / 180 + 2 * 3.14 / 3));
      lf_3 = (int) abs(254 * sin(i * 3.14 / 180 + 1 * 3.14 / 3));
      dali.transmit(lf_1_add << 1, lf_1);
      delay(5);
      dali.transmit(lf_2_add << 1, lf_2);
      delay(5);
      dali.transmit(lf_3_add << 1, lf_3);
      delay(5);
      delay(20);
    }
  }
}

void command(const char *cmdStr) {
  int cmd1;
  int cmd2;
  if (0 == strcmp(cmdStr, "sinus"))
    sinus();
  else if (0 == strcmp(cmdStr, "scan"))
    dali.scanShortAdd();                 // scan short addresses 
  else if (0 == strcmp(cmdStr, "on"))
    dali.transmit(BROADCAST_C, ON_C);    // broadcast MAX-LEVEL
  else if (0 == strcmp(cmdStr, "off"))
    dali.transmit(BROADCAST_C, OFF_C);   // broadcast OFF
  else if (0 == strcmp(cmdStr, "initialise"))
    dali.initialisation();              // initialisation
  else if (0 == strcmp(cmdStr, "randomise"))
    dali.randomise(); 
  else if (0 == strcmp(cmdStr, "assignaddr"))
    dali.assignaddr();
  else if (0 == strcmp(cmdStr, "searchall"))
    dali.searchaddr(0xFFFFFF);
  else if (0 == strcmp(cmdStr, "reset"))
    dali.busInit();
  else if (0 == strcmp(cmdStr, "terse")) 
    dali.msgMode = 0;
  else if (0 == strcmp(cmdStr, "verbose")) 
    dali.msgMode = 1;
  else if (0 == strcmp(cmdStr, "help")) 
    help();                             //help
  else if (dali.cmdCheck(cmdStr, cmd1, cmd2)) {
    if (dali.msgMode) {
      PN("Sending HEX command: %02X,%02X", cmd1, cmd2);
    }
    dali.transmit(cmd1, cmd2);  // command in binary format: (address byte, command byte)
    int response = dali.receive();
    if (dali.getResponse) {
      if (dali.msgMode)
        PN("Response: 0x%02X", response);
      else
        Serial.println(response);
    }
    else if (dali.msgMode) {
      PN("No response:%d", response);
    }
  }
}

unsigned long ledMicros = 0L;
void loop() {

  if (ledMicros && micros() - ledMicros > 200*1000L) {
    digitalWrite(LED_BUILTIN, LOW);
    ledMicros = 0L;
  }

  if (!Serial.available()) 
    return;

  digitalWrite(LED_BUILTIN, HIGH);
  ledMicros = micros();

  char ch = (char)(Serial.read());
  if (ch == '\r' || ch == '\n') {
    Serial.println("");
    command(cmdStr);
    Serial.print(dali.msgMode ? "OK> " : ":");
    memset(cmdStr, 0, sizeof(cmdStr));
    return;
  }
  cmdStr[strlen(cmdStr)] = ch;
}
// vim: set sw=2 sts=2 et:
