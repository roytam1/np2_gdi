#include	"compiler.h"
#include	"strres.h"
#include	"bmpdata.h"
#include	"dosio.h"
#include	"commng.h"
#include	"sysmng.h"
#include	"dialogs.h"


static const TCHAR str_nc[] = _T("N/C");

const TCHAR str_int0[] = _T("INT0");
const TCHAR str_int1[] = _T("INT1");
const TCHAR str_int2[] = _T("INT2");
const TCHAR str_int4[] = _T("INT4");
const TCHAR str_int5[] = _T("INT5");
const TCHAR str_int6[] = _T("INT6");


// ---- file select

BOOL dlgs_selectfile(HWND hWnd, const FILESEL *item,
											TCHAR *path, UINT size, int *ro) {

	OPENFILENAME	ofn;

	TCHAR filename_[MAX_PATH];
	TCHAR filepath_[MAX_PATH];

	if ((item == NULL) || (path == NULL) || (size == 0)) {
		return(FALSE);
	}
	lstrcpy(filepath_, path);
	lstrcpy(filename_, path);

	int i = 0;
	int j = -1;

	while(filepath_[i] != TEXT('\0'))
	{
		if(filepath_[i] == TEXT('\\'))
		{
			j = i;
		}
		i++;
	}

	int x = 0;
	for (i = j+1; filepath_[i] != TEXT('\0'); i++)
	{
		filename_[x++] = filepath_[i];
	}
	filename_[x++] = TEXT('\0');
	filepath_[j+1] = TEXT('\0');

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = item->filter;
	ofn.nFilterIndex = item->defindex;
	ofn.lpstrFile = filename_;
	ofn.nMaxFile = size;
	ofn.lpstrInitialDir = filepath_;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = item->ext;
	ofn.lpstrTitle = item->title;
	if (!GetOpenFileName(&ofn)) {
		return(FALSE);
	}
	lstrcpy(path, filename_);
	if (ro) {
		*ro = ofn.Flags & OFN_READONLY;
	}
	return(TRUE);
}

BOOL dlgs_selectwritefile(HWND hWnd, const FILESEL *item,
											TCHAR *path, UINT size) {

	OPENFILENAME	ofn;

	TCHAR filename_[MAX_PATH];
	TCHAR filepath_[MAX_PATH];

	if ((item == NULL) || (path == NULL) || (size == 0)) {
		return(FALSE);
	}
	lstrcpy(filepath_, path);
	lstrcpy(filename_, path);

	int i = 0;
	int j = -1;

	while(filepath_[i] != TEXT('\0'))
	{
		if(filepath_[i] == TEXT('\\'))
		{
			j = i;
		}
		i++;
	}

	int x = 0;
	for (i = j+1; filepath_[i] != TEXT('\0'); i++)
	{
		filename_[x++] = filepath_[i];
	}
	filename_[x++] = TEXT('\0');
	filepath_[j+1] = TEXT('\0');

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = item->filter;
	ofn.nFilterIndex = item->defindex;
	ofn.lpstrFile = filename_;
	ofn.nMaxFile = size;
	ofn.lpstrInitialDir = filepath_;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = item->ext;
	ofn.lpstrTitle = item->title;
	if (!GetSaveFileName(&ofn)) {
		return(FALSE);
	}
	lstrcpy(path, filename_);
	return(TRUE);
}

BOOL dlgs_selectwritenum(HWND hWnd, const FILESEL *item,
											TCHAR *path, UINT size) {

	TCHAR	*file;
	TCHAR	*p;
	TCHAR	*q;
	UINT	i;
	BOOL	r;

	if ((item == NULL) || (path == NULL) || (size == 0)) {
		return(FALSE);
	}
	file = (TCHAR *)_MALLOC((size + 16) * sizeof(TCHAR), path);
	if (file == NULL) {
		return(FALSE);
	}
	p = file_getname(path);
	milstr_ncpy(file, path, size);
	file_cutname(file);
	q = file + lstrlen(file);

	for (i=0; i<10000; i++) {
		wsprintf(q, p, i);
		if (file_attr(file) == (short)-1) {
			break;
		}
	}
	r = dlgs_selectwritefile(hWnd, item, file, size);
	if (r) {
		milstr_ncpy(path, file, size);
	}
	_MFREE(file);
	return(r);
}


// ---- list

void dlgs_setliststr(HWND hWnd, UINT res, const TCHAR **item, UINT items) {

	HWND	wnd;
	UINT	i;

	wnd = GetDlgItem(hWnd, res);
	for (i=0; i<items; i++) {
		SendMessage(wnd, CB_INSERTSTRING, (WPARAM)i, (LPARAM)item[i]);
	}
}

void dlgs_setlistuint32(HWND hWnd, UINT res, const UINT32 *item, UINT items) {

	HWND	wnd;
	UINT	i;
	TCHAR	str[16];

	wnd = GetDlgItem(hWnd, res);
	for (i=0; i<items; i++) {
		wsprintf(str, str_u, item[i]);
		SendMessage(wnd, CB_INSERTSTRING, (WPARAM)i, (LPARAM)str);
	}
}


// ---- draw

void dlgs_drawbmp(HDC hdc, UINT8 *bmp) {

	BMPFILE		*bf;
	BMPINFO		*bi;
	BMPDATA		inf;
	HBITMAP		hbmp;
	UINT8		*image;
	HDC			hmdc;

	if (bmp == NULL) {
		return;
	}
	bf = (BMPFILE *)bmp;
	bi = (BMPINFO *)(bf + 1);
	if (bmpdata_getinfo(bi, &inf) != SUCCESS) {
		goto dsdb_err1;
	}
	//hbmp = CreateDIBSection(hdc, (BITMAPINFO *)bi, DIB_RGB_COLORS,
	//											(void **)&image, NULL, 0);
	if (hbmp == NULL) {
		goto dsdb_err1;
	}
	CopyMemory(image, bmp + (LOADINTELDWORD(bf->bfOffBits)),
													bmpdata_getdatasize(bi));
	hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hbmp);
	if (inf.height < 0) {
		inf.height *= -1;
	}
	BitBlt(hdc, 0, 0, inf.width, inf.height, hmdc, 0, 0, SRCCOPY);
	DeleteDC(hmdc);
	DeleteObject(hbmp);

dsdb_err1:
	_MFREE(bmp);
}

