
#include <windows.h>

#define WIDTHBYTES(i)   ((((i)+31) >> 5) << 2)
#define NEW_DIB_FORMAT(lpbih) (lpbih->biSize != sizeof(BITMAPCOREHEADER))
#define ISDIB(bft) ((bft) == BFT_BITMAP)
#define BFT_BITMAP 0x4d42   /* 'BM' */

int CaptureAnImage(HWND hWnd)
{
        HDC hdcScreen;
        HDC hdcWindow;
        HDC hdcMemDC = NULL;
        HBITMAP hbmScreen = NULL;
        BITMAP bmpScreen;

        // Retrieve the handle to a display device context for the client 
        // area of the window. 
        hdcScreen = GetDC(NULL);
        hdcWindow = GetDC(hWnd);

        // Create a compatible DC which is used in a BitBlt from the window DC
        hdcMemDC = CreateCompatibleDC(hdcWindow);

        if (!hdcMemDC)
        {
                MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
                //goto done;
        }

        // Get the client area for size calculation
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        //This is the best stretch mode
        SetStretchBltMode(hdcWindow, HALFTONE);

        //The source DC is the entire screen and the destination DC is the current window (HWND)
        if (!StretchBlt(hdcWindow,
                0, 0,
                rcClient.right, rcClient.bottom,
                hdcScreen,
                0, 0,
                GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN),
                SRCCOPY))
        {
                MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
                //goto done;
        }

        // Create a compatible bitmap from the Window DC
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

        if (!hbmScreen)
        {
                MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
                //goto done;
        }

        // Select the compatible bitmap into the compatible memory DC.
        SelectObject(hdcMemDC, hbmScreen);

        // Bit block transfer into our compatible memory DC.
        if (!BitBlt(hdcMemDC,
                0, 0,
                rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                hdcWindow,
                0, 0,
                SRCCOPY))
        {
                MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
                //goto done;
        }

        // Get the BITMAP from the HBITMAP
        GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmpScreen.bmWidth;
        bi.biHeight = bmpScreen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

        // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
        // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
        // have greater overhead than HeapAlloc.
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);

        // Gets the "bits" from the bitmap and copies them into a buffer 
        // which is pointed to by lpbitmap.
        GetDIBits(hdcWindow, hbmScreen, 0,
                (UINT)bmpScreen.bmHeight,
                lpbitmap,
                (BITMAPINFO*)& bi, DIB_RGB_COLORS);

        // A file is created, this is where we will save the screen capture.
        HANDLE hFile = CreateFile(L"captureqwsx.bmp",
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, NULL);

        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

        //Size of the file
        bmfHeader.bfSize = dwSizeofDIB;

        //bfType must always be BM for Bitmaps
        bmfHeader.bfType = 0x4D42; //BM   

        DWORD dwBytesWritten = 0;
        WriteFile(hFile, (LPSTR)& bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)& bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

        //Unlock and Free the DIB from the heap
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);

        //Close the handle for the file that was created
        CloseHandle(hFile);

        //Clean up
