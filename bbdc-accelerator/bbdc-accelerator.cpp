#include <Windows.h>
#include <iostream>

HHOOK keyboardHook = 0;
HWND bbdcWindow = 0;

const INT DELTA_X = 280;
const INT DELTA_Y = 70;

void getBBDCWindowRect(RECT& rect)
{
	GetWindowRect(bbdcWindow, &rect);
}

void click(int x, int y)
{
	SetCursorPos(x, y);
	SendMessage(bbdcWindow, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	SendMessage(bbdcWindow, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(x, y));
}

void clickLeftBottom()
{
	RECT rect;
	getBBDCWindowRect(rect);
	int x = rect.left + DELTA_X;
	int y = rect.bottom - DELTA_Y;
	click(x, y);
}

void clickMiddleBottom()
{
	RECT rect;
	getBBDCWindowRect(rect);
	int x = rect.left + (rect.right - rect.left) / 2;
	int y = rect.bottom - DELTA_Y;
	click(x, y);
}

void clickRightBottom()
{
	RECT rect;
	getBBDCWindowRect(rect);
	int x = rect.right - DELTA_X;
	int y = rect.bottom - DELTA_Y;
	click(x, y);
}


LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int nCode,		// 规定钩子如何处理消息，小于 0 则直接 CallNextHookEx
	_In_ WPARAM wParam,	// 消息类型
	_In_ LPARAM lParam	// 指向某个结构体的指针，这里是 KBDLLHOOKSTRUCT（低级键盘输入事件）
) {
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息
	/*
	typedef struct tagKBDLLHOOKSTRUCT {
		DWORD     vkCode;		// 按键代号
		DWORD     scanCode;		// 硬件扫描代号，同 vkCode 也可以作为按键的代号。
		DWORD     flags;		// 事件类型，一般按键按下为 0 抬起为 128。
		DWORD     time;			// 消息时间戳
		ULONG_PTR dwExtraInfo;	// 消息附加信息，一般为 0。
	}KBDLLHOOKSTRUCT,*LPKBDLLHOOKSTRUCT,*PKBDLLHOOKSTRUCT;
	*/
	if ((ks->flags == 128 || ks->flags == 129) && GetForegroundWindow() == bbdcWindow)
	{
		// 监控键盘
		std::cout << "vkCode: " << ks->vkCode << std::endl;
		switch (ks->vkCode)
		{
		case 0x51: // Q
			clickLeftBottom();
			return 1;		// 使按键失效
			break;
		case 0x57: // W
			clickMiddleBottom();
			return 1;		// 使按键失效
			break;
		case 0x45: // E
			clickRightBottom();
			return 1;		// 使按键失效
			break;
		}

	}

	// 将消息传递给钩子链中的下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, LPCTSTR argv[])
{
	// 运行不背单词app。如果已经运行，会自动切换到前台
	system("start %LOCALAPPDATA%\\Microsoft\\WindowsApps\\MicrosoftCorporationII.WindowsSubsystemForAndroid_8wekyb3d8bbwe\\WsaClient.exe /launch wsa://cn.com.langeasy.LangEasyLexis");

	while (bbdcWindow == 0 || keyboardHook == 0) {

		Sleep(2000);

		bbdcWindow = FindWindow(TEXT("cn.com.langeasy.LangEasyLexis"), NULL);
		if (bbdcWindow == NULL)
		{
			std::cerr << "Could not find BBDC window" << std::endl << std::endl;
			continue;
		}
		std::cout << "BBDC window handle: " << bbdcWindow << std::endl;

		DWORD threadId = GetWindowThreadProcessId(bbdcWindow, NULL);
		std::cout << "BBDC window thread id: " << threadId << std::endl;

		// 安装钩子
		keyboardHook = SetWindowsHookEx(
			WH_KEYBOARD_LL,			// 钩子类型，WH_KEYBOARD_LL 为键盘钩子
			LowLevelKeyboardProc,	// 指向钩子函数的指针
			GetModuleHandleA(NULL),	// Dll 句柄
			NULL
		);
		if (keyboardHook == 0) { std::cout << "挂钩键盘失败" << std::endl << std::endl; continue; }
		std::cout << "挂钩键盘成功" << std::endl;

	}

	// 不可漏掉消息处理，不然程序会卡死
	MSG msg;
	while (1)
	{
		// 如果消息队列中有消息
		if (PeekMessageA(
			&msg,
			NULL,
			NULL,
			NULL,
			PM_REMOVE
		)) {
			// 把按键消息传递给字符消息
			TranslateMessage(&msg);

			// 将消息分派给窗口程序
			DispatchMessageW(&msg);
		}
		else
			Sleep(0);    //避免CPU全负载运行
	}
	// 删除钩子
	UnhookWindowsHookEx(keyboardHook);

	return 0;
}
