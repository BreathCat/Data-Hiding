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

/***********************代码解释************************
信息隐藏方案：
隐藏信息存在txt文件里
然后调用exe读取txt文件，修改8*8PU（全都是在64*64CTU里的，边界非正常尺寸CTU都没隐藏。
开始写入后几秒删除txt文件
提取bin文件PU信息
根据PU信息得到隐藏信息

代码特点：
变量只能定义在encmain函数里，然后用def.h声明。不能用def.c，不然链接失败，
全局变量不能引用

***********************代码解释*************************/
//自定义变量
//提取PU用的

int InterPU = 0;
int IntraPU = 0;
int SkipPU = 0;
int intra_pre_mode_index=0;//自己定义的：第几帧数
long intra_pre_mode[100][35]={0} ;//自己定义的帧内预测模式的统计数组
long I_PU_number[100][5]={1};//自己定义的I帧PU划分统计  4*4------------0;8*8------------1;16*16------------2;32*32------------3;64*64------------4;
long P_PU_number[100][25]={1};//自己定义的P帧PU划分统计
long I_CU_number[100][4]={0};//自己定义的I帧CU划分统计
long P_CU_number[100][4]={0};//自己定义的P帧CU划分统计
 long intra[100]={0};//intra 的PU数目
 long inter[100]={0};//inter 的PU数目
 long skip[100]={0};//skip 的PU数目
 int MessStr[100] = {0};//原始隐秘信息

int TotalNum = 0; //数组长度
//提取PU用的

int CTUIndex = 0;//累计CTU的数量
int CUComCount = -1;//累计CUCom调用
int CurrentPOC = 0;//标记当前帧号

int CUDepth[85] = { 0 };//保存CTU的划分深度
int CUPartSize[85] = { 255 };//保存CU内PU的划分模式

int CUResetPart= 0;//是否重设CU的划分模式标志位
int CUTargetMode[85] = { 255 };//当前CU的目标划分模式
int judgeMode = 0; //8-64*64 4-32*32 2-16*16 1-8*8(后四位) 15-all 31-屏蔽8*8的2N*2N

double Capacity = 0;//统计嵌入容量
int isorg = 0;//是否为未嵌入信息压缩
int PUcategeory[4][8];//统计类型总数  0-64*64 1-32*32 2-16*16 3-8*8   0-2N*2N 1-2N*N 2-N*2N 3-N*N 4-2NxnU 5-2NxnD 6-nLx2N 7-nRx2N 
int FPUcategeory[4][8];//统计第一个P帧的类型
FILE* fp;


//By lzh自定义变量
int EMD_16_CUTargetMode[16] = { 111 };  //存下分成16X16的CU的下标，从85个总PU中判断16个。如果一个CTU有8个16x16的CU，则前8个全都赋下标.
int CUnum_16 = 0;//一个CTU里的16x16的CU总数
int EMD_32_CUTargetMode[4] = { 111 };  
int CUnum_32 = 0;

int EMD_64_CUTargetMode[1] = { 111 };  
int CUnum_64 = 0;
int EMD_8_CUTargetMode[64] = { 111 };  
int CUnum_8 = 0;

int TOTAL_8 = 0; //8x8CUnumber 总的
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
	  CUResetPart= 0;//是否重设CU的划分模式标志位
	  for(int i=0;i<85;i++) CUTargetMode[i] =  255 ;//当前CU的目标划分模式
	  for(int i=0;i<85;i++) CUDepth[i] =  0 ;//保存CTU的划分深度
      for(int i=0;i<85;i++) CUPartSize[i] =  255 ;//保存CU内PU的划分模式
	  CTUIndex = 0;//累计CTU的数量
      CUComCount = -1;//累计CUCom调用
      CurrentPOC = 0;//标记当前帧号
	  for(int i=0;i<4;i++)
		  for(int j=0;j<8;j++)
			PUcategeory[i][j]=0;//统计类型总数  0-64*64 1-32*32 2-16*16 3-8*8   0-2N*2N 1-2N*N 2-N*2N 3-N*N 4-2NxnU 5-2NxnD 6-nLx2N 7-nRx2N 
	  for(int i=0;i<4;i++)
		  for(int j=0;j<8;j++)
			FPUcategeory[i][j]=0;//统计第一个P帧的类型
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
  printf("---------------------------第一个P帧------------------------------------\n");
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
