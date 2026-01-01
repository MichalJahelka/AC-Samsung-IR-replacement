// Formát zprávy
//
// 1 Typ zprávy
// 2 Komu
// 3 Odkud
// 4 Délka zprávy
// 5 Parametry[Délka zprávy] + CRC

// Parametry zprávy msgByte
// 1. Typ zařízení (DevType)
// 2. Hodnota (0-255)
// 3. CRC

#include <stdint.h>

// Definice Komu

#define AllDevices      0

// Definice typů zpráv

#define msgConfig       0x01  // Konfigurace
#define msgAccept       0x02  // Potvrzení příjmu
#define msgByte         0x03  // Vyslání bytu
#define msgArray        0x04  // Vyslání pole
#define msgQuestion     0x05  // Vyslání otázky
#define msgAnswer       0x06  // Odpověď na otázku
#define msgConfigAnswer 0x07  // Konfigurace
#define msgBufferFull   0x08  // Zaplnění bufferu, zprávu neopakovat, ale zaznamenat, že je třeba později opakovat (u důležitých zařízení)
#define msgRepeatNA     0x09  // Opakuj poslední zprávu, pokud jsi nedostal potvrzení (zpráva obecně zasílána všem zařízením)
#define msgWait         0x0A  // Čekej
#define msgDouble       0x0B  // Vyslání reálného čísla
#define msgError        0x0C  // Chyba (byte+datlší možná doplňková data)
#define msgLong         0x0D  // Vyslání long
#define msgLine         0x0E  // Stav obsazení linky
#define msgExtended     0x0F  // Vysílá správu s jedinečným identifikátorem
#define msgText         0x10  // Zpráva jako textový řetězec
#define msgInternal     0xFF  // Vnitřní zpráva

// Typy otázek

#define quStatus           1
#define quData             2

// Další definice

#define eMaxCmdRepeat      3
#define eMaxTrErrors       5
#define eMaxConflicts     50

#define BudiciImpuls       5  // Délka budícího impulsu
#define CasKolize         10  // Poslední příjatý znak nastavuje
#define CasZtratyDat      12  // Možnost přistoupit ke sběrnici
#define CasOpakovani      14  // Čas opakování vysílání zprávy
#define CasZakazu         17  // Zákaz vysílání
#define CasZnovuBuzeni    18  // Vyhodnocovací čas pro buzení -1 až 0 je zákaz vysílání
#define CasUsnuti         20  // Čas kdy zařízení usne
#define CasChyby          50

