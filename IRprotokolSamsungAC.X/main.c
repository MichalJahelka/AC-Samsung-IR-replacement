#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// CONFIG1L
#pragma config FEXTOSC = OFF    // External Oscillator mode Selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINTOSC_1MHZ// Power-up default value for COSC bits (HFINTOSC with HFFRQ = 4 MHz and CDIV = 4:1)

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // Master Clear Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (Power up timer disabled)
#pragma config LPBOREN = OFF    // Low-power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown Out Reset Voltage selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = OFF    // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be set and cleared repeatedly (subject to the unlock sequence))
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = LFINTOSC// WDT input clock selector (WDT reference clock is the 31.0 kHz LFINTOSC)

// CONFIG4L
#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000-00FFFFh) not write-protected)
#pragma config WRT4 = OFF       // Write Protection Block 3 (Block 4 (010000-013FFFh) not write-protected)
#pragma config WRT5 = OFF       // Write Protection Block 3 (Block 5 (014000-017FFFh) not write-protected)
#pragma config WRT6 = OFF       // Write Protection Block 3 (Block 6 (018000-01BFFFh) not write-protected)
#pragma config WRT7 = OFF       // Write Protection Block 3 (Block 7 (01C000-01FFFFh) not write-protected)

// CONFIG4H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-30000Bh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config SCANE = OFF      // Scanner Enable bit (Scanner module is NOT available for use, SCANMD bit is ignored)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

// CONFIG5L
#pragma config CP = OFF         // UserNVM Program Memory Code Protection bit (UserNVM code protection disabled)
#pragma config CPD = OFF        // DataNVM Memory Code Protection bit (DataNVM code protection disabled)

// CONFIG5H

// CONFIG6L
#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR4 = OFF      // Table Read Protection Block 4 (Block 4 (010000-013FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR5 = OFF      // Table Read Protection Block 5 (Block 5 (014000-017FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR6 = OFF      // Table Read Protection Block 6 (Block 6 (018000-01BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR7 = OFF      // Table Read Protection Block 7 (Block 7 (01C000-01FFFFh) not protected from table reads executed in other blocks)

// CONFIG6H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ      16000000UL

// === Parametry ===
#define MAX_SAMPLES     512   // musí být dost velké na celý rámec

#define MAX_BLOCKS      3
#define BITS_PER_BLOCK  56


#define VystupDO        LATAbits.LA3
#define VystupTest      LATAbits.LA2
#define Activate_TX()   { VystupDO=1;VystupTest=0; }
#define Deactivate_TX() { VystupDO=0;VystupTest=1; }

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

/*
 * Bloky
 * Blok1 - Zapnutí/Vypnutí
 * Blok2 - Doplňkové informace, nemusí být, používá se jen při zapnutí a vypnutí
 * Blok3 - Data
 */

typedef union {
    uint8_t raw[7];
    struct {
        uint8_t hdr;          // 0x40 = Block 1
        uint8_t CHK:4;        // Checksum
        uint8_t unused1:4;    // Musí být 0b0100
        uint8_t unused2;      // Musí být 0xF0
        uint8_t unused3:2;    // Musí být 0b00
        uint8_t Direction:2;  // Určuje směr 00 - žádný, 01 - direct, 10 - indirect
        uint8_t unused4:4;    // Musí být 0b0000
        uint8_t unused5;      // Musí být 00
        uint8_t unused6:2;    // Musí být 0b00
        uint8_t Quiet:1;      // Aktivní Quiet
        uint8_t unused7:5;    // Musí být 0b00000
        uint8_t pwr;          // Je buďto 0x0F pro zapnutí nebo 0x03 pro vypnutí
    } data;
} SamsungAcBlock1_t;

