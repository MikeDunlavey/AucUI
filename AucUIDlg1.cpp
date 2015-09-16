// AucUIDlg1.cpp : implementation file
/*****
Copyright (C) 2007 Michael R. Dunlavey
For license terms, see the copyright notice in file AucUI.h
*****/

#include "stdafx.h"
#include "AucUI.h"
#include "AucUIDlg.h"

#include <winsock.h>

#ifdef USE_MYSQL
	#include "C:\\Program Files\\MySQL\\MySQL Connector C 6.0.2\\include\\mysql.h"
#endif

// Which page is being shown
enum {
	PAGE_ADMIN,
	PAGE_STATUS,
	PAGE_ITEM,
	PAGE_CUSTOMER,
	PAGE_ADDEXTRA,
	PAGE_ITEMDETAIL,
	PAGE_CUSTDETAIL,
	PAGE_N,
};
int iWhichPage = PAGE_ADMIN;

void UpdateCustAllFields(int ic);
void UpdateItemAllFields(int ic);
#define TCOV ASSERT(FALSE)
#define _COV ASSERT(TRUE)

//- Customer Page -----------------------------------------------------------
class CCustomer {
public:;
	long iCustomerId;
	long iExtraCharge;
	long iAmtPaid;
	CString sPayMethod;
	CString sDescription;
	CCustomer() : iCustomerId(0), iExtraCharge(0), iAmtPaid(0), sPayMethod(""), sDescription("") {}
	CCustomer(int id, LPCTSTR s) : iCustomerId(id), iExtraCharge(0), iAmtPaid(0), sPayMethod(""), sDescription(s) {}
	void CopyTo(CCustomer* p){
		_COV;
		p->iCustomerId = iCustomerId;
		p->iExtraCharge = iExtraCharge;
		p->iAmtPaid = iAmtPaid;
		p->sPayMethod = sPayMethod;
		p->sDescription = sDescription;
	}
};
class CItem {
public:;
	long iItemId;
	long iCustomerId;
	long iBidAmount;
	long iValue;
	CString sCategory;
	CString sDescription;
	void CopyTo(CItem* p){
		_COV;
		p->iItemId = iItemId;
		p->iCustomerId = iCustomerId;
		p->iBidAmount = iBidAmount;
		p->iValue = iValue;
		p->sCategory = sCategory;
		p->sDescription = sDescription;
	}
};
typedef CTypedPtrArray<CPtrArray, CCustomer*> CCustomerArray;
typedef CTypedPtrArray<CPtrArray, CItem*> CItemArray;
CItemArray paItem;
CCustomerArray paCust;
CItemArray paCustItem;
CDWordArray ithItem;
#define CLEAR(pa) while((pa).GetSize() > 0){delete (pa)[0]; (pa).RemoveAt(0);}

#ifdef USE_MYSQL
MYSQL mySql;
#endif
BOOL bMySqlOpen;

CString sHost		= "192.168.0.7";
CString sUser		= "root";
CString sPassword	= "root";
CString sDB			= "lfn12";
CString sDirectory	= "c:\\lfn12";
CString sCustFileName = "c:\\lfn12\\Cust12.txt";
CString sItemFileName = "c:\\lfn12\\Item12.txt";
CString sPrintCommand =
	"cmd.exe /c c:\\lfn12\\prFile32.exe /q "
	;
CString sBanner		= "Black, White, & Read All Over, Sept. 14, 2012";

const int www = 40;

long iExtraChargeToAdd = 100;

int MyFindNoCase(LPCTSTR s, LPCTSTR pat){
	_COV;
	int ns = strlen(s), npat = strlen(pat);
	if (npat==0) return 0;
	int i;
	for (i = 0; i <= (ns - npat); i++){
		if (strnicmp(s+i, pat, npat)==0){
			return i;
		}
	}
	return -1;
}

BOOL bGoodConnection = FALSE;

// routine to open connection
static void OpenMySQL(){
#ifdef USE_MYSQL
	_COV;
	bGoodConnection = FALSE;
	if (!bMySqlOpen){
		_COV;
		BOOL bOK = (mysql_init(&mySql) != NULL);
		if (bOK){
			_COV;
			bOK = (NULL != mysql_real_connect(&mySql
				, sHost
				, sUser
				, sPassword
				, sDB
				, 0							// port
				, NULL						// unix_socket
				, 0							// clientflag
				));
		}
		bMySqlOpen = bOK;
		bGoodConnection = bOK;
	}
#endif
}

// routine to close connection
void CloseMySQL(){
#ifdef USE_MYSQL
	_COV;
	mysql_close(&mySql);
	bMySqlOpen = FALSE;
#endif
}
// routine to load customer list from tab file
void ImportCustFile(LPCTSTR sFile){
#ifdef USE_MYSQL
	_COV;
	BOOL bQueryOK = TRUE;
	TCHAR buf[1024];

	sprintf(buf
		, "delete from Customer"
		);
	bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("ImportCustFile: delete customers failed");
		return;
	}

	sprintf(buf
		, "load data local infile \"%s\" into table Customer"
		, sFile
		);
	for (int i = strlen(buf); --i >= 0;){
		if (buf[i]=='\\') buf[i] = '/';
	}
	bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("ImportCustFile: load file failed");
		return;
	}
	_COV;
#endif
}
// routine to export customer list to tab file
void ExportCustFile(LPCTSTR sFileName){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("ExportCustFile: MySQL not open");
		return;
	}
	BOOL bQueryOK = (0 == mysql_query(&mySql, "select CustomerId, ExtraCharge, AmtPaid, PayMethod, Description from Customer order by CustomerId"));
	if (bQueryOK){
		_COV;
		MYSQL_RES* result = NULL;
		MYSQL_ROW row;
		unsigned int num_fields;
		result = mysql_store_result(&mySql);
		if (result){  // there are rows
			_COV;
			num_fields = mysql_num_fields(result);
			ASSERT(num_fields == 5);
			FILE* f = fopen(sFileName, "w");
			if (f != NULL){
				_COV;
				while ((row = mysql_fetch_row(result))){
					_COV;
					LPCTSTR sId = row[0];
					LPCTSTR sExtraCharge = row[1];
					LPCTSTR sAmtPaid = row[2];
					LPCTSTR sPayMethod = row[3];
					LPCTSTR sDescription = row[4];
					ASSERT(sId != NULL);
					if (sExtraCharge == NULL){_COV; sExtraCharge = "";}
					if (sAmtPaid == NULL){_COV; sAmtPaid = "";}
					if (sPayMethod == NULL){_COV; sPayMethod = "";}
					ASSERT(sDescription != NULL);
					fprintf(f, "%s\t%s\t%s\t%s\t%s\n", sId, sExtraCharge, sAmtPaid, sPayMethod, sDescription);
				}
				fclose(f);
			}
			else {
				AfxMessageBox("ExportCustFile: file open failed");
			}
			mysql_free_result(result);
		}
		else {
			AfxMessageBox("ExportCustFile: no rows selected");
		}
	}
	if (!bQueryOK){
		AfxMessageBox("ExportCustFile: query failed");
	}
#endif
}
void ImportItemFile(LPCTSTR sFile){
#ifdef USE_MYSQL
	BOOL bQueryOK = TRUE;
	TCHAR buf[1024];

	sprintf(buf
		, "delete from Item"
		);
	bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("ImportItemFile: delete old items failed");
		return;
	}

	sprintf(buf
		, "load data local infile \"%s\" into table Item"
		, sFile
		);
	for (int i = strlen(buf); --i >= 0;){
		if (buf[i]=='\\') buf[i] = '/';
	}
	bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("ImportItemFile: load items failed");
		return;
	}