#define cfgFailed           0x00  // Konfigurace se nepodařila, chybný konfigurační příkaz
#define cfgInitConfig       0x01  // Inicializace konfigurace
#define cfgStopConfig       0x02  // Ukončení konfiguračního režimu
#define cfgWriteEE          0x03  // Zápis do EEPROM
#define cfgReadEE           0x04  // Čtení z EEPROM
#define cfgWriteI2C         0x05  // Zápis do I2C (pole)
#define cfgReadI2C          0x06  // Čtení z I2C (pole)
#define cfgOK               0x07  // Konfigurační příkaz proběhl úspěšně
#define cfgData             0x08  // Data (odpověď) <msgData>
#define cfgGetID            0x09  // Získání identfikace: DevID+Verze
#define cfgID               0x0A  // ID (Int), <msgID>
#define cfgReset            0x0B  // Reset
#define cfgHardware         0x0C  // Hardwarové příkazy
#define cfgEraseFlash       0x0D  // Vymazání FLASH paměti (nelze použít pro upgrade firmware)
#define cfgWriteFlash       0x0E  // Zápis do Flash (nelze použít pro upgrade firmware)
#define cfgReadFlash        0x0F  // Čtení Flash (nelze použít pro upgrade firmware)
#define cfgWriteReg         0x10  // Zápis do registru
#define cfgReadReg          0x11  // Čtení z registru
#define cfgMakeAccess       0x12  // Požadavek o přístup k vyšším funkcím zabezpečeného zařízení
#define cfgAccessed         0x13  // Potvrzení přístupu (povolený nebo oprávněný přístup)
#define cfgGetVersion       0x14  // Datum poslední kompilace
#define cfgVersion          0x15  // Verze <msgArray>
#define cfgWriteExtMem      0x16  // Zápis do externí paměti (Adreasa=Long,Delka=Byte,Data)
#define cfgReadExtMem       0x17  // Čtení z externí paměti (Adresa=Long,Delka=Byte)
#define cfgChangeID         0x18  // Změní DevID, pokud je splněn hw požadavek
#define cfgEraseExtMem      0x19  // Smazání externí paměti
#define cfgWriteFlash32     0x1A  // Zápis Flash 32 bit (nelze použít pro upgrade firmware)
#define cfgReadFlash32      0x1B  // Čtení Flash 32 bit (nelze použít pro upgrade firmware)
#define cfgDescription      0x1C  // Informace o popisu
#define cfgProgress         0x1D  // Informace o postupu, kolik kroků zbývá, nula je konec
#define cfgListFirstFile    0x1E  // Výpis souborů bez LFN (příkaz,parametry,atributy,jméno souboru) [ZFindFile]
#define cfgListNextFile     0x1F  // Výpis souborů bez LFN (příkaz)
#define cfgChDir            0x20  // Změna adresáře (Příkaz,Jméno) [ZConfig.Data]
#define cfgFileInfo         0x21
#define cfgOpenFile         0x22  // Otevřít soubor (s textovým popisem otevření pro starší filesystémy) [ZOpenFile]
#define cfgCloseFile        0x23  // Zavřít (naposledy otevřený) soubor
#define cfgReadFile         0x24  // Číst soubor (příkaz,fielstamp,délka) [ZFileData]
#define cfgWriteFile        0x25  // Zapsat data do souboru (příkaz,fielstamp,délka,data) [ZFileData]
#define cfgEraseFile        0x26  // Smazat soubor (příkaz,jméno)
#define cfgLoadMemory       0x27  // Načte stránku do vyrovnávací paměti, aby mohla být později přepsána
#define cfgWriteMemory      0x28  // Zapíe do RAM na stránce (mění hodnoty)
#define cfgClearMemory      0x29  // Vymaže stránku
#define cfgRandChangeID     0x2A  // Na základě náhodného čísla generovanáho/zjištěného pomocí extGetIDTime změní ID. Není nutné inicializace konfigurace
#define cfgReadiDumVar      0x2B  // Čtení proměnných sběrnice iDům
#define cfgWriteiDumVar     0x2C  // Zápis proměnných iDům
#define cfgPripojeniAudio   0x2D  // Krátce sepne audio linku, pokud je v klidu
#define cfgDefaultConfig    0x2E  // Nastaví výchozí konfiguraci
#define cfgTestConnQuality  0x2F  // Odpoví stejnými parametry zvýšenými o jedničku
#define cfgPing             0x30  // Ověření komunikace (pomocí cfgGetID) s jiným zařízením
#define cfgClear            0x31  // Vymazání/nulování parametrů pro otázku
#define cfgAcoustic         0x32  // Test akustiky
#define cfgReload           0x33  // Znovu načíst konfiguraci
#define cfgRenameFile       0x34  // Přejmenovat soubor Jmeno\0NovéJmeno\0
#define cfgListFirstFile2   0x35  // Výpis souborů s podporou LFN (příkaz,parametry,atributy,jméno souboru) [ZFindFile]
#define cfgListNextFile2    0x36  // Výpis souborů s podporou LFN (příkaz)
#define cfgOpenFile2        0x37  // Otevření souboru s jedním parametrem určujícím typ otevření (moderní rozhraní) [ZOpenFile2]
#define cfgPWD              0x38  // Vrací pracovní adresář
#define cfgSeekFile         0x39  // Nastavení pozice v souboru
#define cfgListFiles        0x3A  // Automatický výpis souborů (Dotaz)
#define cfgFileList         0x3B  // Automatický výpis souborů (Odpověď)
#define cfgFileCRC          0x3C  // Spočítá CRC32 souboru
#define cfgPassword         0x3D  // Je vyžadováno heslo
#define cfgWriteLog         0x3E  // Zapíše LOG do souboru (LogFileStamp,Data)  // Pokud je LogFileStamp 0 nastaví se soubor LOGu, Pokud je FileStamp jiný, zapisuje se do LOGu
#define cfgVersionHW        0x3F  // Dotaz nebo návrat verze HW (1 byte)
#define cfgUpdateFW_Status  0x40  // Stav update FW
#define cfgUpdateFW_Erase   0x41  // Smazat flash
#define cfgUpdateFW_Read    0x42  // Přečíst flash
#define cfgUpdateFW_Write   0x43  // Zapsat FW
#define cfgUpdateFW_Update  0x44  // Provede update FW
#define cfgUpdateFW_Who     0x45  // Dotaz na kdo má/nemá
#define cfgUpdateFW_Addr    0x46  // Nastaví adresu pro srovnání
#define cfgUpdateFW_Bootldr 0x47  // Připrav bootloader (nastaví text v RAM pro bootloader a  provede reset)
#define cfgUpdateFW_RunApp  0x48  // Spusť aplikaci po jejím načtení
#define cfgEELog            0x49  // Zapiš LOG do EEPROM zařízení
#define cfgUpdateFW_Direct  0x4A  // Nastavení rychlé aktualizace (ne protokol iDům)

