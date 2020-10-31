#include "ProgramEntrance.h"
#include "MasterWindows.h"

ProgramEntPoint::ProgramEntPoint()
{

}

ProgramEntPoint::~ProgramEntPoint()
{

}

BOOL ProgramEntPoint::InitInstance()
{
	MasterWindows obj_Master;
	m_pMainWnd = &obj_Master;
	obj_Master.DoModal();
	return TRUE;
}