typedef union {
    uint8_t raw[7];
    struct {
        uint8_t hdr;          // 0x80 = Block 2
        uint8_t CHK:4;        // Checksum
        uint8_t unused1:4;    // Musí být 0b0100
        uint8_t unused2;      // Musí být 0xF0
        uint8_t unused3;      // Musí být 0x00
        uint8_t unused4;      // Musí být 0x00
        uint8_t unused5;      // Musí být 0x00
        uint8_t unused6;      // Musí být 0x00 - tím odlišíme od bloku 3
    } bits;
} SamsungAcBlock2_t;

typedef union {
    uint8_t raw[7];  // 7 bajtů bloku 3

    struct {
        uint8_t block_id;     // 0x80 = Block3

        uint8_t Zone_CHK;     // Byte1 – kód pro zónu + chcecksum

        uint8_t Swing;        // Byte2 – detailní swing kód (0x7F, 0x75, 0x73, 0xF5…)

        uint8_t Clean1:1;     // Příkaz Clean
        uint8_t unused1:2;
        uint8_t nLightning:1; // Příkaz Lightning, normálně 1, když je lightning je 0
        uint8_t Turbo:3;      // Když je Turbo je 0b110
        uint8_t unused3:1;

        uint8_t temp:4;       // Kód teploty (dekóduje se tabulkou)
        uint8_t unused4:2;
        uint8_t Clean2:1;     // Příkaz Clean
        uint8_t Cycle:1;      // Aktivní Cycle

        uint8_t modefan;      // Byte5 – mód + nastavení ventilátoru

        uint8_t unused5:5;    // Má být vždy 0b01111
        uint8_t Beep:1;       // Příkaz Beep
        uint8_t Reset:1;      // Příkaz Reset
        uint8_t unused6:1;    // Má být vždy 0
    } data;
} SamsungAcBlock3_t;

volatile uint16_t durations[MAX_SAMPLES];
volatile uint8_t  levels[MAX_SAMPLES];
volatile uint16_t count = 0;

volatile uint16_t last_time = 0;
volatile uint8_t last_level = 1; // klidová úroveň = 1

volatile bool NovaData=false;
volatile uint8_t Aktivita=0;


typedef struct {
    uint8_t b1;
    uint8_t b2;
} ZoneSwingCode_t;

// pořadí: [zone][swing]
//
// zone = 0 → Zone1
// zone = 1 → Zone2
// zone = 2 → Zone3
// zone = 3 → Zone4
// zone = 4 → Zone1234
//
// swing = 0 → None
// swing = 1 → Y
// swing = 2 → X
// swing = 3 → XY
//
const ZoneSwingCode_t zone_swing_table[5][4] = {
    // Zone 1 (0x4?)
    {
        {0x47, 0x7F}, // None
        {0x40, 0xF5}, // Y
        {0x4F, 0x7D}, // X
        {0x40, 0xF3}  // XY
    },
    // Zone 2 (0xC?)
    {
        {0xCB, 0x7F}, // None
        {0xCF, 0x75}, // Y
        {0xC7, 0x7D}, // X
        {0xCF, 0x73}  // XY
    },
    // Zone 3 (0x2?)
    {
        {0x27, 0x7F}, // None
        {0x20, 0xF5}, // Y
        {0x2F, 0x7D}, // X
        {0x20, 0xF3}  // XY
    },
    // Zone 4 (0xA?)
    {
        {0xAB, 0x7F}, // None
        {0xAF, 0x75}, // Y
        {0xA7, 0x7D}, // X
        {0xAF, 0x73}  // XY
    },
    // Zone 1234 (0x9?)
    {
        {0x9B, 0x7F}, // None
        {0x9F, 0x75}, // Y
        {0x97, 0x7D}, // X
        {0x9F, 0x73}  // XY
    }
};

typedef union
{
  struct
  {
    uint8_t On:1;
    uint8_t Off:1;
    uint8_t Turbo:1;
    uint8_t Quiet:1;
    uint8_t Long:1;
    uint8_t Cycle:1;
    uint8_t Beep:1;
    uint8_t Lightning:1;
    uint8_t Clean:1;  
    uint8_t Reset:1;
  } Data;
  uint16_t Value;
} TOptions;

