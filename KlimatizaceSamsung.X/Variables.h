/*
 * Definice a proměnné pro řízení klimatizace Samsung
 */

#ifndef VARIABLES_H
#define	VARIABLES_H

#ifdef	__cplusplus
extern "C"
{
  #endif

  #include "../../DevTypes.h"

  #define MaxBufLen       64
  #define DevType         dtKlimatizaceSamsung

  #include "../../Zpravy.h"

  #define Cas1ms              (0xFFFF-CrystalFreq/4000+1) // (0x10000-CrystalFreq/4000) dává hlášku Arithmetic overflow in constant expression
  #define CasScanu            (Cas1ms/2+0x8000)

  #define Rx              PORTCbits.RC7
  #define Tx              LATBbits.LATB2
  #define TrisTx          TRISBbits.TRISB2
  #define IRControl       LATAbits.LATA2
  #define OnOff           LATAbits.LATA3
  #define Active          PORTCbits.RC0
  #define AccessIO        PORTBbits.RB7
  
  
  #define iDumTxIO()      { RB2PPS = 0x00; }               // LATB2 -> RB2
  #define iDumTxUART()    { RB2PPS = 0x09; }               // TX1   -> RB2
  
  // Pozice konfigurace v paměti EEPROM
  #define EEPROM_DevID            0u
  #define EEPROM_CasZakmitu       EEPROM_DevID+1
  #define EEPROM_Popis            8u
  
  // Příkazy pro modul řízení klimatizace Samsung
  #define rksPower                0x01  // Sepnutí vypínače
  #define rksStatus               0x02  // Změna stavu
  #define rksCommand              0x03  // Příkaz (emulace DO)
  
  // Proměnné pro systém iDům
  TZprava Docasna,Zprava,Vysilani,TempZprava,ZalohaZpravy;
  __near unsigned char Cas,EECheckSum;
  __near unsigned char PoziceZpravy,PrijatyZnak,DevID,Configuring,WaitingTime;
  __near bit PrijimamData,ZpracovavamData,ZpravaPrijata,VysilaniSelhalo,Vysilam;
  __near bit ZpravaVyslana,Sleeping,ByloPreruseni1,ByloPreruseni2;
  unsigned char RandID[4];
  __persistent unsigned int LastRand;   // Náhodné číslo
  __persistent unsigned int IDV_OK,IDV_Sent,IDV_Fail,IDV_Error,IDV_Conflict,IDV_Reset;
  
  // Proměnné specifické pro hardware
  unsigned int CasDelay;              // Čekání pro čekací smyčku
  unsigned char CasZakmitu;           // Čas zákmitů
  unsigned char Vstup;                // Stav vstupů
  unsigned char StaryVstup;           // Původní stav vstupů
    
  bit BylaZmenaVstupu;                // Došlo ke změně vstupu
  
  void ReloadEEPROM(void);

  #ifdef	__cplusplus
}
#endif

#endif	/* VARIABLES_H */

