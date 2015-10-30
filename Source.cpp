#include <Windows.h>
#include <map>
#include <algorithm>
#include <vector>
#include <list>
#include <type_traits>
#include <sstream>

#include "resource.h"
/* HuffTree */
#define MDIWINDOW
#ifdef _UNICODE
typedef std::wstring  tstring;
#else
typedef std::string tstring;
#endif

TCHAR* FileName(HWND hWnd, TCHAR* lpstrFile, LPCWSTR lpstrFilter, int MaxLen)
{
	TCHAR InitDir[MAX_PATH];
	OPENFILENAME OFN = {};
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWnd;
	OFN.lpstrFilter = lpstrFilter;
	OFN.lpstrFile = lpstrFile;
	OFN.nMaxFile = MaxLen;
	OFN.lpstrTitle = TEXT("PLEASE SELECT FILE");
	//OFN.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	GetWindowsDirectory(InitDir, MAX_PATH);
	OFN.lpstrInitialDir = InitDir;

	if (GetOpenFileName(&OFN))
		return lpstrFile;
	else
	{
		return nullptr;
	}
}
template<typename T, typename Enable = void> struct tag_HuffTree;

template<class T>
struct tag_HuffTree<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
private:
	std::map<T, unsigned int> FreqMap;
	std::map<T, tstring> HuffTable;
	
	struct node
	{
		std::pair<T, int> self; //Added Freq Val
		node* leftchild;
		node* rightchild;

		node(std::pair<T, int> self, node* lc = nullptr, node* rc = nullptr) :self{ self }, leftchild{ lc }, rightchild{ rc }{};
		~node(){ delete leftchild; delete rightchild; }

		void print(HDC hdc, int x, int y, int offset, bool Show)
		{
			if (T(~self.first)) //not -1
			{
				tstring Text;
				if (std::is_same<TCHAR, T>::value)
				{
					Text += self.first;
				}
				else
				{
#ifdef _UNICODE
					Text += std::to_wstring(self.first); // = frequency
#else
					Text += std::to_string(self.first);
#endif
				}

				if (Show)
				{


					Text += TEXT(" : ");
#ifdef _UNICODE

					Text += std::to_wstring(self.second); // = frequency
#else
					Text += std::to_string(self.second);
#endif
				}
				const TCHAR* Txt = Text.c_str();
				TextOut(hdc, x, y, Txt, lstrlen(Txt));
			}


			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x - offset / 2, y + 20);
			if (leftchild) leftchild->print(hdc, x - offset / 2, y + 20, offset / 2,Show);

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x + offset / 2, y + 20);
			if (rightchild) rightchild->print(hdc, x + offset / 2, y + 20, offset / 2,Show);

		}
		void print(HDC hdc, int x, int y, int offset, std::map<T, tstring>& HuffTable)
		{
			if (T(~self.first))
			{
				T Check = ~self.first;
				tstring Text;
				if (std::is_same<TCHAR, T>::value)
				{
					Text += self.first;
				}
				else
				{
#ifdef _UNICODE
					Text += std::to_wstring(self.first); // = frequency
#else
					Text += std::to_string(self.first);
#endif
				}
				Text += TEXT(" : ");
				Text += HuffTable[self.first];
				const TCHAR* Txt = Text.c_str();
				TextOut(hdc, x, y, Txt, lstrlen(Txt));
			}

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x - offset / 2, y + 20);
			if (leftchild) leftchild->print(hdc, x - offset / 2, y + 20, offset / 2, HuffTable);

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x + offset / 2, y + 20);
			if (rightchild) rightchild->print(hdc, x + offset / 2, y + 20, offset / 2, HuffTable);

		}


		void RegisterValue(std::map<T, tstring>& HuffTable, tstring Traversal)
		{
			if (T(~self.first))
				HuffTable[self.first] = Traversal;
			if (leftchild) leftchild->RegisterValue(HuffTable, Traversal + (TCHAR)'0');
			if (rightchild) rightchild->RegisterValue(HuffTable, Traversal + (TCHAR)'1');
		}
	} *Head;
