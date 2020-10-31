
#pragma once
#ifndef PROGRAMENTRANCE_H_
#define PROGRAMENTRANCE_H_
#include "stdafx.h"


class ProgramEntPoint : public CWinApp
{
public:
	ProgramEntPoint();
	~ProgramEntPoint();

private:
	virtual BOOL InitInstance() override;
};

ProgramEntPoint g_WinApp;

#endif