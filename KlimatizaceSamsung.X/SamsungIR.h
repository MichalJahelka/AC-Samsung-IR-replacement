/* 
 * File:   SamsungIR.h
 * Author: majk
 *
 * Created on 21. listopadu 2025, 11:13
 */

#ifndef SAMSUNGIR_H
#define	SAMSUNGIR_H

#ifdef	__cplusplus
extern "C"
{
  #endif
  
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
  
  unsigned char SamsungPaket[21];     // Zpráva odesílaná do klimatizace

  // Realizace samotného vysílání na základě časů v SamsungIRData
  void TransmittBlockInterrupt();
  
  // Vytvoří paket pro vyslání dat dálkovým ovladačem
  // Vrací délku paketu
  // Paket musí mít alespoň 21 bytů
  uint8_t CreatePaket(uint8_t *Paket, TMode Mode, uint8_t Temperature, uint8_t Fan, TOptions Options, TSwing Swing, TZone Zone, TDirection Direction);
  
  // Připraví vysílání IR celého paketu a vyšle ho
  void TransmittPacket(uint8_t *Packet, uint8_t PocetBytu);

  #ifdef	__cplusplus
}
#endif

#endif	/* SAMSUNGIR_H */

