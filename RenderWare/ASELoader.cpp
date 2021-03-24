#include <assert.h>
#include <math.h>
#include "def.h"
#include "ASELoader.h"

const int identical = 0 ;
char szLine[256], szString[256], sztemp[256] ;
bool bAnimation=false, bKeyFrameAni=false, bSkinnedAni=false ;

CASELoader::CASELoader()
{
}
CASELoader::~CASELoader()
{
}

inline void farser_quotation(char *psz1, char *psz2)
{
	while(*psz2 != '\0')
	{
		if((*psz2) != '\"')
			*psz1++ = *psz2 ;
		psz2++ ;
	}
	*psz1 = '\0' ;
}
inline void read_aseline(FILE *pf, char *pszString, char *pszLine, int &nLineCount)
{
    fgets(pszLine, 256, pf) ;
	nLineCount++ ;
	sscanf(pszLine, "%s", pszString) ;
}

inline void getfilename(char *pszName, char *pszPath)//경로까지 지정된 파일명을 경로는 빼고 파일명만 읽어낸다.
{
	char *psz=NULL ;

	if(strchr(pszPath, '\\'))
	{
		psz = strrchr(pszPath, '\\')+1 ;
		memcpy(pszName, psz, (strchr(psz, '.')-psz+4)) ;//파일명다음에 '.'에서 확장자3글자까지 읽어낸다.
		pszName[(strchr(psz, '.')-psz+4)] = '\0' ;
	}
	else if(strchr(pszPath, '/'))
	{
		psz = strrchr(pszPath, '/')+1 ;
		memcpy(pszName, psz, (strchr(psz, '.')-psz+4)) ;//파일명다음에 '.'에서 확장자3글자까지 읽어낸다.
		pszName[(strchr(psz, '.')-psz+4)] = '\0' ;
	}
}

bool CASELoader::ReadASEFile(char *pszFileName, CASEData *pcASEData)
{
	FILE *pf=NULL ;

	bAnimation = bKeyFrameAni = bSkinnedAni = false ;

	pf = fopen(pszFileName, "r+") ;
	if(pf == NULL)
		return false ;

	m_nLineCount = 0 ;

	getfilename(pcASEData->m_szFileName, pszFileName) ;

	read_aseline(pf, szString, szLine, m_nLineCount) ;
	if(strcmp(szString, "*3DSMAX_ASCIIEXPORT") != identical)
        return false ;

	TRACE("############################\r\n") ;
	TRACE("Begin ReadASE \"%s\"\r\n", pszFileName) ;
	TRACE("############################\r\n") ;	

    while(!feof(pf))
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*COMMENT") == identical)
			continue ;

		//*SCENE {
		if(strcmp(szString, "*SCENE") == identical)
		{
			ReadSCENE(pf, &pcASEData->m_sSceneInfo) ;
			continue ;
		}

		if(strcmp(szString, "*MATERIAL_LIST") == identical)
		{
			ReadMATERIAL_LIST(pf, pcASEData) ;
			continue ;
		}

		if(strcmp(szString, "*GEOMOBJECT") == identical)
		{
			ReadGEOMOBJECT(pf, pcASEData) ;
			continue ;
		}
		if(strcmp(szString, "*HELPEROBJECT") == identical)
		{
			ReadGEOMOBJECT(pf, pcASEData) ;
			continue ;
		}
		if(strcmp(szString, "*GROUP") == identical)
		{
			ReadGROUP(pf, pcASEData) ;
			continue ;
		}
		if(strcmp(szString, "*SHAPEOBJECT") == identical)
		{
			ReadSHAPEOBJECT(pf, pcASEData) ;
			continue ;
		}
	}

	TRACE("############################\r\n") ;
	TRACE("End ReadASE \"%s\"\r\n", pszFileName) ;
	TRACE("############################\r\n") ;	

	if(bAnimation)
	{
		pcASEData->m_lAttr |= CASEData::ATTR_ANI ;
		if(bSkinnedAni)
			pcASEData->m_lSubAttr |= CASEData::SUBATTR_ANI_SKINNED ;
		else
			pcASEData->m_lSubAttr |= CASEData::SUBATTR_ANI_KEYFRAME ;
	}
	else
		pcASEData->m_lAttr |= CASEData::ATTR_SOLID ;

	fclose(pf) ;
    return true ;
}

bool CASELoader::ReadSCENE(FILE *pf, SSceneInfo *psSceneInfo)
{
	//SCENE_FILENAME
	read_aseline(pf, szString, szLine, m_nLineCount) ;

    //SCENE_FIRSTFRAME
	fgets(szLine, 256, pf) ;
	m_nLineCount++ ;
	sscanf(szLine, "%s%f", szString, &psSceneInfo->fFirstFrame) ;

	//SCENE_LASTFRAME
	fgets(szLine, 256, pf) ;
	m_nLineCount++ ;
	sscanf(szLine, "%s%f", szString, &psSceneInfo->fLastFrame) ;

    //SCENE_FRAMESPEED
	fgets(szLine, 256, pf) ;
	m_nLineCount++ ;
	sscanf(szLine, "%s%f", szString, &psSceneInfo->fFrameSpeed) ;

	//SCENE_TICKSPERFRAME
	fgets(szLine, 256, pf) ;
	m_nLineCount++ ;
	sscanf(szLine, "%s%f", szString, &psSceneInfo->fTicksPerFrame) ;

    //SCENE_BACKGROUND_STATIC
	read_aseline(pf, szString, szLine, m_nLineCount) ;

	//SCENE_AMBIENT_STATIC
	read_aseline(pf, szString, szLine, m_nLineCount) ;

	//Read "}"
	read_aseline(pf, szString, szLine, m_nLineCount) ;

	return true ;
}

