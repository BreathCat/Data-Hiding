/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncCu.cpp
    \brief    Coding Unit (CU) encoder class
*/

#include <stdio.h>
#include "TEncTop.h"
#include "TEncCu.h"
#include "TEncAnalyze.h"
#include "TLibCommon/Debug.h"

#include <cmath>
#include <algorithm>
using namespace std;

//�Զ����������

extern int CTUIndex;
extern int CurrentPOC;
extern int CUComCount;
extern int CUDepth[85];
extern int CUPartSize[85];
extern int CUResetPart;
extern int CUTargetMode[85];
extern double Capacity;//ͳ��Ƕ������   ---emd��Capacity�����͸ĳ���double
extern int isorg;
extern int PUcategeory[4][8];//ͳ��8*8���������� 2N*2N N*N  N*2N  2N*N
extern int FPUcategeory[4][8];//ͳ�Ƶ�һ��P֡8*8������
extern int judgeMode;

int ChangeFlag = 0;


//by lzh
extern int EMD_16_CUTargetMode[16];
extern int CUnum_16 ;
extern int EMD_32_CUTargetMode[4];  
extern int CUnum_32 ;
extern int EMD_64_CUTargetMode[1];  
extern int CUnum_64 ;
extern int EMD_8_CUTargetMode[64];  
extern int CUnum_8 ;
extern int TOTAL_8;
extern int TOTAL_16;
extern int TOTAL_32;
extern int TOTAL_64;
int find_flag = 0; //����Ƕ��Ϣ��
int EMD_SUM = 0; // �ж�Ŀǰ��EMD SUM
int aim_bit =0; //EMD��Ƕ����Ϣ
int n = 0;//ÿ�δ������
int p = 3;//����
int c; 
int m = 0; //ThNumָ��
int ThNum[100] = {25}; //�洢��������������20����δ����
char MessStr[100] = {0}; //�洢������Message�ַ��� 
int MessageFlag = 0;//��¼֮ǰ��û�ж���Message
   

    

extern int CurrentPOC;




//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

/**
 \param    uhTotalDepth  total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 \param    chromaFormat  chroma format
 */
Void TEncCu::create(UChar uhTotalDepth, UInt uiMaxWidth, UInt uiMaxHeight, ChromaFormat chromaFormat)
{
  Int i;

  m_uhTotalDepth   = uhTotalDepth + 1;
  m_ppcBestCU      = new TComDataCU*[m_uhTotalDepth-1];
  m_ppcTempCU      = new TComDataCU*[m_uhTotalDepth-1];

  m_ppcPredYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcPredYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcOrigYuv     = new TComYuv*[m_uhTotalDepth-1];

  UInt uiNumPartitions;
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    uiNumPartitions = 1<<( ( m_uhTotalDepth - i - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> i;
    UInt uiHeight = uiMaxHeight >> i;

    m_ppcBestCU[i] = new TComDataCU; m_ppcBestCU[i]->create( chromaFormat, uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );
    m_ppcTempCU[i] = new TComDataCU; m_ppcTempCU[i]->create( chromaFormat, uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );

    m_ppcPredYuvBest[i] = new TComYuv; m_ppcPredYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcResiYuvBest[i] = new TComYuv; m_ppcResiYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcRecoYuvBest[i] = new TComYuv; m_ppcRecoYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);

    m_ppcPredYuvTemp[i] = new TComYuv; m_ppcPredYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcResiYuvTemp[i] = new TComYuv; m_ppcResiYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcRecoYuvTemp[i] = new TComYuv; m_ppcRecoYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);

    m_ppcOrigYuv    [i] = new TComYuv; m_ppcOrigYuv    [i]->create(uiWidth, uiHeight, chromaFormat);
  }

  m_bEncodeDQP                     = false;
  m_stillToCodeChromaQpOffsetFlag  = false;
  m_cuChromaQpOffsetIdxPlus1       = 0;
  m_bFastDeltaQP                   = false;

  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster( m_uhTotalDepth, 1, 0, piTmp);
  initRasterToZscan( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );

  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );
}

Void TEncCu::destroy()
{
  Int i;

  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    if(m_ppcBestCU[i])
    {
      m_ppcBestCU[i]->destroy();      delete m_ppcBestCU[i];      m_ppcBestCU[i] = NULL;
    }
    if(m_ppcTempCU[i])
    {
      m_ppcTempCU[i]->destroy();      delete m_ppcTempCU[i];      m_ppcTempCU[i] = NULL;
    }
    if(m_ppcPredYuvBest[i])
    {
      m_ppcPredYuvBest[i]->destroy(); delete m_ppcPredYuvBest[i]; m_ppcPredYuvBest[i] = NULL;
    }
    if(m_ppcResiYuvBest[i])
    {
      m_ppcResiYuvBest[i]->destroy(); delete m_ppcResiYuvBest[i]; m_ppcResiYuvBest[i] = NULL;
    }
    if(m_ppcRecoYuvBest[i])
    {
      m_ppcRecoYuvBest[i]->destroy(); delete m_ppcRecoYuvBest[i]; m_ppcRecoYuvBest[i] = NULL;
    }
    if(m_ppcPredYuvTemp[i])
    {
      m_ppcPredYuvTemp[i]->destroy(); delete m_ppcPredYuvTemp[i]; m_ppcPredYuvTemp[i] = NULL;
    }
    if(m_ppcResiYuvTemp[i])
    {
      m_ppcResiYuvTemp[i]->destroy(); delete m_ppcResiYuvTemp[i]; m_ppcResiYuvTemp[i] = NULL;
    }
    if(m_ppcRecoYuvTemp[i])
    {
      m_ppcRecoYuvTemp[i]->destroy(); delete m_ppcRecoYuvTemp[i]; m_ppcRecoYuvTemp[i] = NULL;
    }
    if(m_ppcOrigYuv[i])
    {
      m_ppcOrigYuv[i]->destroy();     delete m_ppcOrigYuv[i];     m_ppcOrigYuv[i] = NULL;
    }
  }
  if(m_ppcBestCU)
  {
    delete [] m_ppcBestCU;
    m_ppcBestCU = NULL;
  }
  if(m_ppcTempCU)
  {
    delete [] m_ppcTempCU;
    m_ppcTempCU = NULL;
  }

  if(m_ppcPredYuvBest)
  {
    delete [] m_ppcPredYuvBest;
    m_ppcPredYuvBest = NULL;
  }
  if(m_ppcResiYuvBest)
  {
    delete [] m_ppcResiYuvBest;
    m_ppcResiYuvBest = NULL;
  }
  if(m_ppcRecoYuvBest)
  {
    delete [] m_ppcRecoYuvBest;
    m_ppcRecoYuvBest = NULL;
  }
  if(m_ppcPredYuvTemp)
  {
    delete [] m_ppcPredYuvTemp;
    m_ppcPredYuvTemp = NULL;
  }
  if(m_ppcResiYuvTemp)
  {
    delete [] m_ppcResiYuvTemp;
    m_ppcResiYuvTemp = NULL;
  }
  if(m_ppcRecoYuvTemp)
  {
    delete [] m_ppcRecoYuvTemp;
    m_ppcRecoYuvTemp = NULL;
  }
  if(m_ppcOrigYuv)
  {
    delete [] m_ppcOrigYuv;
    m_ppcOrigYuv = NULL;
  }
}

/** \param    pcEncTop      pointer of encoder class
 */
Void TEncCu::init( TEncTop* pcEncTop )
{
  m_pcEncCfg           = pcEncTop;
  m_pcPredSearch       = pcEncTop->getPredSearch();
  m_pcTrQuant          = pcEncTop->getTrQuant();
  m_pcRdCost           = pcEncTop->getRdCost();

  m_pcEntropyCoder     = pcEncTop->getEntropyCoder();
  m_pcBinCABAC         = pcEncTop->getBinCABAC();

  m_pppcRDSbacCoder    = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder  = pcEncTop->getRDGoOnSbacCoder();

  m_pcRateCtrl         = pcEncTop->getRateCtrl();
  m_lumaQPOffset       = 0;
  initLumaDeltaQpLUT();
}


void countC(int sizes[85])//------------------------------yyyyyyy
{
	  if (CUDepth[0] != 0)//������ CUTargetMode[0] = 255;
	  {
		  for (int j = 1; j <= 64; j = j + 21)
		  {
			  if (CUDepth[j] == 11)
			  {//32*32
				  PUcategeory[1][sizes[j]]++;
				  if(CurrentPOC==1){
					  FPUcategeory[1][sizes[j]]++;
				  }
			  }else{
				   for (int i = 1+j; i <= 16+j; i = i + 5)//�ж�2��7��12��17�ĸ�CU
				  {
					  if (CUDepth[i] == 22)//���ֳ���16x16
					  {
						  PUcategeory[2][sizes[i]]++;
						  if(CurrentPOC==1){
							  FPUcategeory[2][sizes[i]]++;
						  }
					  }else{//8*8
						   for(int k=1;k<5;k++){
							  PUcategeory[3][sizes[i+k]]++;
							  if(CurrentPOC==1){
								  FPUcategeory[3][sizes[i+k]]++;
							  }
						   }
					  }
				   }
			  }
		  }
	  }else{//64*64
		  PUcategeory[0][sizes[0]]++;
		  if(CurrentPOC==1){
			  FPUcategeory[0][sizes[0]]++;
		  }
	  }

}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** 
 \param  pCtu pointer of CU data class
 */
