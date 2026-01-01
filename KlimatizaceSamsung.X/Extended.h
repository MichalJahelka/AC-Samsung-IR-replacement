// Definice rozšířených příkazů (jednoznačené identifikátory) pro systém iDům
// Pravidlo vytváření názvů:
// ext Jméno_Zařízení Událost

#define extZdrojStav            0x01    // Změna stavu zdroje; array
#define extInterkomHang         0x02    // Ukončení komunikace (zavěšení); byte (priorita)
#define extInterkomCancelRing   0x03    // Informace pro telefony, že mají přestat vyzvánět; byte(0)
#define extCode                 0x04    // Posílání přístupového kódu; array(code[6])
#define extGroupCode            0x05    // Posílání přístupového kódu a skupiny; array(group,code[6])
#define extInterkomGroupCall    0x06    // Volání do skupiny; array(typ zařízení,skupina,priorita,poměr kamery)
#define extCardID               0x07    // Vysílání karty; array(group,reader,cardID[5])
#define extVirtualCall          0x08    // Volání na virtuální číslo; array(NumHi,NumLo,typ zařízení,priorita);
#define extDisconnect           0x09    // Odpojení se od linky byte(linetype)
#define extPower                0x0A    // Zapnutí/vypnutí napájení (byte)
#define extTime                 0x0B    // Informace o čase (Sekunda,Minuta,Hodina,Den,DenVTýdnu,Měsíc,Rok)
#define extGetIDTime            0x0C    // Předej normální zprávou svoje ID pomocí náhodného času
#define extPowerLow             0x0D    // Napájení akumulátoru je nízké (double = Napětí [V])
#define extShutdown             0x0E    // Příliš nízké napětí akumulátoru. Zdroj se vypne byte(čas[s])
#define extControl              0x0F    // Povolit nebo zablokovat uživateli řízení; array("Control",level,group,devtype,devclass)
#define extCardResult           0x10    // Stav karty (zamítnuto,schváleno...)
#define extImportName           0x11    // Příkaz pro importování (změnu) jména seznamu
#define extPSMasterMode         0x12    // Příkaz pro indikaci Master módu; byte (mode)
#define extAlarm                0x13    // Globální alarm (typ 1=Siréna, 2=Havárie, 3=Zrušit, 4=Výstraha):array(typ,skupina,popis)
#define extSMS                  0x14    // Array(GSM Číslo\0,Zpráva\0)
#define extZprava               0x15    // Poslat zprávu do systému iDům (komunikace přes jiné rozhraní); array(Opakovat,Ocekavano,Čekat,Param,Zprava)

#define extAlarm_Sirena         0x01
#define extAlarm_Havarie        0x02
#define extAlarm_Zrusit         0x03
#define extAlarm_Vystraha       0x04
#define extAlarm_Zapnout        0x05