bool CASELoader::ReadMATERIAL_LIST(FILE *pf, CASEData *pcASEData)
{
	int i, nCount ;

	//*MATERIAL_COUNT
	fgets(szLine, 256, pf) ;
	m_nLineCount++ ;
	sscanf(szLine, "%s%d", szString, &pcASEData->m_nNumMaterial) ;

	assert(pcASEData->m_nNumMaterial <= MAXNUM_MATERIAL && "NumMaterial is less than 256") ;

	pcASEData->m_psMaterial = new SMaterial[pcASEData->m_nNumMaterial] ;

	for(i=0 ; i<pcASEData->m_nNumMaterial ; i++)
	{
		//*MATERIAL x {
		fgets(szLine, 256, pf) ;
		m_nLineCount++ ;
		sscanf(szLine, "%s%d", szString, &nCount) ;

		ReadMATERIAL(pf, &pcASEData->m_psMaterial[nCount]) ;
	}

    //Read "}"
	read_aseline(pf, szString, szLine, m_nLineCount) ;
	
	return true ;
}

bool CASELoader::ReadMATERIAL(FILE *pf, SMaterial *psMaterial)
{
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*MATERIAL_AMBIENT") == identical)
		{
			sscanf(szLine, "%s%f%f%f", szString, &psMaterial->sAmbient.r, &psMaterial->sAmbient.g, &psMaterial->sAmbient.b) ;
			continue ;
		}
        if(strcmp(szString, "*MATERIAL_DIFFUSE") == identical)
		{
			sscanf(szLine, "%s%f%f%f", szString, &psMaterial->sDiffuse.r, &psMaterial->sDiffuse.g, &psMaterial->sDiffuse.b) ;
			continue ;
		}
		if(strcmp(szString, "*MATERIAL_SPECULAR") == identical)
		{
			sscanf(szLine, "%s%f%f%f", szString, &psMaterial->sSpecular.r, &psMaterial->sSpecular.g, &psMaterial->sSpecular.b) ;
			continue ;
		}
		if(strcmp(szString, "*MATERIAL_TWOSIDED") == identical)
		{
			psMaterial->nAttr |= MATERIAL_ATTR_TWOSIDED ;
			continue ;
		}

        if(strcmp(szString, "*NUMSUBMTLS") == identical)
		{
			int nCount ;
			sscanf(szLine, "%s%d", szString, &psMaterial->nNumSubMaterial) ;

			if(psMaterial->nNumSubMaterial)
			{
				assert(psMaterial->nNumSubMaterial <= MAXNUM_MATERIAL && "NumSubMaterial is less than 256") ;

				psMaterial->psSubMaterial = new SMaterial[psMaterial->nNumSubMaterial] ;

				for(int i=0 ; i<psMaterial->nNumSubMaterial ; i++)
				{
					//*SUBMATERIAL x {
					fgets(szLine, 256, pf) ;
					m_nLineCount++ ;
					sscanf(szLine, "%s%d", szString, &nCount) ;

					ReadMATERIAL(pf, &psMaterial->psSubMaterial[nCount]) ;
				}
			}
			continue ;
		}

		if(strcmp(szString, "*MAP_DIFFUSE") == identical)
		{
			ReadMAP_DIFFUSE(pf, psMaterial) ;
			continue ;
		}
		if(strcmp(szString, "*MAP_BUMP") == identical)//노말맵을 있을경우
		{
			ReadMAP_BUMP(pf, psMaterial) ;
			continue ;
		}

		if(strcmp(szString, "*MAP_OPACITY") == identical)
		{
			ReadMAP_OPACITY(pf) ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
			return true ;
	}

	return false ;
}

bool CASELoader::ReadMAP_DIFFUSE(FILE *pf, SMaterial *psMaterial)//텍스쳐파일을 저장하는것말고는 없다.
{
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*BITMAP") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			//sscanf(szLine, "%s%s", szString, sz) ;
			getfilename(psMaterial->szDecaleName, sz) ;

			TRACE("TEXTURE BITMAP %s\r\n", psMaterial->szDecaleName) ;

			psMaterial->bUseTexDecal = true ;
			continue ;
		}
		if(strcmp(szString, "*UVW_U_TILING") == identical)
		{
			sscanf(szLine, "%s%f", szString, &psMaterial->sTexTiling.u) ;
			continue ;
		}
		if(strcmp(szString, "*UVW_V_TILING") == identical)
		{
			sscanf(szLine, "%s%f", szString, &psMaterial->sTexTiling.v) ;
			continue ;
		}
        
		if(strcmp(szString, "}") == identical)
			return true ;
	}

	return false ;
}
bool CASELoader::ReadMAP_BUMP(FILE *pf, SMaterial *psMaterial)//노말맵
{
	bool bMap_Generic=false ;

	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*MAP_GENERIC") == identical)
			bMap_Generic = true ;

		if(strcmp(szString, "*BITMAP") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			//sscanf(szLine, "%s%s", szString, sz) ;
			getfilename(psMaterial->szBumpName, sz) ;
			psMaterial->bUseNormalMap = true ;                        //노말맵 사용한다고 명시한다.

			TRACE("BUMP BITMAP %s\r\n", psMaterial->szBumpName) ;
			continue ;
		}
        
		if(strcmp(szString, "}") == identical)
		{
			if(bMap_Generic)
				bMap_Generic = false ;
			else
				return true ;
		}
	}

	return false ;
}

bool CASELoader::ReadMAP_OPACITY(FILE *pf)
{
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "}") == identical)
			return true ;
	}

	return false ;
}

