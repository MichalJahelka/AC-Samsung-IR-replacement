/*
 *   Vrstva iDům pro posílání dat
 */

#include <xc.h>
#include <stdlib.h>
#include <string.h>
#include "mcc_generated_files/memory.h"
#include "Variables.h"
#include "Datum.h"

unsigned char ComputeCRC(unsigned char * CompZprava)
{
  unsigned char i,CRC;
  CRC=0;
  i=(unsigned char)(*(CompZprava+3)-1);
  while(i--)
    CRC+=*(CompZprava+i);
  return CRC;
}

void iDumProcessIntr(void)
{
  unsigned char i;
  if(TMR1IF)
  {
    TMR1+=Cas1ms;             // Každou 1ms se zvětší čas o jedničku
    TMR1IF=0;                 // TMR1=65536-fosc/4000
    if(Cas!=0xFF) Cas++;
    ByloPreruseni1=1;
    if(!RC1STAbits.SPEN && (Cas==CasKolize))
    {
      PPSLOCK = 0x55;
      PPSLOCK = 0xAA;
      PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS
      iDumTxUART();
      PPSLOCK = 0x55;
      PPSLOCK = 0xAA;
      PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
      RC1STAbits.SPEN=1;
      RC1IE=1;
      RC1STAbits.CREN=1;
      asm("MOVF _RCREG1,W,c"); // Při povolení nastaveno přerušení?; zrušit příznak
    }
    if((Cas==CasZtratyDat)&&(PrijimamData))
    {
      PrijimamData=0;
      Vysilam=0;
    }
    if(Cas==CasZnovuBuzeni)
    {
      Tx=0;
      TrisTx=0;
      PPSLOCK = 0x55;
      PPSLOCK = 0xAA;
      PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS
      iDumTxIO();
      PPSLOCK = 0x55;
      PPSLOCK = 0xAA;
      PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
      RC1STAbits.SPEN=0;
      RC1IE=0;
      RC1STAbits.CREN=0;
      CM1CON1=0x01;         // Komparátor na sestupnou hranu
      PIR2bits.C1IF=0;
      PIE2bits.C1IE=1;
      PrijimamData=0;
    }
  }
  if((PIE2bits.C1IE)&&(PIR2bits.C1IF))
  {
    PIR2bits.C1IF=0;
    // if(CM1CON1bits.INTP==Rx) - Kompilátor přeloží chybně!
    if(((CM1CON1bits.INTP)&&(Rx))||((!CM1CON1bits.INTP)&&(!Rx)))
    {
      Vysilam=0;
      if(!Rx) { TMR1=Cas1ms;CM1CON1=0x02;Cas=0; } // Komparátor na vzestupnou hranu
      else
      {
        CM1CON1=0x01;         // Komparátor na sestupnou hranu
        if((!Cas)&&(TMR1<CasScanu)&&(!Sleeping)) Cas=CasZnovuBuzeni;
        else
        {
          TMR1=Cas1ms;
          Cas=0;
          PIE2bits.C1IE=0;
          PoziceZpravy=0;
          PrijimamData=1;
        }
      }
    }
  }
  if(RC1IF)
  {
    TMR1=Cas1ms;
    PrijatyZnak=RC1REG;
    PIR2bits.C1IF=0;
    ByloPreruseni1=1;
    if(Cas>=CasZtratyDat) PoziceZpravy=0;
    Cas=CasKolize;
    if(Vysilam ||(!PoziceZpravy))
    {
      PrijimamData=1;
      ZpracovavamData=1;
    }
    if(PrijimamData && ZpracovavamData && (PoziceZpravy<MaxBufLen))
    {
      Docasna.QuickAccess[PoziceZpravy]=PrijatyZnak;
      if(PoziceZpravy==3)
      {
        if((Docasna.Data.Komu!=DevID)&&(Docasna.Data.Komu!=AllDevices))
        {
          ZpracovavamData=0;
        }
      }
      if(PoziceZpravy>2) if(PoziceZpravy==Docasna.Data.Delka-1)
      {
        i=ComputeCRC(&Docasna.QuickAccess[0]);
        if(i==Docasna.QuickAccess[PoziceZpravy])     // CRC
        {
          if(Vysilam)
          {
            ZpravaVyslana=1;
            i=Docasna.Data.Delka;
            while(i--) if(Docasna.QuickAccess[i]!=Vysilani.QuickAccess[i]) VysilaniSelhalo=1;
          }
          else
          {
            ZpravaPrijata=1;
            i=Docasna.Data.Delka;
            while(i--) Zprava.QuickAccess[i]=Docasna.QuickAccess[i];
          }
        }
      }
    }
    if(PoziceZpravy>2)
    {
      if(PoziceZpravy==Docasna.Data.Delka-1)
      {
        PoziceZpravy=0xFF;
        PrijimamData=0;
        Vysilam=0;
      }
    }
    PoziceZpravy++;
  }
}

