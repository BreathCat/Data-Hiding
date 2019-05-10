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

//By Aslan 自定义变量
int CTUIndex = 0;//累计CTU的数量
int CUComCount = -1;//累计CUCom调用
int CurrentPOC = 0;//标记当前帧号

int CUDepth[85] = { 0 };//保存CTU的划分深度
int CUPartSize[85] = { 255 };//保存CU内PU的划分模式

int CUResetPart= 0;//是否重设CU的划分模式标志位
int CUTargetMode[85] = { 255 };//当前CU的目标划分模式

double Capacity = 0;//统计嵌入容量
double PSNRValue = 0;//记录PSNR结果
double BitRateValue = 0;//记录比特率结果
int RecordFlag = 0;

extern string binname;

//By lzh自定义变量
int EMD_16_CUTargetMode[17] = { 111 };  //存下分成16X16的CU的下标，从85个总PU中判断16个。如果一个CTU有8个16x16的CU，则前8个全都赋下标.
int CUnum_16 = 0;//一个CTU里的16x16的CU总数
// ====================================================================================================================
// Main function
// ====================================================================================================================

int main(int argc, char* argv[])
{
  TAppEncTop  cTAppEncTop;

  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "HM software: Encoder Version [%s] (including RExt)", NV_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
  fprintf( stdout, "\n\n" );

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



  ofstream EncodeDataFile(binname+"Origin.txt");


  if (EncodeDataFile.is_open())
  {
	//  string temp = "";
	  
	  char buffer [20];
	  sprintf(buffer,"%.4f",PSNRValue);
	  string temp = buffer;
	//  temp = to_string(static_cast<long long>(PSNRValue));
	  EncodeDataFile << "PSNR    " + temp + "\n";

	  temp = to_string(static_cast<long long>(BitRateValue));
	  EncodeDataFile << "BITR    " + temp + "\n";

	  temp = to_string(static_cast<long long>(Capacity));
	  EncodeDataFile << "CAPY    " + temp + "\n";

	  EncodeDataFile.close();
	  
  }
  else
  {
	  printf("File Open Filed\n");
  }



  getchar();
  return 0;
}

//! \}