Void TEncCu::compressCtu( TComDataCU* pCtu )////------------------------------yyyyyyy
{
  // initialize CU data
  m_ppcBestCU[0]->initCtu( pCtu->getPic(), pCtu->getCtuRsAddr() );
  m_ppcTempCU[0]->initCtu( pCtu->getPic(), pCtu->getCtuRsAddr() );

    //�ۼ�CTUIndex   ---by emd
  int i = 0;
  int j = 0;
  int n = 0;
  int ii=0;
  int randnum=0;
  int PUMode1;
  int PUMode2;
  int PUMode3;
  int PUModeTemp;
  for(int jj = 0; jj<16;jj++)
       {
	   EMD_16_CUTargetMode[jj] =  111 ;// ����
	   }
  for(int jj = 0; jj<4;jj++)
       {
	   EMD_32_CUTargetMode[jj] =  111 ;// ����
	   }
  for(int jj = 0; jj<64;jj++)
       {
	   EMD_8_CUTargetMode[jj] =  111 ;// ����
	   }
  EMD_64_CUTargetMode[0] = 0;
  CUnum_16=0;// ����
  CUnum_8=0;
  CUnum_32=0;
  CUnum_64=0;

  //���¶�������Ϣ----------------------//
  if(!MessageFlag ){
	  MessageFlag = 1;
	   ifstream  afile;

	afile.open("Message.txt");

   
   afile >> MessStr;           //�����ո����ֹͣ���ո��������޷������'\0'�ǽ�ֹ������ͼ3��ʾ

   cout << MessStr << endl;

   afile .close();

   int TotalNum = 0;

   while(MessStr[TotalNum++]){
   }

   for(int i = 0; i<TotalNum-1;i++)
   {

	  n = (int)MessStr[i];
	  printf("\nn = %d\n",n);

	  int mlast = m;
	  while(n){
	    c=n%p;
		n=n/p;
		m++;
		ThNum[m]=c;  
		printf("%d",c); 
	  }

	  int gap = 5-(m-mlast);
	  for (int loop = 0; loop<gap;loop++)
		  ThNum[++m] = 0;

   }

   // n�����ַ�����,����ThNum�������5λ��m��m-1
   n = 0;
   for(int i =0;MessStr[i]!=0;i++){
	   n++;
   }
	  printf("\nn = %d\n",n);

	  int mlast = m;
	  while(n){
	    c=n%p;
		n=n/p;
		m++;
		ThNum[m]=c;  
		printf("%d",c); 
	  }

	  int gap = 5-(m-mlast);
	  for (int loop = 0; loop<gap;loop++){
		  ThNum[++m] = 0;
		  cout<<"0";
	  }
	  cout<<endl;
  }
  //���϶�������Ϣ----------------------//


  //��ʼ��
  if (pCtu->getSlice()->getSliceType() != I_SLICE)
  {
	  CUComCount = 0;
	//  CTUIndex=pCtu->getCtuRsAddr()+1;
	   CurrentPOC = pCtu->getSlice()->getPOC();
	//  printf("---------------CTUIndex=%d",pCtu->getSlice()->getPOC());
	//  printf("---------------Fram %2d,CTUIndex=%d,CtuRsAddr=%d\n", currentPOC, CTUIndex, pCtu->getCtuRsAddr());
	  //��ʼ���洢������
	  for (int i = 0; i < 85; i++)
	  {
		  CUPartSize[i] = 0;
		  CUDepth[i] = 3;
		  CUTargetMode[i] = 1;//Ĭ�϶�Ϊ1+++7
	  }
	}

  //��һ��ѹ������־λΪ0
  CUResetPart = 0;
  ChangeFlag = 0;
  // analysis of CU
  DEBUG_STRING_NEW(sDebug)




  xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 DEBUG_STRING_PASS_INTO(sDebug) );

  //CTU��Ϣ����  
  srand((unsigned)time(NULL));
  if (CUComCount>84 && pCtu->getSlice()->getSliceType() != I_SLICE)
  {
	  	  ///////// test zzz begin  ///////
	 /* printf("CUPartSize:\n");
	  for (int i=0;i<85;i++)
	  {

		  if((i+1)%5==0)
			  printf("\n");
		  printf("%5d",CUPartSize[i]);
	  }

	  printf("CUTargetMode:\n");
	  for (int i=0;i<85;i++)
	  {

		  if((i+1)%5==0)
			  printf("\n");
		  printf("%5d",CUTargetMode[i]);
	  }*/
	  ///////// test zzz end  ///////




/* ����������Ƕ����Ϣ����--��ʼ **********

	  if (CUDepth[0] != 0)//������ CUTargetMode[0] = 255;
	  {
		  CUTargetMode[0] = 255;
		  for (int j = 1; j <= 64; j = j + 21)
		  {
			  if (CUDepth[j] == 11)
			  {//û��ϸ��----���浱ǰModeΪTargetMode
				// printf("----Fram:%d---CTU:%d--j:%d**32x32***partsize: %d -------------\n",currentPOC, CTUIndex, j, CUPartSize[j]);
				  if((judgeMode&4) == 0) CUTargetMode[j] = CUPartSize[j];
				  else{
					 // printf("%d__________________-----------__________�޸�32*32\n",judgeMode&4 );
					  CUTargetMode[j] = CUPartSize[j];
					   if (CUPartSize[j] == 1)
				  {

					  Capacity++;
					 if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[j] = 4;
						  }
						  else
						  {
							  CUTargetMode[j] = 5;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[j] == 2)
				  {

					  Capacity++;
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[j] = 6;
						  }
						  else
						  {
							  CUTargetMode[j] = 7;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[j] == 4)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  if (rand() % 100 > 50)//%50�����޸�Ϊ5
						  {
							  CUTargetMode[j] = 4;
						  }
						  else
						  {
							  CUTargetMode[j] = 5;
						  }
						  Capacity += 2;
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[j] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[j] == 5)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[j] = 4;
						  }
						  else
						  {
							  CUTargetMode[j] = 5;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[j] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[j] == 6)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ7
						  {
							  CUTargetMode[j] = 6;
						  }
						  else
						  {
							  CUTargetMode[j] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[j] = 2;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[j] == 7)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[j] = 6;
						  }
						  else
						  {
							  CUTargetMode[j] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[j] = 2;
						  ChangeFlag = 1;
					  }
				  }
				  }
			  }
			  else
			  {//�����֣�TargetMode��Ϊ255,Ȼ���ڲ�CU
				  CUTargetMode[j] = 255;
				  for (int i = 1+j; i <= 16+j; i = i + 5)//�ж�2��7��12��17�ĸ�CU
				  {
					  if (CUDepth[i] == 22)//���ֳ���16x16
					  {
						   if((judgeMode&2) == 0) CUTargetMode[i] = CUPartSize[i];
						   else{
							//   printf("%d__________________-----------__________�޸�16*16 \n",judgeMode&2);
							   CUTargetMode[i] = CUPartSize[i];
							   if (CUPartSize[i] == 1)
				  {

					  Capacity++;
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[i] = 4;
						  }
						  else
						  {
							  CUTargetMode[i] = 5;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[i] == 2)
				  {

					  Capacity++;
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[i] = 6;
						  }
						  else
						  {
							  CUTargetMode[i] = 7;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[i] == 4)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  if (rand() % 100 > 50)//%50�����޸�Ϊ5
						  {
							  CUTargetMode[i] = 4;
						  }
						  else
						  {
							  CUTargetMode[i] = 5;
						  }
						  Capacity += 2;
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[i] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[i] == 5)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[i] = 4;
						  }
						  else
						  {
							  CUTargetMode[i] = 5;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[i] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[i] == 6)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ7
						  {
							  CUTargetMode[i] = 6;
						  }
						  else
						  {
							  CUTargetMode[i] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[i] = 2;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[i] == 7)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[i] = 6;
						  }
						  else
						  {
							  CUTargetMode[i] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[i] = 2;
						  ChangeFlag = 1;
					  }
				  }
						   }
					  }
					  else//���ֳ���8x8x4   TargetMode��Ϊ255
					  {
						  CUTargetMode[i] = 255;
						  if((judgeMode&1) == 0){
							CUTargetMode[i + 1] = CUPartSize[i + 1];
							CUTargetMode[i + 2] = CUPartSize[i + 2];
							CUTargetMode[i + 3] = CUPartSize[i + 3];
							CUTargetMode[i + 4] = CUPartSize[i + 4];
							//cout<<"CUPartSize[i + n]="<<CUPartSize[i + 1]<<endl;//--ljd
							//cout<<"CUPartSize[i + n]="<<CUPartSize[i + 2]<<endl;//--ljd
							//cout<<"CUPartSize[i + n]="<<CUPartSize[i + 3]<<endl;//--ljd
							//cout<<"CUPartSize[i + n]="<<CUPartSize[i + 4]<<endl;//--ljd
						  }else{

							  if((judgeMode&16) != 0){//����2N*@N
								  for(int k=1;k<5;k++){
									  if(CUPartSize[i + k]==SIZE_2Nx2N){
										  CUTargetMode[i+k] = SIZE_2Nx2N;
									  }
									  if(CUPartSize[i + k]==SIZE_NxN){
										   if (rand() % 100 > 50)//%50�����޸�
											  {
												  Capacity += 2;
												  if (rand() % 100 > 50)//%50�����޸�Ϊ6
												  {
													   CUTargetMode[i+k] = SIZE_Nx2N;
												  }
												  else
												  {
													   CUTargetMode[i+k] = SIZE_2NxN;
												  }
												  ChangeFlag = 1;
											  }
											  else
											  {
												  Capacity++;
												  CUTargetMode[i+k] = SIZE_NxN;
												  ChangeFlag = 1;
											  }
									  }
									  if(CUPartSize[i + k]==SIZE_Nx2N){
										  if (rand() % 100 > 50)//%50�����޸�
											  {
												  Capacity += 2;
												  if (rand() % 100 > 50)//%50�����޸�Ϊ6
												  {
													   CUTargetMode[i+k] = SIZE_Nx2N;
												  }
												  else
												  {
													   CUTargetMode[i+k] = SIZE_2NxN;
												  }
												  ChangeFlag = 1;
											  }
											  else
											  {
												  Capacity++;
												  CUTargetMode[i+k] = SIZE_NxN;
												  ChangeFlag = 1;
											  }
									  }
									  if(CUPartSize[i + k]==SIZE_2NxN){
										  if (rand() % 100 > 50)//%50�����޸�
											  {
												  Capacity += 2;
												  if (rand() % 100 > 50)//%50�����޸�Ϊ6
												  {
													   CUTargetMode[i+k] = SIZE_Nx2N;
												  }
												  else
												  {
													   CUTargetMode[i+k] = SIZE_2NxN;
												  }
												  ChangeFlag = 1;
											  }
											  else
											  {
												  Capacity++;
												  CUTargetMode[i+k] = SIZE_NxN;
												  ChangeFlag = 1;
											  }
									  }
								  }
							  }else{
							//  printf("%d__________________-----------__________�޸�8*8\n",judgeMode&1 );
								  for(int k=1;k<5;k++){
									//  printf(">>>>>>>>>>>>TargetMode %d ==  %d  ----\n", i+k, CUPartSize[i + k]);
									  if(CUPartSize[i + k]==SIZE_2Nx2N){
										//  PUcategeory[0]++;
										//  if(CurrentPOC==1) FPUcategeory[0]++;
										  //CUTargetMode[i + k] = SIZE_NxN;						 
										 // printf(">>>>>>>>>>>>TargetMode %d ==  %d  ----\n",i+ k, CUTargetMode[i+k]);
												  int randnum = rand() % 100;
												//	int randnum = 1;
												  if (randnum < 25)//%50�����޸�Ϊ6
												  {
													  CUTargetMode[i+k] = SIZE_NxN;
													   ChangeFlag=1;
													   Capacity +=2;
										//			   PUcategeoryC[1]++;
													  // if(CurrentPOC==1) FPUcategeoryC[1]++;
												  }
												  else if(randnum >= 25 && randnum < 50)
												  {
													  CUTargetMode[i+k] = SIZE_Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													//    PUcategeoryC[2]++;
													  // if(CurrentPOC==1) FPUcategeoryC[2]++;
												  }else if(randnum >= 50 && randnum < 75){
													  CUTargetMode[i+k] = SIZE_2NxN;
													   ChangeFlag=1;
													   Capacity +=2;
													//    PUcategeoryC[3]++;
													  // if(CurrentPOC==1) FPUcategeoryC[3]++;
												  }else{
													  CUTargetMode[i+k] = CUPartSize[i + k];
													 //  PUcategeoryC[0]++;
													  // if(CurrentPOC==1) FPUcategeoryC[0]++;
												  }
									  
									  }
									  if(CUPartSize[i + k]==SIZE_NxN){
										 ///  PUcategeory[1]++;
										   //if(CurrentPOC==1) FPUcategeory[1]++;
										  //CUTargetMode[i + k] = SIZE_NxN;
								 
										 // printf(">>>>>>>>>>>>TargetMode %d ==  %d  ----\n",i+ k, CUTargetMode[i+k]);
												  int randnum = rand() % 100;
												  if (randnum < 25)//%50�����޸�Ϊ6
												  {
													  CUTargetMode[i+k] = SIZE_2Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													  //  PUcategeoryC[0]++;
													   //if(CurrentPOC==1) FPUcategeoryC[0]++;
												  }
												  else if(randnum >= 25 && randnum < 50)
												  {
													  CUTargetMode[i+k] = SIZE_Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													 //   PUcategeoryC[2]++;
													   //if(CurrentPOC==1) FPUcategeoryC[2]++;
												  }else if(randnum >= 50 && randnum < 75){
													  CUTargetMode[i+k] = SIZE_2NxN;
													   ChangeFlag=1;
													   Capacity +=2;
													 //   PUcategeoryC[3]++;
													   //if(CurrentPOC==1) FPUcategeoryC[3]++;
												  }else{
													  CUTargetMode[i+k] = CUPartSize[i + k];
													//   PUcategeoryC[1]++;
													  // if(CurrentPOC==1) FPUcategeoryC[1]++;
												  }
									  
									  }
									   if(CUPartSize[i + k]==SIZE_Nx2N){
										  //  PUcategeory[2]++;
											//if(CurrentPOC==1) FPUcategeory[2]++;
										  //CUTargetMode[i + k] = SIZE_NxN;
								 
										 // printf(">>>>>>>>>>>>TargetMode %d ==  %d  ----\n",i+ k, CUTargetMode[i+k]);
												  int randnum = rand() % 100;
												  if (randnum < 25)//%50�����޸�Ϊ6
												  {
													  CUTargetMode[i+k] = SIZE_NxN;
													   ChangeFlag=1;
													   Capacity +=2;
												//	    PUcategeoryC[1]++;
													//   if(CurrentPOC==1) FPUcategeoryC[1]++;
												  }
												  else if(randnum >= 25 && randnum < 50)
												  {
													  CUTargetMode[i+k] = SIZE_2Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													//    PUcategeoryC[0]++;
													  // if(CurrentPOC==1) FPUcategeoryC[0]++;
												  }else if(randnum >= 50 && randnum < 75){
													  CUTargetMode[i+k] = SIZE_2NxN;
													   ChangeFlag=1;
													   Capacity +=2;
													//    PUcategeoryC[3]++;
													  // if(CurrentPOC==1) FPUcategeoryC[3]++;
												  }else{
													  CUTargetMode[i+k] = CUPartSize[i + k];
													//   PUcategeoryC[2]++;
													  // if(CurrentPOC==1) FPUcategeoryC[2]++;
												  }
									  
									  }
									   if(CUPartSize[i + k]==SIZE_2NxN){
										 //   PUcategeory[3]++;
											//if(CurrentPOC==1) FPUcategeory[3]++;
										  //CUTargetMode[i + k] = SIZE_NxN;
								 
										 // printf(">>>>>>>>>>>>TargetMode %d ==  %d  ----\n",i+ k, CUTargetMode[i+k]);
												  int randnum = rand() % 100;
												  if (randnum < 25)//%50�����޸�Ϊ6
												  {
													  CUTargetMode[i+k] = SIZE_NxN;
													   ChangeFlag=1;
													   Capacity +=2;
												//	    PUcategeoryC[1]++;
													//   if(CurrentPOC==1) FPUcategeoryC[1]++;
												  }
												  else if(randnum >= 25 && randnum < 50)
												  {
													  CUTargetMode[i+k] = SIZE_Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													 //   PUcategeoryC[2]++;
													   //if(CurrentPOC==1) FPUcategeoryC[2]++;
												  }else if(randnum >= 50 && randnum < 75){
													  CUTargetMode[i+k] = SIZE_2Nx2N;
													   ChangeFlag=1;
													   Capacity +=2;
													 //   PUcategeoryC[0]++;
													   //if(CurrentPOC==1) FPUcategeoryC[0]++;
												  }else{
													  CUTargetMode[i+k] = CUPartSize[i + k];
													//   PUcategeoryC[3]++;
													  // if(CurrentPOC==1) FPUcategeoryC[3]++;
												  }
									  
									  }
								  }
							 }
						  }
					  }

				  }
			  }
		  }
	  }
	  else
	  {
		  //printf("----Fram:%d---CTU:%d-------64x64-------------\n", currentPOC, CTUIndex);
		  if((judgeMode&8) == 0) CUTargetMode[0] = CUPartSize[0];
		  else{
			 // printf("%d__________________-----------__________�޸�64*64\n",judgeMode&8 );
			  if (CUPartSize[0] == 1)
				  {

					  Capacity++;
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[0] = 4;
						  }
						  else
						  {
							  CUTargetMode[0] = 5;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[0] == 2)
				  {

					  Capacity++;
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity++;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[0] = 6;
						  }
						  else
						  {
							  CUTargetMode[0] = 7;
						  }
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[0] == 4)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  if (rand() % 100 > 50)//%50�����޸�Ϊ5
						  {
							  CUTargetMode[0] = 4;
						  }
						  else
						  {
							  CUTargetMode[0] = 5;
						  }
						  Capacity += 2;
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[0] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[0] == 5)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ4
						  {
							  CUTargetMode[0] = 4;
						  }
						  else
						  {
							  CUTargetMode[0] = 5;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[0] = 1;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[0] == 6)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ7
						  {
							  CUTargetMode[0] = 6;
						  }
						  else
						  {
							  CUTargetMode[0] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[0] = 2;
						  ChangeFlag = 1;
					  }
				  }
				  else if (CUPartSize[0] == 7)
				  {
					  if (rand() % 100 > 50)//%50�����޸�
					  {
						  Capacity += 2;
						  if (rand() % 100 > 50)//%50�����޸�Ϊ6
						  {
							  CUTargetMode[0] = 6;
						  }
						  else
						  {
							  CUTargetMode[0] = 7;
						  }
						  ChangeFlag = 1;
					  }
					  else
					  {
						  Capacity++;
						  CUTargetMode[0] = 2;
						  ChangeFlag = 1;
					  }
				  }
		  }
	  }

	   ����������Ƕ����Ϣ����--���� **********/


// ����emd����--��ʼ *************

	  if (CUDepth[0] != 0 )//������ CUTargetMode[0] = 255; �ж��ǲ���64
	  {
		  CUTargetMode[0] = 255;
		  for (j = 1; j <= 64; j = j + 21)
		  {
			  if (CUDepth[j] == 11) //index��85��PUģʽ��ʮλ�Ǳ���Ӧ��depth����λ�Ǳ����level��       ����Ϊ32X32
			  {
				  CUTargetMode[j] = CUPartSize[j]; //��PU6����PU1���޸�
			      EMD_32_CUTargetMode[CUnum_32]=j; 
				  CUnum_32++; 
				  TOTAL_32++;
			  }
			  else
			  {//�����֣�TargetMode��Ϊ255,Ȼ���ڲ�CU
				  CUTargetMode[j] = 255;
				  for (i = 1+j; i <= 16+j; i = i + 5)//�ж�2��7��12��17�ĸ�CU
				  {					  
					  if (CUDepth[i] == 22)   //        ���ֳ���16x16
					  {
						    CUTargetMode[i] = CUPartSize[i]; //��PU6����PU1���޸�
						    EMD_16_CUTargetMode[CUnum_16]=i; 
						    CUnum_16++; 			
							TOTAL_16++;
					  }
					  else//���ֳ���8x8x4   TargetMode��Ϊ255  Question1
					  {
						  TOTAL_8 += 4;
						  CUTargetMode[i] = 255;
						 for (n = 1; n <= 4; n++)
						  {
							CUTargetMode[i + n] = CUPartSize[i + n];//�ǲ���Ӧ�ü�����仰�� --����
							//cout<<"CUPartSize[i + n]="<<CUPartSize[i + n]<<endl;//--ljd

							EMD_8_CUTargetMode[CUnum_8]=i+n; 
						    CUnum_8++; 

						  }
						 
					  }

				  }
			  }
		  }
	  }
	  else//64û�л���
	  {
		  CUTargetMode[0] = CUPartSize[0];//Level3
		 	CUnum_64=1;
			TOTAL_64++;
		  //printf(">>>>>>>>>>>>TargetMode 0 ==  %d  ----\n",  CUTargetMode[0]);
	  }

/*>>>>>>>>>>>>>>>>>>>>>>lzh 16x16 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/
	  if(CUnum_16> 2 && 0) //���һ��CTU��16x16����,�������if��������lzh���㷨 16X16����
	  {
		  for(ii =0;ii+2<CUnum_16;ii = ii+3) //iiָ����16x16  ��target
		  {			  
			  PUMode1 = CUTargetMode[EMD_16_CUTargetMode[ii]];   //�ñ�������ԭ��PU����ģʽ0-7��������3�������ӳ�䵽EMD��ֵ0-6������3��
			  PUMode2 = CUTargetMode[EMD_16_CUTargetMode[ii+1]];
			  PUMode3 = CUTargetMode[EMD_16_CUTargetMode[ii+2]];

			  if(PUMode1>3) {PUMode1 -= 1;}
			  if(PUMode2>3) {PUMode2 -= 1;}
			  if(PUMode3>3) {PUMode3 -= 1;}   //��4-7��PUģʽ��Ӧ��3-6���γ������0-6��7��PU����ģʽ

			  EMD_SUM = PUMode1+2*PUMode2+3*PUMode3; //3ά N=3

			  randnum = rand() % 7;  //2N+1 ��randnumΪ��Ƕ����Ϣ
			  Capacity += 2.8 ;//2.8=log2(7)
			  ChangeFlag =1;

			  if((EMD_SUM +1) % 7 == randnum)
			  {
				  PUModeTemp =( PUMode1 +1 )%7;   //����Ⱥ���ߵı�������ӳ�䵽EMD��ֵ
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii]] = PUModeTemp + 1;} //���ﻹԭӳ�䵽ԭ��PU����ģʽ��ֵ��CUTargetMode���顣
			  }
			  else if((EMD_SUM +2) % 7 == randnum)
			  {
				  PUModeTemp =( PUMode2 + 1)%7 ;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+1]] = PUModeTemp + 1;}
			  }
			  else if((EMD_SUM +3) % 7 == randnum)
			  {
				  PUModeTemp = ( PUMode3 + 1 )%7 ;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+2]] = PUModeTemp + 1;}
			  }
			  else if(EMD_SUM  % 7 == randnum +1)
			  {
				  PUModeTemp = PUMode1-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii]] = PUModeTemp + 1;}

			  }
			  else if(EMD_SUM  % 7 == randnum +2)
			  {
				  PUModeTemp = PUMode2-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+1]] = PUModeTemp + 1;}

			  }
			  else if(EMD_SUM  % 7 == randnum +3)
			  {
				  PUModeTemp = PUMode3-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+2]] = PUModeTemp + 1;}

			  }
			  else //��Ƕ����Ϣ��Ŀ��CU����ģʽ��ͬ�������޸�
			  {

			  }


			  if(CUTargetMode[EMD_16_CUTargetMode[ii]]<0) //ʱ��ע�⸺��Ҫȡ��
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii]]; //����8�õ�
			  }
			  if(CUTargetMode[EMD_16_CUTargetMode[ii+1]]<0) 
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii+1]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii+1]];
			  }
			  if(CUTargetMode[EMD_16_CUTargetMode[ii+2]]<0) 
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii+2]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii+2]];
			  }
		  }
	  }
	  /*>>>>>>>>>>>>>>>>>>>>>>����Ϊ lzh 16x16 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/


  
	  /*>>>>>>>>>>>>>>>>>>>>>>����Ϊ lzh 32X32 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/
	  if(CUnum_32>=3 && 0) //���һ��CTU�� 32X32����,�������if��������lzh���㷨 32X32����
	  {
		  ChangeFlag =1;
		  for(ii =0;ii+2<CUnum_32;ii = ii+3) //iiָ����
		  {
			  
			  PUMode1 = CUTargetMode[EMD_16_CUTargetMode[ii]];   //�ñ�������ԭ��PU����ģʽ0-7��������3�������ӳ�䵽EMD��ֵ0-6������3��
			  PUMode2 = CUTargetMode[EMD_16_CUTargetMode[ii+1]];
			  PUMode3 = CUTargetMode[EMD_16_CUTargetMode[ii+2]];

			  if(PUMode1>3) {PUMode1 -= 1;}
			  if(PUMode2>3) {PUMode2 -= 1;}
			  if(PUMode3>3) {PUMode3 -= 1;}   //��4-7��PUģʽ��Ӧ��3-6���γ������0-6��7��PU����ģʽ

			  EMD_SUM = PUMode1+2*PUMode2+3*PUMode3; //3ά N=3

			  randnum = rand() % 7;  //2N+1 ��randnumΪ��Ƕ����Ϣ
			  Capacity += 2.8 ;//2.8=log2(7)
			  ChangeFlag =1;

			  if((EMD_SUM +1) % 7 == randnum)
			  {
				  PUModeTemp =( PUMode1 +1 )%7;   //����Ⱥ���ߵı�������ӳ�䵽EMD��ֵ
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii]] = PUModeTemp + 1;} //���ﻹԭӳ�䵽ԭ��PU����ģʽ��ֵ��CUTargetMode���顣
			  }
			  else if((EMD_SUM +2) % 7 == randnum)
			  {
				  PUModeTemp =( PUMode2 + 1)%7 ;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+1]] = PUModeTemp + 1;}
			  }
			  else if((EMD_SUM +3) % 7 == randnum)
			  {
				  PUModeTemp = ( PUMode3 + 1 )%7 ;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+2]] = PUModeTemp + 1;}
			  }
			  else if(EMD_SUM  % 7 == randnum +1)
			  {
				  PUModeTemp = PUMode1-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii]] = PUModeTemp + 1;}

			  }
			  else if(EMD_SUM  % 7 == randnum +2)
			  {
				  PUModeTemp = PUMode2-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+1]] = PUModeTemp + 1;}

			  }
			  else if(EMD_SUM  % 7 == randnum +3)
			  {
				  PUModeTemp = PUMode3-1;
				  if(PUModeTemp >2 ){CUTargetMode[EMD_16_CUTargetMode[ii+2]] = PUModeTemp + 1;}

			  }
			  else //��Ƕ����Ϣ��Ŀ��CU����ģʽ��ͬ�������޸�
			  {

			  }


			  if(CUTargetMode[EMD_16_CUTargetMode[ii]]<0) //ʱ��ע�⸺��Ҫȡ��
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii]]; //����8�õ�
			  }
			  if(CUTargetMode[EMD_16_CUTargetMode[ii+1]]<0) 
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii+1]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii+1]];
			  }
			  if(CUTargetMode[EMD_16_CUTargetMode[ii+2]]<0) 
			  {
				  CUTargetMode[EMD_16_CUTargetMode[ii+2]] = 8 + CUTargetMode[EMD_16_CUTargetMode[ii+2]];
			  }
		  }
	  }
/*>>>>>>>>>>>>>>>>>>>>>>����Ϊ lzh 32x32<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/

 /*>>>>>>>>>>>>>>>>>>>>>>����Ϊlzh 8X8 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/

	  if(CUnum_8>2 &&1) //���һ��CTU��8X8����,�������if��������lzh���㷨 8X8����
	  {
		  ChangeFlag =1;
		for(ii =0;ii<CUnum_8;ii++) //iiָ����8X8  ��target
		  {
			 //1ά N=1
			  //cout<<"CUTargetMode[EMD_8_CUTargetMode[ii]]="<<CUTargetMode[EMD_8_CUTargetMode[ii]]<<endl;
			 if(m>-1){
				cout<<ThNum[m]<<" m="<<m<<" ";
			  randnum = ThNum[m--];  //2N+1 ��randnumΪ��Ƕ����Ϣ
			 
			 // randnum = rand() % 3;  //2N+1 ��randnumΪ��Ƕ����Ϣ
			  Capacity += 1.6 ;//2.8=log2(7)
			  aim_bit = rand() % 2;
			  switch(randnum)
			  {
				  case 0:
					if( aim_bit == 0 )
					{
						CUTargetMode[EMD_8_CUTargetMode[ii]] = 0 ; //2NX2N 8*8
						cout<<"8*8 "<<endl;
					}
					else 
					{
						CUTargetMode[EMD_8_CUTargetMode[ii]] = 3 ; //NXN 4*4
						cout<<"4*4 "<<endl;
					}
					break;

				 case 1:
				    CUTargetMode[EMD_8_CUTargetMode[ii]] = 1 ;	//2N*N 8*4	
					cout<<"8*4 "<<endl;
					break;

				 case 2:
					CUTargetMode[EMD_8_CUTargetMode[ii]] = 2 ;	//N*2N 4*8		
					cout<<"4*8 "<<endl;
					break;
			  }
			  }
			 else{}
	  
        }
    }
/*>>>>>>>>>>>>>>>>>>>>>>����Ϊ lzh 8x8 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/

 /*>>>>>>>>>>>>>>>>>>>>>>����Ϊlzh 64X64 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/
	 if(CUnum_64 && 0 ) // 64X64����
	
	  {
		      //CUTargetMode[EMD_64_CUTargetMode[ii]]; //1ά N=1
			  randnum = rand() % 3;  //2N+1 ��randnumΪ��Ƕ����Ϣ
			  Capacity += 1.6 ;//2.8=log2(7)
			  aim_bit = rand() % 3 ;
			  ChangeFlag =1;
			  switch(randnum)
			  {
				  case 0:
					if( aim_bit == 0 )
					{
						CUTargetMode[0] = 0 ;
					}
					else
					{
						CUTargetMode[0] = 6 ;
					}
					break;

				 case 1:
				   if( aim_bit == 0 )
					{
						CUTargetMode[0] = 1 ;
					}
					else if (aim_bit == 1)
					{
						CUTargetMode[0] = 4 ;
					}
					else
					{
						CUTargetMode[0] = 7 ;
					}

					break;
				 case 2:
					  if( aim_bit == 0 )
					  {
						CUTargetMode[0] = 2 ;
					  }

					else
					{
						CUTargetMode[0] = 5 ;
					}
					break;
        }
    }
/*>>>>>>>>>>>>>>>>>>>>>>����Ϊ lzh 64x64 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<��Ϣ�����㷨*/

 }



