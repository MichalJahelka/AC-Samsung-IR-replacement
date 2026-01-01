No protocol found for Samsung air condition IR control, so I had to catch communication with IR receiver (38 kHz) and decode it with MCU. I did not gues all bits, but the biggest part of protocol was decoded. For decoding is used PIC18F47K40, sends data throug serial line to terminal for display at PC. Program IRprotokolSamsungAC.X is written in MPLAB X, uses free compiler, has CZECH comments (so translator can be handy).
IRprotokolSamsungAC.X can decode protocols and after uncomment
//TransmittPacket(Paket,Delka);
//while(1);
can emulate one time data sending.
I tested it with IR LED, so pin with 38 kHz modulation is there too.

At second part KlimatizaceSamsung.X is practical usage. It uses iDum protocol for communication with control unit, data are passed to CreatePaket function and then created packet is send to output. It does not using IR LED, it connects throug optocoupler to IR receiver at AC unit, so modulation is not needed. Practilal realisation has schematic and PCB.

There is no problem to use it in another own device. Important is creating packet and sending it:
When data arrives:
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

And sending is automatic with TMR5 interrupt:
PIE4=0b00010011;      // TMR5IE,TMR2IE,TMR1IE
IPR4=0b00010001;      // TMR5 (Samsung transmitt), TMR1 (iDum) High priority

void __interrupt() Preruseni(void)
{
  iDumProcessIntr();
  TransmittBlockInterrupt(); // here is sending realized
}

CreatePaket, TransmittPacket and TransmittBlockInterrupt functions are parts of SamsungIR library.

Schematic is in KiCAD, extended documents are in CZECH language, use transaltor if you want.
