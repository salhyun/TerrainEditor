#include "DataStructLib.h"
#include "def.h"
#include <tchar.h>

using namespace data ;

void SCString::format(char* lpFormatStr, ...)
{
	va_list args;
	va_start    (args, lpFormatStr);
	_vsntprintf (str, 256, lpFormatStr, args);
	va_end      (args);
}
char *SCString::GetString()
{	
	return str ;
}

int ClassifyArray(int *pnArray, int nNumArray, data::SClassify **ppnClassify)
{
	int i, n ;
	int nPrevID ;
	SClassify asClassify[256] ;
	int nClassifyCount, nSubCount ;
	bool bExist ;

	nPrevID = pnArray[0] ;
	nClassifyCount = nSubCount = 0 ;

	for(i=0 ; i<nNumArray ; i++)
	{
		if(pnArray[i] != nPrevID)
		{
			bExist=false ;
			for(n=0 ; n<nClassifyCount ; n++)
			{
				if(asClassify[n].nID == nPrevID)//이전에 이미 같은 아이디가 있다면
				{
					asClassify[n].nCount += nSubCount ;
					bExist=true ;
				}
			}

			if(!bExist)
			{
				asClassify[nClassifyCount].nCount = nSubCount ;
				asClassify[nClassifyCount].nID = nPrevID ;
				nClassifyCount++ ;
			}
			nSubCount = 0 ;
		}

		nSubCount++ ;
		nPrevID = pnArray[i] ;
	}

	bExist=false ;
	for(n=0 ; n<nClassifyCount ; n++)
	{
		if(asClassify[n].nID == pnArray[i-1])//이전에 이미 같은 아이디가 있다면
		{
			asClassify[n].nCount += nSubCount ;
			bExist=true ;
		}
	}

	if(!bExist)
	{
		asClassify[nClassifyCount].nCount = nSubCount ;
		asClassify[nClassifyCount].nID = pnArray[i-1] ;
		nClassifyCount++ ;
	}

	SClassify *psClassify = new SClassify[nClassifyCount] ;

	for(i=0 ; i<nClassifyCount ; i++)
	{
		psClassify[i].nID = asClassify[i].nID ;
		psClassify[i].nCount = asClassify[i].nCount ;
	}

	(*ppnClassify) = psClassify ;



	return nClassifyCount ;

	//TRACE("Classify Number=%02d\r\n", nClassifyCount) ;
	//for(i=0 ; i<nClassifyCount ; i++)
	//	TRACE("ID[%02d] Num=%02d\r\n", asClassify[i].nID, asClassify[i].nCount) ;

}