/*
 * Definice a proměnné pro řízení klimatizace Samsung
 */

#include <xc.h>
#include "Variables.h"
#include "../../Interkom/Interkom.h"
#include "mcc_generated_files/memory.h"

__EEPROM_DATA(DevType,            // DevID
              50,                 // Čas zákmitů
              0,
              0,
              0,
              0,
              0,
              0);

// Popis
__EEPROM_DATA('K','l','i','m','a','t','i','a');
__EEPROM_DATA('c','e',' ','S','a','m','s','u');
__EEPROM_DATA('n','g',0x00,0x00,0x00,0x00,0x00,0x00);

// Proměnné pro systém iDům
TZprava Docasna,Zprava,Vysilani,TempZprava,ZalohaZpravy;
__near unsigned char Cas=0xFF,EECheckSum;
__near unsigned char PoziceZpravy,PrijatyZnak,DevID,Configuring,WaitingTime;
__near bit PrijimamData,ZpracovavamData,ZpravaPrijata,VysilaniSelhalo,Vysilam;
__near bit ZpravaVyslana,Sleeping,ByloPreruseni1,ByloPreruseni2;
unsigned char RandID[4]={0,0,0,0};
__persistent unsigned int LastRand;   // Náhodné číslo
__persistent unsigned int IDV_OK,IDV_Sent,IDV_Fail,IDV_Error,IDV_Conflict,IDV_Reset;

// Proměnné specifické pro hardware
unsigned int CasDelay=0;            // Čekání pro čekací smyčku
unsigned char CasZakmitu;           // Čas zákmitů
unsigned char Vstup;                // Stav vstupů
unsigned char StaryVstup;           // Původní stav vstupů

bit BylaZmenaVstupu;                // Došlo ke změně vstupu

void ReloadEEPROM(void)
{
  unsigned char i;
  DevID=DATAEE_ReadByte(EEPROM_DevID);
  EECheckSum=DevID;
}