public:
	void insert(T c)
	{
		++FreqMap[c];
	}
	void CreateTree()
	{
		if (!FreqMap.empty())
		{
			std::vector<std::pair<T, int>> RawMap;
			RawMap.assign(FreqMap.begin(), FreqMap.end());
			std::sort(RawMap.begin(), RawMap.end(), [](std::pair<T, int> a, std::pair<T, int> b){return a.second < b.second; });
			//sort by frequency
			std::list<node*> NodeQueue;
			for (auto &p : RawMap)
			{
				if (p.second)
					NodeQueue.push_back(new node(p));
			}
			while (NodeQueue.size() > 1)
			{
				node* leftchild = NodeQueue.front();
				NodeQueue.pop_front();
				node* rightchild = NodeQueue.front();
				NodeQueue.pop_front();


				node* NewNode = new node{
					std::make_pair(T(~0), leftchild->self.second + rightchild->self.second),
					leftchild,
					rightchild
				};
				NodeQueue.insert(std::find_if(NodeQueue.begin(), NodeQueue.end(),
					[NewNode](node* a){return a->self.second > NewNode->self.second; }), NewNode); //before the element
			}
			Head = NodeQueue.front();
		}
	
	} //from FreqMap.
	void DeleteTree(){ delete Head; Head = nullptr; } //tree & table = connected

	void PrintTree(HDC hdc, int Width, int Height, int DisplayMode)
	{
		enum { Freq, Table, Disable };

		SetTextAlign(hdc, TA_BASELINE | TA_CENTER);
		if (DisplayMode == Table)
		{
			Head->print(hdc, Width / 2, 10, Width / 2, HuffTable);
		}
		else if (DisplayMode == Freq)
		{
			Head->print(hdc, Width / 2, 10, Width / 2, true);
		}
		else
		{
			Head->print(hdc, Width / 2, 10, Width / 2, false);
		}
	}
	void PrintHuffTable(HDC hdc)
	{
		SetTextAlign(hdc, TA_LEFT | TA_TOP);
		int y = 0;
		for (auto& Val:HuffTable)
		{
			tstring SomeText;

			if (std::is_same<TCHAR, T>::value)
			{
				SomeText += Val.first;
			}
			else
			{
#ifdef _UNICODE
				SomeText += std::to_wstring(Val.first); // = frequency
#else
				SomeText += std::to_string(Val.first);
#endif
			}
			SomeText += TEXT(" : ");
			SomeText += Val.second;

			const TCHAR* Txt = SomeText.c_str();
			TextOut(hdc, 0, ++y * 20, Txt, lstrlen(Txt));
			//std::wstring wstr;
			//wstr.assign(HuffTable.at(i).begin(), HuffTable.at(i).end());
			//TextOut(hdc, 0, ++y * 20, &i, 1);
			//TextOut(hdc, 20, y * 20, wstr.c_str(), wstr.length());
		}
	}
	void PrintHuffTable(HWND hList)
	{
		SendMessage(hList, LB_RESETCONTENT, 0, 0);
		for (auto& Val : HuffTable)
		{
			tstring SomeText;

			if (std::is_same<TCHAR, T>::value)
			{
				SomeText += Val.first;
			}
			else
			{
#ifdef _UNICODE
				SomeText += std::to_wstring(Val.first); // = frequency
#else
				SomeText += std::to_string(Val.first);
#endif
			}

			SomeText += TEXT(" : ");
			SomeText += Val.second;
			const TCHAR* Txt = SomeText.c_str();
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)SomeText.c_str());
		}
	}
	void CreateTable()
	{
		HuffTable.clear();
		if (Head)
			Head->RegisterValue(HuffTable, tstring());
	}
	void DeleteTable(){ HuffTable.clear(); }
	void Reset(){ FreqMap.clear(); DeleteTree(); DeleteTable(); }
	tag_HuffTree() :Head{ nullptr }{};
	~tag_HuffTree(){ Reset(); };
};
/**/
template <>
struct tag_HuffTree <tstring>
{
	using T = tstring;
private:
	std::map<T, unsigned int> FreqMap;
	std::map<T, tstring> HuffTable;

	struct node
	{
		std::pair<T, int> self; //Added Freq Val
		node* leftchild;
		node* rightchild;

		node(std::pair<T, int> self, node* lc = nullptr, node* rc = nullptr) :self{ self }, leftchild{ lc }, rightchild{ rc }{};
		~node(){ delete leftchild; delete rightchild; }

