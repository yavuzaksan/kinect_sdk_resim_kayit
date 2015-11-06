#include <iostream>
#include <fstream>
#include <windows.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
 
#include"NuiImageBuffer.h"

#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>

 
using namespace std;
using namespace cv;


int exitflag=0;
int countrgbsaved;
int countdepthsaved;

string countrgb;
string countdepth;

//HOGDescriptor hog;
//vector<Rect> found, found_filtered;


 
interface INuiInstance;
#define COLOR_WIDTH 640
#define COLOR_HIGHT 480
#define DEPTH_WIDTH 640
#define DEPTH_HIGHT 480
#define CHANNEL 3

BYTE buf[DEPTH_WIDTH*DEPTH_HIGHT*CHANNEL];
 

int c;   //variable for assinging  cvWaitkey()


int drawColor(HANDLE h,IplImage* color)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame( h, 0, &pImageFrame );
	if( FAILED( hr ) )
    {
		cout<<"Get Image Frame Failed"<<endl;
		return -1;
    }
 
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
 
	NUI_LOCKED_RECT LockedRect;
 
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );

 
	if( LockedRect.Pitch != 0 )
	{
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		
		cvSetData(color,pBuffer,LockedRect.Pitch);
		
		Mat img =  cvarrToMat(color);

    
		//cvtColor(img,img , CV_RGBA2GRAY);  

				
		if((c=='f')||(c=='F'))
		{
			ostringstream o;
			o<<"rgb"<<countrgbsaved<<".jpg";
			imwrite(o.str(),img);
			cout<<"saved:"<<o.str()<<endl;
			countrgbsaved++;
		}

		
		imshow("RGB image",img);
		
		

		
		 //hog.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, 2);
		
	     //cvShowImage("color image",color);
		
	}
 
	//cvShowImage("color image",color);
 
	NuiImageStreamReleaseFrame( h, pImageFrame );
 
	return 0;
 
}
 
 
int drawDepth(HANDLE h,IplImage* depth)
{
 
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
 
	HRESULT hr = NuiImageStreamGetNextFrame( h, 0, &pImageFrame );
 
	if( FAILED( hr ) )
	{
		cout<<"Get Image Frame Failed"<<endl;
		return -1;
	}
 
	 
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
 
	NUI_LOCKED_RECT LockedRect;
 
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
 
	if( LockedRect.Pitch != 0 )
	{
		USHORT * pBuff = (USHORT*) LockedRect.pBits;
 
		for(int i=0;i<DEPTH_WIDTH*DEPTH_HIGHT;i++)
		{
			BYTE index = pBuff[i]&0x07;
			
			USHORT realDepth = (pBuff[i]&0xFFF8)>>3;
			
			BYTE scale = 255 - (BYTE)(256*realDepth/0x0fff);
			
			buf[CHANNEL*i] = buf[CHANNEL*i+1] = buf[CHANNEL*i+2] = 0;
 
			switch( index )
			{
 
				case 0:
 
					buf[CHANNEL*i]=scale/2;
 
					buf[CHANNEL*i+1]=scale/2;
 
					buf[CHANNEL*i+2]=scale/2;
 
				break;
 
				case 1:
 
					buf[CHANNEL*i]=scale;
 
				break;
 
				case 2:
 
					buf[CHANNEL*i+1]=scale;
 
				break;
	
				case 3:
 
					buf[CHANNEL*i+2]=scale;
 
				break;
 
				case 4:
 
					buf[CHANNEL*i]=scale;
 
					buf[CHANNEL*i+1]=scale;
 
				break;
 
				case 5:
 
					buf[CHANNEL*i]=scale;
 
					buf[CHANNEL*i+2]=scale;
 
				break;
 
				case 6:
 
					buf[CHANNEL*i+1]=scale;
 
					buf[CHANNEL*i+2]=scale;
 
				break;
 
				case 7:
 
					buf[CHANNEL*i]=255-scale/2;
 
					buf[CHANNEL*i+1]=255-scale/2;
 
					buf[CHANNEL*i+2]=255-scale/2;
 
				break;
 
			}
 
		}
 
	cvSetData(depth,buf,DEPTH_WIDTH*CHANNEL);
 
	Mat imgdepth =  cvarrToMat(depth);


		if((c=='f')||(c=='F'))
		{
			ostringstream o;
			o<<"depth"<<countdepthsaved<<".jpg";
			imwrite(o.str(),imgdepth);
			cout<<"saved:"<<o.str()<<endl;
			countdepthsaved++;
		}

		
		imshow("DEPTH image",imgdepth);


	}
 
	NuiImageStreamReleaseFrame( h, pImageFrame );
 	 
	return 0;
 
}
 

 
int main(int argc,char * argv[])
{
	
	ifstream dosya1("counters.txt",ios::app);
	if(!dosya1){
	
		cout<<"counters.txt  can not open !!!!!!!!!!!!!!"<<endl;
	
			return 0;
	}
	
	getline(dosya1,countrgb);
	getline(dosya1,countdepth);
	countrgbsaved= stoi(countrgb);
	countdepthsaved= stoi(countdepth);
		
	dosya1.close();

	cout<<"counters read successfully"<<endl;
		

	//hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	IplImage* color = cvCreateImageHeader(cvSize(COLOR_WIDTH,COLOR_HIGHT),IPL_DEPTH_8U,4);
 
	IplImage* depth = cvCreateImageHeader(cvSize(DEPTH_WIDTH,DEPTH_HIGHT),IPL_DEPTH_8U,CHANNEL);
 
	 
	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH|NUI_INITIALIZE_FLAG_USES_COLOR);
 
	if( hr != S_OK )
	{

		cout<<"NuiInitialize failed"<<endl;
		return hr;

	}
 
	HANDLE h1 = CreateEvent( NULL, TRUE, FALSE, NULL );
 
	HANDLE h2 = NULL;
 
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,NUI_IMAGE_RESOLUTION_640x480, 0, 2, h1, &h2);
 
	if( FAILED( hr ) )
 	{
 
		cout<<"Could not open image stream video"<<endl;
		
		return hr;
 
	}
 
	HANDLE h3 = CreateEvent( NULL, TRUE, FALSE, NULL );
 
	HANDLE h4 = NULL;
 
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480, 0, 2, h3, &h4);
 
	if( FAILED( hr ) )
	{
 
		cout<<"Could not open depth stream video"<<endl;
 
		return hr;
 
	}
 
	
 
	while(1)
	{
 
		WaitForSingleObject(h1,INFINITE);
 
		drawColor(h2,color);
 
		WaitForSingleObject(h3,INFINITE);
 
		drawDepth(h4,depth);
 
		 
		c = cvWaitKey(1);
 
		if( c == 27 || c == 'q' || c == 'Q' ){
		
			ofstream dosya1("counters.txt");
			
			dosya1.clear();

			dosya1<<countrgbsaved<<endl;
			dosya1<<countdepthsaved<<endl;
			
			dosya1.close();
			
			cout<<"closing application is successful"<<endl;

			break;
		
		
		}
		
	}
 
	cvReleaseImageHeader(&depth);
 
	cvReleaseImageHeader(&color);
 	 
	NuiShutdown();
 
	return 0;
	
}