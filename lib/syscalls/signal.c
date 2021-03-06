// FIXME hugo: copied directly from Newlib for now. Modified only to remove the
// Newlib-internal _DEFUN macros.

/*
 * signal.c
 * Original Author:	G. Haley
 *
 * signal associates the function pointed to by func with the signal sig. When
 * a signal occurs, the value of func determines the action taken as follows:
 * if func is SIG_DFL, the default handling for that signal will occur; if func
 * is SIG_IGN, the signal will be ignored; otherwise, the default handling for
 * the signal is restored (SIG_DFL), and the function func is called with sig
 * as its argument. Returns the value of func for the previous call to signal
 * for the signal sig, or SIG_ERR if the request fails.
 */

/* _init_signal initialises the signal handlers for each signal. This function
   is called by crt0 at program startup.  */

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <_syslist.h>
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _init_signal_r(struct _reent *ptr) 
{
	int i;

	if (ptr->_sig_func == NULL)
	{
		ptr->_sig_func = (_sig_func_ptr *)_malloc_r (ptr, sizeof (_sig_func_ptr) * NSIG);
		if (ptr->_sig_func == NULL)
			return -1;

		for (i = 0; i < NSIG; i++)
			ptr->_sig_func[i] = SIG_DFL;
	}

	return 0;
}

PRIVILEGED_FUNCTION _sig_func_ptr _signal_r(struct _reent *ptr, int sig, _sig_func_ptr func) 
{
	_sig_func_ptr old_func;

	if (sig < 0 || sig >= NSIG)
	{
		ptr->_errno = EINVAL;
		return SIG_ERR;
	}

	if (ptr->_sig_func == NULL && _init_signal_r (ptr) != 0)
		return SIG_ERR;

	old_func = ptr->_sig_func[sig];
	ptr->_sig_func[sig] = func;

	return old_func;
}

PRIVILEGED_FUNCTION int _raise_r(struct _reent *ptr, int sig) 
{
	_sig_func_ptr func;

	if (sig < 0 || sig >= NSIG)
	{
		ptr->_errno = EINVAL;
		return -1;
	}

	if (ptr->_sig_func == NULL)
		func = SIG_DFL;
	else
		func = ptr->_sig_func[sig];

	if (func == SIG_DFL)
		return _kill_r (ptr, _getpid_r (ptr), sig);
	else if (func == SIG_IGN)
		return 0;
	else if (func == SIG_ERR)
	{
		ptr->_errno = EINVAL;
		return 1;
	}
	else
	{
		ptr->_sig_func[sig] = SIG_DFL;
		func (sig);
		return 0;
	}
}

PRIVILEGED_FUNCTION int __sigtramp_r(struct _reent *ptr, int sig) 
{
	_sig_func_ptr func;

	if (sig < 0 || sig >= NSIG)
	{
		return -1;
	}

	if (ptr->_sig_func == NULL && _init_signal_r (ptr) != 0)
		return -1;

	func = ptr->_sig_func[sig];
	if (func == SIG_DFL)
		return 1;
	else if (func == SIG_ERR)
		return 2;
	else if (func == SIG_IGN)
		return 3;
	else
	{
		ptr->_sig_func[sig] = SIG_DFL;
		func (sig);
		return 0;
	}
}

