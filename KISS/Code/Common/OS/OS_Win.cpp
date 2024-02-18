#include "stdafx_common.h"
#include "OS.h"

#ifdef WIN32

#include <Windows.h>

namespace OS
{
	
	bool IsProcessAlive(const uint32 PID)
	{
		if (HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID))
		{
			DWORD exitCode;
			if (GetExitCodeProcess(hProcess, &exitCode))
			{
				CloseHandle(hProcess);
				return exitCode == STILL_ACTIVE;
			}
			else
			{
				CloseHandle(hProcess);
				return false;
			}
		}
		return false;
	}

}


#endif