typedef enum
{
  Dir_None,
  Dir_Direct,
  Dir_Indirect,
} TDirection;

typedef enum
{
  Zone_1,
  Zone_2,
  Zone_3,
  Zone_4,
  Zone_1234,
} TZone;

typedef struct
{
  uint8_t X:1;
  uint8_t Y:1;
} TSwing;

typedef enum
{
  Mode_Auto=0,
  Mode_Heat,
  Mode_Cool,
  Mode_Fan,
  Mode_Dry,
} TMode;

// === ISR ===
void __interrupt() ISR(void) {
    if (INT0IF) 
    {
        uint16_t now = TMR1;
        uint16_t dt = now - last_time;
        last_time = now;

        if (count < MAX_SAMPLES) {
            durations[count] = dt;
            levels[count] = !last_level;
            count++;
        }

        // přepnout hranu
        last_level = !last_level;
        INTCONbits.INT0EDG = last_level; // pokud teď 1, čekej sestup; pokud 0, čekej náběh

        INT0IF = 0;
        Aktivita=10;
        NovaData=true;
    }
}

// === UART ===
void UART_Init(void) {
    SP1BRG = 34; // 115200 Bd @ 16 MHz
    TX1STAbits.BRGH = 1;
    BAUD1CONbits.BRG16 = 1;

    RC1STAbits.SPEN = 1;
    TX1STAbits.TXEN = 1;
}

void UART_PutChar(char c) {
    while(!TX1STAbits.TRMT);
    TX1REG = c;
}

void UART_PutString(const char *s) {
    while(*s) UART_PutChar(*s++);
}

int decodeTemp(uint8_t code) {
    for (int t = 16; t <= 30; t++) {
        if (samsung_temp_table[t - 16] == (code & 0x0F))
            return t;
    }
    return -1; // chyba
}

const char* decode_zone(uint8_t b1, uint8_t b2) {
    // rozpoznání zóny
    switch(b1 & 0xF0) {
        case 0x40: return "Zone 1";
        case 0xC0: return "Zone 2";
        case 0x20: return "Zone 3";
        case 0xA0: return "Zone 4";
        case 0x90: return "Zone 1+2+3+4";
        default:   return "Unknown zone";
    }
}

const char* decode_swing(uint8_t b1, uint8_t b2) {
    if (b2 == 0x7F || b2 == 0xFF)  return "Swing OFF";
    if (b2 == 0xF5 || b2 == 0x75) return "Swing Y";
    if (b2 == 0x7D) return "Swing X";
    if (b2 == 0xF3 || b2 == 0x73) return "Swing XY";
    return "Unknown swing";
}

const char* decode_mode(uint8_t code)
{
    switch (code & 0x0F) 
    { 
        case 0b0000: return "Auto";
        case 0b0010: return "Heat";
        case 0b1000: return "Cool";
        case 0b1100: return "Fan";
        case 0b0100: return "Dry";
        default:    return "Unknown";
    }
}

const char* decode_fan(uint8_t code)
{
    switch (code >> 4) 
    {  
        case 0b1000: return "Fan 1";
        case 0b1010: return "Fan 2";
        case 0b1001: return "Fan 3";
        case 0b1101: return "Fan 4 (Turbo)";
        default:    return "Unknown";
    }
}