// ����emd����--���� *************

  if( CUComCount>84 && pCtu->getSlice()->getSliceType() != I_SLICE)
  {
	  if(ChangeFlag==1)
	  {
		  isorg = 0;
		  countC(CUTargetMode);
	  }
	  else
	  {
		  isorg = 1;
		  countC(CUPartSize);
	  }
  }
 

  if (1 && CUComCount>84 && pCtu->getSlice()->getSliceType() != I_SLICE && ChangeFlag==1)
  {													
	  //д��Ŀ�껮������
	  //for (i = 0; i < 85; i++)
	  //{
		 // printf("888888888888888888888888888888=========PartMode %d ==  %d  Depth=%d----\n", i, CUPartSize[i], CUDepth[i]);
	  //}
	 /* for (j = 0; j < 85; j++)
	  {
		 printf("7777777777777777777777777777777777TargetMode %d ==  %d  ----\n", j, CUTargetMode[j]);
	  }*/
	  //�ڶ���ѹ����ǰCTU

	  m_ppcBestCU[0]->initCtu(pCtu->getPic(), pCtu->getCtuRsAddr());
	  m_ppcTempCU[0]->initCtu(pCtu->getPic(), pCtu->getCtuRsAddr());

	  CUResetPart = 1;//�ڶ���ѹ������Ҫѡ��
	  CUComCount = 0;
	  xCompressCU(m_ppcBestCU[0], m_ppcTempCU[0], 0 DEBUG_STRING_PASS_INTO(sDebug));
  }


  DEBUG_STRING_OUTPUT(std::cout, sDebug)

