//
//  omp.hpp
//  OpenCV-CLM
//
//  Created by Ryohei Kameyama on 2015/12/15.
//  Copyright © 2015年 codingcafe.jp. All rights reserved.
//

#ifndef omp_hpp
#define omp_hpp

#include <stdio.h>

void omp_set_num_threads(int n);
int omp_get_thread_num();

#endif /* omp_hpp */
