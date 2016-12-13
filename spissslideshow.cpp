/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spissslideshow.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "windows.h"
#include "scrnsave.h"
#include "resource.h"
#include "math.h"

#include "FreeImage.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
#define MAX_BALLS	7
#define pi			3.141592625
*/
BYTE global_alpha=200;

#define szAppName	"spissslideshow 1.0"
#define szAuthor	"written by Stephane Poirier, © 2014"
#define szPreview	"spissslideshow"

string global_imagefolder="."; //local folder of the .scr file
//string global_imagefolder="c:\\temp\\spivideocaptureframetodisk\\";
//string global_imagefolder="c:\\temp\\spissmandelbrotmidi\\";
//string global_imagefolder="c:\\temp\\spicapturewebcam\\";
vector<string> global_txtfilenames;
int global_imageid=0;
FIBITMAP* global_dib;
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_slideshowrate_ms=1000;

/*
typedef struct _BALLS
{
	UINT	uBallID;
	HICON	hIcon;
	int		x;
	int		y;
	int		angle;
}BALLS;

BALLS gBalls[] = {IDI_REDBALL,		NULL, 0, 0, 0,
				  IDI_GREENBALL,	NULL, 0, 0, 25,
				  IDI_BLUEBALL,		NULL, 0, 0, 50,
				  IDI_PURPLEBALL,	NULL, 0, 0, 75,
				  IDI_LIGHTREDBALL, NULL, 0, 0, 100,
				  IDI_YELLOWBALL,	NULL, 0, 0, 125,
				  IDI_BLACKBALL,	NULL, 0, 0, 150};
*/

LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int i = 0;
	static PAINTSTRUCT ps = {NULL};
	static HDC hDC = NULL;
	//static HBRUSH hBrush = NULL;
	static UINT uTimer = 0;
	static int xpos, ypos;
	//static RECT rc;
	
	switch(message)
	{
	case WM_CREATE:
		{
			//spi, avril 2015, begin
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);
			//SetLayeredWindowAttributes(h, 0, 200, LWA_ALPHA);
			//spi, avril 2015, end
			/*
			for(i = 0; i < MAX_BALLS; i++)
				gBalls[i].hIcon = (HICON)LoadImage(hMainInstance, MAKEINTRESOURCE(gBalls[i].uBallID), IMAGE_ICON, 48, 48, LR_DEFAULTSIZE);

			xpos = GetSystemMetrics(SM_CXSCREEN) / 2;
			ypos = GetSystemMetrics(SM_CYSCREEN) / 2;

			for(i = 0; i < MAX_BALLS; i++)
			{
				double alpha = gBalls[i].angle * pi / 180;
				gBalls[i].x = xpos + int((xpos - 30) * sin(alpha) * cos(alpha) * cos(2 * alpha));
				gBalls[i].y = ypos - 30 + int(265 * cos(alpha));
			}

			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			*/
			//0) execute cmd line to get all folder's image filenames
			string quote = "\"";
			string pathfilter;
			string path=global_imagefolder;
			//pathfilter = path + "\\*.bmp";
			pathfilter = path + "\\*.jpg";
			string systemcommand;
			//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
			systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spiss_filenames.txt"; // /S for adding path into "wsic_filenames.txt"
			system(systemcommand.c_str());

			//2) load in all "spiss_filenames.txt" file
			//vector<string> global_txtfilenames;
			ifstream ifs("spiss_filenames.txt");
			string temp;
			while(getline(ifs,temp))
			{
				//txtfilenames.push_back(path + "\\" + temp);
				global_txtfilenames.push_back(temp);
			}
			/*
			global_imagewidth=GetSystemMetrics(SM_CXSCREEN);
			global_imageheight=GetSystemMetrics(SM_CYSCREEN);
			*/
			RECT rect;
			GetClientRect( hWnd, &rect );
			global_imagewidth = rect.right;         
			global_imageheight = rect.bottom;

			uTimer = SetTimer(hWnd, 1, global_slideshowrate_ms, NULL);
		}
		break;

    case WM_DESTROY:
		if(uTimer) KillTimer(hWnd, uTimer);
		if(global_dib!=NULL) FreeImage_Unload(global_dib);
		/*
		if(hBrush)
			DeleteObject(hBrush);

		for(i = 0; i < MAX_BALLS; i++)
			if(gBalls[i].hIcon)
				DestroyIcon(gBalls[i].hIcon);
		*/
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		{
			xpos = GetSystemMetrics(SM_CXSCREEN) / 2;
			ypos = GetSystemMetrics(SM_CYSCREEN) / 2;
			/*
			for(i = 0; i < MAX_BALLS; i++)
			{
				double alpha = gBalls[i].angle * pi / 180;
				gBalls[i].x = xpos + int((xpos - 30) * sin(alpha) * cos(alpha) * cos(2 * alpha));
				gBalls[i].y = ypos - 30 + int(265 * cos(alpha));
				gBalls[i].angle = (gBalls[i].angle >= 360) ? 0 : gBalls[i].angle + 1;

				rc.left = gBalls[i].x;
				rc.right = gBalls[i].x + 48;
				rc.top = gBalls[i].y;
				rc.bottom = gBalls[i].y + 48;

				InvalidateRect(hWnd, &rc, FALSE);
			}
			*/
			//unload previous image
			if(global_dib!=NULL) FreeImage_Unload(global_dib);
			//if at least one image in the folder
			if(global_txtfilenames.size()>0)
			{
				//reset image id if end of file
				if(global_imageid>=global_txtfilenames.size()) global_imageid=0;
				//load new image
				//global_dib = FreeImage_Load(FIF_BMP, (global_txtfilenames[global_imageid]).c_str(), BMP_DEFAULT);
				global_dib = FreeImage_Load(FIF_JPEG, (global_txtfilenames[global_imageid]).c_str(), JPEG_DEFAULT);
				//trigger WM_PAINT
				InvalidateRect(hWnd, NULL, false);
				global_imageid++;
			}
		}
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		
		if(fChildPreview)
		{
			SetBkColor(hDC, RGB(0, 0, 0));
			SetTextColor(hDC, RGB(255, 255, 0));
			TextOut(hDC, 25, 45, szPreview, strlen(szPreview));
		}
		else
		{
			/*
			SetBkColor(hDC, RGB(0, 0, 0));
			SetTextColor(hDC, RGB(120, 120, 120));
			TextOut(hDC, 0, ypos * 2 - 40, szAppName, strlen(szAppName));
			TextOut(hDC, 0, ypos * 2 - 25, szAuthor, strlen(szAuthor));
			*/

			SetStretchBltMode(hDC, COLORONCOLOR);
			StretchDIBits(hDC, 0, 0, global_imagewidth, global_imageheight,
							0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
							FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);

			/*
			for(i = 0; i < MAX_BALLS; i++)
				DrawIconEx(hDC, gBalls[i].x, gBalls[i].y, gBalls[i].hIcon, 48, 48, 0, (HBRUSH)hBrush, DI_IMAGE);
			*/
		}
		
		EndPaint(hWnd, &ps);
		break;

    default: 
		return DefScreenSaverProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
	return TRUE;
}