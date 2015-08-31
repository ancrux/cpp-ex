
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}


int run_sqlite_test(int argc, ACE_TCHAR* argv[])
{
	::printf("sqlite3 version: %s\n", sqlite3_libversion());

	int thr_safe = sqlite3_threadsafe();
	::printf("sqlite3 thread_safe: %d\n", thr_safe);

	sqlite3* db;
	char* sz_err = 0;
	int rc;

	rc = sqlite3_open("sqlite.db", &db);
	if ( rc ) 
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	char* sz_stmt = "create table my_tbl ( name char(36) );";
	rc = sqlite3_exec(db, sz_stmt, callback, 0, &sz_err);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", sz_err);
		sqlite3_free(sz_err);
	}

	sqlite3_close(db);

	return 0;
}

/*

// Code fragment to compile an SQL statement and read the result set.
// this does not have code to handle an altered schema.

int          col_cnt; 
sqlite3      *db;  
int          rc;
cosnt char   *sqlst_end;
sqlite3_stmt *sqlst;

// Compile the SQL.
rc = sqlite3_prepare_v2(db, sqstmnt, -1, &sqst, &sqst_end);
if (rc != SQLITE_OK) {
	wrapup("9: SQL compile error");
	return(TRUE);
}  // if

// Execute the SQL.  It could be busy.  Count busy polls to break 
// a deadlock.  When busy reset the current step, pause and relaunch it.

// Count columns in statement and access declared types.
col_cnt = sqlite3_column_count(sqst);

finished = FALSE;
while (!finished) {
	rc = sqlite3_step(sqst);
	switch (rc)
	{
	  case SQLITE_DONE:     // Execution finished.
		  finished = TRUE;
		  sqlite3_reset(sqst);  // Resets the compiled statement for re-use.

		  // Execute logic for end of data set.
		  // !!!
		  break;
	  case SQLITE_ROW:      // We have a row.
		  if (rowcount == 0) {
			  // Execute code for start of data set
			  // !!!!
		  }

		  // Scan all the columns.  Return value in "strg"
		  for (a = 0; a < col_cnt; a++)
		  {
			  // Get actual type.
			  switch (sqlite3_column_type(sqst, a))
			  {
			  case SQLITE_INTEGER:
				  result = sqlite3_column_int(sqst, a);
				  sprintf(strg, "%d", result);
				  break;
			  case SQLITE_FLOAT:
				  {
					  // Float to string.
					  double  dub;
					  dub = sqlite3_column_double(sqst, a);
					  sprintf(strg, "%f", dub);
				  }
				  break;
			  case SQLITE_TEXT:
				  vp = sqlite3_column_text(sqst, a);
				  p = strg;
				  while (*vp > 0) *p++ = *vp++;
				  break;
			  case SQLITE_BLOB:
				  // Write to a file, dynamic memory ...
				  break;
			  case SQLITE_NULL:
				  strg[0] = 0;
				  break;
			  }  // switch
		  }    // for

		  rowcount++;
		  break;
	  case SQLITE_BUSY:      // DB is busy, try again
		  int errc = sqlite3_reset(sqst);
		  if (busy_count++ == MAX_BUSY_COUNT) {
			  sqlite3_finalize(sqst);
			  wrapup("9: Database Deadlocked");
			  break;
		  }  // if

		  //To make the polling less intensive give up the time slice.
#if IS_WIN32
		  sleep(0);  // Win32 has a different yield call.
#else
		  yield();   // Relinquish time slice to limit overhead of polling.
#endif
		  break;
	  default:    // A nasty error.
		  sqlite3_finalize(sqst);
		  wrapup("9: Fatal SQL EXEC error");
		  break;
	}  // switch
}    // while

// This will clear the compiled statement so that the DB can be closed.
sqlite3_finalize(sqst);

//*/