		void print(HDC hdc, int x, int y, int offset, bool Show)
		{
			if (!self.first.empty()) //not -1
			{
				tstring Text;
				Text += self.first;
				if (Show)
				{

					Text += TEXT(" : ");
#ifdef _UNICODE
					Text += std::to_wstring(self.second); // = frequency
#else
					Text += std::to_string(self.second);
#endif
				}
				const TCHAR* Txt = Text.c_str();
				TextOut(hdc, x, y, Txt, lstrlen(Txt));
			}


			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x - offset / 2, y + 20);
			if (leftchild) leftchild->print(hdc, x - offset / 2, y + 20, offset / 2,Show);

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x + offset / 2, y + 20);
			if (rightchild) rightchild->print(hdc, x + offset / 2, y + 20, offset / 2,Show);

		}
		void print(HDC hdc, int x, int y, int offset, std::map<T, tstring>& HuffTable)
		{
			if (!self.first.empty())
			{
				tstring Text;
				Text += self.first;
				Text += TEXT(" : ");
				Text += HuffTable[self.first];
				const TCHAR* Txt = Text.c_str();
				TextOut(hdc, x, y, Txt, lstrlen(Txt));
			}

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x - offset / 2, y + 20);
			if (leftchild) leftchild->print(hdc, x - offset / 2, y + 20, offset / 2, HuffTable);

			MoveToEx(hdc, x, y, NULL);
			LineTo(hdc, x + offset / 2, y + 20);
			if (rightchild) rightchild->print(hdc, x + offset / 2, y + 20, offset / 2, HuffTable);

		}


		void RegisterValue(std::map<T, tstring>& HuffTable, tstring Traversal)
		{
			if (!self.first.empty())
				HuffTable[self.first] = Traversal;
			if (leftchild) leftchild->RegisterValue(HuffTable, Traversal + (TCHAR)'0');
			if (rightchild) rightchild->RegisterValue(HuffTable, Traversal + (TCHAR)'1');
		}
	} *Head;
public:
	void insert(T c)
	{
		++FreqMap[c];
	}
	void CreateTree()
	{
		if (!FreqMap.empty())
		{
			std::vector<std::pair<T, int>> RawMap;
			RawMap.assign(FreqMap.begin(), FreqMap.end());
			std::sort(RawMap.begin(), RawMap.end(), [](std::pair<T, int> a, std::pair<T, int> b){return a.second < b.second; });
			//sort by frequency
			std::list<node*> NodeQueue;
			for (auto &p : RawMap)
			{
				if (p.second)
					NodeQueue.push_back(new node(p));
			}
			while (NodeQueue.size() > 1)
			{
				node* leftchild = NodeQueue.front();
				NodeQueue.pop_front();
				node* rightchild = NodeQueue.front();
				NodeQueue.pop_front();


				node* NewNode = new node{
					std::make_pair(TEXT(""), leftchild->self.second + rightchild->self.second),
					leftchild,
					rightchild
				};
				NodeQueue.insert(std::find_if(NodeQueue.begin(), NodeQueue.end(),
					[NewNode](node* a){return a->self.second > NewNode->self.second; }), NewNode); //before the element
			}
			Head = NodeQueue.front();
		}
	
} //from FreqMap.
	void DeleteTree(){ delete Head; Head = nullptr; } //tree & table = connected

	void PrintTree(HDC hdc, int Width, int Height, int DisplayMode)
	{
		enum { Freq, Table, Disable };

		SetTextAlign(hdc, TA_BASELINE | TA_CENTER);
		if (DisplayMode == Table)
		{
			Head->print(hdc, Width / 2, 10, Width / 2, HuffTable);
		}
		else if (DisplayMode == Freq)
		{
			Head->print(hdc, Width / 2, 10, Width / 2,true);
		}
		else
		{
			Head->print(hdc, Width / 2, 10, Width / 2, false);
		}
	}
	void PrintHuffTable(HDC hdc)
	{
		SetTextAlign(hdc, TA_LEFT | TA_TOP);
		int y = 0;
		for (auto& Val : HuffTable)
		{
			tstring SomeText;
			SomeText += Val.first;
			SomeText += TEXT(" : ");
			SomeText += Val.second;

			const TCHAR* Txt = SomeText.c_str();
			TextOut(hdc, 0, ++y * 20, Txt, lstrlen(Txt));
			//std::wstring wstr;
			//wstr.assign(HuffTable.at(i).begin(), HuffTable.at(i).end());
			//TextOut(hdc, 0, ++y * 20, &i, 1);
			//TextOut(hdc, 20, y * 20, wstr.c_str(), wstr.length());
		}
	}
	void PrintHuffTable(HWND hList)
	{
		SendMessage(hList, LB_RESETCONTENT, 0, 0);
		for (auto& Val : HuffTable)
		{
			tstring SomeText;
			SomeText += Val.first;
			SomeText += TEXT(" : ");
			SomeText += Val.second;
			const TCHAR* Txt = SomeText.c_str();
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)SomeText.c_str());
		}
	}
	void CreateTable()
	{
		HuffTable.clear();
		if (Head)
			Head->RegisterValue(HuffTable, tstring());
	}
	void DeleteTable(){ HuffTable.clear(); }
	void Reset(){ FreqMap.clear(); DeleteTable(); DeleteTree(); }
	tag_HuffTree() :Head{ nullptr }{};
	~tag_HuffTree(){ Reset(); };
};