#if ADAPTIVE_QP_SELECTION
  if( m_pcEncCfg->getUseAdaptQpSelect() )
  {
    if(pCtu->getSlice()->getSliceType()!=I_SLICE) //IIII
    {
      xCtuCollectARLStats( pCtu );
    }
  }
#endif
}
/** \param  pCtu  pointer of CU data class
 */
Void TEncCu::encodeCtu ( TComDataCU* pCtu )
{
  if ( pCtu->getSlice()->getPPS()->getUseDQP() )
  {
    setdQPFlag(true);
  }

  if ( pCtu->getSlice()->getUseChromaQpAdj() )
  {
    setCodeChromaQpAdjFlag(true);
  }

  // Encode CU data
  xEncodeCU( pCtu, 0, 0 );
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncCu::initLumaDeltaQpLUT()
{
  const LumaLevelToDeltaQPMapping &mapping=m_pcEncCfg->getLumaLevelToDeltaQPMapping();

  if ( !mapping.isEnabled() )
  {
    return;
  }

  // map the sparse LumaLevelToDeltaQPMapping.mapping to a fully populated linear table.

  Int         lastDeltaQPValue=0;
  std::size_t nextSparseIndex=0;
  for(Int index=0; index<LUMA_LEVEL_TO_DQP_LUT_MAXSIZE; index++)
  {
    while (nextSparseIndex < mapping.mapping.size() && index>=mapping.mapping[nextSparseIndex].first)
    {
      lastDeltaQPValue=mapping.mapping[nextSparseIndex].second;
      nextSparseIndex++;
    }
    m_lumaLevelToDeltaQPLUT[index]=lastDeltaQPValue;
  }
}

Int TEncCu::calculateLumaDQP(TComDataCU *pCU, const UInt absPartIdx, const TComYuv * pOrgYuv)
{
  const Pel *pY = pOrgYuv->getAddr(COMPONENT_Y, absPartIdx);
  const Int stride  = pOrgYuv->getStride(COMPONENT_Y);
  Int width = pCU->getWidth(absPartIdx);
  Int height = pCU->getHeight(absPartIdx);
  Double avg = 0;

  // limit the block by picture size
  const TComSPS* pSPS = pCU->getSlice()->getSPS();
  if ( pCU->getCUPelX() + width > pSPS->getPicWidthInLumaSamples() )
  {
    width = pSPS->getPicWidthInLumaSamples() - pCU->getCUPelX();
  }
  if ( pCU->getCUPelY() + height > pSPS->getPicHeightInLumaSamples() )
  {
    height = pSPS->getPicHeightInLumaSamples() - pCU->getCUPelY();
  }

  // Get QP offset derived from Luma level
  if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().mode == LUMALVL_TO_DQP_AVG_METHOD )
  {
    // Use avg method
    Int sum = 0;
    for (Int y = 0; y < height; y++)
    {
      for (Int x = 0; x < width; x++)
      {
        sum += pY[x];
      }
      pY += stride;
    }
    avg = (Double)sum/(width*height);
  }
  else
  {
    // Use maximum luma value
    Int maxVal = 0;
    for (Int y = 0; y < height; y++)
    {
      for (Int x = 0; x < width; x++)
      {
        if (pY[x] > maxVal)
        {
          maxVal = pY[x];
        }
      }
      pY += stride;
    }
    // use a percentage of the maxVal
    avg = (Double)maxVal * m_pcEncCfg->getLumaLevelToDeltaQPMapping().maxMethodWeight;
  }

  Int lumaIdx = Clip3<Int>(0, Int(LUMA_LEVEL_TO_DQP_LUT_MAXSIZE)-1, Int(avg+0.5) );
  Int QP = m_lumaLevelToDeltaQPLUT[lumaIdx];
  return QP;
}

//! Derive small set of test modes for AMP encoder speed-up
#if AMP_ENC_SPEEDUP
#if AMP_MRG
Void TEncCu::deriveTestModeAMP (TComDataCU *pcBestCU, PartSize eParentPartSize, Bool &bTestAMP_Hor, Bool &bTestAMP_Ver, Bool &bTestMergeAMP_Hor, Bool &bTestMergeAMP_Ver)
#else
Void TEncCu::deriveTestModeAMP (TComDataCU *pcBestCU, PartSize eParentPartSize, Bool &bTestAMP_Hor, Bool &bTestAMP_Ver)
#endif
{
  if ( pcBestCU->getPartitionSize(0) == SIZE_2NxN )
  {
    bTestAMP_Hor = true;
  }
  else if ( pcBestCU->getPartitionSize(0) == SIZE_Nx2N )
  {
    bTestAMP_Ver = true;
  }
  else if ( pcBestCU->getPartitionSize(0) == SIZE_2Nx2N && pcBestCU->getMergeFlag(0) == false && pcBestCU->isSkipped(0) == false )
  {
    bTestAMP_Hor = true;
    bTestAMP_Ver = true;
  }

#if AMP_MRG
  //! Utilizing the partition size of parent PU
  if ( eParentPartSize >= SIZE_2NxnU && eParentPartSize <= SIZE_nRx2N )
  {
    bTestMergeAMP_Hor = true;
    bTestMergeAMP_Ver = true;
  }

  if ( eParentPartSize == NUMBER_OF_PART_SIZES ) //! if parent is intra
  {
    if ( pcBestCU->getPartitionSize(0) == SIZE_2NxN )
    {
      bTestMergeAMP_Hor = true;
    }
    else if ( pcBestCU->getPartitionSize(0) == SIZE_Nx2N )
    {
      bTestMergeAMP_Ver = true;
    }
  }

  if ( pcBestCU->getPartitionSize(0) == SIZE_2Nx2N && pcBestCU->isSkipped(0) == false )
  {
    bTestMergeAMP_Hor = true;
    bTestMergeAMP_Ver = true;
  }

  if ( pcBestCU->getWidth(0) == 64 )
  {
    bTestAMP_Hor = false;
    bTestAMP_Ver = false;
  }
#else
  //! Utilizing the partition size of parent PU
  if ( eParentPartSize >= SIZE_2NxnU && eParentPartSize <= SIZE_nRx2N )
  {
    bTestAMP_Hor = true;
    bTestAMP_Ver = true;
  }

  if ( eParentPartSize == SIZE_2Nx2N )
  {
    bTestAMP_Hor = false;
    bTestAMP_Ver = false;
  }
#endif
}
#endif


// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
/** Compress a CU block recursively with enabling sub-CTU-level delta QP
 *  - for loop of QP value to compress the current CU with all possible QP
*/
#if AMP_ENC_SPEEDUP
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, const UInt uiDepth DEBUG_STRING_FN_DECLARE(sDebug_), PartSize eParentPartSize )//------------------------------yyyyyyy
#else
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, const UInt uiDepth )
#endif
{
	int TempCount = 0;
  TComPic* pcPic = rpcBestCU->getPic();
  DEBUG_STRING_NEW(sDebug)
  const TComPPS &pps=*(rpcTempCU->getSlice()->getPPS());
  const TComSPS &sps=*(rpcTempCU->getSlice()->getSPS());
  
  // These are only used if getFastDeltaQp() is true
  const UInt fastDeltaQPCuMaxSize    = Clip3(sps.getMaxCUHeight()>>sps.getLog2DiffMaxMinCodingBlockSize(), sps.getMaxCUHeight(), 32u);

  // get Original YUV data from picture
  m_ppcOrigYuv[uiDepth]->copyFromPicYuv( pcPic->getPicYuvOrg(), rpcBestCU->getCtuRsAddr(), rpcBestCU->getZorderIdxInCtu() );

  // variable for Cbf fast mode PU decision
  Bool    doNotBlockPu = true;
  Bool    earlyDetectionSkipMode = false;

  const UInt uiLPelX   = rpcBestCU->getCUPelX();
  const UInt uiRPelX   = uiLPelX + rpcBestCU->getWidth(0)  - 1;
  const UInt uiTPelY   = rpcBestCU->getCUPelY();
  const UInt uiBPelY   = uiTPelY + rpcBestCU->getHeight(0) - 1;
  const UInt uiWidth   = rpcBestCU->getWidth(0);

  Int iBaseQP = xComputeQP( rpcBestCU, uiDepth );
  Int iMinQP;
  Int iMaxQP;
  Bool isAddLowestQP = false;

  const UInt numberValidComponents = rpcBestCU->getPic()->getNumberValidComponents();

  //�������-------------------------------------------------------------------------

  if (rpcBestCU->getSlice()->getSliceType() != I_SLICE)//PocIndex == 6
  {
	  CUComCount++;
	  TempCount = CUComCount-1;
	 // printf("\tCTUIndex=%d\tCUComCount=%d\n", CTUIndex, CUComCount);
	  CUDepth[TempCount] = 10 * uiDepth;

  }


  if( uiDepth <= pps.getMaxCuDQPDepth() )
  {
    Int idQP = m_pcEncCfg->getMaxDeltaQP();
    iMinQP = Clip3( -sps.getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP-idQP );
    iMaxQP = Clip3( -sps.getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP+idQP );
  }
  else
  {
    iMinQP = rpcTempCU->getQP(0);
    iMaxQP = rpcTempCU->getQP(0);
  }

  if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() )
  {
    if ( uiDepth <= pps.getMaxCuDQPDepth() )
    {
      // keep using the same m_QP_LUMA_OFFSET in the same CTU
      m_lumaQPOffset = calculateLumaDQP(rpcTempCU, 0, m_ppcOrigYuv[uiDepth]);
    }
    iMinQP = iBaseQP - m_lumaQPOffset;
    iMaxQP = iMinQP; // force encode choose the modified QO
  }

  if ( m_pcEncCfg->getUseRateCtrl() )
  {
    iMinQP = m_pcRateCtrl->getRCQP();
    iMaxQP = m_pcRateCtrl->getRCQP();
  }

  // transquant-bypass (TQB) processing loop variable initialisation ---

  const Int lowestQP = iMinQP; // For TQB, use this QP which is the lowest non TQB QP tested (rather than QP'=0) - that way delta QPs are smaller, and TQB can be tested at all CU levels.

  if ( (pps.getTransquantBypassEnabledFlag()) )
  {
    isAddLowestQP = true; // mark that the first iteration is to cost TQB mode.
    iMinQP = iMinQP - 1;  // increase loop variable range by 1, to allow testing of TQB mode along with other QPs
    if ( m_pcEncCfg->getCUTransquantBypassFlagForceValue() )
    {
      iMaxQP = iMinQP;
    }
  }

  TComSlice * pcSlice = rpcTempCU->getPic()->getSlice(rpcTempCU->getPic()->getCurrSliceIdx());

  const Bool bBoundary = !( uiRPelX < sps.getPicWidthInLumaSamples() && uiBPelY < sps.getPicHeightInLumaSamples() );

  if ( !bBoundary )
  {
    for (Int iQP=iMinQP; iQP<=iMaxQP; iQP++)
    {
      const Bool bIsLosslessMode = isAddLowestQP && (iQP == iMinQP);

      if (bIsLosslessMode)
      {
        iQP = lowestQP;
      }
      if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() && uiDepth <= pps.getMaxCuDQPDepth() )
      {
        getSliceEncoder()->updateLambda(pcSlice, iQP);
      }

      m_cuChromaQpOffsetIdxPlus1 = 0;
      if (pcSlice->getUseChromaQpAdj())
      {
        /* Pre-estimation of chroma QP based on input block activity may be performed
         * here, using for example m_ppcOrigYuv[uiDepth] */
        /* To exercise the current code, the index used for adjustment is based on
         * block position
         */
        Int lgMinCuSize = sps.getLog2MinCodingBlockSize() +
                          std::max<Int>(0, sps.getLog2DiffMaxMinCodingBlockSize()-Int(pps.getPpsRangeExtension().getDiffCuChromaQpOffsetDepth()));
        m_cuChromaQpOffsetIdxPlus1 = ((uiLPelX >> lgMinCuSize) + (uiTPelY >> lgMinCuSize)) % (pps.getPpsRangeExtension().getChromaQpOffsetListLen() + 1);
      }

      rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

      // do inter modes, SKIP and 2Nx2N
      if( rpcBestCU->getSlice()->getSliceType() != I_SLICE )
      {
		// 2Nx2N
		if (CUResetPart == 0 ||CUTargetMode[TempCount] == 0){
			if(m_pcEncCfg->getUseEarlySkipDetection())
			{
			  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug) );
			  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );//by Competition for inter_2Nx2N
			}
			// SKIP
			xCheckRDCostMerge2Nx2N( rpcBestCU, rpcTempCU DEBUG_STRING_PASS_INTO(sDebug), &earlyDetectionSkipMode );//by Merge for inter_2Nx2N
			rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

			if(!m_pcEncCfg->getUseEarlySkipDetection())
			{
			  // 2Nx2N, NxN
			  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug) );
			  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
			  if(m_pcEncCfg->getUseCbfFastMode())
			  {
				doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
			  }
			}
		}
      }
	  
      if (bIsLosslessMode) // Restore loop variable if lossless mode was searched.
      {
        iQP = iMinQP;
      }
    }

    if(!earlyDetectionSkipMode)
    {
      for (Int iQP=iMinQP; iQP<=iMaxQP; iQP++)
      {
        const Bool bIsLosslessMode = isAddLowestQP && (iQP == iMinQP); // If lossless, then iQP is irrelevant for subsequent modules.

        if (bIsLosslessMode)
        {
          iQP = lowestQP;
        }

        rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

        // do inter modes, NxN, 2NxN, and Nx2N
        if( rpcBestCU->getSlice()->getSliceType() != I_SLICE )
        {
          // 2Nx2N, NxN

          if(!( (rpcBestCU->getWidth(0)==8) && (rpcBestCU->getHeight(0)==8) ))
          {
            if( uiDepth == sps.getLog2DiffMaxMinCodingBlockSize() && doNotBlockPu)
            {
				if (CUResetPart == 0 || CUTargetMode[TempCount] == 3){
				  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_NxN DEBUG_STRING_PASS_INTO(sDebug)   );
				  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				}
            }
          }

          if(doNotBlockPu)
          {
			  if(CUResetPart == 0||CUTargetMode[TempCount] == 2){
				xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_Nx2N DEBUG_STRING_PASS_INTO(sDebug)  );
				rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_Nx2N )
				{
				  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
				}
			  }
          }
          if(doNotBlockPu)
          {
			  if(CUResetPart == 0||CUTargetMode[TempCount] == 1){
				xCheckRDCostInter      ( rpcBestCU, rpcTempCU, SIZE_2NxN DEBUG_STRING_PASS_INTO(sDebug)  );
				rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxN)
				{
				  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
				}
			  }
          }

          //! Try AMP (SIZE_2NxnU, SIZE_2NxnD, SIZE_nLx2N, SIZE_nRx2N)
          if(sps.getUseAMP() && uiDepth < sps.getLog2DiffMaxMinCodingBlockSize() )
          {
#if AMP_ENC_SPEEDUP
            Bool bTestAMP_Hor = false, bTestAMP_Ver = false;

#if AMP_MRG
            Bool bTestMergeAMP_Hor = false, bTestMergeAMP_Ver = false;

            deriveTestModeAMP (rpcBestCU, eParentPartSize, bTestAMP_Hor, bTestAMP_Ver, bTestMergeAMP_Hor, bTestMergeAMP_Ver);
#else
            deriveTestModeAMP (rpcBestCU, eParentPartSize, bTestAMP_Hor, bTestAMP_Ver);
#endif

            //! Do horizontal AMP
            if ( bTestAMP_Hor || (CUResetPart == 1 && CUTargetMode[TempCount]>3))
            {
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 4){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU DEBUG_STRING_PASS_INTO(sDebug) );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnU )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				  }
              }
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 5){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD DEBUG_STRING_PASS_INTO(sDebug) );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnD )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				  }
              }
            }
#if AMP_MRG
            else if ( bTestMergeAMP_Hor|| (CUResetPart == 1 && CUTargetMode[TempCount]>3) )
            {
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 4){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU DEBUG_STRING_PASS_INTO(sDebug), true );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnU )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				  }
              }
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 5){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD DEBUG_STRING_PASS_INTO(sDebug), true );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnD )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				  }
              }
            }
#endif

            //! Do horizontal AMP
            if ( bTestAMP_Ver || (CUResetPart == 1 && CUTargetMode[TempCount]>3))
            {
              if(doNotBlockPu)
              {
				   if(CUResetPart == 0||CUTargetMode[TempCount] == 6){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N DEBUG_STRING_PASS_INTO(sDebug) );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_nLx2N )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				   }
              }
              if(doNotBlockPu)
              {
				   if(CUResetPart == 0||CUTargetMode[TempCount] == 7){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N DEBUG_STRING_PASS_INTO(sDebug) );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				  }
			  }
            }
#if AMP_MRG
            else if ( bTestMergeAMP_Ver || (CUResetPart == 1 && CUTargetMode[TempCount]>3))
            {
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 6){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N DEBUG_STRING_PASS_INTO(sDebug), true );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					if(m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_nLx2N )
					{
					  doNotBlockPu = rpcBestCU->getQtRootCbf( 0 ) != 0;
					}
				  }
              }
              if(doNotBlockPu)
              {
				  if(CUResetPart == 0||CUTargetMode[TempCount] == 7){
					xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N DEBUG_STRING_PASS_INTO(sDebug), true );
					rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				  }
              }
            }
#endif

#else
            xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU );
            rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
            xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD );
            rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
            xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N );
            rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

            xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N );
            rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