void IncVar(unsigned int *var)
{
  if(*var<0xFFFF) (*var)++;
}

void VysliPole(void)
{
  unsigned char i,j,ErrCount,ConflictCount,Kolize=0;
  Vysilani.QuickAccess[Vysilani.Data.Delka-1U]=ComputeCRC(&Vysilani.QuickAccess[0]);
  ErrCount=eMaxTrErrors;
  ConflictCount=eMaxConflicts;
  VysliPoleZnovu:
  CLRWDT();
  while(PrijimamData || !Rx || PIR2bits.C1IF || (Cas<CasKolize))
  {
    NOP();
    if(ByloPreruseni1)
    {
      CLRWDT();
      ByloPreruseni1=0;
    }
    if(Cas>CasZnovuBuzeni) return;              // Něco drží sběrnici v nule (zkrat)
  }
  if(Cas==CasKolize)
  {
    T6CON=0b01000000;     // Vypnout, Předdělička 16
    T6HLT=0b00001000;     // One shot
    T6CLKCON=0b00000001;  // fosc/4
    T6PR=(unsigned char)rand();          // 0 - 1 ms
    T6TMR=0;
    TMR6IF=0;
    T6ON=1;               // Start
    while(!TMR6IF);
    T6ON=0;
    if(PIR2bits.C1IF || PrijimamData)
    {
      IncVar(&IDV_Conflict);
      if(--ConflictCount)
      {
        if(Kolize) while(Cas<CasZnovuBuzeni+1)
        {
          if(i!=Cas+PoziceZpravy)
          {
            i=(unsigned char)(Cas+PoziceZpravy);
            CLRWDT();
          }
        }
        goto VysliPoleZnovu;
      }
      RESET();
    }
  }
  CLRWDT();
  while((Cas==CasZnovuBuzeni-1)||(Cas==CasZnovuBuzeni));
  CLRWDT();
  if(Cas>=CasZnovuBuzeni)
  {
    GIE = 0;
    Tx=0;
    TrisTx=0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS
    iDumTxIO();
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
    GIE = 1;
    Cas=0;
    Tx=1;
    TrisTx=0;
    RC1STAbits.SPEN=0;
    PIE2bits.C1IE=1;
    while(Cas<BudiciImpuls);
    Tx=0;
    while(Cas==BudiciImpuls); // Čekání, až přerušení spadne do nuly
    while(!Rx);
    while(!Cas);
    ZpravaVyslana=0;
    if(!PrijimamData) goto ChybaVysilani;

    // Protikolizní ochrana
    // Vysílá se pomalu (1ms) ID a sleduje se
    // Pokud ID není přijato v polovině správně, došlo k chybě

    i=DevID;
    CLRWDT();
    while(Cas<CasKolize-1)
    {
      Nop();
      if(i&0x80) Tx=0; else Tx=1;
      j=Cas;
      while(TMR1<CasScanu);
      if(Rx==Tx)
      {
        Tx=0;
        IncVar(&IDV_Conflict);
        if(--ConflictCount) goto VysliPoleZnovu;
        INTCON=0;                                 // Obsluha chyby od vysílání (v tomto případě reset)
        Reset();
      }
      while(Cas==j);
      i<<=1;
    }
    Tx=0;
    j=Cas;
    while(TMR1<CasScanu);
    if(!Rx)
    {
      IncVar(&IDV_Conflict);
      if(--ConflictCount) goto VysliPoleZnovu;
      INTCON=0;                                   // Obsluha chyby od vysílání (v tomto případě reset)
      Reset();
    }
    while(Cas==j);
    while(TMR1<CasScanu);
  }
  else
  {
    //Cas=CasKolize;
    if(!RC1STAbits.SPEN) goto ChybaVysilani;
  }

  // Vysílání bajtů

  VysilaniSelhalo=0;
  Vysilam=1;
  i=0;
  while(i<Vysilani.Data.Delka)
  {
    Nop();
    if(!Vysilam) goto ChybaVysilani;
    TX1REG=Vysilani.QuickAccess[i]; // Vyšli znak, automaticky vynuluje TXIF (nejde vynulovat SW)
    NOP();                          // Chviličku čekej, aby data mohl přenést a vynulovat IF
    while(!TX1STAbits.TRMT);        // Čekej na dokončení vyslání znaku
    if((i)&&(Cas>=CasZtratyDat))    // Někde byla chyba
    {
      ZpravaVyslana=0;
      Vysilam=0;
    }
    i++;
    CLRWDT();
  }
  while(Vysilam);                  // Čekej na dokončení příjmu

  if(!ZpravaVyslana) VysilaniSelhalo=1;
  ZpravaVyslana=0;

  if(VysilaniSelhalo)   // Konflikt dvou současných vysílání
  {
    ChybaVysilani:
    IncVar(&IDV_Error);
    if(--ErrCount)
    {
      Kolize=1;
      i=RCREG1;                    // FERR=0
      while(Cas<CasZnovuBuzeni+1)
      {
        Nop();
        if(i!=Cas+PoziceZpravy)
        {
          i=(unsigned char)(Cas+PoziceZpravy);
          CLRWDT();
        }
      }
      goto VysliPoleZnovu;
    }
    INTCON=0;                             // Obsluha chyby od vysílání (v tomto případě reset)
    Reset();
  }
  IncVar(&IDV_Sent);
  return;
}