bool CASELoader::ReadGEOMOBJECT(FILE *pf, CASEData *pcASEData, int nAttr)
{
    SMesh *pMesh = new SMesh ;

	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*NODE_NAME") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			sprintf(pMesh->szMeshName, "%s", sz) ;

			//sscanf(szLine, "%s%s", szString, sztemp) ;
			//farser_quotation(pMesh->szMeshName, sztemp) ;

			if(strstr(pMesh->szMeshName, "Bone") || strstr(pMesh->szMeshName, "Bip"))//메쉬가 '본'이나 '바이패드'일경우
			{
				pMesh->MeshKind = SMESHKIND_BONE ;

				char *pszBoneName = new char[256] ;
				sprintf(pszBoneName, "%s", pMesh->szMeshName) ;
				pcASEData->m_apszBoneName[pcASEData->m_nNumBone++] = pszBoneName ;

				TRACE("BoneList[%d] BoneName=%s\r\n", pcASEData->m_nNumBone-1, pcASEData->m_apszBoneName[pcASEData->m_nNumBone-1]) ;
			}
			else if(strstr(pMesh->szMeshName, "Dummy"))                    //HelperObject
			{
				pMesh->MeshKind = SMESHKIND_DUMMY ;
				pcASEData->m_bUseDummyObject = true ;
			}
			else if(strstr(pMesh->szMeshName, "XBillboard") || strstr(pMesh->szMeshName, "xbillboard"))
				pMesh->MeshKind = SMESHKIND_XBILLBOARD ;
			else if(strstr(pMesh->szMeshName, "Billboard") || strstr(pMesh->szMeshName, "billboard"))
				pMesh->MeshKind = SMESHKIND_BILLBOARD ;
			else
                pMesh->MeshKind = SMESHKIND_RIGID ;

			continue ;
		}
		if(strcmp(szString, "*HELPER_CLASS") == identical)
		{
			if(strstr(szLine, "Dummy") || strstr(szLine, "Particle View") || strstr(szLine, "Tape"))
			{
				pMesh->MeshKind = SMESHKIND_DUMMY ;
				pcASEData->m_bUseDummyObject = true ;
			}
		}
		if(strcmp(szString, "*NODE_PARENT") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			sprintf(pMesh->szParentName, "%s", sz) ;

			pMesh->bParent = true ;

			//sscanf(szLine, "%s%s", szString, sztemp) ;
			//farser_quotation(pMesh->szParentName, sztemp) ;
			continue ;
		}

		if(strcmp(szString, "*NODE_TM") == identical)
		{
			ReadNODE_TM(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*MESH") == identical)
		{
			ReadMESH(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*MATERIAL_REF") == identical)
		{
			sscanf(szLine, "%s%d", szString, &pMesh->nMaterialIndex) ;

			if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial > 0)//SubMaterial을 사용하는 Mesh인경우
				pMesh->nAttr |= SMESHATTR_USESUBMATERIAL ;

			//if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].bUseNormalMap)//NormalMap 사용하는 Mesh인 경우
			//{
			//	pMesh->nAttr |= SMESHATTR_USENORMALMAP ;
			//}

			if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial > 0)
			{
				//int btwosided=false ;
				//for(int n=0 ; n<pcASEData->m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial ; n++)
				//{
				//	if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].psSubMaterial[n].nAttr & MATERIAL_ATTR_TWOSIDED)
				//		btwosided = true ;
				//}
				//if(btwosided)
				//	pMesh->nAttr |= SMESHATTR_TWOSIDED ;
			}
			else
			{
				if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].nAttr & MATERIAL_ATTR_TWOSIDED)
					pMesh->nAttr |= SMESHATTR_TWOSIDED ;
			}

			continue ;
		}

		if(strcmp(szString, "*TM_ANIMATION") == identical)
		{
			bAnimation = true ;
			pMesh->nAttr |= SMESHATTR_ANIMATION ;
			ReadTM_ANIMATION(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*SKIN") == identical)
		{
			bSkinnedAni = true ;
			pMesh->MeshKind = SMESHKIND_SKINNING ;
			ReadSKIN(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*PHYSIQUE") == identical)
		{
			bSkinnedAni = true ;
			pMesh->MeshKind = SMESHKIND_SKINNING ;
			ReadSKIN(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(pMesh->bParent)//이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
			{
				int nIndex ;
				SMesh *psMeshParent ;

				pcASEData->FindMesh(pMesh->szParentName, &psMeshParent, nIndex) ;

				assert((psMeshParent != NULL) && "Cannot Find Parent") ;

				if((nAttr & SMESHATTR_GROUPED) && (psMeshParent->MeshKind == SMESHKIND_DUMMY))//그룹에 속해있고 더미인 부모는 취급안함.
				{
					pMesh->bParent = false ;
				}
				else
				//if(psMeshParent)
				{
					pMesh->psMeshParent = psMeshParent ;
					pMesh->nMeshParentIndex = nIndex ;
				}
			}

			pcASEData->m_apMesh[pcASEData->m_nMeshNode++] = pMesh ;
			pcASEData->m_nNumMesh++ ;

			return true ;
		}
	}

    assert(false && "Read failure GEOMOBJECT") ;
	return false ;
}

bool CASELoader::ReadNODE_TM(FILE *pf, SMesh *pMesh)
{
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*TM_ROW0") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->smatLocal.m11, &pMesh->smatLocal.m13, &pMesh->smatLocal.m12) ;
		if(strcmp(szString, "*TM_ROW1") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->smatLocal.m31, &pMesh->smatLocal.m33, &pMesh->smatLocal.m32) ;
		if(strcmp(szString, "*TM_ROW2") == identical)
            sscanf(szLine, "%s%f%f%f", szString, &pMesh->smatLocal.m21, &pMesh->smatLocal.m23, &pMesh->smatLocal.m22) ;			
		if(strcmp(szString, "*TM_ROW3") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->smatLocal.m41, &pMesh->smatLocal.m43, &pMesh->smatLocal.m42) ;

		pMesh->smatLocal.m14 = pMesh->smatLocal.m24 = pMesh->smatLocal.m34 = 0.0f ;
		pMesh->smatLocal.m44 = 1.0f ;

		if(strcmp(szString, "*TM_POS") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->svPos.x, &pMesh->svPos.z, &pMesh->svPos.y) ;

		if(strcmp(szString, "*TM_ROTAXIS") == identical)
			sscanf(szLine, "%s%f", szString, &pMesh->svRotAxis.x, &pMesh->svRotAxis.z, &pMesh->svRotAxis.y) ;

		if(strcmp(szString, "*TM_ROTANGLE") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->fRotAngle) ;

		if(strcmp(szString, "*TM_SCALE") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &pMesh->svScale.x, &pMesh->svScale.z, &pMesh->svScale.y) ;

		if(strcmp(szString, "}") == identical)
            return true ;
	}
    return false ;
}
bool CASELoader::ReadNODE_TM(FILE *pf, SShapeObject *psShapeObject)
{
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*TM_ROW0") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->smatLocal.m11, &psShapeObject->smatLocal.m13, &psShapeObject->smatLocal.m12) ;
		if(strcmp(szString, "*TM_ROW1") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->smatLocal.m31, &psShapeObject->smatLocal.m33, &psShapeObject->smatLocal.m32) ;
		if(strcmp(szString, "*TM_ROW2") == identical)
            sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->smatLocal.m21, &psShapeObject->smatLocal.m23, &psShapeObject->smatLocal.m22) ;			
		if(strcmp(szString, "*TM_ROW3") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->smatLocal.m41, &psShapeObject->smatLocal.m43, &psShapeObject->smatLocal.m42) ;

		psShapeObject->smatLocal.m14 = psShapeObject->smatLocal.m24 = psShapeObject->smatLocal.m34 = 0.0f ;
		psShapeObject->smatLocal.m44 = 1.0f ;

		if(strcmp(szString, "*TM_POS") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->svPos.x, &psShapeObject->svPos.z, &psShapeObject->svPos.y) ;

		if(strcmp(szString, "*TM_ROTAXIS") == identical)
			sscanf(szLine, "%s%f", szString, &psShapeObject->svRotAxis.x, &psShapeObject->svRotAxis.z, &psShapeObject->svRotAxis.y) ;

		if(strcmp(szString, "*TM_ROTANGLE") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->fRotAngle) ;

		if(strcmp(szString, "*TM_SCALE") == identical)
			sscanf(szLine, "%s%f%f%f", szString, &psShapeObject->svScale.x, &psShapeObject->svScale.z, &psShapeObject->svScale.y) ;

		if(strcmp(szString, "}") == identical)
            return true ;
	}
    return false ;
}
bool CASELoader::ReadMESH(FILE *pf, SMesh *pMesh)
{
	bool bResult ;
	int nNumTexCoord=0 ;
	SMeshTexCoord *psTexCoord=NULL ;

	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*MESH_NUMVERTEX") == identical)
		{
			sscanf(szLine, "%s%d", szString, &pMesh->nNumVertex) ;
			pMesh->psVertex = new SMeshVertex[pMesh->nNumVertex] ;
			memset(pMesh->psVertex, 0, sizeof(SMeshVertex)*pMesh->nNumVertex) ;
			continue ;
		}

        if(strcmp(szString, "*MESH_NUMFACES") == identical)
		{
			sscanf(szLine, "%s%d", szString, &pMesh->nNumTriangle) ;
			pMesh->psTriangle = new SMeshTriangle[pMesh->nNumTriangle] ;
			memset(pMesh->psTriangle, 0, sizeof(SMeshTriangle)*pMesh->nNumTriangle) ;
			continue ;
		}

		if(strcmp(szString, "*MESH_VERTEX_LIST") == identical)
		{
			bResult = ReadMESH_VERTEX_LIST(pf, pMesh) ;
			assert(bResult && "Read Failure ReadMESH_VERTEX_LIST") ;
			continue ;
		}

		if(strcmp(szString, "*MESH_FACE_LIST") == identical)
		{
			bResult = ReadMESH_FACE_LIST(pf, pMesh) ;
			assert(bResult && "Read Failure ReadMESH_FACE_LIST") ;
			continue ;
		}

        if(strcmp(szString, "*MESH_NUMTVERTEX") == identical)
		{
			sscanf(szLine, "%s%d", szString, &nNumTexCoord) ;
			psTexCoord = new SMeshTexCoord[nNumTexCoord] ;
			memset(psTexCoord, 0, sizeof(SMeshTexCoord)*nNumTexCoord) ;
			continue ;
		}

        if(strcmp(szString, "*MESH_TVERTLIST") == identical)
		{
			bResult = ReadMESH_TVERTLIST(pf, psTexCoord, nNumTexCoord) ;
			assert(bResult && "Read Failure ReadMESH_TVERTLIST") ;
			continue ;
		}

//		if(strcmp(szString, "*MESH_NUMTVFACES") == identical)

		if(strcmp(szString, "*MESH_TFACELIST") == identical)
		{
			bResult = ReadMESH_TFACELIST(pf, pMesh, psTexCoord, nNumTexCoord) ;
			assert(bResult && "Read Failure ReadMESH_TFACELIST") ;
			continue ;
		}

		if(strcmp(szString, "*MESH_CVERTLIST") == identical)//이건 사용하지 않을것임.
		{
			bResult = ReadMESH_CVERTLIST(pf, pMesh) ;
			assert(bResult && "Read Failure ReadMESH_CVERTLIST") ;
			continue ;
		}
		if(strcmp(szString, "*MESH_CFACELIST") == identical)//이것도 사용하지 않을것임.
		{
			bResult = ReadMESH_CVERTLIST(pf, pMesh) ;
			assert(bResult && "Read Failure ReadMESH_MESH_CFACELIST") ;
			continue ;
		}

		if(strcmp(szString, "*MESH_MAPPINGCHANNEL") == identical)
		{
			bResult = ReadMESH_MAPPINGCHANNEL(pf) ;
			assert(bResult && "Read Failure ReadMESH_MAPPINGCHANNEL") ;
			continue ;
		}

		if(strcmp(szString, "*MESH_NORMALS") == identical)
		{
			bResult = ReadMESH_NORMALS(pf, pMesh) ;
			assert(bResult && "Read Failure ReadMESH_NORMALS") ;
			continue ;
		}

        if(strcmp(szString, "*TM_ANIMATION") == identical)
		{
			bResult = ReadTM_ANIMATION(pf, pMesh) ;
			assert(bResult && "Read Failure ReadTM_ANIMATION") ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			break ;
		}
	}

	if(psTexCoord)
		delete []psTexCoord ;

	return false ;
}