enum { WM_HUFFMAN = WM_USER + 1, WM_LOADTEXT, WM_GETDATA};

#ifndef MDIWINDOW
/* WinMain*/
ATOM RegisterCustomClass(HINSTANCE& hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

enum { ID_CHILDWINDOW = 100 };

LPCTSTR Title = L"HuffmanTree";
HINSTANCE g_hInst;
HWND hMainWnd;

ATOM RegisterCustomClass(HINSTANCE& hInstance)
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = Title;
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	return RegisterClass(&wc);
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInstance;
	RegisterCustomClass(hInstance);
	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

//Dialog Box?
// 1. Get Data & Data Type
// Create Tree Accordingly.
//Data Type = 1. int
// 2. char(TCHAR)
// 3. string
// option : skip whitespace
// option : display (FreQuency Vs. T
//TRY MDI WINDOW?
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		break;
	case WM_CHAR:
		HuffTree.insert(wParam);
		break;
	case WM_RBUTTONDOWN:
	{
		HDC hdc = GetDC(hWnd);
		HuffTree.CreateTree();
		HuffTree.CreateTable();

		HuffTree.PrintTree(hdc,false);
		ReleaseDC(hWnd, hdc);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		HDC hdc = GetDC(hWnd);
		HuffTree.PrintHuffTable(hdc);
		ReleaseDC(hWnd, hdc);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}
#else

LRESULT CALLBACK MDIWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HuffTableProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HuffGraphProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HuffDataProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK TxtEncodingDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
void DeleteHTree(LONG Type);
enum { ID_CHILDWINDOW = 41000 };

LPCTSTR Title = TEXT("HuffmanTree");

HINSTANCE g_hInst;
HWND hMainWnd;
HWND hMDIClient;

LPCTSTR GraphTitle = TEXT("HuffmanTree_Graph");
LPCTSTR TableTitle = TEXT("HuffmanTree_Table");
LPCTSTR DataTitle = TEXT("HuffmanTree_Data");

void RegisterMDIClass(HINSTANCE& hInstance)
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = MDIWndProc;
	wc.lpszClassName = Title;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.style = 0;
	RegisterClass(&wc);

	/*
	wc.lpszClassName = TEXT("MDI_ChildWindow");
	wc.lpfnWndProc = MDIChildProc;
	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	wc.cbWndExtra = sizeof(DWORD);
	RegisterClass(&wc);
	*/

	wc.lpszClassName = GraphTitle;
	wc.lpfnWndProc = HuffGraphProc;
	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	wc.cbWndExtra = sizeof(DWORD);
	RegisterClass(&wc);

	wc.lpszClassName = TableTitle;
	wc.lpfnWndProc = HuffTableProc;
	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	wc.cbWndExtra = sizeof(DWORD);
	RegisterClass(&wc);

	wc.lpszClassName = DataTitle;
	wc.lpfnWndProc = HuffDataProc;
	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	wc.cbWndExtra = sizeof(DWORD);
	RegisterClass(&wc);

}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInstance;
	RegisterMDIClass(hInstance);
	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!TranslateMDISysAccel(hMDIClient, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

HWND hGraph;
HWND hTable;
HWND hData;
void* H_Tree;
LRESULT CALLBACK MDIWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
	{
		CLIENTCREATESTRUCT ccs;
		ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 1);
		ccs.idFirstChild = ID_CHILDWINDOW;
		hMDIClient = CreateWindow(TEXT("MDICLIENT"), NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
			NULL, NULL, NULL, NULL, hWnd, (HMENU)NULL, g_hInst, &ccs);
		ShowWindow(hMDIClient, SW_SHOW);
	}
	return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEW:
		{
			MDICREATESTRUCT mcs;
			mcs.hOwner = g_hInst;
			mcs.x = mcs.y = CW_USEDEFAULT;
			mcs.cx = mcs.cy = CW_USEDEFAULT;
			mcs.style = MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;
			mcs.szClass = GraphTitle;
			mcs.szTitle = GraphTitle;
			if (!hGraph)
			hGraph = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
			mcs.szClass = TableTitle;
			mcs.szTitle = TableTitle;
			if (!hTable)
			hTable = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
			mcs.szClass = DataTitle;
			mcs.szTitle = DataTitle;
			if (!hData)
			hData = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
			break;
		}

		case ID_FILE_LOAD:
		{
			TCHAR Path[MAX_PATH]{};
			TCHAR Filter[] = TEXT("TXT File(*.TXT*)\0*.TXT*\0");
			if (FileName(hWnd, Path, Filter, MAX_PATH))
			{
				HANDLE TxtFile = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (TxtFile == INVALID_HANDLE_VALUE)
					break;
				if (!hData)
				{
					MDICREATESTRUCT mcs;
					mcs.hOwner = g_hInst;
					mcs.x = mcs.y = CW_USEDEFAULT;
					mcs.cx = mcs.cy = CW_USEDEFAULT;
					mcs.style = MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;
					mcs.szClass = GraphTitle;
					mcs.szTitle = GraphTitle;
					if (!hGraph)
						hGraph = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
					mcs.szClass = TableTitle;
					mcs.szTitle = TableTitle;
					if (!hTable)
						hTable = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
					mcs.szClass = DataTitle;
					mcs.szTitle = DataTitle;
					if (!hData)
						hData = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
				}
				LARGE_INTEGER FileSize;
				GetFileSizeEx(TxtFile, &FileSize);
				char* Data = new char[FileSize.QuadPart+2]{};
				TCHAR* TData = (TCHAR*)Data;
			
				DWORD cBytes;
				ReadFile(TxtFile, Data, FileSize.QuadPart, &cBytes, NULL);
				
				switch (DialogBoxParamW(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TxtEncodingDlgProc, (LPARAM)Data)) //ANSI
				{
				case 256:
				{
					std::string cstr{ (char*)Data};
					std::wstring Astr;
					Astr.assign(cstr.begin(), cstr.end());
					SendMessage(hData, WM_LOADTEXT, 0, (LPARAM)Astr.c_str()); // 100 = ID_EDIT
					break;
				}

				case 65536:
				{
					SendMessage(hData, WM_LOADTEXT, 0, (LPARAM)Data);// 100 = ID_EDIT
					break;
				}

				default:
					MessageBox(hWnd, TEXT("UNSUPPORTED FORMAT"), TEXT("ERROR"), MB_OK);
					break;
				}
				CloseHandle(TxtFile);
				delete Data;
			}
			break;
		}
		case ID_FILE_SAVE:
		{
			tstring StringData;
			HANDLE hBitmap;
			SendMessage(hTable, WM_GETDATA, 0, (LPARAM)&StringData);
			SendMessage(hGraph, WM_GETDATA, 0, (LPARAM)&hBitmap);
			//SaveBitMap...?
			break;
		}
		case ID_FILE_COPY:
		{	
			tstring StringData;
			HANDLE hBitmap;
			SendMessage(hTable, WM_GETDATA, 0, (LPARAM)&StringData);
			SendMessage(hGraph, WM_GETDATA, 0, (LPARAM)&hBitmap);

			HGLOBAL hMem = GlobalAlloc(GHND, sizeof(TCHAR)*(StringData.length() + 1));
			TCHAR* ptr = (TCHAR*)GlobalLock(hMem);
			memcpy(ptr, StringData.c_str(), sizeof(TCHAR)*(StringData.length() + 1));
			GlobalUnlock(hMem);
			if (OpenClipboard(hWnd))
			{
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hMem);
				SetClipboardData(CF_BITMAP, hBitmap); //do not need to erase hBitmap anymore
				CloseClipboard();
			}
			else
			{
				GlobalFree(hMem);
				DeleteObject(hBitmap);
			}

			return 0;
		}
		case ID_FILE_EXIT:
		{
			PostQuitMessage(0);
			return 0;
		}
		case ID_WINDOW_CASCADE:
			SendMessage(hMDIClient, WM_MDICASCADE, MDITILE_SKIPDISABLED, 0);
			break;
		case ID_WINDOW_TILE:
			SendMessage(hMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
			break;
		case ID_WINDOW_ARRANGEICON:
			SendMessage(hMDIClient,WM_MDIICONARRANGE, 0, 0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}
	return DefFrameProc(hWnd, hMDIClient, iMsg, wParam, lParam);
}


