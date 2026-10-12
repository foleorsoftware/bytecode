#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <string.h>
#include <Keypad.h>
#define eeprom 512
#define ssize 255
// ============================================================
// VM MEMORY
// ============================================================
uint8_t x = 0;
uint8_t memory[ssize + 1];

uint16_t pc = 0;
uint8_t a = 0;

// 0 = opcode
// 1 = PC low
// 2 = PC high
// 3 = opcode op foutlocatie
// 4 = register A
uint8_t fault[5] = {
  0, 0, 0, 0, 0
};


// ============================================================
// ERRORS IN FLASH
// ============================================================

const char error0[] PROGMEM = "hid or output not found";
const char error1[] PROGMEM = "hardware fault";
const char error2[] PROGMEM = "error in code";
const char error3[] PROGMEM = "not enough memory";
const char error4[] PROGMEM = "io error";
const char error5[] PROGMEM = "vm panic";
const char error6[] PROGMEM = "eeprom full";

const char* const errors[] PROGMEM = {
  error0,
  error1,
  error2,
  error3,
  error4,
  error5,
  error6
};



// ============================================================
// PRINT ERROR
// ============================================================

void error(bool looping, uint8_t code) {

  digitalWrite(13, HIGH);

  char buffer[32];

  if (code < 7) {
    strcpy_P(
      buffer,
      (PGM_P)pgm_read_ptr(&errors[code])
    );

    Serial.println(buffer);
  } else {
    Serial.println("unknown error");
  }

  Serial.print("fault: ");

  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(fault[i]);

    if (i < 4) {
      Serial.print(" ");
    }
  }

  Serial.println();

  if (looping) {
    while (true) {
      
    }
  }
}


// ============================================================
// LOAD EEPROM
// ============================================================

void loadMemory() {
ask:
  Serial.println("eeprom 1 or eeprom 2");
  while(!Serial.available()){

  }
  char cmd = Serial.read();
    if (cmd == '1') {

      // Eventuele newline verwijderen
      while (Serial.available()) {
        char c = Serial.peek();

        if (c == '\r' || c == '\n') {
          Serial.read();
        } else {
          break;
        }
      }

       for (uint16_t i = ssize + 1; i < ssize + ssize + 2; i++) {
    memory[i] = EEPROM.read(i);
  }
    }

    // -------------------------
    // EXECUTION
    // -------------------------

    else if (cmd == '1' ) {

      while (Serial.available()) {
        Serial.read();
      }

      loadMemory();

      pc = 0;
      a = 0;

      

      while (true) {
         for (uint16_t i = 0; i < ssize; i++) {
    memory[i] = EEPROM.read(i);
  }
      }
    }else{
      goto ask;
    }


  Serial.println("memory loaded.");
}


// ============================================================
// EDIT MODE
// ============================================================

void edit() {

  Serial.println();
  Serial.println("EDIT MODE");
  Serial.println("read address");
  Serial.println("write address value");
  Serial.println("exit");

  while (true) {

    while (Serial.available() == 0) {
      delay(1);
    }

    String command = Serial.readStringUntil('\n');
    command.trim();

    // -------------------------
    // EXIT
    // -------------------------

    if (command == "exit") {

      Serial.println("leaving edit");

      return;
    }

    // -------------------------
    // READ
    // -------------------------

    if (command.startsWith("read ")) {

      int address = command.substring(5).toInt();

      if (address >= 0 && address < EEPROM.length()) {

        uint8_t value = EEPROM.read(address);

        Serial.print("EEPROM[");
        Serial.print(address);
        Serial.print("] = ");
        Serial.println(value);

      } else {

        Serial.println("invalid address");

      }

      continue;
    }

    // -------------------------
    // WRITE
    // -------------------------

    if (command.startsWith("write ")) {

      uint8_t space = command.indexOf(' ', 6);

      if (space == -1) {

        Serial.println("usage: write <address> <value>");
        continue;

      }

      uint16_t address = command.substring(6, space).toInt();
      uint8_t value = command.substring(space + 1).toInt();

      if (address >= 0 &&
          address < EEPROM.length() &&
          value >= 0 &&
          value <= 255) {

        EEPROM.update(address, value);

        Serial.print("EEPROM[");
        Serial.print(address);
        Serial.print("] = ");
        Serial.println(EEPROM.read(address));

      } else {

        Serial.println("invalid address or value");

      }

      continue;
    }

    Serial.println("unknown command");
  }
}