done:
        DeleteObject(hbmScreen);
        DeleteObject(hdcMemDC);
        ReleaseDC(NULL, hdcScreen);
        ReleaseDC(hWnd, hdcWindow);

        return 0;
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
        BITMAP bmp;
        PBITMAPINFO pbmi;
        WORD    cClrBits;

        if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)& bmp))
                return NULL;

        cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
        if (cClrBits == 1)
                cClrBits = 1;
        else if (cClrBits <= 4)
                cClrBits = 4;
        else if (cClrBits <= 8)
                cClrBits = 8;
        else if (cClrBits <= 16)
                cClrBits = 16;
        else if (cClrBits <= 24)
                cClrBits = 24;
        else cClrBits = 32;

        if (cClrBits < 24)
                pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
                        sizeof(BITMAPINFOHEADER) +
                        sizeof(RGBQUAD) * (1 << cClrBits));
        else
                pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
                        sizeof(BITMAPINFOHEADER));

        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = bmp.bmWidth;
        pbmi->bmiHeader.biHeight = bmp.bmHeight;
        pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
        pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
        if (cClrBits < 24)
                pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

        pbmi->bmiHeader.biCompression = BI_RGB;
        pbmi->bmiHeader.biSizeImage = 
                ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
                * pbmi->bmiHeader.biHeight;
        pbmi->bmiHeader.biClrImportant = 0;
        return pbmi;
}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
        HBITMAP hBMP, HDC hDC)
{
        HANDLE hf;                 // file handle  
        BITMAPFILEHEADER hdr;       // bitmap file-header  
        PBITMAPINFOHEADER pbih;     // bitmap info-header  
        LPBYTE lpBits;              // memory pointer  
        DWORD dwTotal;              // total count of bytes  
        DWORD cb;                   // incremental count of bytes  
        BYTE* hp;                   // byte pointer  
        DWORD dwTmp;

        pbih = (PBITMAPINFOHEADER)pbi;
        lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

        if (!lpBits)
                ;

        // Retrieve the color table (RGBQUAD array) and the bits  
        // (array of palette indices) from the DIB.  
        if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
                DIB_RGB_COLORS))
        {
                ;
        }

        // Create the .BMP file.  
        hf = CreateFile(pszFile,
                GENERIC_READ | GENERIC_WRITE,
                (DWORD)0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                (HANDLE)NULL);
        if (hf == INVALID_HANDLE_VALUE)
                ;
        hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
        // Compute the size of the entire file.  
        hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
                pbih->biSize + pbih->biClrUsed
                * sizeof(RGBQUAD) + pbih->biSizeImage);
        hdr.bfReserved1 = 0;
        hdr.bfReserved2 = 0;

        // Compute the offset to the array of color indices.  
        hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
                pbih->biSize + pbih->biClrUsed
                * sizeof(RGBQUAD);

        // Copy the BITMAPFILEHEADER into the .BMP file.  
        if (!WriteFile(hf, (LPVOID)& hdr, sizeof(BITMAPFILEHEADER),
                (LPDWORD)& dwTmp, NULL))
        {
                ;
        }

        // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
        if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
                + pbih->biClrUsed * sizeof(RGBQUAD),
                (LPDWORD)& dwTmp, (NULL)))
                ;

        // Copy the array of color indices into the .BMP file.  
        dwTotal = cb = pbih->biSizeImage;
        hp = lpBits;
        if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)& dwTmp, NULL))
                ;

        // Close the .BMP file.  
        if (!CloseHandle(hf))
                ;

        // Free memory.  
        GlobalFree((HGLOBAL)lpBits);
}

WORD DIBNumColors(LPVOID lpv)
{
        INT                 bits;
        LPBITMAPINFOHEADER  lpbih = (LPBITMAPINFOHEADER)lpv;
        LPBITMAPCOREHEADER  lpbch = (LPBITMAPCOREHEADER)lpv;

        if (NEW_DIB_FORMAT(lpbih)) {
                if (lpbih->biClrUsed != 0)
                        return (WORD)lpbih->biClrUsed;
                bits = lpbih->biBitCount;
        }
        else
                bits = lpbch->bcBitCount;

        if (bits > 8)
                return 0; /* Since biClrUsed is 0, we dont have a an optimal palette */
        else
                return (1 << bits);
}

BOOL DIBInfo(HANDLE hbi, LPBITMAPINFOHEADER lpbih)
{
        if (hbi) {
                *lpbih = *(LPBITMAPINFOHEADER)hbi;

                if (NEW_DIB_FORMAT(lpbih)) {
                        if (lpbih->biSizeImage == 0L)
                                lpbih->biSizeImage = 
                                WIDTHBYTES(lpbih->biWidth * lpbih->biBitCount) * lpbih->biHeight;

                        if (lpbih->biClrUsed == 0L)
                                lpbih->biClrUsed = DIBNumColors(lpbih);
                }

                return TRUE;
        }
        return FALSE;
}

VOID ReadPackedFileHeader(HFILE hFile, LPBITMAPFILEHEADER lpbmfhdr, 
        LPDWORD lpdwOffset)
{
        *lpdwOffset = _llseek(hFile, 0L, (UINT)SEEK_CUR);
        _hread(hFile, (LPSTR)& lpbmfhdr->bfType, sizeof(WORD)); /* read in bfType*/
        _hread(hFile, (LPSTR)& lpbmfhdr->bfSize, sizeof(DWORD) * 3); /* read in last 3 dwords*/
}