enum {ID_LIST = 100};
LRESULT CALLBACK HuffTableProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//simple, shows a table.
	switch (iMsg)
	{
	case WM_CREATE:
	{
		HWND hList = CreateWindow(TEXT("Listbox"), NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | LBS_STANDARD, 0, 0, 0, 0, hWnd, (HMENU)ID_LIST, g_hInst, NULL);
	}
		return 0;
	case WM_HUFFMAN:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, wParam);
		switch (wParam)
		{
		case 0:
			((tag_HuffTree<TCHAR>*)H_Tree)->CreateTable();
			((tag_HuffTree<TCHAR>*)H_Tree)->PrintHuffTable(GetDlgItem(hWnd, ID_LIST));
			break;
		case 1:
			((tag_HuffTree<TCHAR>*)H_Tree)->CreateTable();
			((tag_HuffTree<TCHAR>*)H_Tree)->PrintHuffTable(GetDlgItem(hWnd, ID_LIST));
			break;
		case 2:
			((tag_HuffTree<tstring>*)H_Tree)->CreateTable();
			((tag_HuffTree<tstring>*)H_Tree)->PrintHuffTable(GetDlgItem(hWnd, ID_LIST));
			break;
		case 3:
			((tag_HuffTree<INT>*)H_Tree)->CreateTable();
			((tag_HuffTree<INT>*)H_Tree)->PrintHuffTable(GetDlgItem(hWnd, ID_LIST));
			break;
		}
		
		//create huffman
		return 0;
	case WM_GETDATA:
	{
		HWND hList = GetDlgItem(hWnd, ID_LIST);
		int Cnt = SendMessage(hList, LB_GETCOUNT, 0, 0);

		for (int i = 0; i < Cnt; ++i)
		{
			int Len = SendMessage(hList, LB_GETTEXTLEN, i, 0);
			TCHAR* TextData = new TCHAR[Len + 1];
			SendMessage(hList, LB_GETTEXT, i, (LPARAM)TextData);
			TextData[Len] = 0;

			*(tstring*)(lParam) += TEXT("\r\n");
			*(tstring*)(lParam) += TextData;

			delete TextData;
		}
		*(tstring*)(lParam) += TEXT('\0');
		return 0;
	}
	case WM_SIZE:
	{
		SetWindowPos(GetDlgItem(hWnd, ID_LIST), NULL, 0, 0,LOWORD(lParam),HIWORD(lParam), NULL);
		break;
	}

	case WM_DESTROY:
		hTable = nullptr;
		break;
	}
	return DefMDIChildProc(hWnd, iMsg, wParam, lParam);
}
enum {ID_FREQ = 100, ID_TABLE, ID_DISABLE};
LRESULT CALLBACK HuffGraphProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
	{
		HWND hFreq = CreateWindow(TEXT("Button"), TEXT("Text Freq"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTORADIOBUTTON| WS_GROUP, 0, 0, 0, 0, hWnd, (HMENU)ID_FREQ, g_hInst, NULL);
		HWND hTable = CreateWindow(TEXT("Button"), TEXT("Table Value"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTORADIOBUTTON, 0,0,0,0, hWnd, (HMENU)ID_TABLE, g_hInst, NULL);
		HWND hDisable = CreateWindow(TEXT("Button"), TEXT("Disable"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTORADIOBUTTON, 0,0,0,0, hWnd, (HMENU)ID_DISABLE, g_hInst, NULL);
		SendMessage(hTable, BM_SETCHECK, BST_CHECKED, 0);
	}	
		return 0;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FREQ:
		case ID_TABLE:
		case ID_DISABLE:
			if (HIWORD(wParam) == BN_CLICKED)
				InvalidateRect(hWnd, NULL, TRUE);
		}
	}
	break;
	case WM_HUFFMAN:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, wParam); //store type.
		switch (wParam) //char-dig-str-num
		{
		case 0:
		{
			auto TmpTree = new tag_HuffTree < TCHAR > {};
			std::basic_stringstream<TCHAR> TextStream;
			TextStream << (TCHAR*)lParam;
			TCHAR KeyChar;
			while (TextStream >> KeyChar)
			{
				if (IsCharAlphaNumeric(KeyChar))
					TmpTree->insert(KeyChar);
			}

			TmpTree->CreateTree();
			H_Tree = TmpTree;
		}
		break;
		case 1: //digit
		{
			auto TmpTree = new tag_HuffTree <TCHAR> {};
			std::basic_stringstream<TCHAR> TextStream;
			TextStream << (TCHAR*)lParam;
			TCHAR KeyChar;
			while (TextStream >> KeyChar)
			{
				TmpTree->insert(KeyChar);

			}

			TmpTree->CreateTree();
			H_Tree = TmpTree;
		}
		break;
		case 2:
		{
			auto TmpTree = new tag_HuffTree < tstring > {};

			std::basic_stringstream<TCHAR> TextStream;
			TextStream << (TCHAR*)lParam;
			tstring KeyChar;
			while (TextStream >> KeyChar)
			{
				TmpTree->insert(KeyChar);
			}
			TmpTree->CreateTree();
			H_Tree = TmpTree;
		}
		break;
		case 3:
		{
			auto TmpTree = new tag_HuffTree < INT > {};

			std::basic_stringstream<TCHAR> TextStream;
			TextStream << (TCHAR*)lParam;
			INT KeyChar;
			while (TextStream >> KeyChar)
			{
				TmpTree->insert(KeyChar);
			}
			TmpTree->CreateTree();
			H_Tree = TmpTree;
		}
		break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_GETDATA:
	{
		RECT R;
		GetClientRect(hWnd, &R);
		HDC hdc = GetDC(hWnd);
		*(HBITMAP*)lParam = CreateCompatibleBitmap(hdc, R.right - R.left, R.bottom - R.top);
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, *(HBITMAP*)lParam);
		BitBlt(MemDC, 0, 0, R.right - R.left, R.bottom - R.top, hdc, 0, 0, SRCCOPY);
		SelectObject(MemDC, OldBit);
		DeleteDC(MemDC);
		ReleaseDC(hWnd, hdc);
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		int RadioList[] = { ID_FREQ, ID_TABLE, ID_DISABLE };
		int i;
		for (i = 0; i < _countof(RadioList); ++i)
		{
			if (SendMessage(GetDlgItem(hWnd, RadioList[i]), BM_GETCHECK, 0, 0)) break;
		}

		if (H_Tree)
		{
			LONG type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			RECT R;
			GetClientRect(hWnd, &R);
			int Width = R.right - R.left;
			int Height = R.bottom - R.top;
			switch (type)
			{
			case 0:
				((tag_HuffTree<TCHAR>*)H_Tree)->PrintTree(hdc, Width, Height,i);
				break;
			case 1:
				((tag_HuffTree<TCHAR>*)H_Tree)->PrintTree(hdc, Width, Height,i);
				break;
			case 2:
				((tag_HuffTree<tstring>*)H_Tree)->PrintTree(hdc, Width, Height,i);
				break;
			case 3:
				((tag_HuffTree<INT>*)H_Tree)->PrintTree(hdc, Width, Height,i);
				break;
			}
		}

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_SIZE:
	{
		int RadioList[] = { ID_FREQ, ID_TABLE, ID_DISABLE };
		
		int Width = LOWORD(lParam);
		int Height = HIWORD(lParam);

		for (int i = 0; i < _countof(RadioList); ++i)
		{
			SetWindowPos(GetDlgItem(hWnd, RadioList[i]), 0, i*Width / 3, Height * 7 / 8, Width / 3, Height / 8, NULL);
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	case WM_DESTROY:
	{
		LONG type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		DeleteHTree(type);
		hGraph = nullptr;
		break;
	}
	}
	return DefMDIChildProc(hWnd, iMsg, wParam, lParam);
}

enum {ID_EDIT = 100, ID_SENDDATA, ID_ALNUM, ID_ALLCHAR, ID_STRING,ID_NUM};
LRESULT CALLBACK HuffDataProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
	{
		RECT R;
		GetClientRect(hWnd, &R);
		int Width = R.right - R.left;
		int Height = R.bottom - R.top;
		int CurIndex = 0;
		HWND hEdit = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN,0,0, Width/2,Height,hWnd,(HMENU)ID_EDIT, g_hInst, NULL);
		HWND hButton = CreateWindow(TEXT("Button"), TEXT("SEND DATA"), WS_CHILD | WS_VISIBLE | WS_BORDER|BS_CENTER|BS_VCENTER, Width/2+Width/5, 0, Width-Width/2-Width/5, Height, hWnd,(HMENU)ID_SENDDATA, g_hInst, NULL);
		HWND hAlnum1 = CreateWindow(TEXT("Button"), TEXT("Data:AlphaNumeric"), WS_CHILD | WS_VISIBLE | WS_BORDER |BS_AUTORADIOBUTTON|WS_GROUP, Width/2, 0, Width/5, Height/4, hWnd, (HMENU)ID_ALNUM, g_hInst, NULL);
		HWND hAllChar2 = CreateWindow(TEXT("Button"), TEXT("Data:All Char"), WS_CHILD | WS_VISIBLE | WS_BORDER |BS_AUTORADIOBUTTON, Width/2, Height/4, Width/5, Height/4, hWnd, (HMENU)ID_ALLCHAR, g_hInst, NULL);
		HWND hString3 = CreateWindow(TEXT("Button"), TEXT("Data:String"), WS_CHILD | WS_VISIBLE | WS_BORDER |BS_AUTORADIOBUTTON, Width/2, Height*2/4, Width/5, Height/4, hWnd, (HMENU)ID_STRING, g_hInst, NULL);
		HWND hNum4 = CreateWindow(TEXT("Button"), TEXT("Data:Number"), WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTORADIOBUTTON, Width/2, Height * 3 / 4, Width/5, Height / 4, hWnd, (HMENU)ID_NUM, g_hInst, NULL);
		SendMessage(hAlnum1, BM_SETCHECK, BST_CHECKED, 0);
	}
		return 0;
	case WM_LOADTEXT:
	{
		HWND hEdit;
		if (!(hEdit = GetDlgItem(hWnd,ID_EDIT)))
			hEdit = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 0, 0,0,0, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);
		SendMessage(hEdit, WM_SETTEXT, 0, lParam);
		break;
	}
	return 0;
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case ID_SENDDATA:
			if (MessageBox(hWnd, TEXT("CREATE HUFFMAN TREE/TABLE?"), TEXT("Sending Data..."), MB_OKCANCEL) == IDOK)
			{
				LONG type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				DeleteHTree(type);

				int RadioList[] = {ID_ALNUM, ID_ALLCHAR, ID_STRING, ID_NUM };
				int i = 0;
				for (i = 0; i < _countof(RadioList); ++i)
					if (SendMessage(GetDlgItem(hWnd, RadioList[i]), BM_GETCHECK, 0, 0))
						break;

				SetWindowLongPtr(hWnd, GWLP_USERDATA, i);

				int TextLen = SendMessage(GetDlgItem(hWnd, ID_EDIT), WM_GETTEXTLENGTH, 0, 0) + 1;
				TCHAR* DataTxt = new TCHAR[TextLen];
				SendMessage(GetDlgItem(hWnd, ID_EDIT), WM_GETTEXT, TextLen, (LPARAM)DataTxt);
				//i = data Type
				SendMessage(hGraph, WM_HUFFMAN, i, (LPARAM)DataTxt);
				SendMessage(hTable, WM_HUFFMAN, i, (LPARAM)DataTxt);
				delete DataTxt;

			
			}
			break;
		}
		break;
	}
	case WM_SIZE:
	{
		int Width = LOWORD(lParam);
		int Height = HIWORD(lParam);

		int CurIndex = 0;
		HWND hEdit = GetDlgItem(hWnd, ID_EDIT);
		HWND hButton = GetDlgItem(hWnd, ID_SENDDATA);
		HWND hChar1 = GetDlgItem(hWnd, ID_ALNUM);
		HWND hDigit2 = GetDlgItem(hWnd, ID_ALLCHAR);
		HWND hString3 = GetDlgItem(hWnd, ID_STRING);
		HWND hNum4 = GetDlgItem(hWnd, ID_NUM);

		SetWindowPos(hEdit, NULL, 0, 0, Width / 2, Height, NULL);
		SetWindowPos(hButton, NULL, Width / 2 + Width / 5, 0, Width - Width / 2 - Width / 5, Height, NULL);
		SetWindowPos(hChar1, NULL, Width / 2, 0, Width / 5, Height / 4, NULL);
		SetWindowPos(hDigit2, NULL, Width / 2, Height / 4, Width / 5, Height / 4, NULL);
		SetWindowPos(hString3, NULL, Width / 2, Height * 2 / 4, Width / 5, Height / 4, NULL);
		SetWindowPos(hNum4, NULL, Width / 2, Height * 3 / 4, Width / 5, Height / 4, NULL);
		break;
	}

	case WM_DESTROY:
	{
		LONG type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		DeleteHTree(type);
		hData = nullptr;
		break;
	}

	}
	return DefMDIChildProc(hWnd, iMsg, wParam, lParam);
}

