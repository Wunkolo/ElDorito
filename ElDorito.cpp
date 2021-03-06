#include "ElDorito.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <conio.h> // _getch()
#include <cctype> //isprint

#include <codecvt>
#include <cvt/wstring> // wstring_convert

#include "Utils/Utils.h"
#include "ElModules.h"

const char* Credits[] = {
	"Wunk",
	"stoker25 (emoose)"
};

size_t ElDorito::MainThreadID = 0;

ElDorito::ElDorito()
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(437);
	unsigned int ConsoleWidth = 80;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleBuf;
	if( GetConsoleScreenBufferInfo(hStdout, &ConsoleBuf) )
	{
		ConsoleWidth = ConsoleBuf.dwSize.X;
	}

	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

#ifdef _ELDEBUG
	std::string buildType = "Debug ";
#else
	std::string buildType = "";
#endif
	std::cout << "ElDorito" << "\xC3\xC4\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xB4 " << buildType << "Build Date: " << __DATE__ << " @ " << __TIME__ << std::endl;
	Terminal.SetTextColor(Console::Input);

	std::srand(GetTickCount());
	for( size_t i = 0; i < sizeof(Credits) / sizeof(char*); i++ )
	{
		for( size_t k = 0; k < 8; k++ )
		{
			Terminal.SetTextColor(
				(std::rand() & 1 ? Console::Red : Console::Green) |
				std::rand() & 1 * Console::Bright
				);
			std::cout << " \x10 \x11 \x1E \x1E "[std::rand() & 7];
		}
		Terminal.SetTextColor(Console::Input);
		std::cout << "  " << "\xC3\xC0"[(i == sizeof(Credits) / sizeof(char*) - 1) ^ 0];
		std::cout << Credits[i] << std::endl;;
	}

	Terminal.SetTextColor(Console::Green | Console::Bright);
	std::cout << "Enter \"";
	Terminal.SetTextColor(Console::Input);
	std::cout << "help";
	Terminal.SetTextColor(Console::Green | Console::Bright);
	std::cout << "\" or \"";
	Terminal.SetTextColor(Console::Input);
	std::cout << "help (command)";
	Terminal.SetTextColor(Console::Green | Console::Bright);

	std::cout << "\" to get started!" << std::endl;

	std::cout << "Please take the time to read our ";
	Terminal.SetTextColor(Console::Input);
	std::cout << "disclaimer";
	Terminal.SetTextColor(Console::Green | Console::Bright);
	std::cout << " as well." << std::endl;

	std::cout << "Current directory: ";
	Terminal.SetTextColor(Console::Input);
	std::cout << GetDirectory() << std::endl;
	Terminal.SetTextColor(Console::Red | Console::Blue);
	std::cout << std::string(ConsoleWidth - 1, '\xC4') << std::endl;
	Terminal.SetTextColor(Console::Info);

	::CreateDirectoryA(GetDirectory().c_str(), NULL);

	// Enable write to all executable memory
	size_t Offset, Total;
	Offset = Total = 0;
	MEMORY_BASIC_INFORMATION MemInfo;

	//printf("\nUnprotecting memory...");
	while( VirtualQuery((uint8_t*)GetBasePointer() + Offset, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION)) )
	{
		Offset += MemInfo.RegionSize;
		if( MemInfo.Protect == PAGE_EXECUTE_READ )
		{
			//printf("%0X\n", (size_t)((uint8_t*)GetBasePointer() + Offset));
			Total += MemInfo.RegionSize;
			VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_EXECUTE_READWRITE, &MemInfo.Protect);
		}
	}
	//printf("\nDone! Unprotected %u bytes of memory\n", Total);

	// English patch
	Patch(0x2333FD, { 0 }).Apply();

	// Enable tag edits
	Patch(0x101A5B, { 0xEB }).Apply();
	Patch::NopFill(Pointer::Base(0x102874), 2);

	// No --account args patch
	Patch(0x43731A, { 0xEB, 0x0E }).Apply();
	Patch(0x4373AD, { 0xEB, 0x03 }).Apply();

	// Fix gamepad option in settings (todo: find out why it doesn't detect gamepads
	// and find a way to at least enable pressing ESC when gamepad is enabled)
	Patch::NopFill(Pointer::Base(0x20D7F2), 2);

	// Fix "Network" setting in lobbies (change broken 0x100B7 menuID to 0x100B6)
	Patch(0x6C34B0, { 0xB6 }).Apply();

	// Fix UI elements automatically closing on mainmenu-loaded maps (todo: find real reason why they close)
	Patch(0x6AA870, { 0xC3 }).Apply();

	// Fix controller not working on UI elements when in mainmenu-loaded maps (todo: find why this patch works)
	Patch(0x6AB33D, { 0xB8, 0x00, 0x00, 0x00, 0x00 }).Apply();
	Patch(0x6B538F, { 0xB8, 0x00, 0x00, 0x00, 0x00 }).Apply();

	// Update window title patch
	const uint8_t windowData[] = { 0x3A, 0x20, 0x45, 0x6C, 0x20, 0x44, 0x6F, 0x72, 0x69, 0x74, 0x6F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00 };
	Pointer::Base(0x159C02F).Write(windowData, sizeof(windowData));
	Pointer::Base(0x159C06F).Write(windowData, sizeof(windowData));
	Pointer::Base(0x1EAAEE0).Write(windowData, sizeof(windowData));

	// Register Modules
	PushModule<Test>("test");
	PushModule<Ammo>("ammo");
	PushModule<Camera>("camera");
	PushModule<DebugLogging>("debug");
	PushModule<Fov>("fov");
	PushModule<Globals>("global");
	PushModule<Godmode>("god");
	PushModule<Hud>("hud");
	PushModule<LoadLevel>("load");
	PushModule<Spawn>("spawn");
	PushModule<Information>("info");
	PushModule<ShowGameUI>("show_ui");
	PushModule<Disclaimer>("disclaimer");
	//PushModule<ThirdPerson>("third");

	SetSessionMessage("ElDorito | Version: " + buildType + Utils::Version::GetInfo("File Version") + " | Build Date: " __DATE__);

	// Parse command-line commands
	int numArgs = 0;
	LPWSTR* szArgList = CommandLineToArgvW(GetCommandLineW(), &numArgs);
	if( szArgList && numArgs > 1 )
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		for( int i = 1; i < numArgs; i++ )
		{
			std::wstring arg = std::wstring(szArgList[i]);
			if( arg.compare(0, 1, L"-") != 0 ) // if it doesn't start with -
				continue;

			size_t pos = arg.find(L"=");
			if( pos == std::wstring::npos || arg.length() <= pos + 1 ) // if it doesn't contain an =, or there's nothing after the =
				continue;

			std::string argname = converter.to_bytes(arg.substr(1, pos - 1));
			std::string argvalue = converter.to_bytes(arg.substr(pos + 1));

			if( Commands.count(argname) != 1 || Commands[argname] == nullptr ) // command not registered
				continue;

			Commands[argname]->Run({ argname, argvalue });
		}
	}

	Terminal.PrintLine();
}