#endif
          }
        }

        // do normal intra modes
        // speedup for inter frames
        if((rpcBestCU->getSlice()->getSliceType() == I_SLICE)                                        ||
            ((!m_pcEncCfg->getDisableIntraPUsInInterSlices()) && (
              (rpcBestCU->getCbf( 0, COMPONENT_Y  ) != 0)                                            ||
             ((rpcBestCU->getCbf( 0, COMPONENT_Cb ) != 0) && (numberValidComponents > COMPONENT_Cb)) ||
             ((rpcBestCU->getCbf( 0, COMPONENT_Cr ) != 0) && (numberValidComponents > COMPONENT_Cr))  // avoid very complex intra if it is unlikely
            )))
        {
			if(CUResetPart==0){
			  xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug) );
			  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
			  if( uiDepth == sps.getLog2DiffMaxMinCodingBlockSize() )
			  {
				if( rpcTempCU->getWidth(0) > ( 1 << sps.getQuadtreeTULog2MinSize() ) )
				{
				  xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_NxN DEBUG_STRING_PASS_INTO(sDebug)   );
				  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				}
			  }
			}
        }
		if(rpcBestCU->getSlice()->getSliceType() != I_SLICE && CUResetPart!=0){
				if(CUTargetMode[TempCount]==SIZE_2Nx2N){
					xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug) );
					 rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
				}
				if( uiDepth == sps.getLog2DiffMaxMinCodingBlockSize() )
				  {
					if( rpcTempCU->getWidth(0) > ( 1 << sps.getQuadtreeTULog2MinSize() ) )
					{
						if(CUTargetMode[TempCount]==SIZE_NxN){
					  xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_NxN DEBUG_STRING_PASS_INTO(sDebug)   );
					  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );}
					}
				  }
			}
        // test PCM
        if(sps.getUsePCM()
          && rpcTempCU->getWidth(0) <= (1<<sps.getPCMLog2MaxSize())
          && rpcTempCU->getWidth(0) >= (1<<sps.getPCMLog2MinSize()) )
        {
          UInt uiRawBits = getTotalBits(rpcBestCU->getWidth(0), rpcBestCU->getHeight(0), rpcBestCU->getPic()->getChromaFormat(), sps.getBitDepths().recon);
          UInt uiBestBits = rpcBestCU->getTotalBits();
          if((uiBestBits > uiRawBits) || (rpcBestCU->getTotalCost() > m_pcRdCost->calcRdCost(uiRawBits, 0)))
          {
            xCheckIntraPCM (rpcBestCU, rpcTempCU);
            rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
          }
        }

        if (bIsLosslessMode) // Restore loop variable if lossless mode was searched.
        {
          iQP = iMinQP;
        }
      }
    }

    if( rpcBestCU->getTotalCost()!=MAX_DOUBLE )
    {
      m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeSplitFlag( rpcBestCU, 0, uiDepth, true );
      rpcBestCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
      rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      rpcBestCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcBestCU->getTotalBits(), rpcBestCU->getTotalDistortion() );
      m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);
    }
  }

  // copy original YUV samples to PCM buffer
  if( rpcBestCU->getTotalCost()!=MAX_DOUBLE && rpcBestCU->isLosslessCoded(0) && (rpcBestCU->getIPCMFlag(0) == false))
  {
    xFillPCMBuffer(rpcBestCU, m_ppcOrigYuv[uiDepth]);
  }

  if( uiDepth == pps.getMaxCuDQPDepth() )
  {
    Int idQP = m_pcEncCfg->getMaxDeltaQP();
    iMinQP = Clip3( -sps.getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP-idQP );
    iMaxQP = Clip3( -sps.getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP+idQP );
  }
  else if( uiDepth < pps.getMaxCuDQPDepth() )
  {
    iMinQP = iBaseQP;
    iMaxQP = iBaseQP;
  }
  else
  {
    const Int iStartQP = rpcTempCU->getQP(0);
    iMinQP = iStartQP;
    iMaxQP = iStartQP;
  }

  if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() )
  {
    iMinQP = iBaseQP - m_lumaQPOffset;
    iMaxQP = iMinQP;
  }

  if ( m_pcEncCfg->getUseRateCtrl() )
  {
    iMinQP = m_pcRateCtrl->getRCQP();
    iMaxQP = m_pcRateCtrl->getRCQP();
  }

  if ( m_pcEncCfg->getCUTransquantBypassFlagForceValue() )
  {
    iMaxQP = iMinQP; // If all TUs are forced into using transquant bypass, do not loop here.
  }

  const Bool bSubBranch = bBoundary || !( m_pcEncCfg->getUseEarlyCU() && rpcBestCU->getTotalCost()!=MAX_DOUBLE && rpcBestCU->isSkipped(0) );

  if( bSubBranch && uiDepth < sps.getLog2DiffMaxMinCodingBlockSize() && (!getFastDeltaQp() || uiWidth > fastDeltaQPCuMaxSize || bBoundary))
  {
    // further split
    Double splitTotalCost = 0;

    for (Int iQP=iMinQP; iQP<=iMaxQP; iQP++)
    {
      const Bool bIsLosslessMode = false; // False at this level. Next level down may set it to true.

      rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

      UChar       uhNextDepth         = uiDepth+1;
      TComDataCU* pcSubBestPartCU     = m_ppcBestCU[uhNextDepth];
      TComDataCU* pcSubTempPartCU     = m_ppcTempCU[uhNextDepth];
      DEBUG_STRING_NEW(sTempDebug)

      for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
      {
        pcSubBestPartCU->initSubCU( rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP );           // clear sub partition datas or init.
        pcSubTempPartCU->initSubCU( rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP );           // clear sub partition datas or init.

        if( ( pcSubBestPartCU->getCUPelX() < sps.getPicWidthInLumaSamples() ) && ( pcSubBestPartCU->getCUPelY() < sps.getPicHeightInLumaSamples() ) )
        {
          if ( 0 == uiPartUnitIdx) //initialize RD with previous depth buffer
          {
            m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
          }
          else
          {
            m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
          }

#if AMP_ENC_SPEEDUP
          DEBUG_STRING_NEW(sChild)
          if ( !(rpcBestCU->getTotalCost()!=MAX_DOUBLE && rpcBestCU->isInter(0)) )
          {
            xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth DEBUG_STRING_PASS_INTO(sChild), NUMBER_OF_PART_SIZES );
          }
          else
          {

            xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth DEBUG_STRING_PASS_INTO(sChild), rpcBestCU->getPartitionSize(0) );
          }
          DEBUG_STRING_APPEND(sTempDebug, sChild)
#else
          xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth );
#endif

          rpcTempCU->copyPartFrom( pcSubBestPartCU, uiPartUnitIdx, uhNextDepth );         // Keep best part data to current temporary data.
          xCopyYuv2Tmp( pcSubBestPartCU->getTotalNumPart()*uiPartUnitIdx, uhNextDepth );
          if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() && pps.getMaxCuDQPDepth() >= 1 )
          {
            splitTotalCost += pcSubBestPartCU->getTotalCost();
          }
        }
        else
        {
          pcSubBestPartCU->copyToPic( uhNextDepth );
          rpcTempCU->copyPartFrom( pcSubBestPartCU, uiPartUnitIdx, uhNextDepth );
        }
      }

      m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
      if( !bBoundary )
      {
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 0, uiDepth, true );
        if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() && pps.getMaxCuDQPDepth() >= 1 )
        {
          Int splitBits = m_pcEntropyCoder->getNumberOfWrittenBits();
          Double splitBitCost = m_pcRdCost->calcRdCost( splitBits, 0 );
          splitTotalCost += splitBitCost;
        }

        rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
        rpcTempCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }

      if ( m_pcEncCfg->getLumaLevelToDeltaQPMapping().isEnabled() && pps.getMaxCuDQPDepth() >= 1 )
      {
        rpcTempCU->getTotalCost() = splitTotalCost;
      }
      else
      {
        rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
      }

      if( uiDepth == pps.getMaxCuDQPDepth() && pps.getUseDQP())
      {
        Bool hasResidual = false;
        for( UInt uiBlkIdx = 0; uiBlkIdx < rpcTempCU->getTotalNumPart(); uiBlkIdx ++)
        {
          if( (     rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Y)
                || (rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Cb) && (numberValidComponents > COMPONENT_Cb))
                || (rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Cr) && (numberValidComponents > COMPONENT_Cr)) ) )
          {
            hasResidual = true;
            break;
          }
        }

        if ( hasResidual )
        {
          m_pcEntropyCoder->resetBits();
          m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
          rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
          rpcTempCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
          rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

          Bool foundNonZeroCbf = false;
          rpcTempCU->setQPSubCUs( rpcTempCU->getRefQP( 0 ), 0, uiDepth, foundNonZeroCbf );
          assert( foundNonZeroCbf );
        }
        else
        {
          rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
        }
      }

      m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

      // If the configuration being tested exceeds the maximum number of bytes for a slice / slice-segment, then
      // a proper RD evaluation cannot be performed. Therefore, termination of the
      // slice/slice-segment must be made prior to this CTU.
      // This can be achieved by forcing the decision to be that of the rpcTempCU.
      // The exception is each slice / slice-segment must have at least one CTU.
      if (rpcBestCU->getTotalCost()!=MAX_DOUBLE)
      {
        const Bool isEndOfSlice        =    pcSlice->getSliceMode()==FIXED_NUMBER_OF_BYTES
                                         && ((pcSlice->getSliceBits()+rpcBestCU->getTotalBits())>pcSlice->getSliceArgument()<<3)
                                         && rpcBestCU->getCtuRsAddr() != pcPic->getPicSym()->getCtuTsToRsAddrMap(pcSlice->getSliceCurStartCtuTsAddr())
                                         && rpcBestCU->getCtuRsAddr() != pcPic->getPicSym()->getCtuTsToRsAddrMap(pcSlice->getSliceSegmentCurStartCtuTsAddr());
        const Bool isEndOfSliceSegment =    pcSlice->getSliceSegmentMode()==FIXED_NUMBER_OF_BYTES
                                         && ((pcSlice->getSliceSegmentBits()+rpcBestCU->getTotalBits()) > pcSlice->getSliceSegmentArgument()<<3)
                                         && rpcBestCU->getCtuRsAddr() != pcPic->getPicSym()->getCtuTsToRsAddrMap(pcSlice->getSliceSegmentCurStartCtuTsAddr());
                                             // Do not need to check slice condition for slice-segment since a slice-segment is a subset of a slice.
        if(isEndOfSlice||isEndOfSliceSegment)
        {
          rpcBestCU->getTotalCost()=MAX_DOUBLE;
        }
      }
	  if (CUResetPart == 0)//���Reset��־λΪFalse����ô�ǵ�һ��ִ�У���Ҫ���С����Reset��־ΪTrue���Ҹ�CU��ָ���Ļ������ͣ���ô����Ҫ�������С����û�У���Ҫ���С�
	  {
		  //printf("------RD compare current larger prediction with sub---------\n");
		  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTempDebug) DEBUG_STRING_PASS_INTO(false)); // RD compare current larger prediction with sub partitioned prediction.
	  }
	  else if (CUTargetMode[TempCount] == 255)
	  {
		  //printf("ResetCheckBestMode---------%d", TempCount);
		  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTempDebug) DEBUG_STRING_PASS_INTO(false));
		  //printf("done\n");
	  }
	  else
	  {
		  //printf("No Need To Check%d\n", TempCount);
	  }
      //xCheckBestMode( rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTempDebug) DEBUG_STRING_PASS_INTO(false) ); // RD compare current larger prediction
     //  if(( CUResetPart == 0))	  printf("%d============Nx2N ==============%d %d %d\n", rpcBestCU->getWidth(0),rpcBestCU->getPartitionSize(0),TempCount,CUTargetMode[TempCount]);                                                                                                                                                // with sub partitioned prediction.
    }
  }

  //���������汣��CU���ս��
	if (rpcBestCU->getSlice()->getSliceType() != I_SLICE)//PocIndex==6
	{
		CUDepth[TempCount] += (int)rpcBestCU->getDepth(0);
		//  printf("============TempCount==============%d \n", TempCount);
		CUPartSize[TempCount] = rpcBestCU->getPartitionSize(0);
		//cout<<"CUPartSize["<<TempCount<<"]="<<CUPartSize[TempCount]<<endl;
		//if(( CUResetPart != 0))	  printf("%d============Nx2N ==============%d %d %d\n", rpcBestCU->getWidth(0),rpcBestCU->getPartitionSize(0),TempCount,CUTargetMode[TempCount]);
	}

	//*****************



	//*****************






  DEBUG_STRING_APPEND(sDebug_, sDebug);

  rpcBestCU->copyToPic(uiDepth);                                                     // Copy Best data to Picture for next partition prediction.

  xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getCtuRsAddr(), rpcBestCU->getZorderIdxInCtu(), uiDepth, uiDepth );   // Copy Yuv data to picture Yuv
  if (bBoundary)
  {
    return;
  }

  // Assert if Best prediction mode is NONE
  // Selected mode's RD-cost must be not MAX_DOUBLE.
  assert( rpcBestCU->getPartitionSize ( 0 ) != NUMBER_OF_PART_SIZES       );
  assert( rpcBestCU->getPredictionMode( 0 ) != NUMBER_OF_PREDICTION_MODES );
  assert( rpcBestCU->getTotalCost     (   ) != MAX_DOUBLE                 );
}

/** finish encoding a cu and handle end-of-slice conditions
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TEncCu::finishCU( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  TComPic* pcPic = pcCU->getPic();
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());

  //Calculate end address
  const Int  currentCTUTsAddr = pcPic->getPicSym()->getCtuRsToTsAddrMap(pcCU->getCtuRsAddr());
  const Bool isLastSubCUOfCtu = pcCU->isLastSubCUOfCtu(uiAbsPartIdx);
  if ( isLastSubCUOfCtu )
  {
    // The 1-terminating bit is added to all streams, so don't add it here when it's 1.
    // i.e. when the slice segment CurEnd CTU address is the current CTU address+1.
    if (pcSlice->getSliceSegmentCurEndCtuTsAddr() != currentCTUTsAddr+1)
    {
      m_pcEntropyCoder->encodeTerminatingBit( 0 );
    }
  }
}

/** Compute QP for each CU
 * \param pcCU Target CU
 * \param uiDepth CU depth
 * \returns quantization parameter
 */
Int TEncCu::xComputeQP( TComDataCU* pcCU, UInt uiDepth )
{
  Int iBaseQp = pcCU->getSlice()->getSliceQp();
  Int iQpOffset = 0;
  if ( m_pcEncCfg->getUseAdaptiveQP() )
  {
    TEncPic* pcEPic = dynamic_cast<TEncPic*>( pcCU->getPic() );
    UInt uiAQDepth = min( uiDepth, pcEPic->getMaxAQDepth()-1 );
    TEncPicQPAdaptationLayer* pcAQLayer = pcEPic->getAQLayer( uiAQDepth );
    UInt uiAQUPosX = pcCU->getCUPelX() / pcAQLayer->getAQPartWidth();
    UInt uiAQUPosY = pcCU->getCUPelY() / pcAQLayer->getAQPartHeight();
    UInt uiAQUStride = pcAQLayer->getAQPartStride();
    TEncQPAdaptationUnit* acAQU = pcAQLayer->getQPAdaptationUnit();

    Double dMaxQScale = pow(2.0, m_pcEncCfg->getQPAdaptationRange()/6.0);
    Double dAvgAct = pcAQLayer->getAvgActivity();
    Double dCUAct = acAQU[uiAQUPosY * uiAQUStride + uiAQUPosX].getActivity();
    Double dNormAct = (dMaxQScale*dCUAct + dAvgAct) / (dCUAct + dMaxQScale*dAvgAct);
    Double dQpOffset = log(dNormAct) / log(2.0) * 6.0;
    iQpOffset = Int(floor( dQpOffset + 0.49999 ));
  }

  return Clip3(-pcCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQp+iQpOffset );
}

