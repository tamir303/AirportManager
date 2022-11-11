#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Flight.h"
#include "fileHelper.h"

void	initFlight(Flight* pFlight, const AirportManager* pManager)
{
	Airport* pPortOr = setAiportToFlight(pManager, "Enter name of origin airport:");
	pFlight->nameSource = _strdup(pPortOr->name);
	int same;
	Airport* pPortDes;
	do {
		pPortDes = setAiportToFlight(pManager, "Enter name of destination airport:");
		same = isSameAirport(pPortOr, pPortDes);
		if (same)
			printf("Same origin and destination airport\n");
	} while (same);
	pFlight->nameDest = _strdup(pPortDes->name);
	initPlane(&pFlight->thePlane);
	getCorrectDate(&pFlight->date);
}

int		isFlightFromSourceName(const Flight* pFlight, const char* nameSource)
{
	if (strcmp(pFlight->nameSource, nameSource) == 0)
		return 1;
		
	return 0;
}


int		isFlightToDestName(const Flight* pFlight, const char* nameDest)
{
	if (strcmp(pFlight->nameDest, nameDest) == 0)
		return 1;

	return 0;


}

int		isPlaneCodeInFlight(const Flight* pFlight, const char*  code)
{
	if (strcmp(pFlight->thePlane.code, code) == 0)
		return 1;
	return 0;
}

int		isPlaneTypeInFlight(const Flight* pFlight, ePlaneType type)
{
	if (pFlight->thePlane.type == type)
		return 1;
	return 0;
}


void	printFlight(const Flight* pFlight)
{
	printf("Flight From %s To %s\t",pFlight->nameSource, pFlight->nameDest);
	printDate(&pFlight->date);
	printPlane(&pFlight->thePlane);
}

void	printFlightV(const void* val)
{
	const Flight* pFlight = *(const Flight**)val;
	printFlight(pFlight);
}


Airport* setAiportToFlight(const AirportManager* pManager, const char* msg)
{
	char name[MAX_STR_LEN];
	Airport* port;
	do
	{
		printf("%s\t", msg);
		myGets(name, MAX_STR_LEN,stdin);
		port = findAirportByName(pManager, name);
		if (port == NULL)
			printf("No airport with this name - try again\n");
	} while(port == NULL);

	return port;
}

void	freeFlight(Flight* pFlight)
{
	free(pFlight->nameSource);
	free(pFlight->nameDest);
	free(pFlight);
}


int saveFlightToFile(const Flight* pF, FILE* fp)
{
	if (!writeStringToFile(pF->nameSource, fp, "Error write flight source name\n"))
		return 0;

	if (!writeStringToFile(pF->nameDest, fp, "Error write flight destination name\n"))
		return 0;

	if (!savePlaneToFile(&pF->thePlane,fp))
		return 0;

	if (!saveDateToFile(&pF->date,fp))
		return 0;

	return 1;
}


int loadFlightFromFile(Flight* pF, const AirportManager* pManager, FILE* fp)
{

	pF->nameSource = readStringFromFile(fp, "Error reading source name\n");
	if (!pF->nameSource)
		return 0;

	pF->nameDest = readStringFromFile(fp, "Error reading destination name\n");
	if (!pF->nameDest)
		return 0;

	if (!loadPlaneFromFile(&pF->thePlane, fp))
		return 0;

	if (!loadDateFromFile(&pF->date, fp))
		return 0;

	return 1;
}

int	compareFlightBySourceName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameSource, pFlight2->nameSource);
}

int	compareFlightByDestName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameDest, pFlight2->nameDest);
}

int	compareFlightByPlaneCode(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->thePlane.code, pFlight2->thePlane.code);
}

int		compareFlightByDate(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;


	return compareDate(&pFlight1->date, &pFlight2->date);
	

	return 0;
}

int	writeCompressFlight(Flight* pFlight, FILE* file)
{
	BYTE flight[2];
	BYTE plane[3];
	BYTE date[1];

	flight[0] = strlen(pFlight->nameSource) << 3 | (strlen(pFlight->nameDest) >> 2);
	flight[1] = strlen(pFlight->nameDest) << 6 | pFlight->thePlane.type << 4 | pFlight->date.month;
	plane[0] = (pFlight->thePlane.code[0] - 'A') << 3 | (pFlight->thePlane.code[1] -'A') >> 2;
	plane[1] = (pFlight->thePlane.code[1] - 'A') << 6 | (pFlight->thePlane.code[2] - 'A') << 1 | (pFlight->thePlane.code[3] - 'A') >> 4;
	plane[2] = (pFlight->thePlane.code[3]  - 'A') << 4 | pFlight->date.year - 2021;
	date[0] = pFlight->date.day;
	if (fwrite(flight, sizeof(BYTE), 2, file) != 2) return 0;
	if (fwrite(plane, sizeof(BYTE), 3, file) != 3) return 0;
	if (fwrite(pFlight->nameSource, sizeof(char), strlen(pFlight->nameSource), file) != strlen(pFlight->nameSource)) return 0;
	if (fwrite(pFlight->nameDest, sizeof(char), strlen(pFlight->nameDest), file) != strlen(pFlight->nameDest)) return 0;
	if (fwrite(date, sizeof(BYTE), 1, file) != 1) return 0;
	return 1;
}

int	readCompressFlight(Flight* pFlight, FILE* file)
{
	BYTE flight[2];
	BYTE plane[3];
	BYTE date[1];


	if (fread(flight, sizeof(BYTE),2, file) != 2)return 0;
	int sourcelen = flight[0] >> 3;
	int destlen = (flight[0] & 0x7) << 2 | flight[1] >> 6;
	pFlight->thePlane.type = (flight[1] >> 4) & 0x3;
	pFlight->date.month = flight[1] & 0xF;

	if (fread(plane, sizeof(BYTE),3, file) != 3)return 0;
	pFlight->thePlane.code[0] = 'A' + (plane[0] >> 3);
	pFlight->thePlane.code[1] = 'A' + ((plane[0] & 0x7) << 2 || plane[1] >> 6);
	pFlight->thePlane.code[2] = 'A' + ((plane[1] >> 1) & 0x1F);
	pFlight->thePlane.code[3] = 'A' + ((plane[1] & 0x1) << 4 | plane[2] >> 4);
	pFlight->date.year = (plane[2] & 0xF) + 2021;

	if (fread(date, sizeof(BYTE), 1, file) != 1) return 0;
	pFlight->date.day = (date[0] & 0x1F);

	pFlight->nameSource = (char*)calloc(sourcelen + 1, sizeof(char));
	pFlight->nameDest = (char*)calloc(destlen + 1, sizeof(char));

	if (fread(pFlight->nameSource, sizeof(char), sourcelen, file) != sourcelen) {
		return 0;
	};
	if (fread(pFlight->nameDest, sizeof(char), destlen, file) != destlen) {
		return 0;
	};
	return 1;
}

