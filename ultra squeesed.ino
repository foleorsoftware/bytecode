
#define rsize 1
uint8_t ram;

uint8_t a = 0;
// ============================================================
// VM MEMORY
// ============================================================




// 0 = opcode
// 1 = PC low
// 2 = PC high
// 3 = opcode op foutlocatie
// 4 = register A



// ============================================================
// ERRORS IN FLASH
// ============================================================





// ============================================================
// PRINT ERROR
// ============================================================





// ============================================================
// EDIT MODE
// ============================================================






      


// ============================================================
// VM
// ============================================================

bool vm(uint8_t opcode, uint8_t operand) {

 
  

 

  switch (opcode) {
    case 1:

      lcdNumber(a);

      break;
    case 2:

      
        a = operand;
      

      break;
    case 3:

   

      a = ram;

      

      break;
    case 4:

      

      ram = a ;

      

      break;
    case 5:

      a = a + operand;

      

      break;
    case 6:

      a = a - operand;

      

      break;
    case 7:

      a++;

      break;
    case 8:

      a--;

      break;
    default:
      return false;
  }

 
stop:


  return true;
}
void setup() {
lcdInit();
 

 
}


// ============================================================
// LOOP
// ============================================================

void loop() {

uint8_t op = ui();

uint8_t ope = ui();
vm(op,ope);
}
uint8_t ui(){
  uint8_t x = 0;
while(!PINB & 0b00000011 == 0b00000011){
lcdNumber(x);
if(PINB & 0b00000001){
  x++;
}
if(PINB & 0b00000010){
  x--;
}
}
return x;
}
#include <avr/io.h>
#include <util/delay.h>

#define SDA PB0
#define SCL PB1
#define LCD 0x27

void i2c(uint8_t b) {
  for (uint8_t i=0;i<8;i++) {
    if (b&128) PORTB|=1<<SDA;
    else PORTB&=~(1<<SDA);
    PORTB|=1<<SCL;
    PORTB&=~(1<<SCL);
    b<<=1;
  }
  DDRB&=~(1<<SDA);
  PORTB|=1<<SCL;
  PORTB&=~(1<<SCL);
  DDRB|=1<<SDA;
}

void lcd4(uint8_t x) {
  i2c(x|4);
  i2c(x);
}

void lcdCmd(uint8_t x) {
  i2c(LCD<<1);
  lcd4(x&0xF0);
  lcd4(x<<4);
}

void lcdChar(uint8_t x) {
  i2c((LCD<<1));
  lcd4((x&0xF0)|1);
  lcd4((x<<4)|1);
}

void lcdInit() {
  DDRB|=(1<<SDA)|(1<<SCL);
  PORTB|=(1<<SDA)|(1<<SCL);

  _delay_ms(50);

  lcd4(0x30);
  _delay_ms(5);
  lcd4(0x30);
  _delay_us(150);
  lcd4(0x30);
  lcd4(0x20);

  lcdCmd(0x28);
  lcdCmd(0x0C);
  lcdCmd(0x06);
  lcdCmd(0x01);
  _delay_ms(2);
}

void lcdNumber(uint8_t n) {
  if(n>=100) lcdChar('0'+n/100);
  if(n>=10)  lcdChar('0'+(n/10)%10);
  lcdChar('0'+n%10);
}