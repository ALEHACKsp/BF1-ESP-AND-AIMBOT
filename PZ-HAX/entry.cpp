
#include "SetupColor.h"
#include "KeyMan.h"
#include "direct3d.h"
#include "render.h"
HWND enumWindow = NULL;
void mainLoop() {

	SetupColors();
	for (;;) {
		MSG uMessage;
		uMessage.message = WM_NULL;
		ShowWindow(Globals::hWnd, 1);
		UpdateWindow(Globals::hWnd);
		SetLayeredWindowAttributes(Globals::hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
		if (uMessage.message != WM_QUIT)
		{
			if (PeekMessage(&uMessage, Globals::hWnd, 0, 0, PM_REMOVE)) {
				DispatchMessage(&uMessage);
				TranslateMessage(&uMessage);
			}
			UpdateWindow(Globals::hWnd);
			WINDOWINFO info;
			ZeroMemory(&info, sizeof(info));
			info.cbSize = sizeof(info);
			GetWindowInfo(Globals::tWnd, &info);	
			RECT rekt;
			GetClientRect(Globals::hWnd, &rekt);
			Globals::rWidth = ((rekt.right) - (rekt.left));
			Globals::rHeight = ((rekt.bottom) - (rekt.top));
			if (!IsIconic(Globals::hWnd)) {
				SetWindowPos(Globals::hWnd, HWND_TOPMOST, info.rcClient.left, info.rcClient.top, ((info.rcClient.right) - (info.rcClient.left)), ((info.rcClient.bottom) - (info.rcClient.top)), SWP_SHOWWINDOW);
			}
			D3DRender();
		}
		hookkeys();
		Sleep(1);

	}
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uiMessage)
	{
	case WM_PAINT:
		D3DRender();
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uiMessage, wParam, lParam);
	}
	return 0;
}

void d2oSetup(HWND tWindow) {
	Globals::tWnd = tWindow;
	WNDCLASSEX wc = { NULL };
	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = "d3do";
	wc.lpszMenuName = "d3do";
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClassEx(&wc))
	{
		MessageBox(Globals::tWnd, "Failed Register Class", "Fatal Error", MB_ICONERROR);
		return;
	}
	RECT rekt = { 0,0,200,200 };
	AdjustWindowRectEx(&rekt, WS_POPUP, FALSE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_COMPOSITED);

	Globals::hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, wc.lpszClassName, "D3D Overlay", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
	MARGINS mar = { -1 };
	DwmExtendFrameIntoClientArea(Globals::hWnd, &mar);
	D3DInitialize(Globals::hWnd);
}

DWORD WINAPI OverlayThread(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam);
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	printf("[BF1]Hooked By Zakaria\n");
	printf("[github]www.github.com/Zakaria-Codex\n");
	printf("[Discord]Zakaria#1077\n");
	printf("[unknowncheats]www.unknowncheats.me/forum/members/1930085.html\n");
	printf("fix aimbot disable Raw mouse input in game \n");
	printf("inject before join any server \n");

	enumWindow = FindWindow(0, "Battlefield™ 1");
	d2oSetup(enumWindow);
		mainLoop();
		return true;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {

	g_hModule = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OverlayThread, lpReserved, NULL, NULL);

	}
	else if (fdwReason == DLL_PROCESS_DETACH) {
		UpdateWindow(Globals::hWnd);
	}
	return TRUE;
}