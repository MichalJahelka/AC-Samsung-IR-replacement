/* 
 * File:   iDum.h
 * Author: majk
 *
 * Created on 21. listopadu 2025, 8:07
 */

#ifndef IDUM_H
#define	IDUM_H

#ifdef	__cplusplus
extern "C"
{
  #endif

  // Spočítá kontrolní součet iDům
  unsigned char ComputeCRC(unsigned char * CompZprava);
  // Zpracuje požadavky přerušení od systému iDům
  void iDumProcessIntr(void);
  // Odešle zprávu Vysilani do systému iDům bez příjmu potvrzení
  void VysliPole(void);
  // Uloží zprávu Zprava do UZprava
  void UlozZpravu(void);
  // Zálohuje zprávu Zprava do TempZprava
  void ZalohujZpravu(void);
  // Obnoví zprávu Zprava z TempZprava, pokud TempZprava obsahuje data, nastaví ZpravaPrijata na 1
  void ObnovZpravu(void);
  // Čeká na to až bude čas sběrnice v systému větší než zadaný čas
  void iDumWait(unsigned char iDumTime);
  // Odešle zprávu Vysilani do systému iDům, čeká na potvrzení
  bit StandardSend(void);
  // Odešle konfigurační zprávu, čeká na potvrzení msgConfigAnswer
  bit ConfigSend(void);
  // Vyšle pole a čeká na opdověď
  bit WaitForAnswer(unsigned char Typ,unsigned char Odkud);
  // Vyšle pole a čeká na konfigurační odpověď
  bit WaitForConfigAnswer(unsigned char Typ,unsigned char Odkud);
  // Opdoví Accept na zprávu
  void VysliAccept(void);
  // Odpoví verzí na dotaz
  void VysliVerzi(void);
  // Odpoví ID
  void VysliID(void);
  // Odpoví popisem
  void VysliPopis(void);
  // Odpoví konfiguračním stavem (např. cfgOK)
  void VysliConfigState(unsigned char ConfigState);
  // Vyšle výsledek hardwarového úkonu
  void VysliHWRes(unsigned char Komu,unsigned char HWCmd,unsigned char HWRes);
  // Vyšle zprávu Wait (například při delší činnosti, která vyžaduje čas na zpracování a následně potvrzení)
  void VysliWait(unsigned char TimeMs);
  // Vyšle obsah EEPROM jako odpověď
  void VysliObsahEEPROM(void);
  // Zapíše data do EEPROM
  void ZapisEEPROMData(void);
  bit VysliByte(unsigned char Komu,unsigned char ByteID,unsigned char Value);
  // Vyšle chybu msgError jako odpověď
  void VysliChybu(unsigned char Komu, unsigned char ErrorID);
  // Vyšle stav linky (používáno pro osazení audio/video)
  void VysliStavLinky(unsigned char i);
  // Zjistí stav obsazenosti linky iDům (používáno pro osazení audio/video)
  bit ZjistiStavLinky(void);
  // Odpověď - čeká náhodný čas (rozsah daný v dozazu) a pak vyšle svoje ID
  void SendIDTime(unsigned char Odkud);
  // Čtení výsledků komunikačních parametrů
  void ReadiDumVar(void);
  // Zápis (nulování) výsledků komunikačních parametrů
  void WriteiDumVar(void);
  // Čekání v ms
  void DelayMs(unsigned int CasMs);

  #ifdef	__cplusplus
}
#endif

#endif	/* IDUM_H */