void UlozZpravu()
{
  unsigned char i=Zprava.Data.Delka;
  while(i--) TempZprava.QuickAccess[i]=Zprava.QuickAccess[i];
}

void ZalohujZpravu()
{
  unsigned char i=Zprava.Data.Delka;
  while(i--) ZalohaZpravy.QuickAccess[i]=Zprava.QuickAccess[i];
}

void ObnovZpravu()
{
  unsigned char i=ZalohaZpravy.Data.Delka;
  if(!i) return;
  while(i--) Zprava.QuickAccess[i]=ZalohaZpravy.QuickAccess[i];
  ZpravaPrijata=1;
}

void iDumWait(unsigned char iDumTime)
{
  unsigned char t=0;
  while(Cas<=iDumTime)
  {
    Nop();
    if(t!=Cas)
    {
      t=Cas;
      ClrWdt();
    }
  }
}

bit StandardSend(void)
{
  unsigned char ErrCount;
  ZalohaZpravy.Data.Delka=0;
  ErrCount=eMaxCmdRepeat;
  Opakuj:
  VysliPole();
  WaitingTime=0;
  while((Cas<CasOpakovani)||(WaitingTime)|| PrijimamData || ZpravaPrijata)
  {
    Nop();
    if((TMR2IE)&&(TMR2ON)) CLRWDT();
    if(ZpravaPrijata)
    {
      ZpravaPrijata=0;
      if((Zprava.Data.Typ==msgAccept)&&
         (Zprava.Data.Obsah.ZAnswerAccept.Action==Vysilani.Data.Typ)&&
         (Zprava.Data.Obsah.ZAnswerAccept.Value==Vysilani.QuickAccess[(unsigned char)(Vysilani.Data.Delka-1)]))
      {
        ObnovZpravu();
        IncVar(&IDV_OK);
        return 1;
      }
      if((Zprava.Data.Typ==msgBufferFull)&&
         (Zprava.Data.Odkud==Vysilani.Data.Komu))
      {
        ObnovZpravu();
        return 0;
      }
      if((Zprava.Data.Typ==msgWait)&&
         (Zprava.Data.Komu==DevID))
      {
        WaitingTime=Zprava.Data.Obsah.ZWait.TimeMs;
        ErrCount=1;
        continue;
      }
      ZalohujZpravu();
    }
  }
  if(--ErrCount) goto Opakuj;
  ObnovZpravu();
  IncVar(&IDV_Fail);
  return 0;
}