void decode_options(SamsungAcBlock1_t Samsung1, SamsungAcBlock3_t Samsung2, char* out, size_t outlen)
{ 
    strcpy(out, "Options: ");

    if(Samsung2.data.Turbo == 0b110) strcat(out, "Turbo, ");
    if(Samsung2.data.Turbo == 0b011) strcat(out, "Long, ");
    if(Samsung1.data.Quiet) strcat(out, "Quiet, ");
    if(Samsung2.data.Cycle) strcat(out, "Cycle, ");
    if(Samsung1.data.Direction == 0b01) strcat(out, "Direct, ");
    if(Samsung1.data.Direction == 0b10) strcat(out, "Indirect, ");
    if(Samsung2.data.Beep) strcat(out, "Beep, ");
    if(Samsung2.data.Reset) strcat(out, "Filter reset, ");
    if(Samsung2.data.Clean1 && Samsung2.data.Clean2) strcat(out, "Clean, ");
    if(!Samsung2.data.nLightning) strcat(out, "Lightning, ");

    // --- odstranit poslední čárku + mezeru ---
    size_t len = strlen(out);
    if (len > 2) 
    {
      out[len-2] = '\n';
      out[len-1] = 0;
    }
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

// --- Příklad použití při generování rámce ---
void Update_Block_Checksum(uint8_t *b)
{
    uint8_t chk = Compute_Checksum(b);
    b[1] = (b[1] & 0xF0) | chk;  // přepiš spodní nibble
}

void decode_and_print(void) 
{
    uint8_t bitbuf[MAX_BLOCKS][BITS_PER_BLOCK/8+1] = {{0}};
    uint16_t bitcount[MAX_BLOCKS] = {0};
    uint8_t block_index = 0;
    bool in_data = false;
    uint16_t i;
    uint16_t dur;
    uint8_t lvl;
    char buf[128];
    SamsungAcBlock1_t Samsung1;
    SamsungAcBlock3_t Samsung3;
    uint8_t j,k;

    for (i=0; i+1<count && block_index < MAX_BLOCKS; i++) 
    {
        //sprintf(buf, "%04X", i);
        //UART_PutString(buf);
        dur = durations[i]/2;   // µs
        lvl  = levels[i];

        // --- čekáme na 3ms sync puls v log.1 ---
        if (lvl==0 && dur>2500 && dur<3500) 
        {
          if(in_data) block_index++;
          in_data = true;
          bitcount[block_index] = 0;
          memset(bitbuf[block_index], 0, sizeof(bitbuf[block_index]));
          continue;
        }
        
        if(!in_data) continue;

        // --- ignoruj cokoliv delší než 3ms ---
        if (dur > 3000) 
        {
          Nop();
          continue;
        }

        // --- dekódování bitů: mark + space ---
        if ((lvl==0)&&(dur>300)&&(dur<800))
        { // puls ~500 µs
            if ((i+1<count) && (levels[i+1]==1)) 
            {
                uint16_t space = durations[i+1]/2;

                if (space>300 && space<800) 
                {
                    // bit 0
                    bitbuf[block_index][bitcount[block_index]/8] <<= 1;
                    bitcount[block_index]++;
                } 
                else
                {
                    // bit 1
                    bitbuf[block_index][bitcount[block_index]/8] <<= 1;
                    bitbuf[block_index][bitcount[block_index]/8] |= 1;
                    bitcount[block_index]++;
                }
            }
        }

        // --- konec bloku ---
        if (bitcount[block_index] >= BITS_PER_BLOCK) 
        {
            block_index++;
            in_data = false;   // zpět k hledání syncu
        }
    }

    // --- výpis všech bloků ---
    for (uint8_t b=0; b<block_index; b++) 
    {
        sprintf(buf, "Block %u: Bits %u\r\n", b+1, bitcount[b]);
        
        UART_PutString(buf);

        //k=0;
        for (i=0; i<(bitcount[b]+7)/8; i++) 
        {
            sprintf(buf, "%02X ", bitbuf[b][i]);
            UART_PutString(buf);
            /*if(i==1) k+=SpocitejPocetJednicekB(bitbuf[b][i] & 0xF0);
               else if(i==2) k+=SpocitejPocetJednicekB(bitbuf[b][i] & 0x0F);
                        else k+=SpocitejPocetJednicekB(bitbuf[b][i]);*/
        }
        //sprintf(buf, "- %X:%i", Check_LUT[k&0x0F], Check_LUT[k&0x0F] == (bitbuf[b][1] & 0x0F));
        k=Compute_Checksum(bitbuf[b]);
        sprintf(buf, "- %X:%i", k, k == (bitbuf[b][1] & 0x0F));
        UART_PutString(buf);

        UART_PutString("\r\n");
    }
    
    memcpy(Samsung1.raw, bitbuf[0], 7);
    if(Samsung1.data.pwr==0x0F) UART_PutString("On\n");
                           else UART_PutString("Off\n");
    // Samsung2 nebudeme vůbec dekódovat
    if(bitbuf[1][6]==0x00) memcpy(Samsung3.raw, bitbuf[2], 7);
                      else memcpy(Samsung3.raw, bitbuf[1], 7);
    sprintf(buf,"Teplota: %u °C\n", decodeTemp(Samsung3.data.temp));
    UART_PutString(buf);
    sprintf(buf,"Mód: %s\n",decode_mode(Samsung3.data.modefan));
    UART_PutString(buf);
    sprintf(buf,"Ventilátor: %s\n", decode_fan(Samsung3.data.modefan));
    UART_PutString(buf);
    sprintf(buf,"%s\n",decode_zone(Samsung3.data.Zone_CHK,Samsung3.data.Swing));
    UART_PutString(buf);
    sprintf(buf,"%s\n",decode_swing(Samsung3.data.Zone_CHK,Samsung3.data.Swing));
    UART_PutString(buf);
    decode_options(Samsung1, Samsung3, buf, sizeof(buf));
    UART_PutString(buf);
}


// === Debug výpis surových dat ===
void debug_print_samples(void) {
    char buf[32];
    UART_PutString("Samples: ");
    for (uint16_t i=0; i<count; i++) {
        sprintf(buf, "%u:%u,", durations[i]/2, levels[i]); // převod na µs
        UART_PutString(buf);
    }
    UART_PutString("\r\n");
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

// Vytvoří paket pro vyslání dat dálkovým ovladačem
// Vrací délku paketu
// Paket musí mít alespoň 21 bytů
uint8_t CreatePaket(uint8_t *Packet, TMode Mode, uint8_t Temperature, uint8_t Fan, TOptions Options, TSwing Swing, TZone Zone, TDirection Direction)
{
  uint8_t n;
  
  // Nemůže být současně zapnutí a vypnutí
  if(Options.Data.On) Options.Data.Off=0;
  
  // Blok1
  Packet[0]=0x40;         // Hdr pro blok1
  Packet[1]=0b01000000;   // 0b0100 a Checksum
  Packet[2]=0xF0;
  switch(Direction)
  {
    case Dir_Direct:Packet[3]=0b00000100;break;
    case Dir_Indirect:Packet[3]=0b00001000;break;
    default:Packet[3]=0b00000000;break;
  }
  Packet[4]=0x00;
  if(Options.Data.Quiet) Packet[5]=0b00000100;
                    else Packet[5]=0b00000000;
  if(Options.Data.Off) Packet[6]=0x03;
                  else Packet[6]=0x0F;
  Update_Block_Checksum(&Packet[0]);
  
  // Blok2
  if(Options.Data.On || Options.Data.Off)
  {
    Packet[7]=0x80;         // Hdr pro blok2
    Packet[8]=0b01000000;   // 0b0100 a Checksum
    Packet[9]=0xF0;         // Musí být 0xF0
    Packet[10]=0x00;        // Musí být 0x00
    Packet[11]=0x00;        // Musí být 0x00
    Packet[12]=0x00;        // Musí být 0x00
    Packet[13]=0x00;        // Musí být 0x00 - tím odlišíme od bloku 3
    Update_Block_Checksum(&Packet[7]);
    n=14;
  }
  else n=7;
  
  if(Options.Data.Quiet) Options.Data.Turbo=0;
  if(Options.Data.Long) Options.Data.Turbo=0;
  // Blok3
  Packet[n]=0x80;                 // Hdr pro blok3
  Packet[n+1]=EncodeZone(Zone);
  Packet[n+2]=EncodeSwing(Swing);
  Packet[n+3]=0b10001110;
  if(Options.Data.Turbo)     Packet[n+3]|=0b01100000;
  if(Options.Data.Long)      Packet[n+3]|=0b00110000;
  if(Options.Data.Lightning) Packet[n+3]&=0b11110111;
  if(Options.Data.Clean)     Packet[n+3]|=0b00000001;
  Packet[n+4]=EncodeTemp(Temperature);
  if(Options.Data.Cycle)     Packet[n+4]|=0b10000000;
  if(Options.Data.Clean)     Packet[n+4]|=0b01000000;
  Packet[n+5]=EncodeMode(Mode);
  if(Mode==Mode_Auto) Fan=1;
  Packet[n+5]|=EncodeFan(Fan);
  if(Options.Data.Off) Packet[n+6]=0x03;
                  else Packet[n+6]=0x0F;
  if(Options.Data.Reset) Packet[n+5]|=0b01000000;
  if(Options.Data.Beep)  Packet[n+5]|=0b00100000;
  
  Update_Block_Checksum(&Packet[n]);
  
  return n+7;
}

void Transmitt0()
{
  Activate_TX();
  __delay_us(501);
  Deactivate_TX();
  __delay_us(501);
}

void Transmitt1()
{
  Activate_TX();
  __delay_us(502);
  Deactivate_TX();
  __delay_us(1502);
}

void TransmittByte(uint8_t Data)
{
  uint8_t b=8;
  while(b--)
  {
    if(Data&0x80) Transmitt1(); else Transmitt0();
    Data<<=1;
  }
}

void TransmittPacket(uint8_t *Packet, uint8_t PocetBytu)
{
  uint8_t i;
  
  CCPR1=_XTAL_FREQ/4/38500/2;   // Nastavíme frekvenci na 38.5kHz
  
  // Začátek vysílání
  Activate_TX();
  __delay_us(600);
  Deactivate_TX();
  CCPR1=_XTAL_FREQ/4/37000/2;   // Nastavíme frekvenci na 37kHz
  __delay_us(18000);
  
  // Začátek bloku 1
  Activate_TX();
  __delay_us(3050);
  Deactivate_TX();
  __delay_us(9050);
  
  // Blok
  for(i=0;i<7;i++) TransmittByte(Packet[i]);
  
  // Konec bloku
  Activate_TX();
  __delay_us(500);
  Deactivate_TX();
  __delay_us(3000);
  
  // Začátek bloku 2
  Activate_TX();
  __delay_us(3000);
  Deactivate_TX();
  __delay_us(9050);
  
  // Blok
  for(i=0;i<7;i++) TransmittByte(Packet[i+7]);
  
  // Konec bloku
  Activate_TX();
  __delay_us(500);
  Deactivate_TX();
  __delay_us(3000);
  
  if(PocetBytu>14)
  {
    // Začátek bloku 3
    Activate_TX();
    __delay_us(3000);
    Deactivate_TX();
    __delay_us(9000);

    // Blok
    for(i=0;i<7;i++) TransmittByte(Packet[i+14]);

    // Konec bloku
    Activate_TX();
    __delay_us(500);
    Deactivate_TX();
    __delay_us(3000);
  }
}

// === Main ===
void main(void) 
{
  uint8_t Paket[3*7];
  uint8_t Delka;
  TOptions Opt;
  TSwing Swing;

  // Oscilátor 16 MHz
  OSCCON1 = 0x60;
  OSCFRQ = 0x05;

  // PPS
  PPSLOCK = 0x55; PPSLOCK = 0xAA; PPSLOCKbits.PPSLOCKED = 0;
  RC6PPS = 0x09;    // TX -> RC6
  RX1PPS = 0x17;    // RX <- RC7 (není potřeba, ale pro jistotu)
  INT0PPS = 0x08;   // RB0 -> INT0
  RB3PPS = 0x05;    // CCP1 -> RB3
  RA0PPS = 0x15;    // DSM -> RA0
  MDSRCPPS = 0x03;  // RA3  -> MDSRC
  PPSLOCK = 0x55; PPSLOCK = 0xAA; PPSLOCKbits.PPSLOCKED = 1;
  
  TRISAbits.TRISA3=0;
  ANSELAbits.ANSELA3=0;
  LATAbits.LATA3=0;
  TRISAbits.TRISA2=0;
  ANSELAbits.ANSELA2=0;
  LATAbits.LATA2=1;
  TRISAbits.TRISA0=0;
  ANSELAbits.ANSELA0=0;
  LATAbits.LATA3=0;
  TRISBbits.TRISB3=0;
  ANSELBbits.ANSELB3=0;
  
  T3CON=0b00000101;             // Zapnout TMR3
  TMR3CLK=0b0001;               // Hodiny fosc (4 MHz)
  CCPTMRS=0b10101010;           // CCP bude používat TMR3, PWM - TMR4
  CCPR1=_XTAL_FREQ/4/38500/2;   // Nastavíme frekvenci na 38.5kHz
  CCP1CON=0b10000001;           // Enable, Compare mode, toggle output and clear TMR3
  
  MDCON0=0b10000000;            // Zapneme DSM
  MDCON1=0b00010001;            // Synchronně, pulsy budou neořezané a dokončené v celé délce
  MDCARH=0b00000100;            // Vstup modulace H je CCP1
  MDCARL=0b00000000;            // Není (výběr PPS, ale tam není nic)
  MDSRC=0b00000000;             // PIN vybraný MDSRCPPS
  
  
  __delay_ms(100);

  UART_Init();

  // Timer1 @ 2 MHz (0,5 µs/tick)
  T1CONbits.CKPS = 0b01; // 1:2
  T1CLK = 0x01;          // Fosc/4
  TMR1 = 0;
  T1CONbits.TMR1ON = 1;

  // INT0
  TRISBbits.TRISB0 = 1;
  ANSELBbits.ANSELB0 = 0;
  INTCONbits.INT0EDG = 0; // začneme sestupnou hranou
  last_level = 0;         // protože čekáme na sestup
  INT0IF = 0;
  INT0IE = 1;
  INTCONbits.GIE = 1;

  Opt.Value=0;
  Opt.Data.On=0;
  Opt.Data.Off=1;
  Swing.X=0;        // Není u této klimatizace funkční
  Swing.Y=0;
  //Delka=CreatePaket(Paket,Mode_Heat,24,3,Opt,Swing,Zone_1,Dir_None);
  Delka=CreatePaket(Paket,Mode_Fan,25,1,Opt,Swing,Zone_1,Dir_None);
  
  
  /*Delka=21;
  Paket[0]=0x40;
  Paket[1]=0x49;
  Paket[2]=0xF0;
  Paket[3]=0x00;
  Paket[4]=0x00;
  Paket[5]=0x00;
  Paket[6]=0x0F;
  
  Paket[7]=0x80;
  Paket[8]=0x4B;
  Paket[9]=0xF0;
  Paket[10]=0x00;
  Paket[11]=0x00;
  Paket[12]=0x00;
  Paket[13]=0x00;
  
  Paket[14]=0x80;
  Paket[15]=0x4B;
  Paket[16]=0x7F;
  Paket[17]=0x8E;
  Paket[18]=0x03;
  Paket[19]=0x82;
  Paket[20]=0x0F;*/
  
  //TransmittPacket(Paket,Delka);
  //while(1);
  
  UART_PutString("Samsung AC IR capture ready\r\n");

  while (1) 
  {
    __delay_ms(5);
    if((!Aktivita)&&(NovaData))
    {
      INT0IE = 0;
      //debug_print_samples(); // surový výpis
      decode_and_print();
      count = 0;
      NovaData=false;
      INT0IE = 1;
    }
    if(Aktivita) Aktivita--;
  }
}