// ============================================================
// MAIN MENU
// ============================================================

void question1() {

  while (true) {

    Serial.println();
    Serial.println("VM MENU ");
    Serial.println("e = edit");
    Serial.println("x = execution");
    Serial.println("d = debug");

    // Wacht op een commando
    while (Serial.available() == 0) {
      delay(1);
    }

    char cmd = Serial.read();

    // Enter/line ending negeren
    if (cmd == '\r' || cmd == '\n') {
      continue;
    }

    // -------------------------
    // EDIT
    // -------------------------

    if (cmd == 'e' || cmd == 'E') {

      // Eventuele newline verwijderen
      while (Serial.available()) {
        char c = Serial.peek();

        if (c == '\r' || c == '\n') {
          Serial.read();
        } else {
          break;
        }
      }

      edit();
    }

    // -------------------------
    // EXECUTION
    // -------------------------

    else if (cmd == 'x' || cmd == 'X') {

      while (Serial.available()) {
        Serial.read();
      }

      loadMemory();

      pc = 0;
      a = 0;

      Serial.println("starting VM");

      while (true) {
        vm();
      }
    }

    // -------------------------
    // DEBUG
    // -------------------------

    else if (cmd == 'd' || cmd == 'D') {

      while (Serial.available()) {
        Serial.read();
      }

      loadMemory();

      pc = 0;
      a = 0;

      Serial.println("memory:");

      for (uint16_t i = 0; i < eeprom; i++) {

        if (i % 16 == 0) {
          Serial.println();
        }

        if (memory[i] < 16) {
          Serial.print("0");
        }

        Serial.print(memory[i], HEX);
        Serial.print(" ");
      }

      Serial.println();
      Serial.println("starting debugger");

      while (true) {
        vmdebug();
      }
    }

    // -------------------------
    // UNKNOWN
    // -------------------------

    else {

      Serial.println("unknown command");

      while (Serial.available()) {
        Serial.read();
      }
    }
  }
}


// ============================================================
// VM
// ============================================================

