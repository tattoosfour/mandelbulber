/*
 * undo.hpp
 *
 *  Created on: 2010-06-05
 *      Author: krzysztof
 */

#ifndef UNDO_HPP_
#define UNDO_HPP_

#include "fractparams.h"

class CParamsUndo
{
public:
	CParamsUndo(void);
	void SaveUndo(sParamRender *params);
	bool GetUndo(sParamRender *params);
	bool GetRedo(sParamRender *params);
	void LoadStatus(void);
private:
	void SaveStatus(void);
	int count;
	int last;
	int level;
	int max;
};

extern CParamsUndo undoBuffer;

#endif /* UNDO_HPP_ */