bit ConfigSend(void)
{
  unsigned char ErrCount;
  ZalohaZpravy.Data.Delka=0;
  ErrCount=eMaxCmdRepeat;
  Opakuj:
  VysliPole();
  WaitingTime=0;
  while((Cas<CasOpakovani)||(WaitingTime)|| PrijimamData || ZpravaPrijata)
  {
    Nop();
    if((TMR2IE)&&(TMR2ON)) CLRWDT();
    if(ZpravaPrijata)
    {
      ZpravaPrijata=0;
      if((Zprava.Data.Typ==msgConfigAnswer)&&
         (Zprava.Data.Odkud==Vysilani.Data.Komu))
      {
        ObnovZpravu();
        IncVar(&IDV_OK);
        return 1;
      }
      if((Zprava.Data.Typ==msgWait)&&(Zprava.Data.Komu==DevID))
      {
        WaitingTime=Zprava.Data.Obsah.ZWait.TimeMs;
        ErrCount=1;
        continue;
      }
      ZalohujZpravu();
    }
  }
  if(--ErrCount) goto Opakuj;
  ObnovZpravu();
  IncVar(&IDV_Fail);
  return 0;
}

bit WaitForAnswer(unsigned char Typ,unsigned char Odkud)
{
  unsigned char ErrCount;
  ErrCount=eMaxCmdRepeat;
  Opakuj:
  VysliPole();
  WaitingTime=0;
  while((Cas<CasOpakovani)||(WaitingTime)|| PrijimamData || ZpravaPrijata)
  {
    Nop();
    if((TMR1IE)&&(TMR1ON)) CLRWDT();
    if(ZpravaPrijata)
    {
      ZpravaPrijata=0;
      if((Zprava.Data.Typ==msgAnswer)&&(Zprava.Data.Obsah.ZAnswer.Typ=Typ)&&
         ((!Odkud)||(Zprava.Data.Odkud==Odkud)))
      {
        IncVar(&IDV_OK);
        return 1;
      }
      if((Zprava.Data.Typ==msgWait)&&
         (Zprava.Data.Komu==DevID))
      {
        WaitingTime=Zprava.Data.Obsah.ZWait.TimeMs;
        ErrCount=1;
      }
    }
  }
  if(--ErrCount) goto Opakuj;
  IncVar(&IDV_Fail);
  return 0;
}

bit WaitForConfigAnswer(unsigned char Typ,unsigned char Odkud)
{
  unsigned char ErrCount=eMaxCmdRepeat;
  while(ErrCount--)
  {
    VysliPole();
    WaitingTime=0;
    while((Cas<CasOpakovani)||(WaitingTime)|| PrijimamData || ZpravaPrijata)
    {
      Nop();
      if((TMR1IE)&&(TMR1ON)) CLRWDT();
      if(ZpravaPrijata)
      {
        ZpravaPrijata=0;
        if((Zprava.Data.Typ==msgConfigAnswer)&&(Zprava.Data.Obsah.ZAnswer.Typ=Typ)&&
           ((!Odkud)||(Zprava.Data.Odkud==Odkud)))
        {
          IncVar(&IDV_OK);
          return 1;
        }
        if((Zprava.Data.Typ==msgWait)&&
           (Zprava.Data.Komu==DevID))
        {
          WaitingTime=Zprava.Data.Obsah.ZWait.TimeMs;
          ErrCount=0;
        }
      }
    }
  }
  IncVar(&IDV_Fail);
  return 0;
}

