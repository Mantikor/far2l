#include <mutex>
#include <map>
#include <vector>

#include "WinPort.h"
#include "Backend.h"

static DWORD g_winport_con_mode = ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS;
static std::mutex g_winport_con_mode_mutex;

extern "C" {
	
	WINPORT_DECL(GetLargestConsoleWindowSize,COORD,(HANDLE hConsoleOutput))
	{
		return g_winport_con_out->GetLargestConsoleWindowSize();
	}

	WINPORT_DECL(SetConsoleWindowInfo,BOOL,(HANDLE hConsoleOutput, BOOL bAbsolute, const SMALL_RECT *lpConsoleWindow))
	{
		g_winport_con_out->SetWindowInfo(bAbsolute!=FALSE, *lpConsoleWindow);
		return TRUE;
	}

	WINPORT_DECL(SetConsoleTitle,BOOL,(const WCHAR *title))
	{
		g_winport_con_out->SetTitle(title);
		return TRUE;
	}

	WINPORT_DECL(GetConsoleTitle,DWORD,(WCHAR *title, DWORD max_size))
	{
		const std::wstring &s = g_winport_con_out->GetTitle();
		wcsncpy(title, s.c_str(), max_size);
		return (DWORD)(s.size() + 1);
	}

	WINPORT_DECL(SetConsoleScreenBufferSize,BOOL,(HANDLE hConsoleOutput,COORD dwSize))
	{
		g_winport_con_out->SetSize(dwSize.X, dwSize.Y);
		return TRUE;
	}

	WINPORT_DECL(SetConsoleDisplayMode,BOOL,(DWORD ModeFlags))
	{
		return (ModeFlags==CONSOLE_WINDOWED_MODE) ? TRUE : FALSE;
	}
	WINPORT_DECL(GetConsoleDisplayMode,BOOL,(LPDWORD lpModeFlags))
	{
		*lpModeFlags = 0;//WTF??? GetConsoleDisplayMode/SetConsoleDisplayMode returns different meanings!!!
		return TRUE;
	}
	WINPORT_DECL(ScrollConsoleScreenBuffer,BOOL,(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, 
		const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill))
	{
		return g_winport_con_out->Scroll(lpScrollRectangle, lpClipRectangle, dwDestinationOrigin, lpFill) ? TRUE : FALSE;
	}

	WINPORT_DECL(SetConsoleWindowMaximized,VOID,(BOOL Maximized))
	{
		g_winport_con_out->SetWindowMaximized(Maximized!=FALSE);
	}


	WINPORT_DECL(GetConsoleScreenBufferInfo,BOOL,(HANDLE hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFO *lpConsoleScreenBufferInfo))
	{
		unsigned int width = 0, height = 0;
		g_winport_con_out->GetSize(width, height);
		lpConsoleScreenBufferInfo->dwCursorPosition = g_winport_con_out->GetCursor();
		lpConsoleScreenBufferInfo->wAttributes = g_winport_con_out->GetAttributes();
		lpConsoleScreenBufferInfo->dwSize.X = width;
		lpConsoleScreenBufferInfo->dwSize.Y = height;
		lpConsoleScreenBufferInfo->srWindow.Left = 0;
		lpConsoleScreenBufferInfo->srWindow.Top = 0;
		lpConsoleScreenBufferInfo->srWindow.Right = width - 1;
		lpConsoleScreenBufferInfo->srWindow.Bottom = height - 1;
		lpConsoleScreenBufferInfo->dwMaximumWindowSize.X = width;
		lpConsoleScreenBufferInfo->dwMaximumWindowSize.Y = height;
		
		return TRUE;
	}

	WINPORT_DECL(SetConsoleCursorPosition,BOOL,(HANDLE hConsoleOutput,COORD dwCursorPosition))
	{
		g_winport_con_out->SetCursor(dwCursorPosition);
		return TRUE;
	}

	WINPORT_DECL(SetConsoleCursorInfo,BOOL,(HANDLE hConsoleOutput,const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo))
	{
		DWORD height = lpConsoleCursorInfo->dwSize;
		if (height > 100) height = 100;
		else if (height == 0) height = 1;
		g_winport_con_out->SetCursor((UCHAR)height, lpConsoleCursorInfo->bVisible!=FALSE);
		return TRUE;
	}

	WINPORT_DECL(GetConsoleCursorInfo,BOOL,(HANDLE hConsoleOutput,CONSOLE_CURSOR_INFO *lpConsoleCursorInfo))
	{
		UCHAR height;
		bool visible;
		g_winport_con_out->GetCursor(height, visible);
		lpConsoleCursorInfo->dwSize = height;
		lpConsoleCursorInfo->bVisible = visible ? TRUE : FALSE;
		return TRUE;
	}

	WINPORT_DECL(GetConsoleMode,BOOL,(HANDLE hConsoleHandle,LPDWORD lpMode))
	{
		std::lock_guard<std::mutex> lock(g_winport_con_mode_mutex);
		*lpMode = g_winport_con_mode;
		*lpMode|= g_winport_con_out->GetMode();
		return TRUE;
	}
	
	WINPORT_DECL(SetConsoleMode,BOOL,(HANDLE hConsoleHandle, DWORD dwMode))
	{
		std::lock_guard<std::mutex> lock(g_winport_con_mode_mutex);
		if ((dwMode&ENABLE_EXTENDED_FLAGS)==0) {
			dwMode&= ~(ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE);
			dwMode|= (g_winport_con_mode & (ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE));
		}
		g_winport_con_mode = dwMode;
		g_winport_con_out->SetMode(g_winport_con_mode);
		return TRUE;
	}


	WINPORT_DECL(SetConsoleTextAttribute,BOOL,(HANDLE hConsoleOutput, DWORD64 qAttributes))
	{
		g_winport_con_out->SetAttributes(qAttributes);
		return TRUE;
	}

	WINPORT_DECL(WriteConsole,BOOL,(HANDLE hConsoleOutput, const WCHAR *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved))
	{
		*lpNumberOfCharsWritten = g_winport_con_out->WriteString(lpBuffer, nNumberOfCharsToWrite);
		return TRUE;
	}

	WINPORT_DECL(WriteConsoleOutput,BOOL,(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpScreenRegion))
	{
		g_winport_con_out->Write(lpBuffer, dwBufferSize, dwBufferCoord, *lpScreenRegion);
		return TRUE;
	}

	WINPORT_DECL(ReadConsoleOutput, BOOL, (HANDLE hConsoleOutput, CHAR_INFO *lpBuffer, COORD dwBufferSize, COORD dwBufferCoord, PSMALL_RECT lpScreenRegion))
	{
		g_winport_con_out->Read(lpBuffer, dwBufferSize, dwBufferCoord, *lpScreenRegion);
		return TRUE;
	}

	WINPORT_DECL(WriteConsoleOutputCharacter,BOOL,(HANDLE hConsoleOutput, const WCHAR *lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten))
	{
		*lpNumberOfCharsWritten = g_winport_con_out->WriteStringAt(lpCharacter, nLength, dwWriteCoord);
		return TRUE;
	}

	WINPORT_DECL(FillConsoleOutputAttribute, BOOL, (HANDLE hConsoleOutput, DWORD64 qAttributes, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfAttrsWritten))
	{
		*lpNumberOfAttrsWritten = g_winport_con_out->FillAttributeAt(qAttributes, nLength, dwWriteCoord);
		return TRUE;
	}

	WINPORT_DECL(FillConsoleOutputCharacter, BOOL, (HANDLE hConsoleOutput, WCHAR cCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten))
	{
		*lpNumberOfCharsWritten = g_winport_con_out->FillCharacterAt(cCharacter, nLength, dwWriteCoord);
		return TRUE;
	}

	WINPORT_DECL(SetConsoleActiveScreenBuffer, BOOL,(HANDLE hConsoleOutput))
	{
		return TRUE;
	}

	WINPORT_DECL(FlushConsoleInputBuffer,BOOL,(HANDLE hConsoleInput))
	{
		g_winport_con_in->Flush();
		return TRUE;
	}

	WINPORT_DECL(GetNumberOfConsoleInputEvents,BOOL,(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents))
	{
		*lpcNumberOfEvents = g_winport_con_in->Count();
		return TRUE;
	}

	WINPORT_DECL(PeekConsoleInput,BOOL,(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead))
	{
		*lpNumberOfEventsRead = g_winport_con_in->Peek(lpBuffer, nLength);
		return TRUE;
	}

	WINPORT_DECL(ReadConsoleInput,BOOL,(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead))
	{
		*lpNumberOfEventsRead = 0;
		while (nLength) {
			DWORD cnt = g_winport_con_in->Dequeue(lpBuffer, nLength);
			if (cnt) {
				*lpNumberOfEventsRead+= cnt;
				nLength-= cnt;
				lpBuffer+= cnt;
				break;//or not break?
			} else
				g_winport_con_in->WaitForNonEmpty();
		}
		return TRUE;
	}

	WINPORT_DECL(WaitConsoleInput,BOOL,(DWORD dwTimeout))
	{
		return g_winport_con_in->WaitForNonEmpty((dwTimeout == INFINITE) ? -1 : dwTimeout) ? TRUE : FALSE;
	}

	WINPORT_DECL(WriteConsoleInput,BOOL,(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten))
	{
		g_winport_con_in->Enqueue(lpBuffer, nLength);
		*lpNumberOfEventsWritten = nLength;
		return TRUE;
	}


	static PHANDLER_ROUTINE gHandlerRoutine = NULL;

	WINPORT_DECL(GenerateConsoleCtrlEvent, BOOL, (DWORD dwCtrlEvent, DWORD dwProcessGroupId ))
	{
		if (!gHandlerRoutine || !gHandlerRoutine(dwCtrlEvent)) {
			if (dwCtrlEvent == CTRL_CLOSE_EVENT) {
				fprintf(stderr, "GenerateConsoleCtrlEvent(%u) - exiting\n", dwCtrlEvent);
				exit(0);
			}
		}
		fprintf(stderr, "GenerateConsoleCtrlEvent(%u)\n", dwCtrlEvent);
		return TRUE;
	}

	WINPORT_DECL(SetConsoleCtrlHandler, BOOL, (PHANDLER_ROUTINE HandlerRoutine, BOOL Add ))
	{
		if (Add) {
			gHandlerRoutine = HandlerRoutine;
			return TRUE;
		} else if (HandlerRoutine==gHandlerRoutine) {
			gHandlerRoutine = FALSE;
			return TRUE;
		}

		return FALSE;
	}
	
	WINPORT_DECL(SetConsoleScrollRegion, VOID, (HANDLE hConsoleOutput, SHORT top, SHORT bottom))
	{
		g_winport_con_out->SetScrollRegion(top, bottom);
	}
	
	WINPORT_DECL(GetConsoleScrollRegion, VOID, (HANDLE hConsoleOutput, SHORT *top, SHORT *bottom))
	{
		g_winport_con_out->GetScrollRegion(*top, *bottom);
	}
	
	WINPORT_DECL(SetConsoleScrollCallback, VOID, (HANDLE hConsoleOutput, PCONSOLE_SCROLL_CALLBACK pCallback, PVOID pContext))
	{
		g_winport_con_out->SetScrollCallback(pCallback, pContext);
	}
	
	WINPORT_DECL(BeginConsoleAdhocQuickEdit, BOOL, ())
	{
		{
			std::lock_guard<std::mutex> lock(g_winport_con_mode_mutex);
			if (g_winport_con_mode & ENABLE_QUICK_EDIT_MODE) {
				fprintf(stderr, "BeginConsoleAdhocQuickEdit: meaningless when enabled ENABLE_QUICK_EDIT_MODE\n");
				return FALSE;
			}
		}
		
		//here is possible non-critical race with enabling ENABLE_QUICK_EDIT_MODE
		g_winport_con_out->AdhocQuickEdit();
		return TRUE;
	}

	WINPORT_DECL(SetConsoleTweaks, DWORD64, (DWORD64 tweaks))
	{
		return g_winport_con_out->SetConsoleTweaks(tweaks);
	}

	WINPORT_DECL(SaveConsoleWindowState,VOID,())
	{
		return g_winport_con_out->ConsoleSaveWindowState();
	}

	WINPORT_DECL(ConsoleChangeFont, VOID, ())
	{
		return g_winport_con_out->ConsoleChangeFont();
	}

	WINPORT_DECL(IsConsoleActive, BOOL, ())
	{
		return g_winport_con_out->IsActive() ? TRUE : FALSE;
	}

	WINPORT_DECL(ConsoleDisplayNotification, VOID, (const WCHAR *title, const WCHAR *text))
	{
		g_winport_con_out->ConsoleDisplayNotification(title, text);
	}

	WINPORT_DECL(ConsoleBackgroundMode, BOOL, (BOOL TryEnterBackgroundMode))
	{
		return g_winport_con_out->ConsoleBackgroundMode(TryEnterBackgroundMode != FALSE) ? TRUE : FALSE;
	}

	WINPORT_DECL(SetConsoleFKeyTitles, BOOL, (const CHAR **titles))
	{
		return g_winport_con_out->SetFKeyTitles(titles) ? TRUE : FALSE;
	}

	WINPORT_DECL(GetConsoleColorPalette,BYTE,())
	{
		return g_winport_con_out->GetColorPalette();
	}

	WINPORT_DECL(OverrideConsoleColor, VOID, (DWORD Index, DWORD *ColorFG, DWORD *ColorBK))
	{
		return g_winport_con_out->OverrideColor(Index, ColorFG, ColorBK);
	}

	WINPORT_DECL(SetConsoleRepaintsDefer, VOID, (BOOL Deferring))
	{
		if (Deferring) {
			g_winport_con_out->RepaintsDeferStart();
		} else {
			g_winport_con_out->RepaintsDeferFinish();
		}
	}

	static struct {
		struct Cmp
		{
			bool operator()(const WCHAR *a, const WCHAR *b) const { return wcscmp(a, b) < 0; }
		};
		std::mutex mtx;
		std::vector<WCHAR *> id2str;
		std::map<const WCHAR *, COMP_CHAR, Cmp> str2id;
	} s_composite_chars;

	WINPORT_DECL(CompositeCharRegister,COMP_CHAR,(const WCHAR *lpSequence))
	{
		if (!lpSequence[0]) {
			return 0;
		}
		if (!lpSequence[1]) {
			return lpSequence[0];
		}

		std::lock_guard<std::mutex> lock(s_composite_chars.mtx);
		auto it = s_composite_chars.str2id.find(lpSequence);
		if (it != s_composite_chars.str2id.end()) {
			return it->second | COMPOSITE_CHAR_MARK;
		}
		wchar_t *wd = wcsdup(lpSequence);
		try {
			if (!wd)
				throw std::logic_error("wcsdup failed");

			const COMP_CHAR id = COMP_CHAR(s_composite_chars.id2str.size());
			s_composite_chars.id2str.emplace_back(wd);
			s_composite_chars.str2id.emplace(wd, id);
			return id | COMPOSITE_CHAR_MARK;

		} catch (std::exception &e) {
			fprintf(stderr, "%s: %s for '%ls'\n", __FUNCTION__, e.what(), lpSequence);
			free(wd);
		}
		return 0;
	}

	WINPORT_DECL(CompositeCharLookup,const WCHAR *,(COMP_CHAR CompositeChar))
	{
		if ((CompositeChar & COMPOSITE_CHAR_MARK) == 0) {
			fprintf(stderr, "%s: invoked for not composite-char 0x%llx\n",
				__FUNCTION__,  (unsigned long long)CompositeChar);
			return L"\u2022";
		}

		const COMP_CHAR id = CompositeChar & (~COMPOSITE_CHAR_MARK);

		std::lock_guard<std::mutex> lock(s_composite_chars.mtx);
		if (id >= (COMP_CHAR)s_composite_chars.id2str.size()) {
			fprintf(stderr, "%s: out of range composite-char 0x%llx\n",
				__FUNCTION__,  (unsigned long long)CompositeChar);
			return L"\u2022";
		}
		return s_composite_chars.id2str[id];
	}
}
