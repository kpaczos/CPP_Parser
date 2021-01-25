#include <iostream>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <Windows.h>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <string>
#pragma warning(disable:4996)
using namespace std;


#define NOT_VALID  -1

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//=============================================================================================================================================================================
// Byte swap
//=============================================================================================================================================================================
#if defined(_MSC_VER)
static inline uint16_t xSwapBytes16(uint16_t Value) { return _byteswap_ushort(Value); }
static inline  int16_t xSwapBytes16(int16_t Value) { return _byteswap_ushort(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return _byteswap_ulong(Value); }
static inline  int32_t xSwapBytes32(int32_t Value) { return _byteswap_ulong(Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return _byteswap_uint64(Value); }
static inline  int64_t xSwapBytes64(int64_t Value) { return _byteswap_uint64(Value); }
#elif defined (__GNUC__)
static inline uint16 xSwapBytes16(uint16 Value) { return __builtin_bswap16(Value); }
static inline  int16 xSwapBytes16(int16 Value) { return __builtin_bswap16(Value); }
static inline uint32 xSwapBytes32(uint32 Value) { return __builtin_bswap32(Value); }
static inline  int32 xSwapBytes32(int32 Value) { return __builtin_bswap32(Value); }
static inline uint64 xSwapBytes64(uint64 Value) { return __builtin_bswap64(Value); }
static inline  int64 xSwapBytes64(int64 Value) { return __builtin_bswap64(Value); }
#else
#error Unrecognized compiler
#endif




class xTS {
public:
  static constexpr uint32_t TS_PacketLength = 188;
  static constexpr uint32_t TS_HeaderLength = 4;

  static constexpr uint32_t PES_HeaderLength = 6;

  static constexpr uint32_t BaseClockFrequency_Hz = 90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz = 90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz = 27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier = 300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
  enum class ePID : uint16_t
  {
    PAT = 0x0000,
    CAT = 0x0001,
    TSDT = 0x0002,
    IPMT = 0x0003,
    NIT = 0x0010, //DVB specific PID
    SDT = 0x0011, //DVB specific PID
    NuLL = 0x1FFF,
  };

protected:
  uint8_t SB;
  bool E;
  bool S;
  bool T;
  uint16_t PID;
  uint8_t TSC;
  int AFC;
  int CC;
public:

  void  Parse(uint8_t* Input);
  void  Print() const;
  void  Reset();

public:

  uint8_t getSb() const;

  bool isE() const;

  bool isS() const;

  bool isT() const;

  uint16_t getPid() const;

  uint8_t getTsc() const;

  uint8_t getAfc() const;

  int getCc() const;

public:

  bool     hasAdaptationField();
  bool     hasPayload();

};

//=============================================================================================================================================================================

class xTS_AdaptationField {
protected:
  uint8_t L;
  bool DC;
  bool RA;
  bool SP1;
  bool PR;
  bool OR;
  bool SP2;
  bool TP;
  bool EX;
public:
  void Reset();
  void Parse(const uint8_t* Input, uint8_t AdaptationFieldControl);
  void Print() const;
public:
  uint8_t getL();
  bool getDC();
  bool getRA();
  bool getSP1();
  bool getPR();
  bool getOR();
  bool getSP2();
  bool getTP();
  bool getEX();
  uint32_t getNumBytes() const { };
};

//=============================================================================================================================================================================


class xPES_PacketHeader {
protected:
  uint32_t PSCP[3];
  uint8_t SID;
  uint16_t LEN;
  uint8_t PESHDL;

public:
  uint8_t packet[188];
  void Parse();
  void Print() const;
  void Reset();

public:

  uint8_t getSID();
  uint16_t getLEN();
  uint8_t getPESHDL();
};

void xPES_PacketHeader::Parse()
{
  PSCP[0] = packet[0];
  PSCP[1] = packet[1];
  PSCP[2] = packet[2];

  SID = packet[3];
  LEN = xSwapBytes16(*(uint16_t*)&packet[4]);
  PESHDL = packet[8];
}


//=============================================================================================================================================================================

void xTS_PacketHeader::Parse(uint8_t* Input) {
  uint32_t HDR = xSwapBytes32(*(uint32_t*)Input);
  SB = (HDR & 0xFF000000) >> 24;
  E = (HDR & 0x00800000) >> 23;
  S = (HDR & 0x00400000) >> 22;
  T = (HDR & 0x00200000) >> 21;
  PID = (HDR & 0x001FFF00) >> 8;
  TSC = (HDR & 0x000000C0) >> 6;
  AFC = (HDR & 0x00000030) >> 4;
  CC = (HDR & 0x0000000F);
}

uint8_t xTS_PacketHeader::getSb() const {
  return SB;
}

bool xTS_PacketHeader::isE() const {
  return E;
}

bool xTS_PacketHeader::isS() const {
  return S;
}

bool xTS_PacketHeader::isT() const {
  return T;
}

uint16_t xTS_PacketHeader::getPid() const {
  return PID;
}

uint8_t xTS_PacketHeader::getTsc() const {
  return TSC;
}

uint8_t xTS_PacketHeader::getAfc() const {
  return AFC;
}

int xTS_PacketHeader::getCc() const {
  return CC;
}

inline bool xTS_PacketHeader::hasAdaptationField()
{
  if (AFC == 2 || AFC == 3) return true;
  else return false;
}

inline bool xTS_PacketHeader::hasPayload()
{
  if (AFC == 1) return true;
  else return false;
}

void xTS_PacketHeader::Print() const {

  int counter = 0;
  //show_counter(counter);
  counter++;
  std::cout << " TS: ";

  std::cout.width(3);
  std::cout << "SB=" << (int)getSb();

  std::cout << " E=" << (int)isE();

  std::cout << " S=" << (int)isS();

  std::cout << " T=" << (int)isT();

  std::cout << " PID=";
  std::cout.width(4);
  std::cout << std::right << (int)getPid();

  std::cout << " TSC=" << (int)getTsc();

  std::cout << " AFC=" << (int)getAfc();

  std::cout << " CC=";
  std::cout.width(2);
  std::cout << std::right << (int)getCc();
}

void xTS_PacketHeader::Reset()
{
  SB = 0;
  E = 0;
  S = 0;
  T = 0;
  PID = 0;
  TSC = 0;
  AFC = 0;
  CC = 0;
}

void xTS_AdaptationField::Reset()
{
  L = 0;
  DC = 0;
  RA = 0;
  SP1 = 0;
  PR = 0;
  OR = 0;
  SP2 = 0;
  TP = 0;
  EX = 0;
}

void xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
  if (AdaptationFieldControl == 2 || AdaptationFieldControl == 3) {
    uint64_t AF = xSwapBytes64((*(uint64_t*)Input));
    L = (AF & 0x00000000FF000000) >> 24;
    DC = (AF & 0x0000000000800000) >> 23;
    RA = (AF & 0x0000000000400000) >> 22;
    SP1 = (AF & 0x0000000000200000) >> 21;
    PR = (AF & 0x0000000000100000) >> 20;
    OR = (AF & 0x0000000000080000) >> 19;
    SP2 = (AF & 0x0000000000040000) >> 18;
    TP = (AF & 0x0000000000020000) >> 17;
    EX = (AF & 0x0000000000010000) >> 16;
  }
}

uint8_t xTS_AdaptationField::getL()
{
  return L;
}

bool xTS_AdaptationField::getDC()
{
  return DC;
}

bool xTS_AdaptationField::getRA()
{
  return RA;
}

bool xTS_AdaptationField::getSP1()
{
  return SP1;
}

bool xTS_AdaptationField::getPR()
{
  return PR;
}

bool xTS_AdaptationField::getOR()
{
  return OR;
}

bool xTS_AdaptationField::getSP2()
{
  return SP2;
}

bool xTS_AdaptationField::getTP()
{
  return TP;
}

bool xTS_AdaptationField::getEX()
{
  return EX;
}

void xTS_AdaptationField::Print() const {

  cout << " AF: ";
  cout << "L=";
  cout.width(3);
  cout << (int)L;
  cout << " DC=" << (int)DC;
  cout << " RA=" << (int)RA;
  cout << " SP=" << (int)SP1;
  cout << " PR=" << (int)PR;
  cout << " OR=" << (int)OR;
  cout << " SP=" << (int)SP2;
  cout << " TP=" << (int)TP;
  cout << " EX=" << (int)EX;

}


void xPES_PacketHeader::Print() const
{
  if (PSCP[0] == 0 && PSCP[1] == 0 && PSCP[2] == 1)
  {
    std::cout << " PES: ";
    std::cout << "PSCP=" << PSCP[0] + PSCP[1] + PSCP[2];
    std::cout.width(3);
    std::cout << " SID=";
    std::cout << std::right << int(SID);
    std::cout << " L=";
    std::cout.width(4);
    std::cout << std::right << int(LEN);
  }
}

void xPES_PacketHeader::Reset()
{
  for (size_t i = 0; i < 3; i++)
  {
    PSCP[i] = 0;
  }
  SID = 0;
  LEN = 0;
  PESHDL = 0;
}


uint8_t xPES_PacketHeader::getSID()
{
  return SID;
}

uint16_t xPES_PacketHeader::getLEN()
{
  return LEN;
}

uint8_t xPES_PacketHeader::getPESHDL()
{
  return PESHDL;
}


int main()
{
  xTS  TS;
  uint8_t TS_PacketBuffer[TS.TS_PacketLength];
  ifstream file("example_new.ts", ios::binary | ios::in);

  FILE* Fonia;
  Fonia = fopen("PID136.mp2", "wb");

  if (file.good() == true) {

    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_AdaptationField;
    xPES_PacketHeader PES_PacketHeader;

    const int PID136 = 136;
    int CC = 0;
    int PES_ovL = 0;

    int32_t TS_PacketId = 0;
    for (;;) {
      if (file.eof()) break;

      file.read((char*)TS_PacketBuffer, TS.TS_PacketLength);


      TS_PacketHeader.Parse((uint8_t*)TS_PacketBuffer);
      TS_AdaptationField.Parse((uint8_t*)TS_PacketBuffer, (int)TS_PacketHeader.getAfc());

      //printf("%010d ", TS_PacketId);
      //TS_PacketHeader.Print();
      //if (TS_PacketHeader.hasAdaptationField()) TS_AdaptationField.Print();

      if (TS_PacketHeader.getPid() == PID136) {

        if (TS_PacketHeader.isS() == 1) {
          //cout << " Started";
          if (TS_PacketHeader.hasPayload()) {
            for (int i = 4, j = 0; i < 188; i++, j++) {
              PES_PacketHeader.packet[j] = TS_PacketBuffer[i];
            }
          }
          else if (TS_PacketHeader.hasAdaptationField()) {
            for (int i = 5 + TS_PacketBuffer[4], j = 0; i < 188 - TS_PacketBuffer[4]; i++, j++) {
              PES_PacketHeader.packet[j] = TS_PacketBuffer[i];
            }
          }

          PES_PacketHeader.Parse();
          //PES_PacketHeader.Print();

          if (TS_PacketHeader.hasAdaptationField()) {
            int temp = 5 + PES_PacketHeader.getPESHDL() + 6 + 3 + (int)TS_AdaptationField.getL();
            int temp_m = 188 - 5 - PES_PacketHeader.getPESHDL() - 6 - 3 - (int)TS_AdaptationField.getL();
            fwrite(TS_PacketBuffer + temp, temp_m, 1, Fonia);
          }
          PES_ovL = PES_PacketHeader.getPESHDL() + PES_PacketHeader.getLEN();

          CC++;

        }
        else if (TS_PacketHeader.isS() == 0 && TS_PacketHeader.hasAdaptationField()) {

          //cout << " Finished";

          if (TS_PacketHeader.hasAdaptationField()) {
            for (int i = 5 + TS_PacketBuffer[4], j = 0; i < 188 - TS_PacketBuffer[4]; i++, j++) {
              PES_PacketHeader.packet[j] = TS_PacketBuffer[i];
            }
          }

          cout.width(4);
          //cout << right << " PES: Len=" << PES_ovL;

          if (TS_PacketHeader.getAfc() == 3) {

            fwrite(TS_PacketBuffer + 5 + TS_AdaptationField.getL(), 188 - 5 - TS_AdaptationField.getL(), 1, Fonia);
            if (TS_PacketHeader.getCc() == 15) CC = 0;
            if (TS_PacketHeader.getCc() == 1) CC = 2;

          }
          else if (TS_PacketHeader.getAfc() == 1) { fwrite(TS_PacketBuffer + 4, 184, 1, Fonia); }

        }
        else if ((CC == TS_PacketHeader.getCc() && TS_PacketHeader.getAfc() == 1) ||
          TS_PacketHeader.getCc() == 15 && TS_PacketHeader.getAfc() == 1 || TS_PacketHeader.getCc() && TS_PacketHeader.getAfc() == 1) {

          //cout << " Continue";
          if (TS_PacketHeader.getAfc() == 3) {
            fwrite(TS_PacketBuffer + 5 + TS_AdaptationField.getL(), 183, 1, Fonia);
          }
          else if (TS_PacketHeader.getAfc() == 1) {
            fwrite(TS_PacketBuffer + 4, 184, 1, Fonia);
          }

          CC++;

        }
        else if (CC < TS_PacketHeader.getCc() || CC  > TS_PacketHeader.getCc()) {

          //cout << " Packet lost";
          if (TS_PacketHeader.getAfc() == 0b01) {
            fwrite(TS_PacketBuffer + 4, 184, 1, Fonia);
          }

          CC = 0;

        }

      }
      //cout << endl;

      TS_PacketHeader.Reset();
      TS_AdaptationField.Reset();
      PES_PacketHeader.Reset();

      TS_PacketId++;
      //Sleep(300);
    }

  }
  fclose(Fonia);
}