void VysliAccept(void)
{
  Vysilani.Data.Typ=msgAccept;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=Zprava.Data.Komu;
  if(!Vysilani.Data.Odkud) Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=8;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerAccept.Action=Zprava.Data.Typ;
  Vysilani.Data.Obsah.ZAnswerAccept.Value=Zprava.QuickAccess[(unsigned char)(Zprava.Data.Delka-1)];
  VysliPole();
}

void VysliVerzi(void)
{
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Obsah.ZAnswerArray.Data[0]=D_ROK;
  Vysilani.Data.Obsah.ZAnswerArray.Data[1]=D_MESIC;
  Vysilani.Data.Obsah.ZAnswerArray.Data[2]=D_DEN;
  Vysilani.Data.Delka=8+3;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerArray.Action=cfgVersion;
  Vysilani.Data.Obsah.ZAnswerArray.Delka=3;
  VysliPole();
}

void VysliID(void)
{
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=9;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerId.Action=cfgID;
  Vysilani.Data.Obsah.ZAnswerId.DType=DevType;
  VysliPole();
}

void VysliPopis(void)
{
  unsigned char i,j;
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerArray.Action=cfgDescription;
  i=0;
  do
  {
    j=DATAEE_ReadByte((unsigned char)(EEPROM_Popis+i));
    Vysilani.Data.Obsah.ZAnswerArray.Data[i]=j;
    i++;
  } while((i<16)&&(j));
  if(j) i++;
  Vysilani.Data.Obsah.ZAnswerArray.Delka=(unsigned char)(i-1);
  Vysilani.Data.Delka=(unsigned char)(8-1+i);
  VysliPole();
}

void VysliConfigState(unsigned char ConfigState)
{
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=7;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZConfig.Action=ConfigState;
  VysliPole();
}

void VysliHWRes(unsigned char Komu,unsigned char HWCmd,unsigned char HWRes)
{
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Komu;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=9;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZHardware.Action=cfgHardware;
  Vysilani.Data.Obsah.ZHardware.Command=HWCmd;
  Vysilani.Data.Obsah.ZHardware.Data[0]=HWRes;
  VysliPole();
}

void VysliWait(unsigned char TimeMs)
{
  Vysilani.Data.Typ=msgWait;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=7;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZWait.TimeMs=TimeMs;
  VysliPole();
}

void VysliObsahEEPROM(void)
{
  unsigned char i;
  unsigned int adr;

  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Delka=10u+Zprava.Data.Obsah.ZData.Delka;
  Vysilani.Data.Obsah.ZData.Typ=cfgReadEE;
  Vysilani.Data.Obsah.ZData.Adresa=Zprava.Data.Obsah.ZData.Adresa;
  Vysilani.Data.Obsah.ZData.Delka=Zprava.Data.Obsah.ZData.Delka;
  adr=Vysilani.Data.Obsah.ZData.Adresa;
  i=Vysilani.Data.Obsah.ZData.Delka;
  while(i--) Vysilani.Data.Obsah.ZData.Data[i]=DATAEE_ReadByte(adr+i);
  VysliPole();
}

void ZapisEEPROMData(void)
{
  unsigned char i=TempZprava.Data.Obsah.ZData.Delka;
  unsigned int a=TempZprava.Data.Obsah.ZData.Adresa;

  while(i--)
  {
    CLRWDT();
    DATAEE_WriteByte(a+i,TempZprava.Data.Obsah.ZData.Data[i]);
    CLRWDT();
    while(WR);
  }
  CLRWDT();

  while(Cas<CasZnovuBuzeni+1);

  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=TempZprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=7;
  Vysilani.Data.UserID=TempZprava.Data.UserID;
  Vysilani.Data.Obsah.ZConfig.Action=cfgOK;
  VysliPole();

  if(!a) ReloadEEPROM();
}

bit VysliByte(unsigned char Komu,unsigned char ByteID,unsigned char Value)
{
  Vysilani.Data.Typ=msgByte;
  Vysilani.Data.Komu=Komu;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=8;
  Vysilani.Data.UserID=0;
  Vysilani.Data.Obsah.ZByte.ByteID=ByteID;
  Vysilani.Data.Obsah.ZByte.Value=Value;
  return StandardSend();
}

