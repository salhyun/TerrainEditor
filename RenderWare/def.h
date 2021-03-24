#pragma once

#include "SecretDebugMsg.h"

#define MAXNUM_128 128
#define MAXNUM_256 256
#define MAXNUM_512 512
#define MAXNUM_1024 1024
#define MAXNUM_2048 2048

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if(p){delete p; p=NULL;}}
#endif

#ifndef SAFE_DELETEARRAY
#define SAFE_DELETEARRAY(p) {if(p){delete []p; p=NULL;}}
#endif

extern CSecretDebugMsg g_cDebugMsg ;

#if !defined(TRACE) && !defined(ATLTRACE)
void TRACE(char* lpFormatStr, ...) ;
#endif

#define DEBUGGING_FOR_RELEASE
void OutputStringforRelease(char* lpFormatStr, ...) ;


bool GePathfromFullPath(char *pszPath, char *pszFullPath) ;//���ϰ�θ� ���
bool GetFileNameFromPath(char *pszFileName, char *pszPath) ;//Ȯ���� ������ �̸�
void RemoveExt(char *pszName, char *pszFileName) ;//Ȯ���� ����

bool IsExistFile(char *pszFileName, char *pszPath) ;
bool IsStrStrInPath(char *pszName, char *pszPath, char *pszFileName) ;
int GetAllFileNames(char ***pppszFileNames, char *pszPath, char *pszExt, const bool bFullName=true) ;
int GetAllFolderNames(char ***pppszFileNames, char *pszPath, const bool bFullName=true) ;
bool SaveBitmapFile(char *pszFileName, int nWidth, int nHeight, unsigned long lBitCount, BYTE *pbyData) ;

void ConvertRGBtoHSI(float r, float g, float b, float &h, float &s, float &i) ;
void ConvertHSItoRGB(float h, float s, float i, float &r, float &g, float &b) ;