bool CASELoader::ReadMESH_VERTEX_LIST(FILE *pf, SMesh *pMesh)
{
	int count=0 ;
    SMeshVector v ;
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*MESH_VERTEX") == identical)
		{
			sscanf(szLine, "%s%d%f%f%f", szString, &count, &v.x, &v.z, &v.y) ;
			pMesh->psVertex[count].pos = v ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(count == (pMesh->nNumVertex-1))//인덱스 갯수와 NUMVERTEX와 일치해야 성공
				return true ;
			break ;
		}
	}

    return false ;
}

bool CASELoader::ReadMESH_FACE_LIST(FILE *pf, SMesh *pMesh)
{
	//char str[256] ;
	char *psz ;
	int count=0, anIndex[3] ;
	//int ntemp, nMaterialID, nSmooth ;
	int nMaterialID ;

	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*MESH_FACE") == identical)
		{
			//sscanf(szLine, "%s%d%s%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d", szString, &count, str,
			//	str, &anIndex[0],
			//	str, &anIndex[2],
			//	str, &anIndex[1],
			//	str, &ntemp,
			//	str, &ntemp,
			//	str, &ntemp,
			//	str, &nSmooth,
			//	str, &nMaterialID
			//	) ;

            psz = strstr(szLine, "*MESH_FACE")+10 ;
			sscanf(psz, "%d", &count) ;

			psz = strstr(szLine, "A:")+2 ;
			sscanf(psz, "%d", &anIndex[0]) ;
			psz = strstr(szLine, "B:")+2 ;
			sscanf(psz, "%d", &anIndex[2]) ;
			psz = strstr(szLine, "C:")+2 ;
			sscanf(psz, "%d", &anIndex[1]) ;

            psz = strstr(szLine, "*MESH_MTLID")+11 ;
			sscanf(psz, "%d", &nMaterialID) ;

			//TRACE("count=%d index=(%d %d %d) mtlID=%d\r\n", count, anIndex[0], anIndex[1], anIndex[2], nMaterialID) ;

			pMesh->psTriangle[count].anVertexIndex[0] = anIndex[0] ;
			pMesh->psTriangle[count].anVertexIndex[1] = anIndex[1] ;
			pMesh->psTriangle[count].anVertexIndex[2] = anIndex[2] ;

			pMesh->psTriangle[count].nMaterialID = nMaterialID ;

			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(count == (pMesh->nNumTriangle-1))
				return true ;
			break ;
		}
	}

    return false ;
}

