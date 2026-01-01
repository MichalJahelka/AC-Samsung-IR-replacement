/*
 * Řízení klimatizace Samsung - simulace dálkovéhbo ovládání
 * Výstup je napojen přímo na čidlo, není nutný modulovaný signál
 * Kromě toho má obvod pro zjištění stavu a spínač
 */

#include <xc.h>
#include "mcc_generated_files/memory.h"
#include "Variables.h"
#include "iDum.h"
#include "../../Extended.h"
#include "SamsungIR.h"

void __interrupt() Preruseni(void)
{
  iDumProcessIntr();
  TransmittBlockInterrupt();
}

void __interrupt(low_priority) NizkePreruseni(void)
{
  // Systémový časovač 1 ms
  if(TMR2IF)
  {
    TMR2IF=0;
    ByloPreruseni2=1;
    
    if(CasZakmitu) CasZakmitu--;
    if(CasDelay) CasDelay--;
  }
  if(PIR0bits.IOCIF)
  {
    IOCCF=0;
    PIR0bits.IOCIF=0;
  }
}

void ZjistiVstupy()
{
  Vstup=0;
  if(!Active) Vstup|=0x01;
  if(Vstup!=StaryVstup)
  {
    BylaZmenaVstupu=1;
    StaryVstup=Vstup;
    CasZakmitu=DATAEE_ReadByte(EEPROM_CasZakmitu);
  }
}

