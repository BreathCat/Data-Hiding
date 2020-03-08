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

/** \file     encmain.cpp
    \brief    Encoder application main
*/

#include <time.h>
#include <iostream>
#include<math.h>
#include "TAppEncTop.h"
#include "TAppCommon/program_options_lite.h"

//! \ingroup TAppEncoder
//! \{

#include "../Lib/TLibCommon/Debug.h"

/***********************�������************************
��Ϣ���ط�����
������Ϣ����txt�ļ���
Ȼ�����exe��ȡtxt�ļ����޸�8*8PU��ȫ������64*64CTU��ģ��߽�������ߴ�CTU��û���ء�
��ʼд�����ɾ��txt�ļ�
��ȡbin�ļ�PU��Ϣ
����PU��Ϣ�õ�������Ϣ

�����ص㣺
����ֻ�ܶ�����encmain�����Ȼ����def.h������������def.c����Ȼ����ʧ�ܣ�
ȫ�ֱ�����������

***********************�������*************************/
//�Զ������
//��ȡPU�õ�

int InterPU = 0;
int IntraPU = 0;
int SkipPU = 0;
int intra_pre_mode_index=0;//�Լ�����ģ��ڼ�֡��
long intra_pre_mode[100][35]={0} ;//�Լ������֡��Ԥ��ģʽ��ͳ������
long I_PU_number[100][5]={1};//�Լ������I֡PU����ͳ��  4*4------------0;8*8------------1;16*16------------2;32*32------------3;64*64------------4;
long P_PU_number[100][25]={1};//�Լ������P֡PU����ͳ��
long I_CU_number[100][4]={0};//�Լ������I֡CU����ͳ��
long P_CU_number[100][4]={0};//�Լ������P֡CU����ͳ��
 long intra[100]={0};//intra ��PU��Ŀ
 long inter[100]={0};//inter ��PU��Ŀ
 long skip[100]={0};//skip ��PU��Ŀ
 int MessStr[100] = {0};//ԭʼ������Ϣ

int TotalNum = 0; //���鳤��
//��ȡPU�õ�

int CTUIndex = 0;//�ۼ�CTU������
int CUComCount = -1;//�ۼ�CUCom����
int CurrentPOC = 0;//��ǵ�ǰ֡��

int CUDepth[85] = { 0 };//����CTU�Ļ������
int CUPartSize[85] = { 255 };//����CU��PU�Ļ���ģʽ

int CUResetPart= 0;//�Ƿ�����CU�Ļ���ģʽ��־λ
int CUTargetMode[85] = { 255 };//��ǰCU��Ŀ�껮��ģʽ
int judgeMode = 0; //8-64*64 4-32*32 2-16*16 1-8*8(����λ) 15-all 31-����8*8��2N*2N

double Capacity = 0;//ͳ��Ƕ������
int isorg = 0;//�Ƿ�ΪδǶ����Ϣѹ��
int PUcategeory[4][8];//ͳ����������  0-64*64 1-32*32 2-16*16 3-8*8   0-2N*2N 1-2N*N 2-N*2N 3-N*N 4-2NxnU 5-2NxnD 6-nLx2N 7-nRx2N 
int FPUcategeory[4][8];//ͳ�Ƶ�һ��P֡������
FILE* fp;


//By lzh�Զ������
int EMD_16_CUTargetMode[16] = { 111 };  //���·ֳ�16X16��CU���±꣬��85����PU���ж�16�������һ��CTU��8��16x16��CU����ǰ8��ȫ�����±�.
int CUnum_16 = 0;//һ��CTU���16x16��CU����
int EMD_32_CUTargetMode[4] = { 111 };  
int CUnum_32 = 0;

int EMD_64_CUTargetMode[1] = { 111 };  
int CUnum_64 = 0;
int EMD_8_CUTargetMode[64] = { 111 };  
int CUnum_8 = 0;

int TOTAL_8 = 0; //8x8CUnumber �ܵ�
int TOTAL_16 = 0;
int TOTAL_32 = 0;
int TOTAL_64 = 0;



// ====================================================================================================================
// Main function
// ====================================================================================================================