#endif
}
// routine to export item list to tab file
void ExportItemFile(LPCTSTR sFileName){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("ExportItemFile: MySQL not open");
		return;
	}
	BOOL bQueryOK = (0 == mysql_query(&mySql, "select ItemId, CustomerId, Bid, Value, Category, Description from Item"));
	if (bQueryOK){
		_COV;
		MYSQL_RES* result = NULL;
		MYSQL_ROW row;
		unsigned int num_fields;
		result = mysql_store_result(&mySql);
		if (result){  // there are rows
			_COV;
			num_fields = mysql_num_fields(result);
			ASSERT(num_fields == 6);
			FILE* f = fopen(sFileName, "w");
			if (f != NULL){
				_COV;
				while ((row = mysql_fetch_row(result))){
					_COV;
					LPCTSTR sId = row[0];
					LPCTSTR sCustId = row[1];
					LPCTSTR sAmtBid = row[2];
					LPCTSTR sValue = row[3];
					LPCTSTR sCategory = (row[4] != NULL ? row[4] : "");
					LPCTSTR sDescription = (row[5] != NULL ? row[5] : "");
					ASSERT(sId != NULL);
					if (sAmtBid == NULL) sAmtBid = "";
					if (sValue == NULL) sValue = "";
					ASSERT(sCategory != NULL);
					ASSERT(sDescription != NULL);
					fprintf(f, "%s", sId);
					fprintf(f, "\t%s", sCustId);
					fprintf(f, "\t%s", sAmtBid);
					fprintf(f, "\t%s", sValue);
					fprintf(f, "\t%s", sCategory);
					fprintf(f, "\t%s", sDescription);
					fprintf(f, "\n");
				}
				fclose(f);
			}
			else {
				AfxMessageBox("ExportItemFile: file open failed");
			}
			mysql_free_result(result);
		}
		else {
			AfxMessageBox("ExportItemFile: no rows selected");
		}
	}
	else {
		AfxMessageBox("ExportItemFile: select failed");
	}
#endif
}
// routine to fill customer list
void FillCustArray(){
#ifdef USE_MYSQL
	TCHAR buf[1024];
	CLEAR(paCust);
	if (!bMySqlOpen){
//		AfxMessageBox("FillCustArray: MySQL not open");
		return;
	}
	BOOL bQueryOK = (0 == mysql_query(&mySql, "select CustomerId, ExtraCharge, AmtPaid, PayMethod, Description from Customer order by CustomerId"));
	if (bQueryOK){
		_COV;
		MYSQL_RES* result = NULL;
		MYSQL_ROW row;
		unsigned int num_fields;
		result = mysql_store_result(&mySql);
		if (result){  // there are rows
			_COV;
			num_fields = mysql_num_fields(result);
			ASSERT(num_fields == 5);
			while ((row = mysql_fetch_row(result))){
				_COV;
				LPCTSTR sId = row[0];
				LPCTSTR sExtraCharge = row[1];
				LPCTSTR sAmtPaid = row[2];
				LPCTSTR sPayMethod = (row[3] != NULL ? row[3] : "");
				LPCTSTR sDescription = (row[4] != NULL ? row[4] : "");
				ASSERT(sId != NULL);
				CCustomer* p = new CCustomer(atol(sId), sDescription);
				if (sExtraCharge != NULL){_COV; p->iExtraCharge = atol(sExtraCharge);}
				if (sAmtPaid != NULL){_COV; p->iAmtPaid = atol(sAmtPaid);}
				if (sPayMethod != NULL) p->sPayMethod = sPayMethod;
				paCust.Add(p);
			}
			mysql_free_result(result);
		}
		else {
			AfxMessageBox("FillCustArray: No rows selected");
		}
		int i;
		for (i = 0; i < paCust.GetSize(); i++){
			_COV;
			CCustomer* pc = paCust[i];
			strcpy(buf, pc->sDescription);
			int len = strlen(buf);
			if (len > 0 && buf[len-1] == '\r'){
				_COV;
				buf[--len] = 0;
				pc->sDescription = buf;
				UpdateCustAllFields(i);
			}
		}
	}
	else {
		AfxMessageBox("FillCustArray: select failed");
	}
#endif
}
int FindItemIndexById(int iItemId){
	int ii = 0, ni = paItem.GetSize();
	_COV;
	for (ii = ni; --ii >= 0;){
		_COV;
		if (paItem[ii]->iItemId == iItemId){_COV; break;}
	}
	if (ii < 0) _COV;
	return ii;
}
int FindIndexToInsertItemId(int iItemId){
	int ii = 0, ni = paItem.GetSize();
	_COV;
	for (ii = 0; ii < ni; ii++){
		_COV;
		if (paItem[ii]->iItemId == iItemId){_COV; return -1;}
		if (paItem[ii]->iItemId > iItemId){_COV; return ii;}
	}
	if (ii < 0) _COV;
	return ni;
}
int FindCustIndexById(int iCustomerId){
	int ic = 0, nc = paCust.GetSize();
	_COV;
	for (ic = nc; --ic >= 0;){
		if (paCust[ic]->iCustomerId == iCustomerId){_COV; break;}
	}
	if (ic < 0) _COV;
	return ic;
}
int FindIndexToInsertCustId(int iCustomerId){
	int ic = 0, nc = paCust.GetSize();
	_COV;
	for (ic = 0; ic < nc; ic++){
		if (paCust[ic]->iCustomerId == iCustomerId){_COV; return -1;}
		if (paCust[ic]->iCustomerId > iCustomerId){_COV; return ic;}
	}
	if (ic < 0) _COV;
	return nc;
}

// routine to fill item list for one customer
void FillCustItemArray(int iCust){
	_COV;
	if (iCust < 0 || iCust >= paCust.GetSize()){
		_COV;
		return;
	}
	int iCustomerId = paCust[iCust]->iCustomerId;
	int i;
	CLEAR(paCustItem);
	for (i = 0; i < paItem.GetSize(); i++){
		_COV;
		CItem* pi = paItem[i];
		if (pi->iCustomerId != iCustomerId){_COV; continue;}
		CItem* pi1 = new CItem;
		pi1->iCustomerId = pi->iCustomerId;
		pi1->iItemId = pi->iItemId;
		pi1->sCategory = pi->sCategory;
		pi1->sDescription = pi->sDescription;
		pi1->iBidAmount = pi->iBidAmount;
		pi1->iValue = pi->iValue;
		paCustItem.Add(pi1);
	}
}
void FillItemArray(){
#ifdef USE_MYSQL
	CLEAR(paItem);
	if (!bMySqlOpen){
//		AfxMessageBox("FillItemArray: MySQL not open");
		return;
	}
	TCHAR buf[1024];
	sprintf(buf, "select * from Item order by ItemId");
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (bQueryOK){
		_COV;
		MYSQL_RES* result = NULL;
		MYSQL_ROW row;
		unsigned int num_fields;
		result = mysql_store_result(&mySql);
		if (result){  // there are rows
			_COV;
			num_fields = mysql_num_fields(result);
			ASSERT(num_fields == 6);
			while ((row = mysql_fetch_row(result))){
				_COV;
				ASSERT(row[0] != NULL);
				CItem* p = new CItem;
				p->iItemId = atol(row[0]);
				p->iCustomerId = (row[1] != NULL ? atol(row[1]) : -1);
				p->iBidAmount = (row[2] != NULL ? atol(row[2]) : -1);
				p->iValue = (row[3] != NULL ? atol(row[3]) : -1);
				p->sCategory = (row[4] != NULL ? row[4] : "");
				p->sDescription = (row[5] != NULL ? row[5] : "");
				paItem.Add(p);
			}
			mysql_free_result(result);
		}
		else {
			AfxMessageBox("FillItemArray: no rows selected");
		}
		int i;
		for (i = 0; i < paItem.GetSize(); i++){
			_COV;
			// TODO: do escaping as necessary
			CItem* pi = paItem[i];
			strcpy(buf, pi->sDescription);
			int len = strlen(buf);
			if (len > 0 && buf[len-1] == '\r'){
				_COV;
				buf[--len] = 0;
				pi->sDescription = buf;
				UpdateItemAllFields(i);
			}
		}
	}
	else {
		AfxMessageBox("FillItemArray: select failed");
	}
#endif
}
void UpdateItem(int ii){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("UpdateItem: MySQL not open");
		return;
	}
	if (ii < 0 || ii >= paItem.GetSize()){
		_COV;
		return;
	}
	CItem* p = paItem[ii];
	TCHAR buf[1024];
	sprintf(buf
		, "update Item set CustomerId = %d, Bid = %d where ItemId = %d"
		, p->iCustomerId
		, p->iBidAmount
		, p->iItemId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("UpdateItem: update failed");
	}
#endif
}
void UpdateItemAllFields(int ii){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("UpdateItemAllFields: MySQL not open");
		return;
	}
	if (ii < 0 || ii >= paItem.GetSize()){
		_COV;
		return;
	}
	CItem* p = paItem[ii];
	TCHAR buf[1024];
	sprintf(buf
		, "update Item set CustomerId = %d, Bid = %d, Value = %d, Category = \"%s\", Description = \"%s\"  where ItemId = %d"
		, p->iCustomerId
		, p->iBidAmount
		, p->iValue
		, p->sCategory
		, p->sDescription
		, p->iItemId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		//AfxMessageBox("UpdateItemAllFields: update failed");
	}
