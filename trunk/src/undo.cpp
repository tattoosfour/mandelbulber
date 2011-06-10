/*
 * undo.cpp
 *
 *  Created on: 2010-06-05
 *      Author: krzysztof
 */

#include "undo.hpp"
#include "settings.h"
#include "files.h"
#include "interface.h"

CParamsUndo undoBuffer;

CParamsUndo::CParamsUndo(void)
{
	max = 100;
	count = 0;
	last = 0;
	level = 0;
}

void CParamsUndo::SaveUndo(sParamRender *params)
{
	const char *filename1 = "undo/undo";
	char filename2[1000];
	IndexFilename(filename2, filename1, "fract", level % max);
	SaveSettings(filename2, *params);
	printf("Undo information saved (index = %d)\n", level % max);
	level++;
	last = level;
	count++;
	if (count > max - 2) count = max - 2;
	SaveStatus();
}

bool CParamsUndo::GetUndo(sParamRender *params)
{
	bool result = false;
	if (count > 0)
	{
		const char *filename1 = "undo/undo";
		char filename2[1000];

		ReadInterface(params);
		IndexFilename(filename2, filename1, "fract", level % max);
		SaveSettings(filename2, *params);

		count--;
		level--;
		if (level < 0) level = 0;

		IndexFilename(filename2, filename1, "fract", level % max);
		if (FileIfExist(filename2))
		{
			LoadSettings(filename2, *params);
			result = true;
		}
		else
		{
			printf("Unfo file not found (index = %d)\n", level % max);
			result = false;
		}
	}
	else
	{
		printf("I'm sorry, no undo\n");
		result = false;
	}
	SaveStatus();
	return result;
}

bool CParamsUndo::GetRedo(sParamRender *params)
{
	bool result = false;
	if (level < last)
	{
		level++;
		count++;
		if (count > max - 2) count = max - 2;
		const char *filename1 = "undo/undo";
		char filename2[1000];
		IndexFilename(filename2, filename1, "fract", level % max);
		if (FileIfExist(filename2))
		{
			LoadSettings(filename2, *params);
			result = true;
		}
		else
		{
			printf("Undo file not found (index = %d)\n", level % max);
			result = false;
		}
	}
	else
	{
		printf("I'm sorry, no redo\n");
		result = false;
	}
	SaveStatus();
	return result;
}

void CParamsUndo::SaveStatus(void)
{
	FILE *file = fopen("undo/undo.state", "w");
	if (file)
	{
		fprintf(file,"%d %d %d",count,last,level);
		fclose(file);
	}
}

void CParamsUndo::LoadStatus(void)
{
	FILE *file = fopen("undo/undo.state", "r");
	if(file)
	{
		int result = fscanf(file,"%d %d %d",&count,&last,&level);
		(void)result;
		printf("Undo status: count=%d, last=%d, level=%d\n",count,last,level);
		fclose(file);
	}
}