int main(int argc, char* argv[])
{
  //freopen("data_chianspeed_04.txt","w",stdout);
  // print information
  /*fprintf( fp, "\n" );
  fprintf( fp, "HM software: Encoder Version [%s] (including RExt)", NV_VERSION );
  fprintf( fp, NVM_ONOS );
  fprintf( fp, NVM_COMPILEDBY );
  fprintf( fp, NVM_BITS );
  fprintf( fp, "\n\n" );*/
  for(int countj=0;countj<1;countj++){//-----------------------------------------yyy

	  Capacity = 0;
	  CUResetPart= 0;//�Ƿ�����CU�Ļ���ģʽ��־λ
	  for(int i=0;i<85;i++) CUTargetMode[i] =  255 ;//��ǰCU��Ŀ�껮��ģʽ
	  for(int i=0;i<85;i++) CUDepth[i] =  0 ;//����CTU�Ļ������
      for(int i=0;i<85;i++) CUPartSize[i] =  255 ;//����CU��PU�Ļ���ģʽ
	  CTUIndex = 0;//�ۼ�CTU������
      CUComCount = -1;//�ۼ�CUCom����
      CurrentPOC = 0;//��ǵ�ǰ֡��
	  for(int i=0;i<4;i++)
		  for(int j=0;j<8;j++)
			PUcategeory[i][j]=0;//ͳ����������  0-64*64 1-32*32 2-16*16 3-8*8   0-2N*2N 1-2N*N 2-N*2N 3-N*N 4-2NxnU 5-2NxnD 6-nLx2N 7-nRx2N 
	  for(int i=0;i<4;i++)
		  for(int j=0;j<8;j++)
			FPUcategeory[i][j]=0;//ͳ�Ƶ�һ��P֡������
	/*  switch(countj){//-------------------------------------yyy
	  case 0:
		  judgeMode = 0;
		  break;
	  case 1://8*8
		  judgeMode = 17;
		  break;
	  case 2://8*8 16*16
		  judgeMode = 19;
		  break;
	  case 3://8*8 16*16 32*32
		  judgeMode = 23;
		  break;
	  case 4://8*8 16*16 32*32 64*64
		  judgeMode = 31;
		  break;
	  }*/
	  judgeMode = 0;

 TAppEncTop  cTAppEncTop;
  // create application encoder class
  cTAppEncTop.create();
  // parse configuration
  try
  {
    if(!cTAppEncTop.parseCfg( argc, argv ))
    {
      cTAppEncTop.destroy();
#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
      EnvVar::printEnvVar();
#endif
      return 1;
    }
  }
  catch (df::program_options_lite::ParseFailure &e)
  {
    std::cerr << "Error parsing option \""<< e.arg <<"\" with argument \""<< e.val <<"\"." << std::endl;
    return 1;
  }

#if PRINT_MACRO_VALUES
  printMacroSettings();
#endif

#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
  EnvVar::printEnvVarInUse();
#endif

  // starting time
  Double dResult;
  clock_t lBefore = clock();

  // call encoding function
  cTAppEncTop.encode();

  // ending time
  dResult = (Double)(clock()-lBefore) / CLOCKS_PER_SEC;
  printf("\n Total Time: %12.3f sec.\n", dResult);

  // destroy application encoder class
  cTAppEncTop.destroy();

  printf("The capacity of this sequence is : %.2f bits.\n", Capacity);
  printf("The quantity of 8X8 16X16 32X32 64X64 CUs in this sequence is : %d %d %d %d\n", TOTAL_8,TOTAL_16,TOTAL_32,TOTAL_64);
  //printf("--------------------------------------------judgeMode:%d------------------------------------------\n",judgeMode);
  printf("	2N*2N	2N*N	N*2N	N*N	2NxnU	2NxnD	nLx2N	nRx2N\n");
  for(int i=0;i<4;i++){
	  printf("%d*%d",(64/((int)(pow(2,(float)i)))),(64/((int)(pow(2,(float)i)))));
	  for(int j=0;j<8;j++){
		  printf("	%d", PUcategeory[i][j]);
	  }
	  printf("\n");
  }
  printf("---------------------------��һ��P֡------------------------------------\n");
  for(int i=0;i<4;i++){
	  printf("%d*%d",(64/((int)(pow(2,(float)i)))),(64/((int)(pow(2,(float)i)))));
	  for(int j=0;j<8;j++){
		  printf("	%d", FPUcategeory[i][j]);
	  }
	  printf("\n");
  }
	  /* printf("2N*2N : The original total PU number is : %d		First PU number : %d\n", PUcategeory[0],FPUcategeory[0]);
	   printf("N*N : The original total PU number is : %d		First PU number : %d\n", PUcategeory[1],FPUcategeory[1]);
	   printf("N*2N : The original total PU number is : %d		First PU number : %d\n", PUcategeory[2],FPUcategeory[2]);
	   printf("2N*N : The original total PU number is : %d		First PU number : %d\n", PUcategeory[3],FPUcategeory[3]);
	   printf("2N*2N : The changed total PU number is : %d		First PU number : %d\n", PUcategeoryC[0],FPUcategeoryC[0]);
	   printf("N*N : The changed total PU number is : %d		First PU number : %d\n", PUcategeoryC[1],FPUcategeoryC[1]);
	   printf("N*2N : The changed total PU number is : %d		First PU number : %d\n", PUcategeoryC[2],FPUcategeoryC[2]);
	   printf("2N*N : The changed total PU number is : %d		First PU number : %d\n", PUcategeoryC[3],FPUcategeoryC[3]);*/
 // fclose(fp);
  //getchar();
   printf("\n\n\n\n\n");
  }
  return 0;
}

//! \}
