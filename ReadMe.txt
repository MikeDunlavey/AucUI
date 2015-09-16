/*****
Copyright (C) 2007 Michael R. Dunlavey
For license terms, see the copyright notice in file AucUI.h
*****/

========================================================================
       MICROSOFT FOUNDATION CLASS LIBRARY : DDDemo
========================================================================

This is a demonstration program for Dynamic Dialogs.
It is distributed under the GNU GPL.

It is a simple dialog-based application built using
Microsoft Foundation Classes.  It is meant to show
how Dynamic Dialogs can be implemented.  As such, it
provides only a minimal implementation.

Class CDDDemoDlg contains the implementation and the
demonstration program.

DDDemoDlg.h
	This file contains the class definition, including
	the "global" variables used for Differential Execution
	and Dynamic Dialogs.

DDDemoDlg.cpp
	This file contains all the support routines needed
	for the class definition, including a set of routines
	for defining controls.

DDDemoDlg1.cpp
	This file contains the deContents routine that defines
	what the content of the dialog will be.
	It contains a #if statement that can be used to choose
	between two configurations.

	Configuration 1 is a simple exercise of a set of buttons
	and an edit control.

	Configuration 2 allows editing of a list of "employees",
	each having a name and age, and a "smoker" boolean that
	only applies if the age is >= 50.

The other files are just MFC boilerplate.


/////////////////////////////////////////////////////////////////////////////
Other notes:

If your application uses MFC in a shared DLL, and your application is 
in a language other than the operating system's current language, you
will need to copy the corresponding localized resources MFC42XXX.DLL
from the Microsoft Visual C++ CD-ROM onto the system or system32 directory,
and rename it to be MFCLOC.DLL.  ("XXX" stands for the language abbreviation.
For example, MFC42DEU.DLL contains resources translated to German.)  If you
don't do this, some of the UI elements of your application will remain in the
language of the operating system.

/////////////////////////////////////////////////////////////////////////////