bool vm() {
uint32_t begin = micros();
  // 1024 bytes geheugen = adressen 0-1023
  pc = pc & ssize;

  uint16_t pointer = 0;

  switch (memory[pc]) {

    // --------------------------------------------------------
    // 0 = PRINT CHARACTER
    // --------------------------------------------------------

    case 0:

      Serial.print((char)a);

      break;


    // --------------------------------------------------------
    // 1 = PRINT NUMBER
    // --------------------------------------------------------

    case 1:

      Serial.print(a);

      break;


    // --------------------------------------------------------
    // 2 = INPUT
    // --------------------------------------------------------

    case 2:

      if (Serial.available()) {
        a = Serial.read();
      }

      break;


    // --------------------------------------------------------
    // 3 = LOAD
    //
    // opcode
    // high byte
    // low byte
    // --------------------------------------------------------

    case 3:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      a = memory[pointer];

      pc += 2;

      break;


    // --------------------------------------------------------
    // 4 = STORE
    //
    // opcode
    // high byte
    // low byte
    // --------------------------------------------------------

    case 4:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      memory[pointer] = a;

      pc += 2;

      break;


    // --------------------------------------------------------
    // 5 = ADD
    // --------------------------------------------------------

    case 5:

      a = a + memory[pc + 1];

      pc++;

      break;


    // --------------------------------------------------------
    // 6 = SUB
    // --------------------------------------------------------

    case 6:

      a = a - memory[pc + 1];

      pc++;

      break;


    // --------------------------------------------------------
    // 7 = INC
    // --------------------------------------------------------

    case 7:

      a++;

      break;


    // --------------------------------------------------------
    // 8 = DEC
    // --------------------------------------------------------

    case 8:

      a--;

      break;


    // --------------------------------------------------------
    // 9 = JEQ
    //
    // opcode
    // value
    // high byte
    // low byte
    // --------------------------------------------------------

    case 9:

      if (a == memory[pc + 1]) {

        pointer = ((uint16_t)memory[pc + 2] << 8)
                | memory[pc + 3];

        pc = pointer;

        // -1 zodat de pc++ onderaan
        // de juiste instructie uitvoert
        pc--;

      } else {

        pc += 3;
      }

      break;


    // --------------------------------------------------------
    // 10 = JMP
    //
    // opcode
    // high byte
    // low byte
    // --------------------------------------------------------

    case 10:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      pc = pointer;

      // pc++ gebeurt onderaan
      pc--;

      break;
       case 11:
      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      x = memory[pointer];

      pc += 2;
     break;
     case 12:
     x++
     break
     case 13:
     x--
     break;
    case 14:
     memory[x] = a;
     break;
     case 15:
     a = memory[x];
     break;
          case 255:
          goto stop;
      break;

    // --------------------------------------------------------
    // UNKNOWN OPCODE
    // --------------------------------------------------------

    default:

      fault[0] = memory[pc];

      fault[1] = pc & 0xFF;

      fault[2] = (pc >> 8) & 0xFF;

      fault[3] = memory[pc];

      fault[4] = a;

      error(false, 2);

      return false;
  }

  pc++;
stop:
Serial.println(micros() - begin);
while(1){

}
  return true;
}


// ============================================================
// DEBUG VM
// ============================================================

bool vmdebug() {

  pc = pc & ssize;

  Serial.print("PC ");
  Serial.print(pc);

  Serial.print("A ");
  Serial.print(a);

  Serial.print("OPCODE ");
  Serial.println(memory[pc]);

  uint16_t pointer = 0;

  switch (memory[pc]) {

    // PRINT CHARACTER
    case 0:

      Serial.print((char)a);

      break;


    // PRINT NUMBER
    case 1:

      Serial.print(a);

      break;


    // INPUT
    case 2:

      if (Serial.available()) {
        a = Serial.read();
      }

      break;


    // LOAD
    case 3:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      a = memory[pointer];

      pc += 2;

      break;


    // STORE
    case 4:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      memory[pointer] = a;

      pc += 2;

      break;


    // ADD
    case 5:

      a = a + memory[pc + 1];

      pc++;

      break;


    // SUB
    case 6:

      a = a - memory[pc + 1];

      pc++;

      break;


    // INC
    case 7:

      a++;

      break;


    // DEC
    case 8:

      a--;

      break;


    // JEQ
    case 9:

      if (a == memory[pc + 1]) {

        pointer = ((uint16_t)memory[pc + 2] << 8)
                | memory[pc + 3];

        pc = pointer;
        pc--;

      } else {

        pc += 3;

      }

      break;


    // JMP
    case 10:

      pointer = ((uint16_t)memory[pc + 1] << 8)
              | memory[pc + 2];

      pc = pointer;
      pc--;

      break;
      
      case 255:
          goto stop;
      break;

    // UNKNOWN
    default:

      fault[0] = memory[pc];

      fault[1] = pc & 0xFF;

      fault[2] = (pc >> 8) & 0xFF;

      fault[3] = memory[pc];

      fault[4] = a;

      error(false, 2);

      return false;
  }

  pc++;

  delay(100);
stop:
while(1){
  
}
  return true;
}


// ============================================================
// 125 KHZ CLOCK
// ============================================================




// ============================================================
// SLEEP
// ============================================================




// ============================================================
// SETUP
// ============================================================

void setup() {

 

  Serial.begin(115200);

  



  question1();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

}