bool CASELoader::ReadMESH_TVERTLIST(FILE *pf, SMeshTexCoord *psTexCoord, int nNumTexCoord)
{
	int count=0 ;
    float u, v, w ;
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*MESH_TVERT") == identical)
		{
			sscanf(szLine, "%s%d%f%f%f", szString, &count, &u, &v, &w) ;//w : 3차원 텍스쳐좌표

			psTexCoord[count].u = u ;
			psTexCoord[count].v = 1.0f-v ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(count == (nNumTexCoord-1))
				return true ;
			break ;
		}
	}

    return false ;
}

bool CASELoader::ReadMESH_TFACELIST(FILE *pf, SMesh *pMesh, SMeshTexCoord *psTexCoord, int nNumTexCoord)
{
	int count=0, anIndex[3] ;
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*MESH_TFACE") == identical)
		{
			sscanf(szLine, "%s%d%d%d%d", szString, &count, &anIndex[0], &anIndex[2], &anIndex[1]) ;

			pMesh->psTriangle[count].asTexCoord[0] = psTexCoord[anIndex[0]] ;
			pMesh->psTriangle[count].asTexCoord[1] = psTexCoord[anIndex[1]] ;
			pMesh->psTriangle[count].asTexCoord[2] = psTexCoord[anIndex[2]] ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(count == (pMesh->nNumTriangle-1))
				return true ;
			break ;
		}
	}

    return false ;
}