HANDLE ReadDIBBitmapInfo(INT hFile) {
        DWORD              dwOffset;
        HANDLE             hbi = NULL;
        INT                size;
        INT                i;
        WORD               nNumColors;
        LPRGBQUAD          lprgbq;
        BITMAPINFOHEADER   bih;
        BITMAPCOREHEADER   bch;
        LPBITMAPINFOHEADER lpbih;
        BITMAPFILEHEADER   bf;
        DWORD              dwDWMasks = 0;
        DWORD              dwWidth = 0;
        DWORD              dwHeight = 0;
        WORD               wPlanes, wBitCount;

        if (hFile == HFILE_ERROR)
                return NULL;

        /* Read the bitmap file header */
        ReadPackedFileHeader(hFile, &bf, &dwOffset);

        /* Do we have a RC HEADER? */
        if (!ISDIB(bf.bfType)) {
                bf.bfOffBits = 0L;
                _llseek(hFile, dwOffset, (UINT)SEEK_SET); 
        }

        if (sizeof(bih) != _hread(hFile, (LPSTR)& bih, (UINT)sizeof(bih)))
                return FALSE;

        nNumColors = DIBNumColors(&bih);

        /* Check the nature (BITMAPINFO or BITMAPCORE) of the info. block
         * and extract the field information accordingly. If a BITMAPCOREHEADER,
         * transfer it's field information to a BITMAPINFOHEADER-style block
         */
        switch (size = (INT)bih.biSize) {
                case sizeof(BITMAPINFOHEADER) :
                        break;

                        case sizeof(BITMAPCOREHEADER) :

                                bch = *(LPBITMAPCOREHEADER)& bih;

                                dwWidth = (DWORD)bch.bcWidth;
                                dwHeight = (DWORD)bch.bcHeight;
                                wPlanes = bch.bcPlanes;
                                wBitCount = bch.bcBitCount;

                                bih.biSize = sizeof(BITMAPINFOHEADER);
                                bih.biWidth = dwWidth;
                                bih.biHeight = dwHeight;
                                bih.biPlanes = wPlanes;
                                bih.biBitCount = wBitCount;
                                bih.biCompression = BI_RGB;
                                bih.biSizeImage = 0;
                                bih.biXPelsPerMeter = 0;
                                bih.biYPelsPerMeter = 0;
                                bih.biClrUsed = nNumColors;
                                bih.biClrImportant = nNumColors;

                                _llseek(hFile, (LONG)sizeof(BITMAPCOREHEADER) - sizeof(BITMAPINFOHEADER), (UINT)SEEK_CUR);
                                break;

                        default:
                                /* Not a DIB! */
                                return NULL;
        }

        /*  Fill in some default values if they are zero */
        if (bih.biSizeImage == 0) {
                bih.biSizeImage = WIDTHBYTES((DWORD)bih.biWidth * bih.biBitCount) * bih.biHeight;
        }
        if (bih.biClrUsed == 0)
                bih.biClrUsed = DIBNumColors(&bih);

        /* Allocate for the BITMAPINFO structure and the color table. */
        if ((bih.biBitCount == 16) || (bih.biBitCount == 32))
                dwDWMasks = sizeof(DWORD) * 3;
        hbi = GlobalAlloc(GPTR, (LONG)bih.biSize + nNumColors * sizeof(RGBQUAD) + dwDWMasks);
        if (!hbi)
                return NULL;
        lpbih = (LPBITMAPINFOHEADER)hbi;
        *lpbih = bih;

        /* Get a pointer to the color table */
        lprgbq = (LPRGBQUAD)((LPSTR)lpbih + bih.biSize);
        if (nNumColors) {
                if (size == sizeof(BITMAPCOREHEADER)) {
                        /* Convert a old color table (3 byte RGBTRIPLEs) to a new
                         * color table (4 byte RGBQUADs)
                         */
                        _hread(hFile, (LPSTR)lprgbq, (UINT)nNumColors * sizeof(RGBTRIPLE));

                        for (i = nNumColors - 1; i >= 0; i--) {
                                RGBQUAD rgbq;

                                rgbq.rgbRed = ((RGBTRIPLE*)lprgbq)[i].rgbtRed;
                                rgbq.rgbBlue = ((RGBTRIPLE*)lprgbq)[i].rgbtBlue;
                                rgbq.rgbGreen = ((RGBTRIPLE*)lprgbq)[i].rgbtGreen;
                                rgbq.rgbReserved = (BYTE)0;

                                lprgbq[i] = rgbq;
                        }
                }
                else
                        _hread(hFile, (LPSTR)lprgbq, (UINT)nNumColors * sizeof(RGBQUAD));
        }
        else
                if (dwDWMasks)
                        _hread(hFile, (LPSTR)lprgbq, dwDWMasks);

        if (bf.bfOffBits != 0L) {
                _llseek(hFile, dwOffset + bf.bfOffBits, (UINT)SEEK_SET);
        }

        return hbi;
}

