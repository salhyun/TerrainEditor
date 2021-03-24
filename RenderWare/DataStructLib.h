#pragma once

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ArrangedBuffer.h"

#pragma warning(disable:4996)//warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

namespace data
{
	struct SClassify
	{
		int nID ;
		int nCount ;

		SClassify()
		{
			nID = -1 ;
			nCount = 0 ;
		}
	} ;

	//public array management
	//�������� �迭����
	template <typename T>
	struct SArray
	{
		int nMaxNum ;
		int nPushCount ;
		T *ptype ;

		SArray()
		{
			nMaxNum = nPushCount = 0 ;
			ptype = NULL ;
		} ;
		SArray(int _nMaxNum)
		{
			Initialize(_nMaxNum) ;
		}
		~SArray()
		{
			if(ptype) { delete []ptype ; ptype = NULL ; }
		} ;
		void Initialize(int _nMaxNum)
		{
			nMaxNum = _nMaxNum ;
			nPushCount = 0 ;
			ptype = new T[nMaxNum] ;
		} ;
		T &operator [](int n)//�̰� ������. �ӵ��� �䱸�ϴ� ���� ������!
		{
			return ptype[n] ;
		} ;
		void push_back(T t)
		{
			assert("SArray overflow" && nPushCount < nMaxNum)  ;//assert �׽�Ʈ�� ������� ���Ѱ�� �׷��ϱ� ����� false�ΰ��
			ptype[nPushCount++] = t ;
		} ;
	} ;
	//�ѹ� �Էµ� �޸���ġ�� ����ϴ� �迭 ���������� �������� �Ҵ��� �ϴ� ����ü
	//��ü�� �����ɶ� �����޸��Ҵ��� �������� �����ϰ� ���� ���� �� ��ü�� �Ҵ�� �޸𸮸� Ǯ�������.
	template <typename T>
	struct SKeepingArray
	{
		int nMaxNum ;
		int nCurPos ;
		T **pptype ;
		CArrangedBuffer cBuffer ;

		SKeepingArray() {nMaxNum = 0; nCurPos = 0; pptype = NULL;}
		SKeepingArray(int nNum) {Initialize(nNum);}
		~SKeepingArray() {ReleaseAll();}
		void ReleaseAll() {if(pptype){delete []pptype; pptype=NULL;} nMaxNum=0; cBuffer.ReleaseAll();}
		void Initialize(int nNum)
		{
			nCurPos = 0 ;
			nMaxNum = nNum ;
			//ptype = new T[nMaxNum] ;
			pptype = new T*[nMaxNum] ;
			cBuffer.Initialize(nNum*sizeof(T), sizeof(T)) ;
		}
		T &operator [](int n) {return *(pptype[n]);}
		T *GetAt(int n) {return pptype[n];}
		int Find(T *pt)
		{
			for(int i=nCurPos-1; i>=0; i--)
			{
				if(pptype[i] == pt)
					return i ;
			}
			return -1 ;
		}
		void Insert(T *pt)
		{
			//T *p = GetEmpty() ;
			//memcpy((void *)p, (const void *)pt, sizeof(T)) ;
			assert(nCurPos < nMaxNum) ;
			void *p = cBuffer.AllocUnit() ;
			assert(p!=NULL) ;
			memcpy((void *)p, (const void *)pt, sizeof(T)) ;
			pptype[nCurPos++] = (T *)p ;
		}
		T *Delete(int nNum)//�����Լ����� ������ ��ü�� ������.
		{
			//pfDisable(&ptype[nNum]) ;
			T *ptype = pptype[nNum] ;
			cBuffer.DeleteUnit(pptype[nNum]) ;
			if(nCurPos > 1)
			{
				for(int i=nNum; i<nCurPos-1; i++)
					pptype[i] = pptype[i+1] ;
			}
			nCurPos-- ;
			return ptype ;
		}
		T *Delete(T *pt)
		{
			int n = Find(pt) ;
			if(n >= 0)
				return Delete(n) ;
			return NULL ;
		}

		void Reset()
		{
			nCurPos=0 ;
			cBuffer.DeleteAll() ;
		}
	} ;
	struct SCString
	{
		char str[256] ;

		SCString() {sprintf(str, "");}
		void format(char* lpFormatStr, ...) ;
		char *GetString() ;
	} ;
} ;

int ClassifyArray(int *pnArray, int nNumArray, data::SClassify **ppnClassify) ;