bool CASELoader::ReadMESH_NORMALS(FILE *pf, SMesh *pMesh)
{
	int count=0, ntemp ;
    SMeshVector v ;
	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*MESH_FACENORMAL") == identical)
		{
			sscanf(szLine, "%s%d%f%f%f", szString, &count, &v.x, &v.z, &v.y) ;
			pMesh->psTriangle[count].sFaceNormal = v ;

			fgets(szLine, 256, pf) ;
			m_nLineCount++ ;
			sscanf(szLine, "%s%d%f%f%f", szString,  &ntemp, &v.x, &v.z, &v.y) ;

			pMesh->psTriangle[count].asVertexNormal[0] = v ;

//			TRACE("normal : x=%.3f y=%.3f z=%.3f\r\n",
//				pMesh->psTriangle[count].asVertexNormal[0].x,
//				pMesh->psTriangle[count].asVertexNormal[0].y,
//				pMesh->psTriangle[count].asVertexNormal[0].z) ;

			fgets(szLine, 256, pf) ;
			m_nLineCount++ ;
			sscanf(szLine, "%s%d%f%f%f", szString,  &ntemp, &v.x, &v.z, &v.y) ;

			pMesh->psTriangle[count].asVertexNormal[2] = v ;

//			TRACE("normal : x=%.3f y=%.3f z=%.3f\r\n",
//				pMesh->psTriangle[count].asVertexNormal[1].x,
//				pMesh->psTriangle[count].asVertexNormal[1].y,
//				pMesh->psTriangle[count].asVertexNormal[1].z) ;

			fgets(szLine, 256, pf) ;
			m_nLineCount++ ;
			sscanf(szLine, "%s%d%f%f%f", szString,  &ntemp, &v.x, &v.z, &v.y) ;

			pMesh->psTriangle[count].asVertexNormal[1] = v ;

//			TRACE("normal : x=%.3f y=%.3f z=%.3f\r\n",
//				pMesh->psTriangle[count].asVertexNormal[2].x,
//				pMesh->psTriangle[count].asVertexNormal[2].y,
//				pMesh->psTriangle[count].asVertexNormal[2].z) ;

            continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(count == (pMesh->nNumTriangle-1))
				return true ;
			break ;
		}
	}

    return false ;
}

bool CASELoader::ReadTM_ANIMATION(FILE *pf, SMesh *pMesh)
{
    while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*CONTROL_POS_TRACK") == identical)
		{
			ReadCONTROL_POS_TRACK(pf, pMesh) ;
			continue ;
		}

        if(strcmp(szString, "*CONTROL_ROT_TRACK") == identical)
		{
			ReadCONTROL_ROT_TRACK(pf, pMesh) ;
			continue ;
		}
		if(strcmp(szString, "*CONTROL_ROT_TCB") == identical)
		{
			ReadCONTROL_ROT_TCB(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}

	return false ;
}

bool CASELoader::ReadCONTROL_POS_TRACK(FILE *pf, SMesh *pMesh)
{
	int nFrame ;
	SMeshVector v ;
	SMeshKeyPos *pMeshKeyPos ;

	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*CONTROL_POS_SAMPLE") == identical)//여기 CONTROL_POS_SAMPLE는 절대좌표 frame, x, z, y
		{
			sscanf(szLine, "%s%d%f%f%f", szString,  &nFrame, &v.x, &v.z, &v.y) ;

			pMeshKeyPos = new SMeshKeyPos ;
            
			pMeshKeyPos->nFrame = nFrame ;
			pMeshKeyPos->pos = v ;

			pMesh->apMeshKeyPos[pMesh->nNumMeshKeyPos++] = pMeshKeyPos ;

			//TRACE("POS_TRACK[%d] Frame:%d pos: x=%.3f y=%.3f z=%.3f\r\n",
			//	pMesh->nNumMeshKeyPos-1, pMesh->apMeshKeyPos[pMesh->nNumMeshKeyPos-1]->nFrame,
			//	pMesh->apMeshKeyPos[pMesh->nNumMeshKeyPos-1]->pos.x,
			//	pMesh->apMeshKeyPos[pMesh->nNumMeshKeyPos-1]->pos.y,
			//	pMesh->apMeshKeyPos[pMesh->nNumMeshKeyPos-1]->pos.z) ;
		}		

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}

	return false ;
}

bool CASELoader::ReadCONTROL_ROT_TRACK(FILE *pf, SMesh *pMesh)
{
	int nFrame ;
	float ftheta, x, y, z, w ;
	SMeshQuaternion qRecv, qSum ;
	SMeshKeyRot *pMeshKeyRot ;

	TRACE("### %s ROT_TRACK is Begin###\r\n", pMesh->szMeshName) ;

    while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

        if(strcmp(szString, "*CONTROL_ROT_SAMPLE") == identical)//여기 CONTROL_ROT_SAMPLE는 전 프레임의 상대좌표 frame x, z, y, angle
		{
			sscanf(szLine, "%s%d%f%f%f%f", szString,  &nFrame, &x, &z, &y, &w) ;

			ftheta = w/2.0f ;
			qRecv.set(sinf(ftheta)*x, sinf(ftheta)*y, sinf(ftheta)*z, cosf(ftheta)) ;

            qSum *= qRecv ;

			pMeshKeyRot = new SMeshKeyRot ;

			pMeshKeyRot->nFrame = nFrame ;
			pMeshKeyRot->q = qSum ;

			pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot++] = pMeshKeyRot ;

			//TRACE("ROT_SAMPLE[%d] Frame:%d axis: x=%.3f y=%.3f z=%.3f angle: %.3f\r\n",
			//	pMesh->nNumMeshKeyRot-1, pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->nFrame,
			//	pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.x,
			//	pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.y,
			//	pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.z,
			//	pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.w) ;
		}		

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}

	TRACE("### %s ROT_TRACK is end###\r\n", pMesh->szMeshName) ;

	return false ;
}