void VysliChybu(unsigned char Komu, unsigned char ErrorID)
{
  Vysilani.Data.Typ=msgError;
  Vysilani.Data.Komu=Komu;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=8;
  Vysilani.Data.UserID=0;
  Vysilani.Data.Obsah.ZError.Error=ErrorID;
  StandardSend();
}

void VysliStavLinky(unsigned char i)
{
  Vysilani.Data.Typ=msgAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=8;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerByte.Action=msgLine;
  Vysilani.Data.Obsah.ZAnswerByte.Value=i;
  VysliPole();
}

bit ZjistiStavLinky(void)
{
  Vysilani.Data.Typ=msgLine;
  Vysilani.Data.Komu=AllDevices;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=6;
  Vysilani.Data.UserID=0;
  return WaitForAnswer(msgLine,0);
}

void SendIDTime(unsigned char Odkud)
{
  unsigned int temp16=(unsigned int)(rand());
  unsigned char i;
  if(Zprava.Data.Obsah.ELong.Value>5000) Zprava.Data.Obsah.ELong.Value=5000;
  temp16%=Zprava.Data.Obsah.ELong.Value;
  CasDelay=temp16;
  while(CasDelay)
  {
    Nop();
    if(CasDelay!=temp16)
    {
      temp16=CasDelay;
      CLRWDT();
    }
  }
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=13;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZAnswerId2.Action=cfgID;
  Vysilani.Data.Obsah.ZAnswerId2.DType=DevType;
  i=4;
  while(i--) Vysilani.Data.Obsah.ZAnswerId2.RandID[i]=RandID[i]=LastRand=(unsigned int)(rand());
  VysliPole();
  CasDelay=100;
  while(CasDelay)
  {
    Nop();
    if(CasDelay!=temp16)
    {
      temp16=CasDelay;
      CLRWDT();
    }
  }
  ZpravaPrijata=0;
}

void AnswerVar(unsigned int var)
{
  Vysilani.Data.Typ=msgConfigAnswer;
  Vysilani.Data.Komu=Zprava.Data.Odkud;
  Vysilani.Data.Odkud=DevID;
  Vysilani.Data.Delka=10;
  Vysilani.Data.UserID=Zprava.Data.UserID;
  Vysilani.Data.Obsah.ZiDumVar.Action=cfgReadiDumVar;
  Vysilani.Data.Obsah.ZiDumVar.VarNo=Zprava.Data.Obsah.ZiDumVar.VarNo;
  Vysilani.Data.Obsah.ZiDumVar.Var=var;
  VysliPole();
}

void ReadiDumVar(void)
{
  unsigned char cmd=Zprava.Data.Obsah.ZiDumVar.VarNo;
  switch(cmd)
  {
    case idvOK:AnswerVar(IDV_OK);return;
    case idvSent:AnswerVar(IDV_Sent);return;
    case idvFail:AnswerVar(IDV_Fail);return;
    case idvConflict:AnswerVar(IDV_Conflict);return;
    case idvError:AnswerVar(IDV_Error);return;
    case idvReset:AnswerVar(IDV_Reset);return;
  }
}

void WriteiDumVar(void)
{
  unsigned char cmd=Zprava.Data.Obsah.ZiDumVar.VarNo;
  unsigned int var=Zprava.Data.Obsah.ZiDumVar.Var;
  switch(cmd)
  {
    case idvOK:IDV_OK=var;break;
    case idvSent:IDV_Sent=var;break;
    case idvFail:IDV_Fail=var;break;
    case idvConflict:IDV_Conflict=var;break;
    case idvError:IDV_Error=var;break;
    case idvReset:IDV_Reset=var;break;
  }
  VysliConfigState(cfgWriteiDumVar);
}

void DelayMs(unsigned int CasMs)
{
  unsigned int temp16;
  CasDelay=CasMs;
  while(CasDelay)
  {
    Nop();
    if(CasDelay!=temp16)
    {
      temp16=CasDelay;
      ClrWdt();
    }
  }
}

