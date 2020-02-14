#ifndef _DEF_H
#define _DEF_H

extern int intra_pre_mode_index;//自己定义的：第几帧数
extern long intra_pre_mode[100][35] ;//自己定义的帧内预测模式的统计数组
<<<<<<< HEAD
extern long I_PU_number[610][5];//自己定义的帧内PU划分统计 4*4------------0;8*8------------1;16*16------------2;32*32------------3;64*64------------4;
extern long P_PU_number[610][25];//自己定义的帧间PU划分统计 
=======
extern long I_PU_number[100][5];//自己定义的帧内PU划分统计 4*4------------0;8*8------------1;16*16------------2;32*32------------3;64*64------------4;
extern long P_PU_number[100][25];//自己定义的帧间PU划分统计 
>>>>>>> 5f2ebbb95f2fe99e9d4441362507b3cd0f283905
extern long I_CU_number[100][4];//自己定义的I帧CU划分统计8*8------------0;16*16------------1;32*32------------2;64*64------------3;
extern long P_CU_number[100][4];//自己定义的P帧CU划分统计8*8------------0;16*16------------1;32*32------------2;64*64------------3;
extern long intra[100];//intra 的PU数目
extern long inter[100];//inter 的PU数目
extern long skip[100];//skip 的PU数目
<<<<<<< HEAD
extern int GOPsize;
=======
<<<<<<< HEAD
=======
extern int MessStr[100] ;//原始隐秘信息
extern int ThNum[100] ;//三进制数
extern int m ;//三进制数数组下标
extern int TotalNum ;
>>>>>>> 5f93801e4e5046a8b92461b082a5e27f5696033c
>>>>>>> 5f2ebbb95f2fe99e9d4441362507b3cd0f283905
#endif