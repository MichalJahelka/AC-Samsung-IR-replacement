/*
 *   Vrstva pro vysílání příkazu emulujícího IR ovladač klimatizace Samsung
 */

#include <xc.h>
#include "SamsungIR.h"
#include "Variables.h"

const uint8_t samsung_temp_table[15] = {
    0x00, // 16 °C
    0x08, // 17 °C
    0x04, // 18 °C
    0x0C, // 19 °C
    0x02, // 20 °C
    0x0A, // 21 °C
    0x06, // 22 °C
    0x0E, // 23 °C
    0x01, // 24 °C
    0x09, // 25 °C
    0x05, // 26 °C
    0x0D, // 27 °C
    0x03, // 28 °C
    0x0B, // 29 °C
    0x07  // 30 °C
};

const uint8_t Check_LUT[16] =
{
  0xF,0x7,0xB,0x3,0xD,0x5,0x9,0x1,0xE,0x6,0xA,0x2,0xC,0x4,0x8,0x0
};

unsigned int SamsungIRData[400];   // Časy dat v mikrosekundách do další změny
unsigned int SamsungIRLen;         // Délka dat pro vysílání
unsigned int SamsungIRPos;         // Pozice dat pro vysílání
unsigned char SamsungPaket[21];    // Zpráva odesílaná do klimatizace

// Realizace samotného vysílání na základě časů v SamsungIRData
void TransmittBlockInterrupt()
{
  if((TMR5IF)&&(TMR5ON))
  {
    TMR5IF=0;
    if(SamsungIRPos<SamsungIRLen)
    {
      IRControl=!IRControl;
      TMR5ON=0;
      TMR5-=SamsungIRData[SamsungIRPos++]-6;    // Odečíst hodnotu podle frekvence krystalu, aby časy seděly přesně
      TMR5ON=1;
    }
    else
    {
      TMR5ON=0;
    }
  }
}

uint8_t EncodeZone(TZone Zone)
{
  switch(Zone)
  {
    case Zone_1:return 0x40;
    case Zone_2:return 0xC0;
    case Zone_3:return 0x20;
    case Zone_4:return 0xA0;
    case Zone_1234:return 0x90;
    default:return 0x00;
  }
  return 0x00;
}

uint8_t EncodeSwing(TSwing Swing)
{
  if((Swing.X)&&(!Swing.Y)) return 0x7D;
  if((!Swing.X)&&(Swing.Y)) return 0x75;
  if((Swing.X)&&(Swing.Y)) return 0x73;
  return 0x7F;
}

uint8_t EncodeTemp(uint8_t Temperature)
{
  if(Temperature<16) Temperature=16;
  if(Temperature>30) Temperature=30;
  Temperature-=16;
  return samsung_temp_table[Temperature];
}

uint8_t EncodeMode(TMode Mode)
{
  switch(Mode)
  {
    case Mode_Auto:return 0b0000;
    case Mode_Heat:return 0b0010;
    case Mode_Cool:return 0b1000;
    case Mode_Fan: return 0b1100;
    case Mode_Dry: return 0b0100;
  }
  return 0;
}

uint8_t EncodeFan(uint8_t Fan)
{
  if(Fan<1) Fan=1;
  if(Fan>4) Fan=4;
  switch(Fan)
  {
    case 1:return 0b10000000;
    case 2:return 0b10100000;
    case 3:return 0b10010000;
    case 4:return 0b11010000;
  }
  return 0b10000000;
}

uint8_t SpocitejPocetJednicekB(uint8_t Cislo)
{
  uint8_t Res=0;
  
  while(Cislo)
  {
    if(Cislo & 0x01) Res++;
    Cislo>>=1;
  }
  return Res;
}

// Vypočítá 4bitový CHK pro blok (stejné počítání jako při dekódování)
uint8_t Compute_Checksum(const uint8_t *b)
{
  uint8_t k = 0;
  k += SpocitejPocetJednicekB(b[0]);
  k += SpocitejPocetJednicekB(b[1] & 0xF0);  // horní nibble
  k += SpocitejPocetJednicekB(b[2] & 0x0F);  // dolní nibble
  k += SpocitejPocetJednicekB(b[3]);
  k += SpocitejPocetJednicekB(b[4]);
  k += SpocitejPocetJednicekB(b[5]);
  k += SpocitejPocetJednicekB(b[6]);

  return Check_LUT[k & 0x0F];
}

void Update_Block_Checksum(uint8_t *b)
{
    uint8_t chk = Compute_Checksum(b);
    b[1] = (b[1] & 0xF0) | chk;  // přepiš spodní nibble
}

