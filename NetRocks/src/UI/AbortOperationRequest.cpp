#pragma once
#include "Defs.h"
#include "DialogUtils.h"
#include "../lng.h"


/*                                               
345                                            50
 ============= Abort operation ================
| Confirm abort current operation              |
|----------------------------------------------|
|   [ &Abort operation  ]    [ &Continue ]     |
 ==============================================
    6                   27   32           45     
*/


class AbortConfirm : protected BaseDialog
{
	int _i_dblbox = -1;
	int _i_confirm = -1;

public:
	AbortConfirm()
	{
		_i_dblbox = _di.Add(DI_DOUBLEBOX, 3,1,50,5, 0, MAbortTitle);
		_di.Add(DI_TEXT, 5,2,48,2, DIF_CENTERGROUP, MAbortText);
		_di.Add(DI_TEXT, 4,3,49,3, DIF_BOXCOLOR | DIF_SEPARATOR);

		_i_confirm = _di.Add(DI_BUTTON, 6,4,27,4, DIF_CENTERGROUP, MAbortConfirm, nullptr, FDIS_DEFAULT);
		_di.Add(DI_BUTTON, 32,4,46,4, DIF_CENTERGROUP, MAbortNotConfirm);
	}

	bool Ask()
	{
		return (Show(_di[_i_dblbox].Data, 6, 2, FDLG_WARNING) == _i_confirm);
	}
};


///////////////////////////////////////////////////////////

class AbortOperationProgress : protected BaseDialog
{
	ProgressState &_state;
	const time_t _ts;
	int _i_dblbox;
	std::string _title;
	bool _finished;

protected:
	LONG_PTR DlgProc(HANDLE dlg, int msg, int param1, LONG_PTR param2)
	{
		if (msg == DN_ENTERIDLE) {
			if (_state.finished) {
				if (!_finished) {
					_finished = true;
					Close(dlg);
				}
			} else {
				const time_t secs = time(NULL) - _ts;
				if (secs) {
					snprintf(_di[_i_dblbox].Data, sizeof(_di[_i_dblbox].Data) - 1, "%s (%lu)", _title.c_str(), (unsigned long)secs);
					TextToDialogControl(dlg, _i_dblbox, _di[_i_dblbox].Data);
					if (secs == 60) {
						_finished = true;
						Close(dlg);
					}
				}
			}
		}	
		return BaseDialog::DlgProc(dlg, msg, param1, param2);
	}

public:
	AbortOperationProgress(ProgressState &state)
		: _state(state), _ts(time(NULL))
	{
		_i_dblbox = _di.Add(DI_DOUBLEBOX, 3, 1, 50, 3, 0, MAbortingOperationTitle);
		_di.Add(DI_BUTTON, 5,2,48,2, DIF_CENTERGROUP, MAbortOperationForce);
		_title = _di[_i_dblbox].Data;
	}

	void Show()
	{
		for (;;) {
			BaseDialog::Show(_di[_i_dblbox].Data, 6, 2, FDLG_REGULARIDLE);
			if (_state.finished) break;
			if (_state.ao_host) {
				_state.ao_host->ForcefullyAbort();
			} else {
				fprintf(stderr, "NetRocks::AbortOperationProgress: no ao_host\n");
			}
		}
	}
};


void AbortOperationRequest(ProgressState &state)
{
	if (state.aborting)
		return;

	bool saved_paused;
	{ // pause opetation while showing UI abort confirmation
		std::lock_guard<std::mutex> locker(state.mtx);
		saved_paused = state.paused;
		state.paused = true;
	}
	if (!AbortConfirm().Ask()) { //param1 == _i_cancel && 
		std::lock_guard<std::mutex> locker(state.mtx);
		state.paused = saved_paused;
		return;
	}
	{
		std::lock_guard<std::mutex> locker(state.mtx);
		state.aborting = true;
		state.paused = saved_paused;
	}

//	AbortOperationProgress(state).Show();
}