/** encode a CU block recursively
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TEncCu::xEncodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
        TComPic   *const pcPic   = pcCU->getPic();
        TComSlice *const pcSlice = pcCU->getSlice();
  const TComSPS   &sps =*(pcSlice->getSPS());
  const TComPPS   &pps =*(pcSlice->getPPS());

  const UInt maxCUWidth  = sps.getMaxCUWidth();
  const UInt maxCUHeight = sps.getMaxCUHeight();

        Bool bBoundary = false;
        UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  const UInt uiRPelX   = uiLPelX + (maxCUWidth>>uiDepth)  - 1;
        UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  const UInt uiBPelY   = uiTPelY + (maxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX < sps.getPicWidthInLumaSamples() ) && ( uiBPelY < sps.getPicHeightInLumaSamples() ) )
  {
    m_pcEntropyCoder->encodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < sps.getLog2DiffMaxMinCodingBlockSize() ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartitionsInCtu() >> (uiDepth<<1) )>>2;
    if( uiDepth == pps.getMaxCuDQPDepth() && pps.getUseDQP())
    {
      setdQPFlag(true);
    }

    if( uiDepth == pps.getPpsRangeExtension().getDiffCuChromaQpOffsetDepth() && pcSlice->getUseChromaQpAdj())
    {
      setCodeChromaQpAdjFlag(true);
    }

    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      if( ( uiLPelX < sps.getPicWidthInLumaSamples() ) && ( uiTPelY < sps.getPicHeightInLumaSamples() ) )
      {
        xEncodeCU( pcCU, uiAbsPartIdx, uiDepth+1 );
      }
    }
    return;
  }

  if( uiDepth <= pps.getMaxCuDQPDepth() && pps.getUseDQP())
  {
    setdQPFlag(true);
  }

  if( uiDepth <= pps.getPpsRangeExtension().getDiffCuChromaQpOffsetDepth() && pcSlice->getUseChromaQpAdj())
  {
    setCodeChromaQpAdjFlag(true);
  }

  if (pps.getTransquantBypassEnabledFlag())
  {
    m_pcEntropyCoder->encodeCUTransquantBypassFlag( pcCU, uiAbsPartIdx );
  }

  if( !pcSlice->isIntra() )
  {
    m_pcEntropyCoder->encodeSkipFlag( pcCU, uiAbsPartIdx );
  }

  if( pcCU->isSkipped( uiAbsPartIdx ) )
  {
    m_pcEntropyCoder->encodeMergeIndex( pcCU, uiAbsPartIdx );
    finishCU(pcCU,uiAbsPartIdx);
    return;
  }

  m_pcEntropyCoder->encodePredMode( pcCU, uiAbsPartIdx );
  m_pcEntropyCoder->encodePartSize( pcCU, uiAbsPartIdx, uiDepth );

  if (pcCU->isIntra( uiAbsPartIdx ) && pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N )
  {
    m_pcEntropyCoder->encodeIPCMInfo( pcCU, uiAbsPartIdx );

    if(pcCU->getIPCMFlag(uiAbsPartIdx))
    {
      // Encode slice finish
      finishCU(pcCU,uiAbsPartIdx);
      return;
    }
  }

  // prediction Info ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyCoder->encodePredInfo( pcCU, uiAbsPartIdx );

  // Encode Coefficients
  Bool bCodeDQP = getdQPFlag();
  Bool codeChromaQpAdj = getCodeChromaQpAdjFlag();
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, bCodeDQP, codeChromaQpAdj );
  setCodeChromaQpAdjFlag( codeChromaQpAdj );
  setdQPFlag( bCodeDQP );

  // --- write terminating bit ---
  finishCU(pcCU,uiAbsPartIdx);
}

Int xCalcHADs8x8_ISlice(Pel *piOrg, Int iStrideOrg)
{
  Int k, i, j, jj;
  Int diff[64], m1[8][8], m2[8][8], m3[8][8], iSumHad = 0;

  for( k = 0; k < 64; k += 8 )
  {
    diff[k+0] = piOrg[0] ;
    diff[k+1] = piOrg[1] ;
    diff[k+2] = piOrg[2] ;
    diff[k+3] = piOrg[3] ;
    diff[k+4] = piOrg[4] ;
    diff[k+5] = piOrg[5] ;
    diff[k+6] = piOrg[6] ;
    diff[k+7] = piOrg[7] ;

    piOrg += iStrideOrg;
  }

  //horizontal
  for (j=0; j < 8; j++)
  {
    jj = j << 3;
    m2[j][0] = diff[jj  ] + diff[jj+4];
    m2[j][1] = diff[jj+1] + diff[jj+5];
    m2[j][2] = diff[jj+2] + diff[jj+6];
    m2[j][3] = diff[jj+3] + diff[jj+7];
    m2[j][4] = diff[jj  ] - diff[jj+4];
    m2[j][5] = diff[jj+1] - diff[jj+5];
    m2[j][6] = diff[jj+2] - diff[jj+6];
    m2[j][7] = diff[jj+3] - diff[jj+7];

    m1[j][0] = m2[j][0] + m2[j][2];
    m1[j][1] = m2[j][1] + m2[j][3];
    m1[j][2] = m2[j][0] - m2[j][2];
    m1[j][3] = m2[j][1] - m2[j][3];
    m1[j][4] = m2[j][4] + m2[j][6];
    m1[j][5] = m2[j][5] + m2[j][7];
    m1[j][6] = m2[j][4] - m2[j][6];
    m1[j][7] = m2[j][5] - m2[j][7];

    m2[j][0] = m1[j][0] + m1[j][1];
    m2[j][1] = m1[j][0] - m1[j][1];
    m2[j][2] = m1[j][2] + m1[j][3];
    m2[j][3] = m1[j][2] - m1[j][3];
    m2[j][4] = m1[j][4] + m1[j][5];
    m2[j][5] = m1[j][4] - m1[j][5];
    m2[j][6] = m1[j][6] + m1[j][7];
    m2[j][7] = m1[j][6] - m1[j][7];
  }

  //vertical
  for (i=0; i < 8; i++)
  {
    m3[0][i] = m2[0][i] + m2[4][i];
    m3[1][i] = m2[1][i] + m2[5][i];
    m3[2][i] = m2[2][i] + m2[6][i];
    m3[3][i] = m2[3][i] + m2[7][i];
    m3[4][i] = m2[0][i] - m2[4][i];
    m3[5][i] = m2[1][i] - m2[5][i];
    m3[6][i] = m2[2][i] - m2[6][i];
    m3[7][i] = m2[3][i] - m2[7][i];

    m1[0][i] = m3[0][i] + m3[2][i];
    m1[1][i] = m3[1][i] + m3[3][i];
    m1[2][i] = m3[0][i] - m3[2][i];
    m1[3][i] = m3[1][i] - m3[3][i];
    m1[4][i] = m3[4][i] + m3[6][i];
    m1[5][i] = m3[5][i] + m3[7][i];
    m1[6][i] = m3[4][i] - m3[6][i];
    m1[7][i] = m3[5][i] - m3[7][i];

    m2[0][i] = m1[0][i] + m1[1][i];
    m2[1][i] = m1[0][i] - m1[1][i];
    m2[2][i] = m1[2][i] + m1[3][i];
    m2[3][i] = m1[2][i] - m1[3][i];
    m2[4][i] = m1[4][i] + m1[5][i];
    m2[5][i] = m1[4][i] - m1[5][i];
    m2[6][i] = m1[6][i] + m1[7][i];
    m2[7][i] = m1[6][i] - m1[7][i];
  }

  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 8; j++)
    {
      iSumHad += abs(m2[i][j]);
    }
  }
  iSumHad -= abs(m2[0][0]);
  iSumHad =(iSumHad+2)>>2;
  return(iSumHad);
}

Int  TEncCu::updateCtuDataISlice(TComDataCU* pCtu, Int width, Int height)
{
  Int  xBl, yBl;
  const Int iBlkSize = 8;

  Pel* pOrgInit   = pCtu->getPic()->getPicYuvOrg()->getAddr(COMPONENT_Y, pCtu->getCtuRsAddr(), 0);
  Int  iStrideOrig = pCtu->getPic()->getPicYuvOrg()->getStride(COMPONENT_Y);
  Pel  *pOrg;

  Int iSumHad = 0;
  for ( yBl=0; (yBl+iBlkSize)<=height; yBl+= iBlkSize)
  {
    for ( xBl=0; (xBl+iBlkSize)<=width; xBl+= iBlkSize)
    {
      pOrg = pOrgInit + iStrideOrig*yBl + xBl;
      iSumHad += xCalcHADs8x8_ISlice(pOrg, iStrideOrig);
    }
  }
  return(iSumHad);
}

/** check RD costs for a CU block encoded with merge
 * \param rpcBestCU
 * \param rpcTempCU
 * \param earlyDetectionSkipMode
 */
Void TEncCu::xCheckRDCostMerge2Nx2N( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU DEBUG_STRING_FN_DECLARE(sDebug), Bool *earlyDetectionSkipMode )
{
  assert( rpcTempCU->getSlice()->getSliceType() != I_SLICE );
  if(getFastDeltaQp())
  {
    return;   // never check merge in fast deltaqp mode
  }
  TComMvField  cMvFieldNeighbours[2 * MRG_MAX_NUM_CANDS]; // double length for mv of both lists
  UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
  Int numValidMergeCand = 0;
  const Bool bTransquantBypassFlag = rpcTempCU->getCUTransquantBypass(0);

  for( UInt ui = 0; ui < rpcTempCU->getSlice()->getMaxNumMergeCand(); ++ui )
  {
    uhInterDirNeighbours[ui] = 0;
  }
  UChar uhDepth = rpcTempCU->getDepth( 0 );
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to CTU level
  rpcTempCU->getInterMergeCandidates( 0, 0, cMvFieldNeighbours,uhInterDirNeighbours, numValidMergeCand );

  Int mergeCandBuffer[MRG_MAX_NUM_CANDS];
  for( UInt ui = 0; ui < numValidMergeCand; ++ui )
  {
    mergeCandBuffer[ui] = 0;
  }

  Bool bestIsSkip = false;

  UInt iteration;
  if ( rpcTempCU->isLosslessCoded(0))
  {
    iteration = 1;
  }
  else
  {
    iteration = 2;
  }
  DEBUG_STRING_NEW(bestStr)

  for( UInt uiNoResidual = 0; uiNoResidual < iteration; ++uiNoResidual )
  {
    for( UInt uiMergeCand = 0; uiMergeCand < numValidMergeCand; ++uiMergeCand )
    {
      if(!(uiNoResidual==1 && mergeCandBuffer[uiMergeCand]==1))
      {
        if( !(bestIsSkip && uiNoResidual == 0) )
        {
          DEBUG_STRING_NEW(tmpStr)
          // set MC parameters
          rpcTempCU->setPredModeSubParts( MODE_INTER, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setCUTransquantBypassSubParts( bTransquantBypassFlag, 0, uhDepth );
          rpcTempCU->setChromaQpAdjSubParts( bTransquantBypassFlag ? 0 : m_cuChromaQpOffsetIdxPlus1, 0, uhDepth );
          rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setMergeFlagSubParts( true, 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setMergeIndexSubParts( uiMergeCand, 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeCand], 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[0 + 2*uiMergeCand], SIZE_2Nx2N, 0, 0 ); // interprets depth relative to rpcTempCU level
          rpcTempCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[1 + 2*uiMergeCand], SIZE_2Nx2N, 0, 0 ); // interprets depth relative to rpcTempCU level

          // do MC
          m_pcPredSearch->motionCompensation ( rpcTempCU, m_ppcPredYuvTemp[uhDepth] );
          // estimate residual and encode everything
          m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU,
                                                     m_ppcOrigYuv    [uhDepth],
                                                     m_ppcPredYuvTemp[uhDepth],
                                                     m_ppcResiYuvTemp[uhDepth],
                                                     m_ppcResiYuvBest[uhDepth],
                                                     m_ppcRecoYuvTemp[uhDepth],
                                                     (uiNoResidual != 0) DEBUG_STRING_PASS_INTO(tmpStr) );

#if DEBUG_STRING
          DebugInterPredResiReco(tmpStr, *(m_ppcPredYuvTemp[uhDepth]), *(m_ppcResiYuvBest[uhDepth]), *(m_ppcRecoYuvTemp[uhDepth]), DebugStringGetPredModeMask(rpcTempCU->getPredictionMode(0)));
#endif

          if ((uiNoResidual == 0) && (rpcTempCU->getQtRootCbf(0) == 0))
          {
            // If no residual when allowing for one, then set mark to not try case where residual is forced to 0
            mergeCandBuffer[uiMergeCand] = 1;
          }

          Int orgQP = rpcTempCU->getQP( 0 );
          xCheckDQP( rpcTempCU );
          xCheckBestMode(rpcBestCU, rpcTempCU, uhDepth DEBUG_STRING_PASS_INTO(bestStr) DEBUG_STRING_PASS_INTO(tmpStr));

          rpcTempCU->initEstData( uhDepth, orgQP, bTransquantBypassFlag );

          if( m_pcEncCfg->getUseFastDecisionForMerge() && !bestIsSkip )
          {
            bestIsSkip = rpcBestCU->getQtRootCbf(0) == 0;
          }
        }
      }
    }

    if(uiNoResidual == 0 && m_pcEncCfg->getUseEarlySkipDetection())
    {
      if(rpcBestCU->getQtRootCbf( 0 ) == 0)
      {
        if( rpcBestCU->getMergeFlag( 0 ))
        {
          *earlyDetectionSkipMode = true;
        }
        else if(m_pcEncCfg->getMotionEstimationSearchMethod() != MESEARCH_SELECTIVE)
        {
          Int absoulte_MV=0;
          for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
          {
            if ( rpcBestCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
            {
              TComCUMvField* pcCUMvField = rpcBestCU->getCUMvField(RefPicList( uiRefListIdx ));
              Int iHor = pcCUMvField->getMvd( 0 ).getAbsHor();
              Int iVer = pcCUMvField->getMvd( 0 ).getAbsVer();
              absoulte_MV+=iHor+iVer;
            }
          }

          if(absoulte_MV == 0)
          {
            *earlyDetectionSkipMode = true;
          }
        }
      }
    }
  }
  DEBUG_STRING_APPEND(sDebug, bestStr)
}


#if AMP_MRG
Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize DEBUG_STRING_FN_DECLARE(sDebug), Bool bUseMRG)
#else
Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize )
#endif
{
  DEBUG_STRING_NEW(sTest)

  if(getFastDeltaQp())
  {
    const TComSPS &sps=*(rpcTempCU->getSlice()->getSPS());
    const UInt fastDeltaQPCuMaxSize = Clip3(sps.getMaxCUHeight()>>(sps.getLog2DiffMaxMinCodingBlockSize()), sps.getMaxCUHeight(), 32u);
    if(ePartSize != SIZE_2Nx2N || rpcTempCU->getWidth( 0 ) > fastDeltaQPCuMaxSize)
    {
      return; // only check necessary 2Nx2N Inter in fast deltaqp mode
    }
  }

  // prior to this, rpcTempCU will have just been reset using rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
  UChar uhDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setPartSizeSubParts  ( ePartSize,  0, uhDepth );
  rpcTempCU->setPredModeSubParts  ( MODE_INTER, 0, uhDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_cuChromaQpOffsetIdxPlus1, 0, uhDepth );

#if AMP_MRG
  rpcTempCU->setMergeAMP (true);
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] DEBUG_STRING_PASS_INTO(sTest), false, bUseMRG );
#else
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] );
#endif

#if AMP_MRG
  if ( !rpcTempCU->getMergeAMP() )
  {
    return;
  }
#endif

  m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcResiYuvBest[uhDepth], m_ppcRecoYuvTemp[uhDepth], false DEBUG_STRING_PASS_INTO(sTest) );
  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

#if DEBUG_STRING
  DebugInterPredResiReco(sTest, *(m_ppcPredYuvTemp[uhDepth]), *(m_ppcResiYuvBest[uhDepth]), *(m_ppcRecoYuvTemp[uhDepth]), DebugStringGetPredModeMask(rpcTempCU->getPredictionMode(0)));
#endif

  xCheckDQP( rpcTempCU );
  xCheckBestMode(rpcBestCU, rpcTempCU, uhDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTest));
}