HGLOBAL GlobalFreeDIB(HGLOBAL hDIB)
{
        LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)hDIB;

        if (!lpbi->biClrImportant)
                return GlobalFree(hDIB);

        if (GlobalFlags((HGLOBAL)lpbi->biClrImportant) == GMEM_INVALID_HANDLE) {
                SetLastError(0);
                return GlobalFree(hDIB);
        }
        else
                return GlobalFree((HANDLE)lpbi->biClrImportant);
}

WORD ColorTableSize(LPVOID lpv)
{
        LPBITMAPINFOHEADER lpbih = (LPBITMAPINFOHEADER)lpv;

        if (NEW_DIB_FORMAT(lpbih))
        {
                if (((LPBITMAPINFOHEADER)(lpbih))->biCompression == BI_BITFIELDS)
                        /* Remember that 16/32bpp dibs can still have a color table */
                        return (sizeof(DWORD) * 3) + (DIBNumColors(lpbih) * sizeof(RGBQUAD));
                else
                        return (DIBNumColors(lpbih) * sizeof(RGBQUAD));
        }
        else
                return (DIBNumColors(lpbih) * sizeof(RGBTRIPLE));
}

HANDLE OpenDIB(LPSTR szFilename)
{
        HFILE               hFile;
        BITMAPINFOHEADER    bih;
        LPBITMAPINFOHEADER  lpbih;
        DWORD               dwLen = 0;
        DWORD               dwBits;
        HANDLE              hDIB;
        HANDLE              hMem;
        OFSTRUCT            of;

        /* Open the file and read the DIB information */
        hFile = OpenFile(szFilename, &of, (UINT)OF_READ);
        if (hFile == HFILE_ERROR)
                return NULL;

        hDIB = ReadDIBBitmapInfo(hFile);
        if (!hDIB)
                return NULL;
        DIBInfo(hDIB, &bih);

        /* Calculate the memory needed to hold the DIB */
        dwBits = bih.biSizeImage;
        dwLen = bih.biSize + (DWORD)ColorTableSize(&bih) + dwBits;

        /* Try to increase the size of the bitmap info. buffer to hold the DIB */
        hMem = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE);

        if (!hMem) {
                GlobalFreeDIB(hDIB);
                hDIB = NULL;
        }
        else
                hDIB = hMem;

        /* Read in the bits */
        if (hDIB) {
                lpbih = (LPBITMAPINFOHEADER)hDIB;
                _hread(hFile, (LPSTR)lpbih + (WORD)lpbih->biSize + ColorTableSize(lpbih), dwBits);
        }
        _lclose(hFile);

        return hDIB;
}

HBITMAP BitmapFromDIB(HANDLE hDIB, HPALETTE  hPal)
{
        LPBITMAPINFOHEADER  lpbih;
        HPALETTE            hPalOld;
        HDC                 hDC;
        HBITMAP             hBitmap;

        
        if (!hDIB)
                return NULL;

        lpbih = (LPBITMAPINFOHEADER)hDIB;

        if (!lpbih)
                return NULL;

        hDC = GetDC(NULL);

        if (hPal) {
                hPalOld = SelectPalette(hDC, hPal, FALSE);
                RealizePalette(hDC);
        }
        else
                hPalOld = NULL;

        hBitmap = CreateDIBitmap(hDC,
                lpbih,
                CBM_INIT,
                (LPSTR)lpbih + lpbih->biSize + ColorTableSize(lpbih),
                (LPBITMAPINFO)lpbih,
                DIB_RGB_COLORS);

        if (hPal)
                SelectPalette(hDC, hPalOld, FALSE);

        ReleaseDC(NULL, hDC);
        
        return hBitmap;
}

BOOL DrawBitmap(HDC hDC, INT x, INT y, HBITMAP hBitmap, DWORD dwROP)
{
        HDC       hDCBits;
        BITMAP    Bitmap;
        BOOL      bResult;

        if (!hDC || !hBitmap)
                return FALSE;

        hDCBits = CreateCompatibleDC(hDC);
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)& Bitmap);
        SelectObject(hDCBits, hBitmap);
        bResult = BitBlt(hDC, x, y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBits, 0, 0, dwROP);
        DeleteDC(hDCBits);

        return bResult;
}