void ElDorito::Tick(const std::chrono::duration<double>& DeltaTime)
{
	if( _kbhit() )
	{
		Terminal.HandleInput(_getch());
		Terminal.PrintLine();
	}
	for( auto Command : Commands )
	{
		if( Command.second )
		{
			Command.second->Tick(DeltaTime);
		}
	}
}

namespace
{
	static void HandleFinder()
	{
	};
}

std::string ElDorito::GetDirectory()
{
	char Path[MAX_PATH];
	HMODULE hMod;
	if( !GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&::HandleFinder, &hMod) )
	{
		int Error = GetLastError();
		std::cout << "Unable to resolve current directory, error code: " << Error << std::endl;
	}
	GetModuleFileNameA(hMod, Path, sizeof(Path));
	std::string Dir(Path);
	Dir = Dir.substr(0, std::string(Dir).find_last_of('\\') + 1);
	return Dir;
}

void ElDorito::SetSessionMessage(const std::string& Message)
{
	static wchar_t msgBuf[256];
	wmemset(msgBuf, 0, 256);

	std::wstring msgUnicode = Utils::String::WidenString(Message);
	wcscpy_s(msgBuf, 256, msgUnicode.c_str());

	Pointer::Base(0x2E5338).Write<uint8_t>(0x68);
	Pointer::Base(0x2E5339).Write(&msgBuf);
	Pointer::Base(0x2E533D).Write<uint8_t>(0x90);
	Pointer::Base(0x2E533E).Write<uint8_t>(0x90);

	// todo: some way of undoing this
}

Pointer ElDorito::GetMainTls(size_t Offset)
{
	static Pointer ThreadLocalStorage;
	if( !ThreadLocalStorage && GetMainThreadID() )
	{
		size_t MainThreadID = GetMainThreadID();

		HANDLE MainThreadHandle = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, false, MainThreadID);

		// Get thread context
		CONTEXT MainThreadContext;
		MainThreadContext.ContextFlags = CONTEXT_FULL;

		SuspendThread(MainThreadHandle);
		BOOL success = GetThreadContext(MainThreadHandle, &MainThreadContext);
		if( !success )
		{
			std::cout << "Error getting thread context: " << GetLastError() << std::endl;
			std::exit(1);
		}
		ResumeThread(MainThreadHandle);

		// Get thread selector

		LDT_ENTRY MainThreadLdt;

		success = GetThreadSelectorEntry(MainThreadHandle, MainThreadContext.SegFs, &MainThreadLdt);
		if( !success )
		{
			std::cout << "Error getting thread context: " << GetLastError() << std::endl;
		}
		size_t TlsPtrArrayAddress = (size_t)((size_t)(MainThreadLdt.HighWord.Bits.BaseHi << 24) | (MainThreadLdt.HighWord.Bits.BaseMid << 16) | MainThreadLdt.BaseLow) + 0x2C;
		size_t TlsPtrAddress = Pointer(TlsPtrArrayAddress).Read<uint32_t>();

		// Index has been consistantly 0. Keep a look out.
		ThreadLocalStorage = Pointer(TlsPtrAddress)[0];
	}

	return ThreadLocalStorage(Offset);
}