#define cfgNotSupported     0xFF  // Nepodporovaný příkaz

#define cfgMagic1           0x55
#define cfgMagic2           0xAA

// iDům variables (červen 2017)
#define idvOK               0x01    // Byla odpověď na odeslanou zprávu
#define idvSent             0x02    // Odeslaná zpráva/odpověď
#define idvFail             0x03    // Nebyla odpověď na zprávu, zpráva však byla odeslána
#define idvConflict         0x04    // Došlo ke konfliktu na sběrnici iDům
#define idvError            0x05    // Chyba na sběrnici iDům
#define idvReset            0x06    // Počet resetů

#define StavLinkyAudio      0x01
#define StavLinkyVideo      0x02
#define StavVyzvani         0x04

#ifdef __C30__
  #define __speccmd__ __attribute__ ((packed))
#elif defined __C32__
  #define __speccmd__ __attribute__ ((packed))
#elif defined __packed
  #define __speccmd__ __packed
#else
  #define __speccmd__
#endif

#define data1B        uint8_t
#define data2B        uint16_t
#define data4B        uint32_t

#ifdef __C32__
  #define datadouble    float
#elif defined __STM32__
  #define datadouble    float
#else
  #define datadouble    double
#endif

#ifdef __STM32__
  #undef CRC
#endif