// Vytvoří paket pro vyslání dat dálkovým ovladačem
// Vrací délku paketu
// Paket musí mít alespoň 21 bytů
uint8_t CreatePaket(uint8_t *Paket, TMode Mode, uint8_t Temperature, uint8_t Fan, TOptions Options, TSwing Swing, TZone Zone, TDirection Direction)
{
  uint8_t n;
  
  // Nemůže být současně zapnutí a vypnutí
  if(Options.Data.On) Options.Data.Off=0;
  
  // Blok1
  Paket[0]=0x40;         // Hdr pro blok1
  Paket[1]=0b01000000;   // 0b0100 a Checksum
  Paket[2]=0xF0;
  switch(Direction)
  {
    case Dir_Direct:Paket[3]=0b00000100;break;
    case Dir_Indirect:Paket[3]=0b00001000;break;
    default:Paket[3]=0b00000000;break;
  }
  Paket[4]=0x00;
  if(Options.Data.Quiet) Paket[5]=0b00000100;
                    else Paket[5]=0b00000000;
  if(Options.Data.Off) Paket[6]=0x03;
                  else Paket[6]=0x0F;
  Update_Block_Checksum(&Paket[0]);
  
  // Blok2
  if(Options.Data.On || Options.Data.Off)
  {
    Paket[7]=0x80;         // Hdr pro blok2
    Paket[8]=0b01000000;   // 0b0100 a Checksum
    Paket[9]=0xF0;         // Musí být 0xF0
    Paket[10]=0x00;        // Musí být 0x00
    Paket[11]=0x00;        // Musí být 0x00
    Paket[12]=0x00;        // Musí být 0x00
    Paket[13]=0x00;        // Musí být 0x00 - tím odlišíme od bloku 3
    Update_Block_Checksum(&Paket[7]);
    n=14;
  }
  else n=7;
  
  if(Options.Data.Quiet) Options.Data.Turbo=0;
  if(Options.Data.Long) Options.Data.Turbo=0;
  // Blok3
  Paket[n]=0x80;                 // Hdr pro blok3
  Paket[n+1]=EncodeZone(Zone);
  Paket[n+2]=EncodeSwing(Swing);
  if(Options.Data.Off) Paket[n+2]|=0x80;
  Paket[n+3]=0b10001110;
  if(Options.Data.Turbo)     Paket[n+3]|=0b01100000;
  if(Options.Data.Long)      Paket[n+3]|=0b00110000;
  if(Options.Data.Lightning) Paket[n+3]&=0b11110111;
  if(Options.Data.Clean)     Paket[n+3]|=0b00000001;
  Paket[n+4]=EncodeTemp(Temperature);
  if(Options.Data.Cycle)     Paket[n+4]|=0b10000000;
  if(Options.Data.Clean)     Paket[n+4]|=0b01000000;
  Paket[n+5]=EncodeMode(Mode);
  if(Mode==Mode_Auto) Fan=1;
  Paket[n+5]|=EncodeFan(Fan);
  if(Options.Data.Off) Paket[n+6]=0x03;
                  else Paket[n+6]=0x0F;
  if(Options.Data.Reset) Paket[n+5]|=0b01000000;
  if(Options.Data.Beep)  Paket[n+5]|=0b00100000;
  
  Update_Block_Checksum(&Paket[n]);
  
  return n+7;
}

// Připraví vysílání IR 1
void Transmitt0()
{
  SamsungIRData[SamsungIRLen++]=500;
  SamsungIRData[SamsungIRLen++]=500;
}

// Připraví vysílání IR 0
void Transmitt1()
{
  SamsungIRData[SamsungIRLen++]=500;
  SamsungIRData[SamsungIRLen++]=1500;
}

// Připraví vysílání IR 8 bitů
void TransmittByte(uint8_t Data)
{
  uint8_t b=8;
  while(b--)
  {
    if(Data&0x80) Transmitt1(); else Transmitt0();
    Data<<=1;
  }
}

// Připraví vysílání IR jeden blok (56 bitů)
void TransmittBlock(uint8_t *Packet)
{
  unsigned char i;
  
  // Začátek bloku 1
  SamsungIRData[SamsungIRLen++]=3000;
  SamsungIRData[SamsungIRLen++]=9000;
  
  // Blok
  for(i=0;i<7;i++) TransmittByte(Packet[i]);
  
  // Konec bloku
  SamsungIRData[SamsungIRLen++]=500;
  SamsungIRData[SamsungIRLen++]=3000;
}

// Připraví vysílání IR celého paketu a vyšle ho
void TransmittPacket(uint8_t *Packet, uint8_t PocetBytu)
{
  SamsungIRLen=0;
  SamsungIRData[SamsungIRLen++]=1000;   // Aby se mi začástek vysílání nezdržel v přerušení, pošlu ho o 1000 us později
  
  // Začátek vysílání
  SamsungIRData[SamsungIRLen++]=600;
  SamsungIRData[SamsungIRLen++]=18000;

  TransmittBlock(Packet);
  TransmittBlock(Packet+7);
  
  if(PocetBytu>14)
  {
    TransmittBlock(Packet+14);
  }
  
  T5CON=0b00100000;             // Prescaler 4, RD/WR 8 bit, off
  T5GCON=0;                     // No gating
  TMR5CLK=0b0001;               // fosc/4
  SamsungIRPos=1;
  IRControl=0;
  TMR5=-SamsungIRData[0];
  TMR5ON=1;
}

