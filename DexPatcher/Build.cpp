#include "Build.h"
#include <stdio.h>
#include <Windows.h>

void GetBuildDateBase(char* year, char* month, char* day)
{
	const char* pMonth[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	const char date[12] = __DATE__;//取编译时间
	int i;
	for (i = 0; i < 12; i++)if (memcmp(date, pMonth[i], 3) == 0)*month = i + 1, i = 12;
	*year = (char)atoi(date + 9); //Date[9]为２位年份，Date[7]为完整年份
	*day = (char)atoi(date + 4);
}
char g_dateBuffer[50];

char* Build::GetBuildDate(void)
{
	char  year, month, day;
	GetBuildDateBase(&year, &month, &day);//取编译时间
	sprintf_s(g_dateBuffer, "20%02d.%02d.%02d", year, month, day);//任意格式化
	return g_dateBuffer;
}

void getBuildTime()
{
	/*char Mmm[4] = "Jan";
	sprintf(__DATE__, "%3s %hu %hu", Mmm, sizeof(Mmm),
		&lpCompileTime->wDay, &lpCompileTime->wYear);
	Mmm[3] = Mmm[2]; Mmm[2] = Mmm[0]; Mmm[0] = Mmm[3]; Mmm[3] = 0;

	const char* szMonth = "Jan";
	short month = 0;
	int dwMonth = *(int*)szMonth;
	switch (dwMonth)
	{
	case 'Jan': month = 1; break;
	case 'Feb': month = 2; break;
	case 'Mar': month = 3; break;
	case 'Apr': month = 4; break;
	case 'May': month = 5; break;
	case 'Jun': month = 6; break;
	case 'Jul': month = 7; break;
	case 'Aug': month = 8; break;
	case 'Sep': month = 9; break;
	case 'Oct': month = 10; break;
	case 'Nov': month = 11; break;
	case 'Dec': month = 12; break;
	default:month = 0; break;
	}*/

}