#endif
}
void UpdateCust(int ic){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("UpdateCust: MySQL not open");
		return;
	}
	if (ic < 0 || ic >= paCust.GetSize()){
		_COV;
		return;
	}
	_COV;
	CCustomer* p = paCust[ic];
	TCHAR buf[1024];
	sprintf(buf
		, "update Customer set AmtPaid = %d, ExtraCharge = %d, PayMethod = \"%s\" where CustomerId = %d"
		, p->iAmtPaid
		, p->iExtraCharge
		, p->sPayMethod
		, p->iCustomerId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("UpdateCust: update failed");
	}
#endif
}
void UpdateCustAllFields(int ic){
#ifdef USE_MYSQL
	TCHAR buf[1024];
	if (!bMySqlOpen){
//		AfxMessageBox("UpdateCustAllFields: MySQL not open");
		return;
	}
	if (ic < 0 || ic >= paCust.GetSize()){
		_COV;
		return;
	}
	_COV;
	CCustomer* p = paCust[ic];
	sprintf(buf
		, "update Customer set AmtPaid = %d, ExtraCharge = %d, PayMethod = \"%s\", Description = \"%s\" where CustomerId = %d"
		, p->iAmtPaid
		, p->iExtraCharge
		, p->sPayMethod
		, p->sDescription
		, p->iCustomerId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("UpdateCustAllFields: update failed");
	}
#endif
}
void InsertItem(int ii){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("InsertItem: MySQL not open");
		return;
	}
	if (ii < 0 || ii >= paItem.GetSize()){
		_COV;
		return;
	}
	CItem* p = paItem[ii];
	TCHAR buf[1024];
	sprintf(buf
		, "insert into Item (ItemId, Value, Category, Description) values (%d, %d, \"%s\", \"%s\")"
		, p->iItemId
		, p->iValue
		, p->sCategory
		, p->sDescription
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (!bQueryOK){
		AfxMessageBox("InsertItem: insert failed");
	}
#endif
}
void DeleteItem(int ii){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("DeleteItem: MySQL not open");
		return;
	}
	if (ii < 0 || ii >= paItem.GetSize()){
		_COV;
		return;
	}
	CItem* p = paItem[ii];
	TCHAR buf[1024];
	sprintf(buf
		, "delete from Item where ItemId = %d"
		, p->iItemId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (bQueryOK){
		_COV;
		paItem.RemoveAt(ii);
		delete p;
	}
	else {
		AfxMessageBox("DeleteItem: delete failed");
	}
#endif
}
void InsertCust(int ic){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("InsertCust: MySQL not open");
		return;
	}
	if (ic < 0 || ic >= paCust.GetSize()){
		_COV;
		return;
	}
	CCustomer* p = paCust[ic];
	TCHAR buf[1024];
	sprintf(buf
		, "insert into Customer (CustomerId, AmtPaid, ExtraCharge, PayMethod, Description) values (%d, 0, 0, \"\", \"%s\")"
		, p->iCustomerId
		, p->sDescription
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (bQueryOK){
		_COV;
	}
	else {
		AfxMessageBox("InsertCust: insert failed");
	}
#endif
}
void DeleteCust(int ic){
#ifdef USE_MYSQL
	if (!bMySqlOpen){
//		AfxMessageBox("DeleteCust: MySQL not open");
		return;
	}
	if (ic < 0 || ic >= paCust.GetSize()){
		_COV;
		return;
	}
	CCustomer* p = paCust[ic];
	TCHAR buf[1024];
	sprintf(buf
		, "delete from Customer where CustomerId = %d"
		, p->iCustomerId
		);
	BOOL bQueryOK = (0 == mysql_query(&mySql, buf));
	if (bQueryOK){
		_COV;
		paCust.RemoveAt(ic);
		delete p;
	}
	else {
		AfxMessageBox("DeleteCust: delete failed");
	}
#endif
}
void MergeSortItemsByCust(int i, int n){
	if (n >= 2){
		_COV;
		int na = n / 2, nb = n - na;
		int ia0 = i, ib0 = i + na;
		MergeSortItemsByCust(ia0, na);
		MergeSortItemsByCust(ib0, nb);
		int* iaTemp = new int[n];
		int k = 0;
		int ia = 0, ib = 0;
		while(ia < na && ib < nb){
			_COV;
			int iia = ithItem[ia0+ia];
			int iib = ithItem[ib0+ib];
			if (paItem[iia]->iCustomerId <= paItem[iib]->iCustomerId){
				_COV;
				iaTemp[k++] = iia; ia++;
			}
			else {
				_COV;
				iaTemp[k++] = iib; ib++;
			}
		}
		while (ia < na){_COV; iaTemp[k++] = ithItem[ia0+ia]; ia++;}
		while (ib < nb){_COV; iaTemp[k++] = ithItem[ib0+ib]; ib++;}
		for (k = 0; k < n; k++){_COV; ithItem[i+k] = iaTemp[k];}
		delete iaTemp;
	}
	else {
		_COV;
	}
}
void SortItemsByCust(){
	_COV;
	int ii, ni = paItem.GetSize();
	ithItem.SetSize(ni);
	for (ii = 0; ii < ni; ii++){_COV; ithItem[ii] = ii;}
	MergeSortItemsByCust(0, ni);
	// cound the number of customers who have items
	int iCustomerId = -1;
	int nCust = 0;
	// for each item in customer order
	for (ii = 0; ii < ni; ii++){
		_COV;
		CItem* pi = paItem[ithItem[ii]];
		ASSERT(iCustomerId <= pi->iCustomerId);
		// if this is a customer we haven't seen, count him
		if (iCustomerId < pi->iCustomerId){
			_COV;
			iCustomerId = pi->iCustomerId;
			nCust++;
		}
		else {
			_COV;
		}
	}
	//ASSERT(nCust <= paCust.GetSize() + 1);
}

CItem dummyItem;
CCustomer dummyCustomer(0, "");

int iLastTimerCount = -1;

int iCurrentCustomer;