bool CASELoader::ReadCONTROL_ROT_TCB(FILE *pf, SMesh *pMesh)
{
	return true ;
	//int nFrame ;
	//float ftheta, x, y, z, w ;
	//SMeshQuaternion qRecv, qSum ;
	//SMeshKeyRot *pMeshKeyRot ;

	//while(true)
	//{
	//	read_aseline(pf, szString, szLine, m_nLineCount) ;

	//	if(strcmp(szString, "*CONTROL_TCB_ROT_KEY") == identical)
	//	{
	//		sscanf(szLine, "%s%d%f%f%f%f", szString,  &nFrame, &x, &z, &y, &w) ;

	//		ftheta = w/2.0f ;
	//		qRecv.set(sinf(ftheta)*x, sinf(ftheta)*y, sinf(ftheta)*z, cosf(ftheta)) ;

	//		qSum *= qRecv ;

	//		pMeshKeyRot = new SMeshKeyRot ;

	//		pMeshKeyRot->nFrame = nFrame ;
	//		pMeshKeyRot->q = qSum ;

	//		pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot++] = pMeshKeyRot ;

	//		TRACE("TCB_ROT_KEY[%d] Frame:%d axis: x=%.3f y=%.3f z=%.3f angle: %.3f\r\n",
	//			pMesh->nNumMeshKeyRot-1, pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->nFrame,
	//			pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.x,
	//			pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.y,
	//			pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.z,
	//			pMesh->apMeshKeyRot[pMesh->nNumMeshKeyRot-1]->q.w) ;
	//	}		

	//	if(strcmp(szString, "}") == identical)
	//	{
	//		return true ;
	//	}
	//}

	//return false ;
}

bool CASELoader::ReadSKIN(FILE *pf, SMesh *pMesh)
{
	int nVertexIndex ;
	char sztemp[256], szBoneName[256] ;
	float fWeight ;
	SMeshSkin *psSkin ;
	SMeshVertex *psVertex ;

	while(true)
	{
        read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*VERTEX") == identical)
		{
			sscanf(szLine, "%s%s%d%s%f%s%s", szString, sztemp, &nVertexIndex, sztemp, &fWeight, sztemp, szBoneName) ;

			psSkin = new SMeshSkin ;

			psSkin->fWeight = fWeight ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(psSkin->szBoneName, pszStart, pszEnd-pszStart) ;
			psSkin->szBoneName[pszEnd-pszStart] = '\0' ;
			psSkin->bEnable = true ;

			psVertex = &pMesh->psVertex[nVertexIndex] ;
			psVertex->apsSkin[psVertex->nNumSkin++] = psSkin ;

			//TRACE("VertexIndex[%d] Weight=%f BoneName=%s\r\n", psSkin->nVertexIndex, psSkin->fWeight, psSkin->szBoneName) ;
		}

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}
}
bool CASELoader::ReadSHAPE_LINE(FILE *pf, SShapeLine *psShapeLine)
{
	int ntemp ;
	SMeshVector pos ;
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*SHAPE_CLOSED") == identical)
		{
			psShapeLine->bClosed = true ;
			continue ;
		}
		if(strcmp(szString, "*SHAPE_VERTEXCOUNT") == identical)
		{
			sscanf(szLine, "%s%d", szString, &psShapeLine->nNumPos) ;
			psShapeLine->psPos = new SMeshVector[psShapeLine->nNumPos] ;
			continue ;
		}
		if(strcmp(szString, "*SHAPE_VERTEX_KNOT") == identical)
		{
			sscanf(szLine, "%s%d%f%f%f", szString, &ntemp, &pos.x, &pos.z, &pos.y) ;
			psShapeLine->psPos[ntemp] = pos ;
			continue ;
		}
		if(strcmp(szString, "*SHAPE_VERTEX_INTERP") == identical)
		{
			sscanf(szLine, "%s%d%f%f%f", szString, &ntemp, &pos.x, &pos.z, &pos.y) ;
			psShapeLine->psPos[ntemp] = pos ;
			continue ;
		}
		if(strcmp(szString, "}") == identical)
			return true ;
	}
	return false ;
}
bool CASELoader::ReadSHAPEOBJECT(FILE *pf, CASEData *pcASEData)
{
	int ntemp ;
	SShapeObject *psShapeObject = new SShapeObject() ;

    while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*NODE_NAME") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			sprintf(psShapeObject->szName, "%s", sz) ;
			continue ;
		}
		if(strcmp(szString, "*NODE_TM") == identical)
		{
			ReadNODE_TM(pf, psShapeObject) ;
			continue ;
		}
		if(strcmp(szString, "*SHAPE_LINECOUNT") == identical)
		{
			sscanf(szLine, "%s%d", szString, &psShapeObject->nNumLine) ;
			psShapeObject->psShapeLine = new SShapeLine[psShapeObject->nNumLine] ;
			continue ;
		}
		if(strcmp(szString, "*SHAPE_LINE") == identical)
		{
            sscanf(szLine, "%s%d", szString, &ntemp) ;
			ReadSHAPE_LINE(pf, &psShapeObject->psShapeLine[ntemp]) ;
			continue ;
		}
		if(strcmp(szString, "}") == identical)
		{
			pcASEData->m_apsShapeObject[pcASEData->m_nNumShapeObject++] = psShapeObject ;
			pcASEData->m_lKind = CASEData::KIND_SHAPE ;
			return true ;
		}
	}
	return false ;
}

