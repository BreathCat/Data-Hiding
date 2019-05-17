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
#include <cstdio> 
#include "TAppEncTop.h"
#include "TAppCommon/program_options_lite.h"

//! \ingroup TAppEncoder
//! \{

#include "../Lib/TLibCommon/Debug.h"

//By Aslan �Զ������
int CTUIndex = 0;//�ۼ�CTU������
int CUComCount = -1;//�ۼ�CUCom����
int CurrentPOC = 0;//��ǵ�ǰ֡��

int CUDepth[85] = { 0 };//����CTU�Ļ������
int CUPartSize[85] = { 255 };//����CU��PU�Ļ���ģʽ

int CUResetPart= 0;//�Ƿ�����CU�Ļ���ģʽ��־λ
int CUTargetMode[85] = { 255 };//��ǰCU��Ŀ�껮��ģʽ

double Capacity = 0;//ͳ��Ƕ������
double PSNRValue = 0;//��¼PSNR���
double PSNRValue_Y = 0;//��¼PSNR���
double PSNRValue_U = 0;//��¼PSNR���
double PSNRValue_V = 0;//��¼PSNR���
double BitRateValue = 0;//��¼�����ʽ��
int RecordFlag = 0;
int TOTAL_8 = 0; //8x8CUnumber �ܵ�
int TOTAL_16 = 0;
int TOTAL_32 = 0;
int TOTAL_64 = 0;
extern string binname;

//By lzh�Զ������
int EMD_16_CUTargetMode[16] = { 111 };  //���·ֳ�16X16��CU���±꣬��85����PU���ж�16�������һ��CTU��8��16x16��CU����ǰ8��ȫ�����±�.
int CUnum_16 = 0;//һ��CTU���16x16��CU����
int EMD_32_CUTargetMode[4] = { 111 };  
int CUnum_32 = 0;
int EMD_64_CUTargetMode[1] = { 111 };  
int CUnum_64 = 0;
int EMD_8_CUTargetMode[64] = { 111 };  
int CUnum_8 = 0;
int PUcategeory[4][8];//ͳ����������  0-64*64 1-32*32 2-16*16 3-8*8   0-2N*2N 1-2N*N 2-N*2N 3-N*N 4-2NxnU 5-2NxnD 6-nLx2N 7-nRx2N 
int FPUcategeory[4][8];//ͳ�Ƶ�һ��P֡������

// ====================================================================================================================
// Main function
// ====================================================================================================================

int main(int argc, char* argv[])
{
  TAppEncTop  cTAppEncTop;
  //freopen("data_parksence.txt","w",stdout);
  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "HM software: Encoder Version [%s] (including RExt)", NV_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
  fprintf( stdout, "\n\n" );
    CurrentPOC = 0;//��ǵ�ǰ֡��
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


  //int i = 0;

  //for (i = 0; i < 85; i++)
  //{
	 // printf("%2dDepth:  %d \tPartMod:  %d\n", i, CUDepth[i], CUPartSize[i]);
  //}

  printf("The capacity of this sequence is : %.2f bits.\n", Capacity);
  printf("The quantity of 8X8 16X16 32X32 64X64 CUs in this sequence is : %d %d %d %d\n", TOTAL_8,TOTAL_16,TOTAL_32,TOTAL_64);
  
 // printf("--------------------------------------------judgeMode:%d------------------------------------------\n",judgeMode);
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

  //����Ϊԭл�ĳ������ļ���ʽ

 // ofstream EncodeDataFile(binname+"Origin.txt");


 // if (EncodeDataFile.is_open())
 // {
	////  string temp = "";
	//  
	//  char buffer [20];
	//  sprintf(buffer,"%.4f",PSNRValue);
	//  string temp = buffer;
	////  temp = to_string(static_cast<long long>(PSNRValue));
	//  EncodeDataFile << "PSNR    " + temp + "\n";

	//  sprintf(buffer,"%.4f",PSNRValue_Y);
	//  temp = buffer;
	//  EncodeDataFile << "PSNR_Y    " + temp + "\n";
	//  
	//  sprintf(buffer,"%.4f",PSNRValue_U);
	//  temp = buffer;
	//  EncodeDataFile << "PSNR_U    " + temp + "\n";

	//  sprintf(buffer,"%.4f",PSNRValue_V);
	//  temp = buffer;
	//  EncodeDataFile << "PSNR_V  " + temp + "\n";

	//  temp = to_string(static_cast<long long>(BitRateValue));
	//  EncodeDataFile << "BITR    " + temp + "\n";

	//  temp = to_string(static_cast<long long>(Capacity));
	//  EncodeDataFile << "CAPY    " + temp + "\n";

	//  EncodeDataFile.close();
	//  
 // }
 // else
 // {
	//  printf("File Open Filed\n");
 // }

    //����Ϊԭл�ĳ������ļ���ʽ

 // getchar(); //���������ʱ�����������
  return 0;
}

//! \}