void DeleteHTree(LONG type)
{
	switch (type)
	{
	case 0:
		delete ((tag_HuffTree<TCHAR>*)H_Tree);
		break;
	case 1:
		delete ((tag_HuffTree<TCHAR>*)H_Tree);
		break;
	case 2:
		delete ((tag_HuffTree<tstring>*)H_Tree);
		break;
	case 3:
		delete ((tag_HuffTree<INT>*)H_Tree);
		break;
	}
	H_Tree = nullptr;
}
#endif


BOOL CALLBACK TxtEncodingDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
	{
		SendDlgItemMessage(hDlg, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0);
		
		std::wstring Wstr{ (wchar_t*)lParam };
		
		std::string cstr{ (char*)lParam };
		std::wstring Astr;
		Astr.assign(cstr.begin(), cstr.end());

		SetDlgItemTextW(hDlg, IDC_EDIT2, (LPCTSTR)Wstr.c_str());
		SetDlgItemTextW(hDlg, IDC_EDIT1, (LPCTSTR)Astr.c_str());
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (SendDlgItemMessage(hDlg, IDC_RADIO1, BM_GETCHECK, 0,0))
				EndDialog(hDlg, 256); //ANSI
			else
				EndDialog(hDlg, 65536); //UNICODE
			break;
		case IDCANCEL:
			EndDialog(hDlg, false);
			break;
		}
		break;
	case WM_DESTROY:
		break;
	default:
		return FALSE;
	}
	return TRUE;
}