typedef union __speccmd__
{
  struct __speccmd__
  {
    data1B ByteID;
    data1B Value;
    data1B CRC;
  } ZByte;
  struct __speccmd__
  {
    data1B DoubleID;
    datadouble Value;
    data1B CRC;
  } ZDouble;
  struct __speccmd__
  {
    data1B Delka;
    data1B Data[];
  } ZArray;
  struct __speccmd__
  {
    data1B Action;
    data1B Data[];
  } ZConfig;
  struct __speccmd__
  {
    data1B Action;
    data1B Magic1;      // 0x55
    data1B Magic2;      // 0xAA
    data1B Password[];  // Nepovinné doplnění nešifrovaného hesla pro přístup
  } ZConfigInit;
  struct __speccmd__
  {
    data1B QuestionType;
    data1B Param[];
  } ZQuestion;
  struct __speccmd__
  {
    data1B Typ;
  } ZAnswer;
  struct __speccmd__
  {
    data1B TimeMs;
    data1B CRC;
  } ZWait;
  struct __speccmd__
  {
    data1B Error;
    data1B Data[];
  } ZError;
  struct __speccmd__
  {
    data1B Action;
    data1B Command;
    data1B Data[];
  } ZHardware;
  struct __speccmd__
  {
    data1B Typ;       // Použití pro přenos i odpověď
    data2B Adresa;
    data1B Delka;
    data1B Data[];
  } ZData;
  struct __speccmd__
  {
    data1B Typ;
    data4B Adresa;
    data1B Delka;
    data1B Data[];
  } ZDataLong;
  struct __speccmd__
  {
    data1B LongID;
    data4B Value;
    data1B CRC;
  } ZLong;
  struct __speccmd__
  {
    data1B LongID;
    int32_t Value;
    data1B CRC;
  } ZSLong;
  data1B ZText[1];
  struct __speccmd__
  {
    data1B Action;
    data1B FileNames[];
  } ZRenameFile;
  struct __speccmd__
  {
    data2B EID;
    data1B Value;
    data1B CRC;
  } EByte;
  struct __speccmd__
  {
    data2B EID;
    datadouble Value;
    data1B CRC;
  } EDouble;
  struct __speccmd__
  {
    data2B EID;
    data4B Value;
    data1B CRC;
  } ELong;
  struct __speccmd__
  {
    data2B EID;
    data1B Delka;
    data1B Data[];
  } EArray;
  struct __speccmd__
  {
    data2B EID;
    data1B Opakovat;
    data1B Ocekavano;
    data2B Cekat;
    data1B Param;
    data1B Msg[];
  } EZprava;
  struct __speccmd__
  {
    data2B EID;
    data1B CRC;
  } Extended;

  // Odpovědi
  struct __speccmd__
  {
    data1B Action;
    data1B Value;
    data1B CRC;
  } ZAnswerByte;
  struct __speccmd__
  {
    data1B Action;
    data4B Value;
    data1B CRC;
  } ZAnswerLong;
  struct __speccmd__
  {
    data1B Action;
    datadouble Value;
    data1B CRC;
  } ZAnswerDouble;
  struct __speccmd__
  {
    data1B Action;
    data1B Delka;
    data1B Data[];
  } ZAnswerArray;
  struct __speccmd__
  {
    data1B Action;
    data1B Value;
    data1B CRC;
  } ZAnswerAccept;
  struct __speccmd__
  {
    data1B Action;
    data2B DType;
    data1B CRC;
  } ZAnswerId;
  struct __speccmd__
  {
    data1B Action;
    data2B DType;
    data1B RandID[4];
    data1B CRC;
  } ZAnswerId2;
  struct __speccmd__
  {
    data1B LineID;
    data1B Params[];
  } ZLine;
  struct __speccmd__
  {
    data1B Action;
    data1B ListParam;
    data1B Attr;
    data1B FileName[];
  } ZFindFile;
  struct __speccmd__
  {
    data1B Action;
    data1B Mode[4];
    data1B FileName[];
  } ZOpenFile;
  struct __speccmd__
  {
    data1B Action;
    data1B Mode;
    data1B FileName[];
  } ZOpenFile2;
  struct __speccmd__
  {
    data1B Action;
    data1B ListParam;
    data1B FileName[13];
    data1B Info[];
  } ZFileFound;
  struct __speccmd__
  {
    data1B Action;
    data1B ListParam;
    data1B FileNameLen;
    data1B Data[];      // Jméno souboru a parametry
  } ZFileFound2;
  struct __speccmd__
  {
    data1B Action;
    data1B Uspech;
    data1B Attr;
    data4B FileSize,TimeStamp;
  } ZFileInfo;
  struct __speccmd__
  {
    data1B Action;
    data1B FileStamp,Delka;
    data1B Data[];
  } ZFileData;
  struct __speccmd__
  {
    data1B Action;
    data4B Position;
    data1B CRC;
  } ZFileSeek;
  struct __speccmd__
  {
    data1B Action;
    data1B VarNo;
    data2B Var;
    data1B CRC;
  } ZiDumVar;
  struct __speccmd__
  {
    data1B Action;
    data4B CRC32;
    data1B CRC;
  } ZCRC32;
  struct __speccmd__
  {
    data1B Action;
    data1B FileStamp;
    data1B FileData[];
  } ZLog;
  struct __speccmd__
  {
    data1B Action;
    data2B DType;
    data4B Adresa;
    data4B CRC32;
    data1B Delka;
    data1B Data[];
  } ZFirmware;
  struct __speccmd__
  {
    data1B Action;
    data2B DType;
    data4B Adresa;
    data4B CRC32;
    data1B Stav;
    data1B Dotaz;
    data2B Delay;
  } ZFWState;
  struct __speccmd__
  {
    data1B Action;
    data2B D1,D2;
    data1B CRC;
  } Z2Int;

} TTelo;

typedef union
{
  struct __speccmd__
  {
    data1B Typ;
    data1B Komu;
    data1B Odkud;
    data1B Delka;
    data1B UserID;
    TTelo Obsah;
  } Data;
  data1B QuickAccess[MaxBufLen];
} TZprava;

typedef union
{
  struct
  {
    uint8_t Lo,Hi;
  } Byte;
  uint16_t Int;
} TByteInt;

typedef union
{
  struct
  {
    uint8_t Lo,Hi;
  } Byte;
  int16_t Int;
} TByteSInt;

typedef union
{
  uint8_t Bytes[4];
  datadouble Double;
} TByteDouble;

typedef union
{
  uint8_t Bytes[4];
  uint32_t Long;
} TByteLong;

typedef enum
{
  fwNone=0,
  fwError,
  fwErase,
  fwEraseError,
  fwRead,
  fwReadError,
  fwWrite,
  fwWriteError,
  fwCRCError,
  fwAddressError,
  fwGlobalCRCError,
  fwCompleted,
} TFWState;

typedef enum
{
  fwdHasState=1,
  fwdNotState,
  fwdHasAddress,
  fwdNotAddress,
  fwdHasCRC,
  fwdNotCRC,
  fwdHasStateAddressCRC,
  fwdNotStateAddressCRC
} TFWQuestion;

#define Release     1
#define Debug       2

