#include "io_github_melvincabatuan_voronoi_MainActivity.h"

#include <android/bitmap.h>
#include <stdlib.h>

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;


Mat edge, dist, labels, output;
int edgeThresh = 80;

static const Scalar colors[] =
    {
        Scalar(0,0,0),
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };


/*
 * Class:     io_github_melvincabatuan_voronoi_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_io_github_melvincabatuan_voronoi_MainActivity_process
  (JNIEnv * pEnv, jobject pClass, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent;

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   ///  YUV420sp source --->  cv::Mat
    Mat srcGray(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
  
   ///  BGRA --->  cv::Mat
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);

/***************************************************************************************************/
 
    if (edge.empty())
        edge = Mat(srcGray.size(), srcGray.type());     
       
    edge = srcGray >= edgeThresh;   /// 21 fps with 'pEdge == NULL' routine
                                    /// 23-24 fps ' .empty() ' routine and SIMPLER!!!
                                    
    if (dist.empty()) 
        dist = Mat(srcGray.size(), CV_32FC1); 
        
    if (labels.empty()) 
        labels = Mat(srcGray.size(), CV_32SC1);               
                                    
    distanceTransform( edge, dist, labels, CV_DIST_C, DIST_MASK_3);                                 
                                    
  
    if (output.empty())         
        output.create(labels.size(), CV_8UC3);
    
    for( int i = 0; i < labels.rows; i++ )
        {
            const int* ll = (const int*)labels.ptr(i);
            const float* dd = (const float*)dist.ptr(i);
            uchar* d = (uchar*)output.ptr(i);
            for( int j = 0; j < labels.cols; j++ )
            {
                int idx = ll[j] == 0 || dd[j] == 0 ? 0 : (ll[j]-1)%8 + 1;
                float scale = 1.f/(1 + dd[j]*dd[j]*0.0004f);
                int b = cvRound(colors[idx][0]*scale);
                int g = cvRound(colors[idx][1]*scale);
                int r = cvRound(colors[idx][2]*scale);
                d[j*3] = (uchar)b;
                d[j*3+1] = (uchar)g;
                d[j*3+2] = (uchar)r;
            }
    }

    /// BGR ---> BGRA
    cvtColor(output, mbgra, CV_BGR2BGRA);

/***************************************************************************************************/

    /// Release Java byte buffer and unlock backing bitmap
    pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();
}