void main(void)
{
  unsigned char i,j;
  TOptions *Opt;
  TSwing *Swing;
  
  // NOSC HFINTOSC; NDIV 1
  OSCCON1 = 0x60;
  // CSWHOLD may proceed; SOSCPWR Low power; 
  OSCCON3 = 0x00;
  // MFOEN disabled; LFOEN disabled; ADOEN disabled; SOSCEN disabled; EXTOEN disabled; HFOEN disabled; 
  OSCEN = 0x00;
  // HFFRQ 16MHz; 
  OSCFRQ = 0x05;
  // TUN 0; 
  OSCTUNE = 0x00;
  
  LATA=0b00000000;
  LATB=0b00000000;
  LATC=0b00000000;
  
  TRISA=0b00000000;
  TRISB=0b11001000;
  TRISC=0b00000001;
  
  ANSELA=0b00000000;
  ANSELB=0b00000000;
  ANSELC=0b00001000;  
  
  WPUA=0b00000000;
  WPUB=0b11000000;        // Pull-Up na ICSP
  WPUC=0b00000000;
  
  ODCONA = 0x00;
  ODCONB = 0x00;
  ODCONC = 0x00;
  
  IOCCP=0b00000001;
  IOCCN=0b00000001;
  
  // D/A převodník pro komparátor
  DAC1CON0=0b10000000;  // Zapnut, bez výstupu, AVDD a AVSS
  DAC1CON1=16;          // Polovina (2.5V)
  
  // Komparátor
  CM1CON0=0b11010000; // Zapnut, inverted, nemá hysterezi, async
  CM1CON1=0b00000000; // Nepovolit přerušení
  CM1NCH=0b000000010; // NCH = Cin2-
  CM1PCH=0b000000101; // PCH = DACOUT
  
  if(!PCON0bits.nPOR)
  {
    IDV_OK=0;
    IDV_Sent=0;
    IDV_Fail=0;
    IDV_Error=0;
    IDV_Conflict=0;
    IDV_Reset=0;
    PCON0bits.nPOR=1;
  }
  IDV_Reset++;
  
  ReloadEEPROM();
  
  CasZakmitu=0;
  
  GIE = 0;
  PPSLOCK = 0x55;
  PPSLOCK = 0xAA;
  PPSLOCKbits.PPSLOCKED = 0x00;         // unlock PPS
  
  RC7PPS = 0x0D;                        // C1OUT -> RC7
  RX1PPSbits.RXPPS=0b10111;             // RC7   -> RX1
  iDumTxIO();
    
  PPSLOCK = 0x55;
  PPSLOCK = 0xAA;
  PPSLOCKbits.PPSLOCKED = 0x01;         // lock PPS
  
  BAUDCON1=0b01010000;           // Tx inverted
  SPBRG1=CrystalFreq/16/19200-1;
  RCSTA1=0b00000000;             // Serial port disabled
  TXSTA1=0b00100110;             // TX enable, BRGH
  i=RC1REG;                      // Při povolení nastaveno přerušení; zrušit příznak
  
  TMR1=Cas1ms;
  T1GCON=0;
  TMR1CLK=0b00000001;           // fosc/4
  T1CON=0b00000001;             // Zapnout TMR1
  
  TMR2=0;
  T2HLT=0b00000000;             // Free running, software control
  T2CLKCON=0b00000001;          // fosc/4
  T2CON=0b11000000;             // Enable TMR2, prescaler 16, postscaler 1
  PR2=CrystalFreq/4/16/1/1000;  // 1 ms
  
  TMR3CLK=0b0100;               // LFINTOSC (pro náhodné číslo)
  T3CON=0b00000001;             // TMR3ON
  
  PIR0=0;
  PIR1=0;
  PIR3=0;
  PIR4=0;
  PIR5=0;
  PIR6=0;
  
  PIE0=0b00000000;
  PIE1=0b00000000;
  PIE2=0b00000001;      // Komparátor CMP1
  PIE3=0b00000000;
  PIE4=0b00010011;      // TMR5IE,TMR2IE,TMR1IE
  PIE5=0b00000000;
  PIE6=0b00000000;
  PIE7=0b00000000;
  
  IPR0=0b00000000;
  IPR1=0b00000000;
  IPR2=0b00000001;      // CMP1 (iDum) High priority
  IPR3=0b00110000;      // USART1 (iDum) High priority
  IPR4=0b00010001;      // TMR5 (Samsung transmitt), TMR1 (iDum) High priority
  CM1CON1=0x01;         // Komparátor na sestupnou hranu
  
  INTCON=0b11100000;    // GIEH, GIEL, IPEN
  
  DelayMs(100);
  
  LastRand+=TMR3;               // Inicializace náhodného čísla využívá toho, že vnitřní LF oscilátor není stejně stabilní, jako krystalový
  srand(LastRand);
  TMR3CLK=0b0001;               // fosc/4
  T3CON=0b00000000;             // TMR3OFF
  
  ZjistiVstupy();
  
  while(1)
  {
    if((ByloPreruseni1)&&(ByloPreruseni2)) 
    {
      ByloPreruseni1=0;
      ByloPreruseni2=0;
      ClrWdt();
      i=DevID;
      if(i!=EECheckSum) ReloadEEPROM();
    }
    
    ZjistiVstupy();
    if((BylaZmenaVstupu)&&(!CasZakmitu))
    {
      i=Vstup;
      BylaZmenaVstupu=0;
      VysliByte(0x01,rksStatus,i);
    }
    
    #if(Build==Release)
    if((Cas>=CasUsnuti)&&(!PrijimamData)&&(!ZpravaPrijata)&&(!TMR5ON)&&(!BylaZmenaVstupu))
    {
      IOCCF=0;
      PIR0bits.IOCIF=0;
      PIE0bits.IOCIE=1;
      Sleeping=1;
      SLEEP();
      NOP();
      NOP();
      Sleeping=0;
      GIE=0;
      if(Cas>CasUsnuti) Cas=CasUsnuti;
      GIE=1;
      PIE0bits.IOCIE=0;
      CLRWDT();
      i=DevID;
      if(EECheckSum!=i) ReloadEEPROM();
      LastRand=(unsigned int)rand();
    }
    #endif
    
    if(ZpravaPrijata)
    {
      ZpravaPrijata=0;
      switch(Zprava.Data.Typ)
      {
        case msgByte:
        {
          if(Zprava.Data.Obsah.ZByte.ByteID==rksPower)
          {
            OnOff=Zprava.Data.Obsah.ZByte.Value;
            VysliAccept();
            break;
          }
          break;
        }
        case msgArray:
        {
          if(Zprava.Data.Obsah.ZArray.Data[0]==rksCommand)
          {
            Opt=(TOptions *)&Zprava.Data.Obsah.ZArray.Data[4];
            Swing=(TSwing *)&Zprava.Data.Obsah.ZArray.Data[6];
            i=CreatePaket(SamsungPaket,Zprava.Data.Obsah.ZArray.Data[1],    // Mode
                                       Zprava.Data.Obsah.ZArray.Data[2],    // Temperature
                                       Zprava.Data.Obsah.ZArray.Data[3],    // Fan
                                       *Opt,                                // Options
                                       *Swing,                              // Swing
                                       Zprava.Data.Obsah.ZArray.Data[7],    // Zone
                                       Zprava.Data.Obsah.ZArray.Data[8]);   // Direction
            VysliAccept();
            TransmittPacket(SamsungPaket,i);
            break;
          }
          break;
        }
        case msgQuestion:
        {
          if(Zprava.Data.Obsah.ZQuestion.QuestionType==quStatus)
          {
            Vysilani.Data.Typ=msgAnswer;
            Vysilani.Data.Komu=Zprava.Data.Odkud;
            Vysilani.Data.Odkud=DevID;
            Vysilani.Data.Delka=8;
            Vysilani.Data.UserID=Zprava.Data.UserID;
            Vysilani.Data.Obsah.ZAnswerByte.Action=quStatus;
            Vysilani.Data.Obsah.ZAnswerByte.Value=Vstup;
            VysliPole();
            break;
          }
          break;
        }
        case msgExtended: // Globální příkazy
        {
          if(Zprava.Data.Obsah.Extended.EID==extGetIDTime)
          {
            SendIDTime(Zprava.Data.Odkud);
            break;
          }
          break;
        }
        case msgConfig:
        {
          if((Zprava.Data.Komu!=DevID)&&(Zprava.Data.Komu)) break;
          switch(Zprava.Data.Obsah.ZConfigInit.Action)
          {
            case cfgInitConfig:
            {
              if((Zprava.Data.Obsah.ZConfigInit.Magic1==cfgMagic1)&&
                 (Zprava.Data.Obsah.ZConfigInit.Magic2==cfgMagic2)&&
                 (Zprava.Data.Komu))
              {
                Configuring=cfgMagic1;
                VysliConfigState(cfgOK);
              } else
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
              }
              break;
            }
            case cfgStopConfig:
            {
              Configuring=0;
              if(Zprava.Data.Komu) VysliConfigState(cfgOK);
              break;
            }
            case cfgReadEE:
            {
              if((Configuring!=cfgMagic1)||
                 (Zprava.Data.Obsah.ZData.Adresa>=_EEPROMSIZE)||
                 (Zprava.Data.Obsah.ZData.Delka>MaxBufLen)||
                 (Zprava.Data.Obsah.ZData.Adresa+Zprava.Data.Obsah.ZData.Delka>_EEPROMSIZE)||
                 (!Zprava.Data.Komu))
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              VysliObsahEEPROM();
              break;
            }
            case cfgWriteEE:
            {
              if((Configuring!=cfgMagic1)||
                 (Zprava.Data.Obsah.ZData.Adresa>=_EEPROMSIZE)||
                 (Zprava.Data.Obsah.ZData.Delka>MaxBufLen)||
                 (Zprava.Data.Obsah.ZData.Adresa+Zprava.Data.Obsah.ZData.Delka>_EEPROMSIZE)||
                 (!Zprava.Data.Komu))
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              UlozZpravu();
              VysliWait((unsigned char)(10*Zprava.Data.Obsah.ZData.Delka+10));
              ZapisEEPROMData();
              break;
            }
            case cfgGetID:    // Možno i bez inicializace konfigurace
            {
              VysliID();
              break;
            }
            case cfgGetVersion:
            {
              VysliVerzi();
              break;
            }
            case cfgReset:
            {
              if(Configuring!=cfgMagic1)
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              if(Zprava.Data.Komu) VysliConfigState(cfgOK);
              Reset();
            }
            case cfgChangeID:
            {
              if(!AccessIO)
              {
                i=Zprava.Data.Obsah.ZConfig.Data[0];
                VysliConfigState(cfgOK);
                DevID=i;
                DATAEE_WriteByte(EEPROM_DevID,DevID);
              }
              break;
            }
            case cfgRandChangeID:
            {
              // Pokud nebylo generováno RandID, konec
              i=4;j=4;
              while(i--) if(!RandID[j]) j--;
              if(!j) break;
              // Pokud se nerovnají RandID, konec
              i=4;
              while(i--) if(RandID[i]!=Zprava.Data.Obsah.ZConfig.Data[i]) j=0;
              if(!j) break;
              // Zapsat do EEPROM nové DevID
              i=Zprava.Data.Obsah.ZConfig.Data[4];
              VysliConfigState(cfgOK);
              DevID=i;
              DATAEE_WriteByte(EEPROM_DevID,DevID);
              break;
            }
            case cfgDescription:
            {
              if(Zprava.Data.Komu) VysliPopis();
              break;
            }
            /*case cfgHardware:
            {
              if(Configuring!=cfgMagic1)
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              HardwareAction();
              break;
            }*/
            case cfgReadiDumVar:
            {
              if(Configuring!=cfgMagic1)
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              ReadiDumVar();
              break;
            }
            case cfgWriteiDumVar:
            {
              if(Configuring!=cfgMagic1)
              {
                Configuring=0;
                if(Zprava.Data.Komu) VysliConfigState(cfgFailed);
                break;
              }
              WriteiDumVar();
              break;
            }
            default:
            {
              Configuring=0;
              VysliConfigState(cfgNotSupported);
              break;
            }
          }
          break;
        }
      }
    }
  }
}