//just pass
bool CASELoader::ReadMESH_MAPPINGCHANNEL(FILE *pf)
{
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		//be *MESH_TVERTLIST
		if(strcmp(szString, "*MESH_TVERTLIST") == identical)
		{
			while(true)
			{
				read_aseline(pf, szString, szLine, m_nLineCount) ;
				if(strcmp(szString, "}") == identical)
				{
					read_aseline(pf, szString, szLine, m_nLineCount) ;
					break ;
				}
			}
		}
		//be *MESH_TFACELIST
		if(strcmp(szString, "*MESH_TFACELIST") == identical)
		{
			while(true)
			{
				read_aseline(pf, szString, szLine, m_nLineCount) ;
				if(strcmp(szString, "}") == identical)
				{
					read_aseline(pf, szString, szLine, m_nLineCount) ;
					break ;
				}
			}
		}

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}
	return false ;
}
//just pass
bool CASELoader::ReadMESH_CVERTLIST(FILE *pf, SMesh *pMesh)
{
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*MESH_VERTCOL") == identical)
		{
		}

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}
    return false ;
}
//just pass
bool CASELoader::ReadMESH_CFACELIST(FILE *pf, SMesh *pMesh)
{
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*MESH_CFACE") == identical)
		{
		}

		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}
    return false ;
}

bool CASELoader::ReadGROUP(FILE *pf, CASEData *pcASEData)
{
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*HELPEROBJECT") == identical)
		{
			ReadGEOMOBJECT(pf, pcASEData, SMESHATTR_GROUPED) ;
			continue ;
		}
		if(strcmp(szString, "*GROUP") == identical)
		{
			ReadGROUP(pf, pcASEData) ;
			continue ;
		}
		if(strcmp(szString, "*GEOMOBJECT") == identical)
		{
			ReadGEOMOBJECT(pf, pcASEData, SMESHATTR_GROUPED) ;
			continue ;
		}
		if(strcmp(szString, "}") == identical)
		{
			return true ;
		}
	}
	return false ;
}

/*
bool ReadSHAPEOBJECT(FILE *pf)
{
	while(true)
	{
		read_aseline(pf, szString, szLine, m_nLineCount) ;

		if(strcmp(szString, "*NODE_NAME") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			sprintf(pMesh->szMeshName, "%s", sz) ;

			//sscanf(szLine, "%s%s", szString, sztemp) ;
			//farser_quotation(pMesh->szMeshName, sztemp) ;

			if(strstr(pMesh->szMeshName, "Bone") || strstr(pMesh->szMeshName, "Bip"))//메쉬가 '본'이나 '바이패드'일경우
			{
				pMesh->MeshKind = SMESHKIND_BONE ;

				char *pszBoneName = new char[256] ;
				sprintf(pszBoneName, "%s", pMesh->szMeshName) ;
				pcASEData->m_apszBoneName[pcASEData->m_nNumBone++] = pszBoneName ;

				TRACE("BoneList[%d] BoneName=%s\r\n", pcASEData->m_nNumBone-1, pcASEData->m_apszBoneName[pcASEData->m_nNumBone-1]) ;
			}
			else if(strstr(pMesh->szMeshName, "Dummy"))                    //HelperObject
			{
				pMesh->MeshKind = SMESHKIND_DUMMY ;
				pcASEData->m_bUseDummyObject = true ;
			}
			else
                pMesh->MeshKind = SMESHKIND_RIGID ;

			continue ;
		}

		if(strcmp(szString, "*NODE_PARENT") == identical)
		{
			char sz[256] ;

			char *pszStart = strchr(szLine, '\"')+1 ;
			char *pszEnd = strrchr(szLine, '\"') ;

			memcpy(sz, pszStart, pszEnd-pszStart) ;
			sz[pszEnd-pszStart] = '\0' ;

			sprintf(pMesh->szParentName, "%s", sz) ;

			pMesh->bParent = true ;

			//sscanf(szLine, "%s%s", szString, sztemp) ;
			//farser_quotation(pMesh->szParentName, sztemp) ;
			continue ;
		}

		if(strcmp(szString, "*NODE_TM") == identical)
		{
			ReadNODE_TM(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*MESH") == identical)
		{
			ReadMESH(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*MATERIAL_REF") == identical)
		{
			sscanf(szLine, "%s%d", szString, &pMesh->nMaterialIndex) ;

			if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].nNumSubMaterial > 0)//SubMaterial을 사용하는 Mesh인경우
				pMesh->nAttr |= SMESHATTR_USESUBMATERIAL ;

			if(pcASEData->m_psMaterial[pMesh->nMaterialIndex].bUseNormalMap)//NormalMap 사용하는 Mesh인 경우
			{
				pMesh->nAttr |= SMESHATTR_USENORMALMAP ;
			}

			continue ;
		}

		if(strcmp(szString, "*TM_ANIMATION") == identical)
		{
			bAnimation = true ;
			pMesh->nAttr |= SMESHATTR_ANIMATION ;
			ReadTM_ANIMATION(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*SKIN") == identical)
		{
			bSkinnedAni = true ;
			pMesh->MeshKind = SMESHKIND_SKINNING ;
			ReadSKIN(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "*PHYSIQUE") == identical)
		{
			bSkinnedAni = true ;
			pMesh->MeshKind = SMESHKIND_SKINNING ;
			ReadSKIN(pf, pMesh) ;
			continue ;
		}

		if(strcmp(szString, "}") == identical)
		{
			if(pMesh->bParent)//이렇게 부모메쉬를 받을수 있는것은 ASE파일이 계층구조 순서대로 되어있기때문에 이전메쉬에서 부모를 찾을수 있다.
			{
				int nIndex ;
				SMesh *psMeshParent ;

				pcASEData->FindMesh(pMesh->szParentName, &psMeshParent, nIndex) ;

				assert((psMeshParent != NULL) && "Cannot Find Parent") ;

				if(psMeshParent)
				{
					pMesh->psMeshParent = psMeshParent ;
					pMesh->nMeshParentIndex = nIndex ;
				}
			}

			pcASEData->m_apMesh[pcASEData->m_nMeshNode++] = pMesh ;
			pcASEData->m_nNumMesh++ ;

			return true ;
		}
	}

    return true ;
}
*/