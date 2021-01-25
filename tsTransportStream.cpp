#include "tsTransportStream.h"
#include <iostream> //std::cout
#include <iomanip>  //std::setw

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

void xTS_PacketHeader::Reset()
{
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
	SB = Input[0];
	E = Input[1] & 128 ? 1 : 0;
	S = Input[1] & 64 ? 1 : 0;
	T = Input[1] & 32 ? 1 : 0;
	PID = (Input[1] & 31) << 8 | Input[2];
	TSC = (Input[3] & 192) >> 6;
	AFC = (Input[3] & 48) >> 4;
	CC = Input[3] & 15;

	return int32_t();
}

void xTS_PacketHeader::Print() const
{
	std::cout << "TS: SB=" << SB << " E=" << E << " S=" << S << " P=" << T << " PID=" << std::setw(4) << PID << " TSC=" << TSC << " AF=" << AFC << " CC=" << std::setw(2) << CC;
}

uint16_t xTS_PacketHeader::getAFC()
{
	return AFC;
}

bool  xTS_PacketHeader::hasAdaptationField() const
{
	if (AFC == 2 || AFC == 3)
		return true;
	return false;
}
bool xTS_PacketHeader::hasPayload() const
{
	if (AFC == 2)
		return false;
	return true;
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================

void xTS_AdaptationField::Reset()
{
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl)
{
	AFL = Input[4];

	if (AFL > 0)
	{
		DC = (Input[5] & 128) >> 7;
		RA = (Input[5] & 64) >> 6;
		SPI = (Input[5] & 32) >> 5;
		PR = (Input[5] & 16) >> 4;
		OR = (Input[5] & 8) >> 3;
		SPF = (Input[5] & 4) >> 2;
		TP = (Input[5] & 2) >> 1;
		EX = Input[5] & 1;
	}

	return int32_t();
}

void xTS_AdaptationField::Print() const
{
	std::cout << " AF: L=" << std::setw(3) << AFL << " DC=" << DC << " RA=" << RA << " SP=" << SPI << " PR=" << PR << " OR="
		<< OR << " SP=" << SPF << " TP=" << TP << " EX=" << EX;
}

//=============================================================================================================================================================================