void CAucUIDlg::deContentsAdmin(){
	_COV;
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Host:");
		if (deEdit(140, 20, &sHost)){
			_COV;
			bGoodConnection = FALSE;
			iLastTimerCount--;
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "User:");
		if (deEdit(140, 20, &sUser)){
			_COV;
			bGoodConnection = FALSE;
			iLastTimerCount--;
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Passwd:");
		if (deEdit(140, 20, &sPassword)){
			_COV;
			bGoodConnection = FALSE;
			iLastTimerCount--;
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "DB:");
		if (deEdit(140, 20, &sDB)){
			_COV;
			bGoodConnection = FALSE;
			iLastTimerCount--;
			DD_THROW;
		}
		CString sTemp;
		if (mode != DD_ERASE){
			_COV;
			sTemp.Format(" (%d items, %d customers)", paItem.GetSize(), paCust.GetSize());
		}
		deStatic(160, 20, (LPCTSTR)sTemp);
	deEndHorizontal(20);
	IF(!bGoodConnection)
		deStartHorizontal();
			P(gx += 20);
			P(gx += 60);
			if (deButton(60, 20, "Connect:")){
				{
					CWaitCursor wait;   // display wait cursor
					CLEAR(paItem);
					CLEAR(paCust);
					CLEAR(paCustItem);
					OpenMySQL();
					CloseMySQL();
				}
				DD_THROW;
			}
		deEndHorizontal(20);
	END
	IF(bGoodConnection)
		P(gy += 20);
	END
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Banner:");
		if (deEdit(Width()-gx, 20, &sBanner)){
			_COV;
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Directory:");
		if (deEdit(140, 20, &sDirectory)){
			_COV;
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Print:");
		if (deEdit(Width()-gx, 20, &sPrintCommand)){
			_COV;
			DD_THROW;
		}
//		deStatic(200, 20, "Example: //192.168.1.100/LaserJet");
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Cust File:");
		if (deEdit(140, 20, &sCustFileName)){
			_COV;
			DD_THROW;
		}
		if (deButton(80, 20, "Save to File")){
			_COV;
			OpenMySQL();
			ExportCustFile(sCustFileName);
			CloseMySQL();
			DD_THROW;
		}
		P(gx += 30);
		if (deButton(80, 20, "Load from File")){
			_COV;
			CString sTemp; sTemp.Format("Are you sure you want to load customer data from '%s' ?", sCustFileName);
			if (AfxMessageBox(sTemp, MB_YESNO)==IDYES){
				_COV;
				OpenMySQL();
				ImportCustFile(sCustFileName);
				CloseMySQL();
			}
			else {
				_COV;
			}
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Item File:");
		if (deEdit(140, 20, &sItemFileName)){
			_COV;
			DD_THROW;
		}
		if (deButton(80, 20, "Save to File")){
			_COV;
			OpenMySQL();
			ExportItemFile(sItemFileName);
			CloseMySQL();
			DD_THROW;
		}
		P(gx += 30);
		if (deButton(80, 20, "Load from File")){
			_COV;
			CString sTemp; sTemp.Format("Are you sure you want to load item data from '%s' ?", sItemFileName);
			if (AfxMessageBox(sTemp, MB_YESNO)==IDYES){
				_COV;
				OpenMySQL();
				ImportItemFile(sItemFileName);
				CloseMySQL();
			}
			else {
				_COV;
			}
			DD_THROW;
		}
	deEndHorizontal(20);
}
void CAucUIDlg::deContentsStatus(){
	P(gy += 20);
	// dollars collected / bid
	// users checked-out / total
	// print who not checked out, with amount owed
	int nTotalReceived = 0;
	int nTotalBid = 0;
	TCHAR buf[1024];
	if (mode != DD_ERASE){
		_COV;
		// dollars collected / bid
		// get dollars collected
		int nc = paCust.GetSize(), ni = paItem.GetSize();
		int ic, ii;
		for (ic = 0; ic < nc; ic++) nTotalReceived += paCust[ic]->iAmtPaid;
		for (ii = 0; ii < ni; ii++) nTotalBid += paItem[ii]->iBidAmount;
		for (ic = 0; ic < nc; ic++) nTotalBid += paCust[ic]->iExtraCharge;
	}
	P(sprintf(buf, "    $ bid %5d, collected %5d, remaining %5d", nTotalBid, nTotalReceived, nTotalBid - nTotalReceived));
	deStatic(Width()-gx, 20, buf);
	int nCustTotal = 0;
	int nCustBidding = 0;
	int nCustPaid = 0;
	if (mode != DD_ERASE){
		_COV;
		int ii = 0, ic = 0, ni = paItem.GetSize(), nc = paCust.GetSize();
		// do a merge of cust with items sorted by cust
		while (ii < ni && ic < nc){
			_COV;
			CItem* pi = paItem[ithItem[ii]];
			CCustomer* pc = paCust[ic];
			if (0);
			// if customer comes before item, this customer has no items (but may have extra charge)
			else if (pc->iCustomerId < pi->iCustomerId){
				_COV;
				nCustTotal++;
				if (pc->iExtraCharge > 0){
					_COV;
					nCustBidding++;
					if (pc->iAmtPaid > 0){_COV; nCustPaid++;}
				}
				else {
					_COV;
				}
				ic++;
			}
			// if item comes before customer, that's ok, this is not the first item for the prior customer
			else if (pi->iCustomerId < pc->iCustomerId){
				_COV;
				ii++;
			}
			// if customer matches item, this is the first item for that customer
			else {
				_COV;
				nCustTotal++;
				nCustBidding++;
				if (pc->iAmtPaid > 0){_COV; nCustPaid++;}
				ic++;
				ii++;
			}
		}
		while (ic < nc){
			_COV;
			nCustTotal++;
			ic++;
		}
		while (ii < ni){
			_COV;
			ii++;
		}
		ASSERT(nCustTotal == nc);
		ASSERT(nCustBidding <= nc);
		ASSERT(nCustPaid <= nCustBidding);
	}
	P(sprintf(buf, "    # cust %5d, bidding %5d, paid %5d, remaining %5d"
		, nCustTotal
		, nCustBidding
		, nCustPaid
		, nCustBidding - nCustPaid
		));
	deStatic(Width()-gx, 20, buf);
}

long iCurrentItem;
long iCurrentItemId;
CString sItemSearch = "";

void CAucUIDlg::deContentsItemFind(){
	_COV;
	int nItem = paItem.GetSize();
	CItem* pi = &dummyItem;
	deStartHorizontal();
		P(gx += 250);
		deStatic(50, 20, "Search:");
		if (deEdit(80, 20, &sItemSearch)){
			_COV;
			DD_THROW;
		}
		if (deButton(20, 20, "<")){
			_COV;
			int i, ni = paItem.GetSize(), ii = iCurrentItem - 1;
			for (i = 0; i < ni; i++, ii--){
				_COV;
				if (ii < 0){_COV; ii += ni;}
				CItem* pi = paItem[ii];
				int iFound = MyFindNoCase(pi->sDescription, sItemSearch);
				if (iFound >= 0){
					_COV;
					iCurrentItem = ii;
					break;
				}
				else {
					_COV;
				}
			}
			DD_THROW;
		}
		if (deButton(20, 20, ">")){
			_COV;
			int i, ni = paItem.GetSize(), ii = iCurrentItem + 1;
			for (i = 0; i < ni; i++, ii++){
				_COV;
				if (ii >= ni){_COV; ii -= ni;}
				CItem* pc = paItem[ii];
				int iFound = MyFindNoCase(pc->sDescription, sItemSearch);
				if (iFound >= 0){
					_COV;
					iCurrentItem = ii;
					break;
				}
				else {
					_COV;
				}
			}
			DD_THROW;
		}
	deEndHorizontal(20);
}

void CAucUIDlg::deContentsItem(){
	_COV;
	int nItem = paItem.GetSize();
	CItem* pi = &dummyItem;

	deContentsItemFind();

	// for selected item, edit bidder, amount, & show description
	deStartHorizontal();
		P(gx += 20);
		deStatic(www, 20, "Item");
		deStatic(www, 20, "Cust");
		deStatic(www, 20, "Amt");
		deStatic(300, 20, "Description");
	deEndHorizontal(20);

	IF(iCurrentItem >= 0 && iCurrentItem < nItem)
		_COV;
		deStartHorizontal();
			if (mode != DD_ERASE){
				_COV;
				pi = paItem[iCurrentItem];
				iCurrentItemId = pi->iItemId;
			}
			P(gx += 20);
			if (deEdit(www, 20, &iCurrentItemId)){
				_COV;
				int ii = FindItemIndexById(iCurrentItemId);
				if (ii >= 0){
					_COV;
					iCurrentItem = ii;
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			if (deEdit(www, 20, &(pi->iCustomerId))){
				_COV;
				OpenMySQL();
				UpdateItem(iCurrentItem);
				CloseMySQL();
				DD_THROW;
			}
			if (deEdit(www, 20, &(pi->iBidAmount))){
				_COV;
				OpenMySQL();
				UpdateItem(iCurrentItem);
				CloseMySQL();
				DD_THROW;
			}

			deStatic(300, 20, pi->sDescription);

		deEndHorizontal(20);

	END

}

void PrintCustomerItemArray(){
	if (iCurrentCustomer < 0 || iCurrentCustomer >= paCust.GetSize()){
		_COV;
		return;
	}
	CString sFileName;
	sFileName.Format("%s\\invoice.txt", sDirectory);
	FILE* f = fopen(sFileName, "w");
	if (f == NULL){
		CString sMsg;
		sMsg.Format("PrintCustomerItemArray: unable to open file '%s'", sFileName);
		AfxMessageBox(sMsg);
		return;
	}
	_COV;
	CCustomer* pc = paCust[iCurrentCustomer];
	fprintf(f, "       %s\n", sBanner);
	fprintf(f, "\n");
	fprintf(f, "                              INVOICE\n");
	fprintf(f, "\n");
	fprintf(f, "\n");
	fprintf(f, "            [%d] %s\n", pc->iCustomerId, pc->sDescription);
	fprintf(f, "\n");
	fprintf(f, "\n");
	int iBidSum = 0;
	int iValueSum = 0;
	int i;
		fprintf(f, "           %5s    %5s    %5s\n\n"
			, "  Bid"
			, "Value"
			, " Item"
			);
	for (i = 0; i < paCustItem.GetSize(); i++){
		_COV;
		CItem* pi = paCustItem[i];
		iBidSum += pi->iBidAmount;
		iValueSum += pi->iValue;
		fprintf(f, "          $%5d    %5d    %5d   %s\n\n"
			, pi->iBidAmount
			, pi->iValue
			, pi->iItemId
			, pi->sDescription
			);
	}
	fprintf(f, "\n");
	fprintf(f, "          $%5d    %5d            Total items bid, value\n", iBidSum, iValueSum);
	fprintf(f, "\n");
	fprintf(f, "          $%5d    Extra contribution\n", pc->iExtraCharge);
	fprintf(f, "\n");
	fprintf(f, "\n");
	fprintf(f, "          $%5d    Total Bid + Extra\n", iBidSum + pc->iExtraCharge);
	fprintf(f, "        - $%5d    Paid %s\n", pc->iAmtPaid, pc->sPayMethod);
	fprintf(f, "\n");
	fprintf(f, "        = $%5d    OWED\n", iBidSum + pc->iExtraCharge - pc->iAmtPaid);
	fprintf(f, "\f");
	fclose(f);

	STARTUPINFO sti;
	memset(&sti, 0, sizeof(sti));
	sti.cb = sizeof(sti);
	sti.dwFillAttribute |= 
		sti.dwFlags |= STARTF_USESHOWWINDOW
		;
	sti.wShowWindow = (SW_SHOWMINNOACTIVE);

	static PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	CString sCmmd;
	sCmmd.Format("%s %s\\invoice.txt\n", sPrintCommand, sDirectory);

	if (CreateProcess(
		NULL
		, (LPTSTR)(LPCTSTR)sCmmd
		, NULL
		, NULL
		, TRUE		// bInheritHandles 
		, HIGH_PRIORITY_CLASS
		, NULL		// lpEnvironment
		, NULL		// lpCurrentDirectory
		, &sti
		, &pi
		)
	){
		_COV;
		CWaitCursor wait;   // display wait cursor
		if (WaitForSingleObject(pi.hProcess, 1000000)==WAIT_TIMEOUT) {
//			goto error;
		}
	}
	else {
		AfxMessageBox("PrintCustomerItemArray: print process failed");
	}
}

long iCurrentCustomerId;
CString sCustSearch = "";

void CAucUIDlg::deContentsCustomerFind(){
	_COV;
	deStartHorizontal();
		P(gx += 250);
		deStatic(50, 20, "Search:");
		if (deEdit(80, 20, &sCustSearch)){
			_COV;
			DD_THROW;
		}
		if (deButton(20, 20, "<")){
			_COV;
			int i, nc = paCust.GetSize(), ic = iCurrentCustomer - 1;
			for (i = 0; i < nc; i++, ic--){
				_COV;
				if (ic < 0){_COV; ic += nc;}
				CCustomer* pc = paCust[ic];
				int iFound = MyFindNoCase(pc->sDescription, sCustSearch);
				if (iFound >= 0){
					_COV;
					iCurrentCustomer = ic;
					FillCustItemArray(iCurrentCustomer);
					break;
				}
				else {
					_COV;
				}
			}
			if (i < 0){
				//TCOV;
			}
			FillCustItemArray(iCurrentCustomer);
			DD_THROW;
		}
		if (deButton(20, 20, ">")){
			_COV;
			int i, nc = paCust.GetSize(), ic = iCurrentCustomer + 1;
			for (i = 0; i < nc; i++, ic++){
				_COV;
				if (ic >= nc) ic -= nc;
				CCustomer* pc = paCust[ic];
				int iFound = MyFindNoCase(pc->sDescription, sCustSearch);
				if (iFound >= 0){
					_COV;
					iCurrentCustomer = ic;
					FillCustItemArray(iCurrentCustomer);
					break;
				}
				else {
					_COV;
				}
			}
			if (i >= nc){
				//TCOV;
			}
			FillCustItemArray(iCurrentCustomer);
			DD_THROW;
		}
	deEndHorizontal(20);
}

BOOL CAucUIDlg::GetCustomerNumbers(int iCust, int& iBid, int& iExtra, int& iPaid, int& iOwe){
	_COV;
	if (iCust < 0 || iCust >= paCust.GetSize()){
		return FALSE;
	}
	CCustomer* pc = paCust[iCust];
	iExtra = pc->iExtraCharge;
	iPaid = pc->iAmtPaid;
	int ii, ni = paItem.GetSize();
	iBid = 0;
	for (ii = 0; ii < ni; ii++){
		CItem* pi = paItem[ii];
		if (pi->iCustomerId != pc->iCustomerId) continue;
		iBid += pi->iBidAmount;
	}
	iOwe = iBid + iExtra - iPaid;
	return TRUE;
}

void CAucUIDlg::deContentsCustomer(){
	int i;
	_COV;
	if (mode == DD_SHOW){
		_COV;
		FillCustItemArray(iCurrentCustomer);
	}
	deContentsCustomerFind();

	int nCust = paCust.GetSize();
	CCustomer* pc = &dummyCustomer;
	TCHAR buf[1000];
	IF(iCurrentCustomer >= 0 && iCurrentCustomer < nCust)
		_COV;
		deStartHorizontal();
			if (mode != DD_ERASE){
				_COV;
				pc = paCust[iCurrentCustomer];
				iCurrentCustomerId = pc->iCustomerId;
			}
			P(gx += 20);
			if (deEdit(www, 20, &iCurrentCustomerId)){
				_COV;
				int ic = FindCustIndexById(iCurrentCustomerId);
				if (ic >= 0){
					_COV;
					iCurrentCustomer = ic;
					FillCustItemArray(iCurrentCustomer);
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			deStatic(300, 20, pc->sDescription);
			if (deButton(20, 20, "<")){
				_COV;
				iCurrentCustomer = (iCurrentCustomer+nCust - 1) % nCust;
				FillCustItemArray(iCurrentCustomer);
				DD_THROW;
			}
			if (deButton(20, 20, ">")){
				_COV;
				iCurrentCustomer = (iCurrentCustomer+nCust + 1) % nCust;
				FillCustItemArray(iCurrentCustomer);
				DD_THROW;
			}
		deEndHorizontal(20);
	END

	deStartHorizontal();
		P(gx += 320);
		deStatic(40, 20, "OWE");
		if (deButton(20, 20, "<")){
			_COV;
			int i, ic = iCurrentCustomer, nc = paCust.GetSize();
			if (nc > 0){
				ic--;
				for(i = 0; i < nc-1; i++, ic--){
					if (ic < 0) ic += nc;
					if (ic >= nc) ic -= nc;
					int iBid = 0, iExtra = 0, iPaid = 0, iOwe = 0;
					if (GetCustomerNumbers(ic, iBid, iExtra, iPaid, iOwe) && iOwe > 0){
						_COV;
						iCurrentCustomer = ic;
						FillCustItemArray(iCurrentCustomer);
						break;
					}
				}
			}
			DD_THROW;
		}
		if (deButton(20, 20, ">")){
			_COV;
			int i, ic = iCurrentCustomer, nc = paCust.GetSize();
			if (nc > 0){
				ic++;
				for(i = 0; i < nc-1; i++, ic++){
					if (ic < 0) ic += nc;
					if (ic >= nc) ic -= nc;
					int iBid = 0, iExtra = 0, iPaid = 0, iOwe = 0;
					if (GetCustomerNumbers(ic, iBid, iExtra, iPaid, iOwe) && iOwe > 0){
						_COV;
						iCurrentCustomer = ic;
						FillCustItemArray(iCurrentCustomer);
						break;
					}
				}
			}
			DD_THROW;
		}
	deEndHorizontal(20);
	// for selected customer, show items
	int iBidSum = 0;
	FOR(i = 0, i < paCustItem.GetSize(), i++)
		_COV;
		CItem* pi = &dummyItem;
		if (mode != DD_ERASE){
			_COV;
			pi = paCustItem[i];
			iBidSum += pi->iBidAmount;
		}
		deStartHorizontal();
			P(gx += 20);
			TCHAR buf[100];
			P(sprintf(buf, "%5d", pi->iItemId));
			deStatic(40, 20, buf);
			P(sprintf(buf, "$%5d", pi->iBidAmount));
			deStatic(40, 20, buf);
//			deEdit(www, 20, &(pi->iBidAmount));
			deStatic(300, 20, pi->sDescription);
			if (deButton(50, 20, "-> Item")){
				_COV;
				int ii = FindItemIndexById(pi->iItemId);
				if (ii >= 0){
					_COV;
					iCurrentItem = ii;
					iWhichPage = PAGE_ITEM;
				}
				else {
					//TCOV;
				}
				DD_THROW;
			}
		deEndHorizontal(20);
	END
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Extra $");
		if (deEdit(www, 20, &(pc->iExtraCharge))){
			_COV;
			OpenMySQL();
			UpdateCust(iCurrentCustomer);
			CloseMySQL();
			DD_THROW;
		}
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Total $");
		if (mode != DD_ERASE){
			_COV;
			sprintf(buf, "%4d", iBidSum + pc->iExtraCharge);
		}
		deStatic(www, 20, buf);
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Paid  $");
		if (deEdit(www, 20, &(pc->iAmtPaid))){
			_COV;
			OpenMySQL();
			UpdateCust(iCurrentCustomer);
			CloseMySQL();
			DD_THROW;
		}
		P(gx += 20);
		if (mode != DD_ERASE){
			_COV;
			sprintf(buf, "OWE %4d", iBidSum + pc->iExtraCharge - pc->iAmtPaid);
		}
		deStatic(www * 2, 20, buf);
	deEndHorizontal(20);

	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, pc->sPayMethod);
	deEndHorizontal(20);

//	if (deButton(160, 20, "Mark Paid and Print")){
//		_COV;
//		pc->iAmtPaid = iBidSum + pc->iExtraCharge;
//		OpenMySQL();
//		UpdateCust(iCurrentCustomer);
//		CloseMySQL();
//		PrintCustomerItemArray();
//		SetFocus();
//		DD_THROW;
//	}
	BOOL bUpdateThisCust = FALSE;
	BOOL bPrint = FALSE;

	deStartHorizontal();
		P(gx +=   0);
		if (deButton(100, 20, "Paid Cash")){
			_COV;
			pc->sPayMethod = "Cash";
			bUpdateThisCust = TRUE;
			bPrint = TRUE;
		}
	deEndHorizontal(20);

	deStartHorizontal();
		P(gx += 100);
		if (deButton(100, 20, "Paid Check")){
			_COV;
			pc->sPayMethod = "Check";
			bUpdateThisCust = TRUE;
			bPrint = TRUE;
		}
	deEndHorizontal(20);

	deStartHorizontal();
		P(gx += 200);
		if (deButton(100, 20, "Paid Credit")){
			_COV;
			pc->sPayMethod = "Credit";
			bUpdateThisCust = TRUE;
			bPrint = TRUE;
		}
	deEndHorizontal(20);

	if (bUpdateThisCust){
		pc->iAmtPaid = iBidSum + pc->iExtraCharge;
		OpenMySQL();
		UpdateCust(iCurrentCustomer);
		CloseMySQL();

		if (bPrint){
			_COV;
			PrintCustomerItemArray();
			SetFocus();
		}

		DD_THROW;
	}

//	if (deButton(80, 20, "Print")){
//		_COV;
//		PrintCustomerItemArray();
//		SetFocus();
//		DD_THROW;
//	}
}

const int nTempCustId = 10;
long iTempCustId[nTempCustId];

void CAucUIDlg::deContentsAddExtraOneRow(long& iCustomerId){
	_COV;
	TCHAR buf[80];
	int ic = -1;
	deStartHorizontal();
		P(gx += 20);
		if (deEdit(40, 20, &iCustomerId)){
			_COV;
			DD_THROW;
		}
		if (mode != DD_ERASE){
			_COV;
			ic = FindCustIndexById(iCustomerId);
			buf[0] = 0;
			if (ic >= 0){
				_COV;
				sprintf(buf, "$%5d", paCust[ic]->iExtraCharge);
			}
			else {
				_COV;
			}
		}
		if (deButton(60, 20, "Add")){
			_COV;
			if (ic >= 0){
				_COV;
				paCust[ic]->iExtraCharge += iExtraChargeToAdd;
				OpenMySQL();
				UpdateCust(ic);
				CloseMySQL();
			}
			else {
				_COV;
			}
		}
		deStatic(40, 20, buf);
		if (deButton(60, 20, "Subtract")){
			_COV;
			if (ic >= 0){
				_COV;
				paCust[ic]->iExtraCharge -= iExtraChargeToAdd;
				OpenMySQL();
				UpdateCust(ic);
				CloseMySQL();
			}
			else {
				_COV;
			}
		}
	deEndHorizontal(20);
}
void CAucUIDlg::deContentsAddExtra(){
	int i;
	// have a field saying how much will be added
	// when each one is entered, modify that customer's record
	deStartHorizontal();
		P(gx += 20);
		deStatic(160, 20, "Extra contribution to add: $");
		if (deEdit(40, 20, &(iExtraChargeToAdd))){
			_COV;
			DD_THROW;
		}
	deEndHorizontal(20);
	// have a series of fields to enter customerid
	for (i = 0; i < nTempCustId; i++){
		_COV;
		deContentsAddExtraOneRow(iTempCustId[i]);
	}
	if (nTempCustId == 0){
		//TCOV;
	}
	if (deButton(80, 20, "Add To All")){
		_COV;
		OpenMySQL();
		for (i = 0; i < nTempCustId; i++){
			_COV;
			int ic = FindCustIndexById(iTempCustId[i]);
			if (ic >= 0){
				_COV;
				CCustomer* pc = paCust[ic];
				pc->iExtraCharge += iExtraChargeToAdd;
				UpdateCust(ic);
			}
			else {
				_COV;
			}
		}
		CloseMySQL();
		DD_THROW;
	}
	if (deButton(80, 20, "Clear")){
		_COV;
		for (i = 0; i < nTempCustId; i++){
			_COV;
			iTempCustId[i] = 0;
		}
		DD_THROW;
	}
}
CItem itmToEdit;
void CopyCurrentItemIntoItmToEdit(){
	if (iCurrentItem >= 0 && iCurrentItem < paItem.GetSize()){
		paItem[iCurrentItem]->CopyTo(&itmToEdit);
	}
}
int iCurrentItemGap;	// index of item following the gap, range 0..ni
CItem itmTemp;
void CAucUIDlg::deContentsItemDetail(){
	_COV;
	int nItem = paItem.GetSize();
	CItem* pi = &dummyItem;
	TCHAR buf[1024];

	deContentsItemFind();
	// edit found item
	IF(iCurrentItem >= 0 && iCurrentItem < nItem)
		_COV;
		deStartHorizontal();
			P(gx += 20);
			deStatic(www, 20, "Item");
			deStatic(www, 20, "Cust");
			deStatic(www, 20, "Amt");
			deStatic(www, 20, "Value");
			deStatic(www, 20, "Cat.");
			deStatic(300, 20, "Description");
		deEndHorizontal(20);
		deStartHorizontal();
			if (mode != DD_ERASE){
				_COV;
				pi = paItem[iCurrentItem];
				iCurrentItemId = pi->iItemId;
			}
			BOOL bCurrentItemChanged = deChanged(iCurrentItem);
			if (mode == DD_SHOW || bCurrentItemChanged){
				CopyCurrentItemIntoItmToEdit();
			}
			if (deButton(20, 20, "X")){
				_COV;
				if (AfxMessageBox("Really delete this item?", MB_YESNO)==IDYES){
					_COV;
					OpenMySQL();
					DeleteItem(iCurrentItem);
					CloseMySQL();
					if (iCurrentItem > 0 && iCurrentItem >= paItem.GetSize()){
						_COV;
						iCurrentItem--;
					}
					if (iCurrentItem >= 0 && iCurrentItem < paItem.GetSize()){
						paItem[iCurrentItem]->CopyTo(&itmToEdit);
					}
					CopyCurrentItemIntoItmToEdit();
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			if (deEdit(www, 20, &iCurrentItemId)){
				_COV;
				int ii = FindItemIndexById(iCurrentItemId);
				if (ii >= 0){
					_COV;
					iCurrentItem = ii;
					CopyCurrentItemIntoItmToEdit();
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			if (deEdit(www, 20, &(itmToEdit.iCustomerId))){
				_COV;
				DD_THROW;
			}
			if (deEdit(www, 20, &(itmToEdit.iBidAmount))){
				_COV;
				DD_THROW;
			}
			if (deEdit(www, 20, &(itmToEdit.iValue))){
				_COV;
				DD_THROW;
			}
			if (deEdit(www, 20, &(itmToEdit.sCategory))){
				_COV;
				DD_THROW;
			}
			if (deEdit(200, 20, &(itmToEdit.sDescription))){
				_COV;
				DD_THROW;
			}
			if (deButton(www, 20, "Update")){
				_COV;
				itmToEdit.CopyTo(paItem[iCurrentItem]);
				OpenMySQL();
				UpdateItemAllFields(iCurrentItem);
				CloseMySQL();
			}

			if (deButton(20, 20, "<")){
				_COV;
				iCurrentItem = (iCurrentItem+nItem - 1) % nItem;
				CopyCurrentItemIntoItmToEdit();
				DD_THROW;
			}
			if (deButton(20, 20, ">")){
				_COV;
				iCurrentItem = (iCurrentItem+nItem + 1) % nItem;
				CopyCurrentItemIntoItmToEdit();
				DD_THROW;
			}
		deEndHorizontal(20);

		deStartHorizontal();
			P(gx += 20);
			if (deButton(90, 20, "-> Customer")){
				_COV;
				int ic = FindCustIndexById(pi->iCustomerId);
				if (ic >= 0){
					_COV;
					iCurrentCustomer = ic;
					FillCustItemArray(iCurrentCustomer);
					iWhichPage = PAGE_CUSTOMER;
				}
				else {
					_COV;
				}
				DD_THROW;
			}
		deEndHorizontal(20);

	END

	P(gy += 20*3);

	// edit additional item
	BOOL bItemAlreadyExists = FALSE;
	if (mode != DD_ERASE){
		_COV;
		int ii = FindItemIndexById(itmTemp.iItemId);
		if (ii >= 0) bItemAlreadyExists = TRUE;
	}
	// hunt for a gap
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Find Gap: ");
		if (deButton(20, 20, "<")){
			_COV;
			int ii, ni = paItem.GetSize();
			iCurrentItemGap--; if (iCurrentItemGap < 0) iCurrentItemGap = ni;
			for (ii = 0; ii < ni; ii++){
				_COV;
				if (iCurrentItemGap == ni){_COV; break;}
				if (iCurrentItemGap == 0 && paItem[iCurrentItemGap]->iItemId > 1){_COV; break;}
				if (iCurrentItemGap > 0
					&& paItem[iCurrentItemGap-1]->iItemId < paItem[iCurrentItemGap]->iItemId-1
					){_COV; break;}
				iCurrentItemGap--; if (iCurrentItemGap < 0){_COV; iCurrentItemGap = ni;}
			}
			if (iCurrentItemGap == 0){
				_COV;
				itmTemp.iItemId = 1;
			}
			else {
				_COV;
				itmTemp.iItemId = paItem[iCurrentItemGap - 1]->iItemId + 1;
			}
			DD_THROW;
		}
		if (deButton(20, 20, ">")){
			_COV;
			int ii, ni = paItem.GetSize();
			iCurrentItemGap++; if (iCurrentItemGap > ni) iCurrentItemGap = 0;
			for (ii = 0; ii < ni; ii++){
				_COV;
				if (iCurrentItemGap == ni){_COV; break;}
				if (iCurrentItemGap == 0 && paItem[iCurrentItemGap]->iItemId > 1){_COV; break;}
				if (iCurrentItemGap > 0
					&& paItem[iCurrentItemGap-1]->iItemId < paItem[iCurrentItemGap]->iItemId-1
					){_COV; break;}
				iCurrentItemGap++; if (iCurrentItemGap > ni){_COV; iCurrentItemGap = 0;}
			}
			if (iCurrentItemGap == 0){
				_COV;
				itmTemp.iItemId = 1;
			}
			else {
				_COV;
				itmTemp.iItemId = paItem[iCurrentItemGap - 1]->iItemId + 1;
			}
			DD_THROW;
		}
		if (mode != DD_ERASE){
			_COV;
			buf[0] = 0;
			int ni = paItem.GetSize();
			if (ni == 0){
				//TCOV;
				sprintf(buf, "  0  ..");
			}
			else if (iCurrentItemGap == 0){
				_COV;
				sprintf(buf, "  0  .. %d", paItem[iCurrentItemGap]->iItemId);
			}
			else if (iCurrentItemGap >= ni){
				_COV;
				sprintf(buf, "  %d  .."
					, paItem[iCurrentItemGap-1]->iItemId
					);
			}
			else {
				_COV;
				sprintf(buf, "  %d  .. %d"
					, paItem[iCurrentItemGap-1]->iItemId
					, paItem[iCurrentItemGap]->iItemId
					);
			}
		}
		deStatic(80, 20, buf);
	deEndHorizontal(20);
	// for selected item, edit bidder, amount, & show description
	deStartHorizontal();
		P(gx += 20);
		deStatic(www, 20, "Item");
		P(gx += www); //		deStatic(www, 20, "Cust");
		P(gx += www); //		deStatic(www, 20, "Amt");
		deStatic(www, 20, "Value");
		deStatic(www, 20, "Cat.");
		deStatic(300, 20, "Description");
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		if (deEdit(www, 20, &itmTemp.iItemId)){
			_COV;
			DD_THROW;
		}
		P(gx += www);	// placeholder for Cust
		P(gx += www);	// placeholder for Amt
		if (deEdit(www, 20, &itmTemp.iValue)){
			_COV;
			DD_THROW;
		}
		if (deEdit(www, 20, &itmTemp.sCategory)){
			_COV;
			DD_THROW;
		}
		if (deEdit(200, 20, &itmTemp.sDescription)){
			_COV;
			DD_THROW;
		}
		IF(!bItemAlreadyExists)
			_COV;
			if (deButton(www, 20, "Add")){
				_COV;
				itmTemp.iBidAmount = 0;
				itmTemp.iCustomerId = 0;
				// create the new item
				int ii = FindIndexToInsertItemId(itmTemp.iItemId);
				if (ii >= 0){
					_COV;
					CItem* pi = new CItem;
					itmTemp.CopyTo(pi);
					paItem.InsertAt(ii, pi);
					OpenMySQL();
					InsertItem(ii);
					CloseMySQL();
					itmTemp.iItemId++;
//					itmTemp.iValue = 0;
//					itmTemp.sCategory = "";
					itmTemp.sDescription = "";
				}
				else {
					//TCOV;
				}
				DD_THROW;
			}
		END
	deEndHorizontal(20);
}
CCustomer custToEdit;
void CopyCurrentCustIntoCustToEdit(){
	if (iCurrentCustomer >= 0 && iCurrentCustomer < paCust.GetSize()){
		paCust[iCurrentCustomer]->CopyTo(&custToEdit);
	}
}
int iCurrentCustGap;	// index of cust following the gap, range 0..nc
CCustomer custTemp;
void CAucUIDlg::deContentsCustDetail(){
	_COV;
	int nCust = paCust.GetSize();
	CCustomer* pc = &dummyCustomer;
	TCHAR buf[1024];

	deContentsCustomerFind();
	// edit found cust
	IF(iCurrentCustomer >= 0 && iCurrentCustomer < nCust)
		_COV;
		deStartHorizontal();
			P(gx += 20);
			deStatic(www, 20, "Cust");
			deStatic(www, 20, "Extra");
			deStatic(www, 20, "Paid");
			deStatic(www, 20, "PaidBy");
			deStatic(300, 20, "Description");
		deEndHorizontal(20);
		deStartHorizontal();
			if (mode != DD_ERASE){
				_COV;
				pc = paCust[iCurrentCustomer];
				iCurrentCustomerId = pc->iCustomerId;
			}
			BOOL bCurrentCustChanged = deChanged(iCurrentCustomer);
			if (mode == DD_SHOW || bCurrentCustChanged){
				CopyCurrentCustIntoCustToEdit();
			}
			if (deButton(20, 20, "X")){
				_COV;
				if (AfxMessageBox("Really delete this customer?", MB_YESNO)==IDYES){
					_COV;
					OpenMySQL();
					DeleteCust(iCurrentCustomer);
					CloseMySQL();
					if (iCurrentCustomer > 0 && iCurrentCustomer >= paCust.GetSize()){
						_COV;
						iCurrentCustomer--;
					}
					if (iCurrentCustomer >= 0 && iCurrentCustomer < paCust.GetSize()){
						paCust[iCurrentCustomer]->CopyTo(&custToEdit);
					}
					CopyCurrentCustIntoCustToEdit();
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			if (deEdit(www, 20, &iCurrentCustomerId)){
				_COV;
				int ii = FindCustIndexById(iCurrentCustomerId);
				if (ii >= 0){
					_COV;
					iCurrentCustomer = ii;
					CopyCurrentCustIntoCustToEdit();
				}
				else {
					_COV;
				}
				DD_THROW;
			}
			if (deEdit(www, 20, &(custToEdit.iExtraCharge))){
				_COV;
				DD_THROW;
			}
			if (deEdit(www, 20, &(custToEdit.iAmtPaid))){
				_COV;
				DD_THROW;
			}
			if (deEdit(www, 20, &(custToEdit.sPayMethod))){
				_COV;
				DD_THROW;
			}
			if (deEdit(200, 20, &(custToEdit.sDescription))){
				_COV;
				DD_THROW;
			}
			if (deButton(www, 20, "Update")){
				_COV;
				custToEdit.CopyTo(paCust[iCurrentCustomer]);
				OpenMySQL();
				UpdateCustAllFields(iCurrentCustomer);
				CloseMySQL();
			}

			if (deButton(20, 20, "<")){
				_COV;
				iCurrentCustomer = (iCurrentCustomer+nCust - 1) % nCust;
				CopyCurrentCustIntoCustToEdit();
				DD_THROW;
			}
			if (deButton(20, 20, ">")){
				_COV;
				iCurrentCustomer = (iCurrentCustomer+nCust + 1) % nCust;
				CopyCurrentCustIntoCustToEdit();
				DD_THROW;
			}
		deEndHorizontal(20);

		deStartHorizontal();
			P(gx += 20);
			if (deButton(90, 20, "-> Customer")){
				_COV;
				int ic = FindCustIndexById(pc->iCustomerId);
				if (ic >= 0){
					_COV;
					iCurrentCustomer = ic;
					iWhichPage = PAGE_CUSTOMER;
				}
				else {
					_COV;
				}
				DD_THROW;
			}
		deEndHorizontal(20);

	END

	P(gy += 20*3);

	BOOL bCustAlreadyExists = FALSE;
	_COV;
	if (mode != DD_ERASE){
		_COV;
		int ic = FindCustIndexById(custTemp.iCustomerId);
		if (ic >= 0) bCustAlreadyExists = TRUE;
	}
	// hunt for a gap
	deStartHorizontal();
		P(gx += 20);
		deStatic(60, 20, "Find Gap: ");
		if (deButton(20, 20, "<")){
			_COV;
			int ic, nc = paCust.GetSize();
			iCurrentCustGap--; if (iCurrentCustGap < 0){_COV; iCurrentCustGap = nc;}
			for (ic = 0; ic < nc; ic++){
				_COV;
				if (iCurrentCustGap == nc){_COV; break;}
				if (iCurrentCustGap == 0 && paCust[iCurrentCustGap]->iCustomerId > 1){_COV; break;}
				if (iCurrentCustGap > 0
					&& paCust[iCurrentCustGap-1]->iCustomerId < paCust[iCurrentCustGap]->iCustomerId-1
					){_COV; break;}
				iCurrentCustGap--; if (iCurrentCustGap < 0){_COV; iCurrentCustGap = nc;}
			}
			if (iCurrentCustGap == 0){
				_COV;
				custTemp.iCustomerId = 1;
			}
			else {
				_COV;
				custTemp.iCustomerId = paCust[iCurrentCustGap - 1]->iCustomerId + 1;
			}
			DD_THROW;
		}
		if (deButton(20, 20, ">")){
			_COV;
			int ic, nc = paCust.GetSize();
			iCurrentCustGap++; if (iCurrentCustGap > nc){_COV; iCurrentCustGap = 0;}
			for (ic = 0; ic < nc; ic++){
				_COV;
				if (iCurrentCustGap == nc){_COV; break;}
				if (iCurrentCustGap == 0 && paCust[iCurrentCustGap]->iCustomerId > 1){_COV; break;}
				if (iCurrentCustGap > 0
					&& paCust[iCurrentCustGap-1]->iCustomerId < paCust[iCurrentCustGap]->iCustomerId-1
					){_COV; break;}
				iCurrentCustGap++; if (iCurrentCustGap > nc){_COV; iCurrentCustGap = 0;}
			}
			if (iCurrentCustGap == 0){
				_COV;
				custTemp.iCustomerId = 1;
			}
			else {
				_COV;
				custTemp.iCustomerId = paCust[iCurrentCustGap - 1]->iCustomerId + 1;
			}
			DD_THROW;
		}
		if (mode != DD_ERASE){
			_COV;
			buf[0] = 0;
			int nc = paCust.GetSize();
			if (nc == 0){
				_COV;
				sprintf(buf, "  0  ..");
			}
			else if (iCurrentCustGap == 0){
				_COV;
				sprintf(buf, "  0  .. %d", paCust[iCurrentCustGap]->iCustomerId);
			}
			else if (iCurrentCustGap >= nc){
				_COV;
				sprintf(buf, "  %d  .."
					, paCust[iCurrentCustGap-1]->iCustomerId
					);
			}
			else {
				_COV;
				sprintf(buf, "  %d  .. %d"
					, paCust[iCurrentCustGap-1]->iCustomerId
					, paCust[iCurrentCustGap]->iCustomerId
					);
			}
		}
		deStatic(80, 20, buf);
	deEndHorizontal(20);
	// for selected item, edit bidder, amount, & show description
	deStartHorizontal();
		P(gx += 20);
		deStatic(www, 20, "Cust");
//		deStatic(www, 20, "Cust");
//		deStatic(www, 20, "Amt");
		deStatic(300, 20, "Description");
	deEndHorizontal(20);
	deStartHorizontal();
		P(gx += 20);
		if (deEdit(www, 20, &custTemp.iCustomerId)){
			_COV;
			DD_THROW;
		}
		if (deEdit(300, 20, &custTemp.sDescription)){
			_COV;
			DD_THROW;
		}
		IF(!bCustAlreadyExists)
			_COV;
			if (deButton(60, 20, "Add")){
				_COV;
				custTemp.iAmtPaid = 0;
				custTemp.iExtraCharge = 0;
				// create the new item
				int ic = FindIndexToInsertCustId(custTemp.iCustomerId);
				if (ic >= 0){
					_COV;
					CCustomer* pc = new CCustomer;
					custTemp.CopyTo(pc);
					paCust.InsertAt(ic, pc);
					OpenMySQL();
					InsertCust(ic);
					CloseMySQL();
					custTemp.iCustomerId++;
					custTemp.sPayMethod = "";
					custTemp.sDescription = "";
				}
				else {
					_COV;
				}
				DD_THROW;
			}
		END
	deEndHorizontal(20);
}

//-----------------------------------------------------------
CStringArray saTab1;

void CAucUIDlg::deContents(){
	_COV;
	if (mode == DD_SHOW){
		_COV;
		SetTimer(0, 2000, NULL);
		saTab1.Add("Admin");
		saTab1.Add("Status");
		saTab1.Add("Item");
		saTab1.Add("Cust");
		saTab1.Add("Extra");
		saTab1.Add("Item Detail");
		saTab1.Add("Cust Detail");
	}
	if (mode == DD_UPDATE){
		_COV;
		if (iLastTimerCount < iTimerCount){
			_COV;
			if (bGoodConnection){
				// reload the database
				OpenMySQL();
				FillItemArray();
				FillCustArray();
				CloseMySQL();
				SortItemsByCust();
			}
			iLastTimerCount = iTimerCount;
		}
	}
	// get the client rectangle
	// so we can tell the window size
	GetClientRect(&rc);

#if 0
	if (deTabControl(Width()-gx, 22, &saTab1, &iWhichPage)){
		DD_THROW;
	}
#else
	deStartHorizontal();
		if (deButton(50, 21, "Admin")){
			iWhichPage = PAGE_ADMIN;
			DD_THROW;
		}
		if (deButton(50, 21, "Status")){
			iWhichPage = PAGE_STATUS;
			DD_THROW;
		}
		if (deButton(50, 21, "Item")){
			iWhichPage = PAGE_ITEM;
			DD_THROW;
		}
		if (deButton(50, 21, "Cust")){
			iWhichPage = PAGE_CUSTOMER;
			DD_THROW;
		}
		if (deButton(50, 21, "Extra")){
			iWhichPage = PAGE_ADDEXTRA;
			DD_THROW;
		}
		if (deButton(70, 21, "Item Detail")){
			iWhichPage = PAGE_ITEMDETAIL;
			DD_THROW;
		}
		if (deButton(70, 21, "Cust Detail")){
			iWhichPage = PAGE_CUSTDETAIL;
			DD_THROW;
		}
	deEndHorizontal(21);
#endif

	SWITCH(iWhichPage)
	break; case PAGE_ADMIN:;
		deContentsAdmin();
	break; case PAGE_STATUS:;
		deContentsStatus();
	break; case PAGE_ITEM:;
		deContentsItem();
	break; case PAGE_CUSTOMER:;
		deContentsCustomer();
	break; case PAGE_ADDEXTRA:;
		deContentsAddExtra();
	break; case PAGE_ITEMDETAIL:;
		deContentsItemDetail();
	break; case PAGE_CUSTDETAIL:;
		deContentsCustDetail();
	END
}
// end DynDlg stuff