Void TEncCu::xCheckRDCostIntra( TComDataCU *&rpcBestCU,
                                TComDataCU *&rpcTempCU,
                                PartSize     eSize
                                DEBUG_STRING_FN_DECLARE(sDebug) )
{
  DEBUG_STRING_NEW(sTest)

  if(getFastDeltaQp())
  {
    const TComSPS &sps=*(rpcTempCU->getSlice()->getSPS());
    const UInt fastDeltaQPCuMaxSize = Clip3(sps.getMaxCUHeight()>>(sps.getLog2DiffMaxMinCodingBlockSize()), sps.getMaxCUHeight(), 32u);
    if(rpcTempCU->getWidth( 0 ) > fastDeltaQPCuMaxSize)
    {
      return; // only check necessary 2Nx2N Intra in fast deltaqp mode
    }
  }

  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setSkipFlagSubParts( false, 0, uiDepth );

  rpcTempCU->setPartSizeSubParts( eSize, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_cuChromaQpOffsetIdxPlus1, 0, uiDepth );

  Pel resiLuma[NUMBER_OF_STORED_RESIDUAL_TYPES][MAX_CU_SIZE * MAX_CU_SIZE];

  m_pcPredSearch->estIntraPredLumaQT( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest) );

  m_ppcRecoYuvTemp[uiDepth]->copyToPicComponent(COMPONENT_Y, rpcTempCU->getPic()->getPicYuvRec(), rpcTempCU->getCtuRsAddr(), rpcTempCU->getZorderIdxInCtu() );

  if (rpcBestCU->getPic()->getChromaFormat()!=CHROMA_400)
  {
    m_pcPredSearch->estIntraPredChromaQT( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest) );
  }

  m_pcEntropyCoder->resetBits();

  if ( rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnabledFlag())
  {
    m_pcEntropyCoder->encodeCUTransquantBypassFlag( rpcTempCU, 0,          true );
  }

  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePredMode( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePartSize( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodePredInfo( rpcTempCU, 0 );
  m_pcEntropyCoder->encodeIPCMInfo(rpcTempCU, 0, true );

  // Encode Coefficients
  Bool bCodeDQP = getdQPFlag();
  Bool codeChromaQpAdjFlag = getCodeChromaQpAdjFlag();
  m_pcEntropyCoder->encodeCoeff( rpcTempCU, 0, uiDepth, bCodeDQP, codeChromaQpAdjFlag );
  setCodeChromaQpAdjFlag( codeChromaQpAdjFlag );
  setdQPFlag( bCodeDQP );

  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
  rpcTempCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckDQP( rpcTempCU );

  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTest));
}


/** Check R-D costs for a CU with PCM mode.
 * \param rpcBestCU pointer to best mode CU data structure
 * \param rpcTempCU pointer to testing mode CU data structure
 * \returns Void
 *
 * \note Current PCM implementation encodes sample values in a lossless way. The distortion of PCM mode CUs are zero. PCM mode is selected if the best mode yields bits greater than that of PCM mode.
 */
Void TEncCu::xCheckIntraPCM( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  if(getFastDeltaQp())
  {
    const TComSPS &sps=*(rpcTempCU->getSlice()->getSPS());
    const UInt fastDeltaQPCuMaxPCMSize = Clip3((UInt)1<<sps.getPCMLog2MinSize(), (UInt)1<<sps.getPCMLog2MaxSize(), 32u);
    if (rpcTempCU->getWidth( 0 ) > fastDeltaQPCuMaxPCMSize)
    {
      return;   // only check necessary PCM in fast deltaqp mode
    }
  }
  
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setSkipFlagSubParts( false, 0, uiDepth );

  rpcTempCU->setIPCMFlag(0, true);
  rpcTempCU->setIPCMFlagSubParts (true, 0, rpcTempCU->getDepth(0));
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  rpcTempCU->setTrIdxSubParts ( 0, 0, uiDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_cuChromaQpOffsetIdxPlus1, 0, uiDepth );

  m_pcPredSearch->IPCMSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth]);

  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

  m_pcEntropyCoder->resetBits();

  if ( rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnabledFlag())
  {
    m_pcEntropyCoder->encodeCUTransquantBypassFlag( rpcTempCU, 0,          true );
  }

  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePredMode ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePartSize ( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodeIPCMInfo ( rpcTempCU, 0, true );

  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
  rpcTempCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckDQP( rpcTempCU );
  DEBUG_STRING_NEW(a)
  DEBUG_STRING_NEW(b)
  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(a) DEBUG_STRING_PASS_INTO(b));
}

/** check whether current try is the best with identifying the depth of current try
 * \param rpcBestCU
 * \param rpcTempCU
 * \param uiDepth
 */
Void TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth DEBUG_STRING_FN_DECLARE(sParent) DEBUG_STRING_FN_DECLARE(sTest) DEBUG_STRING_PASS_INTO(Bool bAddSizeInfo) )
{
  if( rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost() )
  {
    TComYuv* pcYuv;
    // Change Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;

    // Change Prediction data
    pcYuv = m_ppcPredYuvBest[uiDepth];
    m_ppcPredYuvBest[uiDepth] = m_ppcPredYuvTemp[uiDepth];
    m_ppcPredYuvTemp[uiDepth] = pcYuv;

    // Change Reconstruction data
    pcYuv = m_ppcRecoYuvBest[uiDepth];
    m_ppcRecoYuvBest[uiDepth] = m_ppcRecoYuvTemp[uiDepth];
    m_ppcRecoYuvTemp[uiDepth] = pcYuv;

    pcYuv = NULL;
    pcCU  = NULL;

    // store temp best CI for next CU coding
    m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);


#if DEBUG_STRING
    DEBUG_STRING_SWAP(sParent, sTest)
    const PredMode predMode=rpcBestCU->getPredictionMode(0);
    if ((DebugOptionList::DebugString_Structure.getInt()&DebugStringGetPredModeMask(predMode)) && bAddSizeInfo)
    {
      std::stringstream ss(stringstream::out);
      ss <<"###: " << (predMode==MODE_INTRA?"Intra   ":"Inter   ") << partSizeToString[rpcBestCU->getPartitionSize(0)] << " CU at " << rpcBestCU->getCUPelX() << ", " << rpcBestCU->getCUPelY() << " width=" << UInt(rpcBestCU->getWidth(0)) << std::endl;
      sParent+=ss.str();
    }
#endif
  }
}

Void TEncCu::xCheckDQP( TComDataCU* pcCU )
{
  UInt uiDepth = pcCU->getDepth( 0 );

  const TComPPS &pps = *(pcCU->getSlice()->getPPS());
  if ( pps.getUseDQP() && uiDepth <= pps.getMaxCuDQPDepth() )
  {
    if ( pcCU->getQtRootCbf( 0) )
    {
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( pcCU, 0, false );
      pcCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
      pcCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      pcCU->getTotalCost() = m_pcRdCost->calcRdCost( pcCU->getTotalBits(), pcCU->getTotalDistortion() );
    }
    else
    {
      pcCU->setQPSubParts( pcCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
    }
  }
}

Void TEncCu::xCopyAMVPInfo (AMVPInfo* pSrc, AMVPInfo* pDst)
{
  pDst->iN = pSrc->iN;
  for (Int i = 0; i < pSrc->iN; i++)
  {
    pDst->m_acMvCand[i] = pSrc->m_acMvCand[i];
  }
}
Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsPartIdx, UInt uiDepth, UInt uiSrcDepth )
{
  UInt uiAbsPartIdxInRaster = g_auiZscanToRaster[uiAbsPartIdx];
  UInt uiSrcBlkWidth = rpcPic->getNumPartInCtuWidth() >> (uiSrcDepth);
  UInt uiBlkWidth    = rpcPic->getNumPartInCtuWidth() >> (uiDepth);
  UInt uiPartIdxX = ( ( uiAbsPartIdxInRaster % rpcPic->getNumPartInCtuWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
  UInt uiPartIdxY = ( ( uiAbsPartIdxInRaster / rpcPic->getNumPartInCtuWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
  UInt uiPartIdx = uiPartIdxY * ( uiSrcBlkWidth / uiBlkWidth ) + uiPartIdxX;
  m_ppcRecoYuvBest[uiSrcDepth]->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsPartIdx, uiDepth - uiSrcDepth, uiPartIdx);

  m_ppcPredYuvBest[uiSrcDepth]->copyToPicYuv( rpcPic->getPicYuvPred (), uiCUAddr, uiAbsPartIdx, uiDepth - uiSrcDepth, uiPartIdx);
}

Void TEncCu::xCopyYuv2Tmp( UInt uiPartUnitIdx, UInt uiNextDepth )
{
  UInt uiCurrDepth = uiNextDepth - 1;
  m_ppcRecoYuvBest[uiNextDepth]->copyToPartYuv( m_ppcRecoYuvTemp[uiCurrDepth], uiPartUnitIdx );
  m_ppcPredYuvBest[uiNextDepth]->copyToPartYuv( m_ppcPredYuvBest[uiCurrDepth], uiPartUnitIdx);
}

/** Function for filling the PCM buffer of a CU using its original sample array
 * \param pCU pointer to current CU
 * \param pOrgYuv pointer to original sample array
 */
Void TEncCu::xFillPCMBuffer     ( TComDataCU* pCU, TComYuv* pOrgYuv )
{
  const ChromaFormat format = pCU->getPic()->getChromaFormat();
  const UInt numberValidComponents = getNumberValidComponents(format);
  for (UInt componentIndex = 0; componentIndex < numberValidComponents; componentIndex++)
  {
    const ComponentID component = ComponentID(componentIndex);

    const UInt width  = pCU->getWidth(0)  >> getComponentScaleX(component, format);
    const UInt height = pCU->getHeight(0) >> getComponentScaleY(component, format);

    Pel *source      = pOrgYuv->getAddr(component, 0, width);
    Pel *destination = pCU->getPCMSample(component);

    const UInt sourceStride = pOrgYuv->getStride(component);

    for (Int line = 0; line < height; line++)
    {
      for (Int column = 0; column < width; column++)
      {
        destination[column] = source[column];
      }

      source      += sourceStride;
      destination += width;
    }
  }
}

#if ADAPTIVE_QP_SELECTION
/** Collect ARL statistics from one block
  */
Int TEncCu::xTuCollectARLStats(TCoeff* rpcCoeff, TCoeff* rpcArlCoeff, Int NumCoeffInCU, Double* cSum, UInt* numSamples )
{
  for( Int n = 0; n < NumCoeffInCU; n++ )
  {
    TCoeff u = abs( rpcCoeff[ n ] );
    TCoeff absc = rpcArlCoeff[ n ];

    if( u != 0 )
    {
      if( u < LEVEL_RANGE )
      {
        cSum[ u ] += ( Double )absc;
        numSamples[ u ]++;
      }
      else
      {
        cSum[ LEVEL_RANGE ] += ( Double )absc - ( Double )( u << ARL_C_PRECISION );
        numSamples[ LEVEL_RANGE ]++;
      }
    }
  }

  return 0;
}

//! Collect ARL statistics from one CTU
Void TEncCu::xCtuCollectARLStats(TComDataCU* pCtu )
{
  Double cSum[ LEVEL_RANGE + 1 ];     //: the sum of DCT coefficients corresponding to data type and quantization output
  UInt numSamples[ LEVEL_RANGE + 1 ]; //: the number of coefficients corresponding to data type and quantization output

  TCoeff* pCoeffY = pCtu->getCoeff(COMPONENT_Y);
  TCoeff* pArlCoeffY = pCtu->getArlCoeff(COMPONENT_Y);
  const TComSPS &sps = *(pCtu->getSlice()->getSPS());

  const UInt uiMinCUWidth = sps.getMaxCUWidth() >> sps.getMaxTotalCUDepth(); // NOTE: ed - this is not the minimum CU width. It is the square-root of the number of coefficients per part.
  const UInt uiMinNumCoeffInCU = 1 << uiMinCUWidth;                          // NOTE: ed - what is this?

  memset( cSum, 0, sizeof( Double )*(LEVEL_RANGE+1) );
  memset( numSamples, 0, sizeof( UInt )*(LEVEL_RANGE+1) );

  // Collect stats to cSum[][] and numSamples[][]
  for(Int i = 0; i < pCtu->getTotalNumPart(); i ++ )
  {
    UInt uiTrIdx = pCtu->getTransformIdx(i);

    if(pCtu->isInter(i) && pCtu->getCbf( i, COMPONENT_Y, uiTrIdx ) )
    {
      xTuCollectARLStats(pCoeffY, pArlCoeffY, uiMinNumCoeffInCU, cSum, numSamples);
    }//Note that only InterY is processed. QP rounding is based on InterY data only.

    pCoeffY  += uiMinNumCoeffInCU;
    pArlCoeffY  += uiMinNumCoeffInCU;
  }

  for(Int u=1; u<LEVEL_RANGE;u++)
  {
    m_pcTrQuant->getSliceSumC()[u] += cSum[ u ] ;
    m_pcTrQuant->getSliceNSamples()[u] += numSamples[ u ] ;
  }
  m_pcTrQuant->getSliceSumC()[LEVEL_RANGE] += cSum[ LEVEL_RANGE ] ;
  m_pcTrQuant->getSliceNSamples()[LEVEL_RANGE] += numSamples[ LEVEL_RANGE ] ;
}
#endif
//! \}
