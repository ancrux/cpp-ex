#ifdef WIN32

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <stdio.h>

#include <vector>
#include <map>

///*
/////////////////////////////////////////////////////////////////////////////// 
// 
//  FUNCTION:     GetCounterValues - This function constructs a counter path
//                from the counter and instance names obtained. The constructed
//                counter path is then used to get the performance data using
//                PdhCollectQueryData.
// 
//  RETURN VALUE: none
// 
/////////////////////////////////////////////////////////////////////////////// 
void WINAPI GetCounterValues(LPSTR serverName, LPSTR objectName,
                             LPSTR counterList, LPSTR instanceList)
{
    PDH_STATUS s;

    HQUERY hQuery;

    PDH_COUNTER_PATH_ELEMENTS *cpe = NULL;
    PDH_COUNTER_PATH_ELEMENTS *cpeBeg;

    DWORD  nCounters;
    DWORD  nInstances;

    HCOUNTER *hCounter = NULL;
    HCOUNTER *hCounterPtr;

    char *counterPtr, *instancePtr;

    char szFullPath[MAX_PATH];
    DWORD cbPathSize;
    DWORD   i, j;

    BOOL  ret = FALSE;

    PDH_FMT_COUNTERVALUE counterValue;

    // Scan through the counter names to find the number of counters.
    nCounters = 0;
    counterPtr = counterList;
    while (*counterPtr)
    {
        counterPtr += strlen(counterPtr) + 1;
        nCounters++;
    }

    // Scan through the instance names to find the number of instances.
    nInstances = 0;
    instancePtr = instanceList;
    while (*instancePtr)
    {
        instancePtr += strlen(instancePtr) + 1;
        nInstances++;
    }

    if (!nCounters || !nInstances) return;

    __try
    {
        cpe = (PDH_COUNTER_PATH_ELEMENTS *)HeapAlloc(GetProcessHeap(), 0, 
                    sizeof(PDH_COUNTER_PATH_ELEMENTS) * nCounters * nInstances);
        hCounter = (HCOUNTER *)HeapAlloc(GetProcessHeap(), 0, 
                    sizeof(HCOUNTER) * nCounters * nInstances);

        if (!cpe || !hCounter) __leave;

        // Only do this once to create a query.
        if ((s = PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS)
        {
            fprintf(stderr, "POQ failed %08x\n", s);
            __leave;
        }

        // For each instance name in the list, construct a counter path.
        cpeBeg = cpe;
        hCounterPtr = hCounter;
        for (i = 0, counterPtr = counterList; i < nCounters;
                i++, counterPtr += strlen(counterPtr) + 1)
        {
            for (j = 0, instancePtr = instanceList; j < nInstances;
                    j++,
                    instancePtr += strlen(instancePtr) + 1,
                    cpeBeg++,
                    hCounterPtr++)
            {
                cbPathSize = sizeof(szFullPath);

                cpeBeg->szMachineName = serverName;
                cpeBeg->szObjectName = objectName;
                cpeBeg->szInstanceName = instancePtr;
                cpeBeg->szParentInstance = NULL;
                cpeBeg->dwInstanceIndex = -1;
                cpeBeg->szCounterName = counterPtr;

                if ((s = PdhMakeCounterPath(cpeBeg,
                    szFullPath, &cbPathSize, 0)) != ERROR_SUCCESS)
                {
                    fprintf(stderr,"MCP failed %08x\n", s);
                    __leave;
                }

                // Add the counter path to the query.
                if ((s = PdhAddCounter(hQuery, szFullPath, 0, hCounterPtr))
                        != ERROR_SUCCESS)
                {
                    fprintf(stderr, "PAC failed %08x\n", s);
                    __leave;
                }
            }
        }

        for (i = 0; i < 2; i++)
        {
            Sleep(100);

            // Collect data as often as you need to.
            if ((s = PdhCollectQueryData(hQuery)) != ERROR_SUCCESS)
            {
                fprintf(stderr, "PCQD failed %08x\n", s);
                __leave;
            }

            if (i == 0) continue;

            // Display the performance data value corresponding to each instance.
            cpeBeg = cpe;
            hCounterPtr = hCounter;
            for (i = 0, counterPtr = counterList; i < nCounters;
                    i++, counterPtr += strlen(counterPtr) + 1)
            {
                for (j = 0, instancePtr = instanceList; j < nInstances;
                        j++,
                        instancePtr += strlen(instancePtr) + 1,
                        cpeBeg++,
                        hCounterPtr++)
                {
                    if ((s = PdhGetFormattedCounterValue(*hCounterPtr,
                        PDH_FMT_DOUBLE,
                        NULL, &counterValue)) != ERROR_SUCCESS)
                    {
                        fprintf(stderr, "PGFCV failed %08x\n", s);
                        continue;
                    }
                    printf("%s\\%s\\%s\t\t : [%3.3f]\n",
                        cpeBeg->szObjectName,
                        cpeBeg->szCounterName,
                        cpeBeg->szInstanceName,
                        counterValue.doubleValue);
                }
            }
        }

        // Remove all the performance counters from the query.
        hCounterPtr = hCounter;
        for (i = 0; i < nCounters; i++)
        {
            for (j = 0; j < nInstances; j++)
            {
                PdhRemoveCounter(*hCounterPtr);
            }
        }
    }
    __finally
    {
        HeapFree(GetProcessHeap(), 0, cpe);
        HeapFree(GetProcessHeap(), 0, hCounter);

        // Only do this cleanup once.
        PdhCloseQuery(hQuery);
    }

    return;
}
//*/

///*
/////////////////////////////////////////////////////////////////////////////// 
// 
//  FUNCTION:     EnumerateExistingInstances - This function displays the names
//                of all the instances that exist on ServerName. PDH is used
//                to enumerate the performance counter and instance names.
// 
//  RETURN VALUE: none
// 
/////////////////////////////////////////////////////////////////////////////// 

void WINAPI EnumerateExistingInstances(LPSTR serverName, LPSTR objectName)
{
    CHAR       mszEmptyList[2];  // An empty list contains 2 NULL characters
    LPSTR      mszInstanceList = NULL;
    LPSTR      szInstanceName;
    DWORD       cchInstanceList;
    LPSTR      mszCounterList = NULL;
    DWORD       cchCounterList;
    PDH_STATUS  pdhStatus;

    __try
    {
        mszCounterList = NULL;
        cchCounterList = 0;
        // Refresh the list of performance objects available on the specified
        // computer by making a call to PdhEnumObjects with bRefresh set to TRUE.
        pdhStatus = PdhEnumObjects(NULL, serverName, mszCounterList,
            &cchCounterList, PERF_DETAIL_WIZARD, TRUE);

        mszCounterList = NULL;
        cchCounterList = 0;
        // Determine the required size for the buffer containing counter names
        // and instance names by calling PdhEnumObjectItems.
        cchInstanceList = sizeof(mszEmptyList);
        pdhStatus = PdhEnumObjectItems(NULL, serverName,
            objectName, mszCounterList, 
            &cchCounterList, mszEmptyList,
            &cchInstanceList, PERF_DETAIL_WIZARD, 0);

        if (pdhStatus == ERROR_SUCCESS)
            return;  // The list is empty so do nothing.
        else if (pdhStatus != PDH_MORE_DATA)
        {
            fprintf(stderr, "PEOI failed %08x\n", pdhStatus);
            return;
        }

        // Allocate a buffer for the counter names.
        mszCounterList = (LPSTR)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, cchCounterList);
        if (!mszCounterList)
        {
            fprintf(stderr, "HA failed %08x\n", GetLastError());
            return;
        }

        // Allocate a buffer for the instance names.
        mszInstanceList = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            cchInstanceList);
        if (!mszInstanceList)
        {
            fprintf(stderr, "HA failed %08x\n", GetLastError());
            return;
        }

        __try
        {
            // Enumerate to get the list of Counters and Instances provided by
            // the specified object on the specified computer.
            pdhStatus = PdhEnumObjectItems(NULL, serverName,
                objectName, mszCounterList, 
                &cchCounterList, mszInstanceList,
                &cchInstanceList, PERF_DETAIL_WIZARD, 0);
            if (pdhStatus != ERROR_SUCCESS)
            {
                fprintf(stderr, "PEOI failed %08x\n", pdhStatus);
                return;
            }

            // Display the items from the buffer.
            szInstanceName = mszInstanceList;
            while (*szInstanceName)
            {
                printf("%s\n", szInstanceName);
                szInstanceName += strlen(szInstanceName) + 1;
            }

            GetCounterValues(serverName, objectName, mszCounterList,
                mszInstanceList);
        }
        __finally
        {
        }

    }
    __finally
    {
        // Free the buffers.
        if (mszInstanceList) HeapFree(GetProcessHeap(), 0, mszInstanceList);
        if (mszCounterList) HeapFree(GetProcessHeap(), 0, mszCounterList);
    }

    return;   
}
//*/

int run_pdh_enum_test(int argc, ACE_TCHAR* argv[])
{
    if (argc > 1)
    {
        // argv[1] - Server Name
        EnumerateExistingInstances(argv[1], "Thread");
    }
    else
    {
        // Local System
        //EnumerateExistingInstances(NULL, "Thread");
		EnumerateExistingInstances(NULL, "Process");
    }

	return 0;
}

void WINAPI GetCounterValues2(LPSTR serverName, LPSTR objectName,
                             LPSTR counterList, LPSTR instanceList)
{
    PDH_STATUS s;

    HQUERY hQuery;

    PDH_COUNTER_PATH_ELEMENTS *cpe = NULL;
    PDH_COUNTER_PATH_ELEMENTS *cpeBeg;

    DWORD  nCounters;
    DWORD  nInstances;

    HCOUNTER *hCounter = NULL;
    HCOUNTER *hCounterPtr;

    char *counterPtr, *instancePtr;

    char szFullPath[MAX_PATH];
    DWORD cbPathSize;
    DWORD   i, j;

    BOOL  ret = FALSE;

    PDH_FMT_COUNTERVALUE counterValue;

    // Scan through the counter names to find the number of counters.
    nCounters = 0;
    counterPtr = counterList;
    while (*counterPtr)
    {
        counterPtr += strlen(counterPtr) + 1;
        nCounters++;
    }

    // Scan through the instance names to find the number of instances.
    nInstances = 0;
    instancePtr = instanceList;
    while (*instancePtr)
    {
        instancePtr += strlen(instancePtr) + 1;
        nInstances++;
    }

    if (!nCounters || !nInstances) return;

    __try
    {
        cpe = (PDH_COUNTER_PATH_ELEMENTS *)HeapAlloc(GetProcessHeap(), 0, 
                    sizeof(PDH_COUNTER_PATH_ELEMENTS) * nCounters * nInstances);
        hCounter = (HCOUNTER *)HeapAlloc(GetProcessHeap(), 0, 
                    sizeof(HCOUNTER) * nCounters * nInstances);

        if (!cpe || !hCounter) __leave;

        // Only do this once to create a query.
        if ((s = PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS)
        {
            fprintf(stderr, "POQ failed %08x\n", s);
            __leave;
        }

        // For each instance name in the list, construct a counter path.
        cpeBeg = cpe;
        hCounterPtr = hCounter;
        for (j = 0, instancePtr = instanceList; j < nInstances;
                j++,
                instancePtr += strlen(instancePtr) + 1
                
                )
        {
			for (i = 0, counterPtr = counterList; i < nCounters;
					i++, counterPtr += strlen(counterPtr) + 1,
					cpeBeg++,
					hCounterPtr++)
			{
                cbPathSize = sizeof(szFullPath);

                cpeBeg->szMachineName = serverName;
                cpeBeg->szObjectName = objectName;
                cpeBeg->szInstanceName = instancePtr;
                cpeBeg->szParentInstance = NULL;
                cpeBeg->dwInstanceIndex = -1;
                cpeBeg->szCounterName = counterPtr;

                if ((s = PdhMakeCounterPath(cpeBeg,
                    szFullPath, &cbPathSize, 0)) != ERROR_SUCCESS)
                {
                    fprintf(stderr,"MCP failed %08x\n", s);
                    __leave;
                }

                // Add the counter path to the query.
                if ((s = PdhAddCounter(hQuery, szFullPath, 0, hCounterPtr))
                        != ERROR_SUCCESS)
                {
                    fprintf(stderr, "PAC failed %08x\n", s);
                    __leave;
                }
            }
        }

        for (int ii = 0; ii < 3; ii++)
        {
            Sleep(1000);

            // Collect data as often as you need to.
            if ((s = PdhCollectQueryData(hQuery)) != ERROR_SUCCESS)
            {
                fprintf(stderr, "PCQD failed %08x\n", s);
                __leave;
            }

            if (ii == 0) continue;

            // Display the performance data value corresponding to each instance.
            cpeBeg = cpe;
            hCounterPtr = hCounter;
            for (j = 0, instancePtr = instanceList; j < nInstances;
                    j++, instancePtr += strlen(instancePtr) + 1
					
                    )
            {
				for (i = 0, counterPtr = counterList; i < nCounters;
						i++, counterPtr += strlen(counterPtr) + 1,
						cpeBeg++,
						hCounterPtr++
						)
				{

                    if ((s = PdhGetFormattedCounterValue(*hCounterPtr,
                        PDH_FMT_DOUBLE,
                        NULL, &counterValue)) != ERROR_SUCCESS)
                    {
                        fprintf(stderr, "PGFCV failed %08x\n", s);
                        continue;
                    }
                    printf("%s\\%s\\%s\t\t : [%3.3f]\n",
                        cpeBeg->szObjectName,
                        cpeBeg->szCounterName,
                        cpeBeg->szInstanceName,
                        counterValue.doubleValue);
                }
				printf("\n");
            }
        }

        // Remove all the performance counters from the query.
        hCounterPtr = hCounter;
        for (i = 0; i < nCounters; i++)
        {
            for (j = 0; j < nInstances; j++)
            {
                PdhRemoveCounter(*hCounterPtr);
            }
        }
    }
    __finally
    {
        HeapFree(GetProcessHeap(), 0, cpe);
        HeapFree(GetProcessHeap(), 0, hCounter);

        // Only do this cleanup once.
        PdhCloseQuery(hQuery);
    }

    return;
}

/////////////////////////////////////////////////////////////////////////////// 
// 
//  FUNCTION:     EnumerateExistingInstances - This function displays the names
//                of all the instances that exist on ServerName. PDH is used
//                to enumerate the performance counter and instance names.
// 
//  RETURN VALUE: none
// 
/////////////////////////////////////////////////////////////////////////////// 

void WINAPI EnumerateExistingInstances2(LPSTR serverName, LPSTR objectName)
{
    CHAR       mszEmptyList[2];  // An empty list contains 2 NULL characters
    LPSTR      mszInstanceList = NULL;
    LPSTR      szInstanceName;
    DWORD       cchInstanceList;
    LPSTR      mszCounterList = NULL;
    DWORD       cchCounterList;
    PDH_STATUS  pdhStatus;

    __try
    {
        mszCounterList = NULL;
        cchCounterList = 0;
        // Refresh the list of performance objects available on the specified
        // computer by making a call to PdhEnumObjects with bRefresh set to TRUE.
        pdhStatus = PdhEnumObjects(NULL, serverName, mszCounterList,
            &cchCounterList, PERF_DETAIL_WIZARD, TRUE);

        mszCounterList = NULL;
        cchCounterList = 0;
        // Determine the required size for the buffer containing counter names
        // and instance names by calling PdhEnumObjectItems.
        cchInstanceList = sizeof(mszEmptyList);
        pdhStatus = PdhEnumObjectItems(NULL, serverName,
            objectName, mszCounterList, 
            &cchCounterList, mszEmptyList,
            &cchInstanceList, PERF_DETAIL_WIZARD, 0);

        if (pdhStatus == ERROR_SUCCESS)
            return;  // The list is empty so do nothing.
        else if (pdhStatus != PDH_MORE_DATA)
        {
            fprintf(stderr, "PEOI failed %08x\n", pdhStatus);
            return;
        }

        // Allocate a buffer for the counter names.
        mszCounterList = (LPSTR)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, cchCounterList);
        if (!mszCounterList)
        {
            fprintf(stderr, "HA failed %08x\n", GetLastError());
            return;
        }

        // Allocate a buffer for the instance names.
        mszInstanceList = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            cchInstanceList);
        if (!mszInstanceList)
        {
            fprintf(stderr, "HA failed %08x\n", GetLastError());
            return;
        }

        __try
        {
            // Enumerate to get the list of Counters and Instances provided by
            // the specified object on the specified computer.
            pdhStatus = PdhEnumObjectItems(NULL, serverName,
                objectName, mszCounterList, 
                &cchCounterList, mszInstanceList,
                &cchInstanceList, PERF_DETAIL_WIZARD, 0);
            if (pdhStatus != ERROR_SUCCESS)
            {
                fprintf(stderr, "PEOI failed %08x\n", pdhStatus);
                return;
            }

            // Display the items from the buffer.
            szInstanceName = mszInstanceList;
            while (*szInstanceName)
            {
                printf("%s\n", szInstanceName);
                szInstanceName += strlen(szInstanceName) + 1;
            }

            GetCounterValues2(serverName, objectName, 
				//"ID Process\0" // pid
				//"Creating Process ID\0" // ppid
				"% Processor Time\0" // cpu
				//"Working Set\0" // mem
				//"Thread Count\0" // thr
				//"Priority Base\0" // priority
				,
                mszInstanceList);
        }
        __finally
        {
        }

    }
    __finally
    {
        // Free the buffers.
        if (mszInstanceList) HeapFree(GetProcessHeap(), 0, mszInstanceList);
        if (mszCounterList) HeapFree(GetProcessHeap(), 0, mszCounterList);
    }

    return;   
}

#define INITIALPATHSIZE 2048

PDH_STATUS GetAllMetricsFor(char *WildCardPath){
	HQUERY hQuery = NULL;

	LPSTR  szCtrPath = NULL;
	char   szWildCardPath[256] = "\000";
	DWORD  dwCtrPathSize = 0;
	DWORD  dwPathCount = 0;

	HCOUNTER* hCounter = NULL;
    HCOUNTER* hCounterPtr;
	PDH_STATUS  pdhStatus;
	sprintf(szWildCardPath, WildCardPath);//works
	// First try with an initial buffer size.
	szCtrPath = (LPSTR) GlobalAlloc(GPTR, INITIALPATHSIZE);
	dwCtrPathSize = INITIALPATHSIZE;
	pdhStatus = PdhExpandWildCardPath(NULL,szWildCardPath, szCtrPath, 
		&dwCtrPathSize,NULL);
	// Check for a too small buffer.
	if (pdhStatus == PDH_MORE_DATA)
	{
		dwCtrPathSize++;
		GlobalFree(szCtrPath);
		szCtrPath =  (LPSTR) GlobalAlloc(GPTR, dwCtrPathSize);;
		pdhStatus = PdhExpandWildCardPath(NULL,szWildCardPath, szCtrPath, 
			&dwCtrPathSize,NULL);
	}

    // Only do this once to create a query.
    if ((pdhStatus = PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS)
    {
		return pdhStatus;
    }

	///*
	// Add the paths to the query
	if (pdhStatus == PDH_CSTATUS_VALID_DATA)
	{
		LPSTR ptr;

		ptr = szCtrPath;
		while (*ptr)
		{
			//printf("%s\n", ptr);
			ptr += strlen(ptr);
			ptr++;
			dwPathCount++;
		}

		hCounter = new HCOUNTER[dwPathCount];

		hCounterPtr = hCounter;
		ptr = szCtrPath;
		while (*ptr)
		{
			pdhStatus = PdhAddCounter(hQuery,ptr,0, hCounterPtr++);
			ptr += strlen(ptr);
			ptr++;
		}
	}
	else printf("PdhExpandCounterPath failed: %d\n", pdhStatus);
	//*/

    //Sleep(100);

    // Collect data as often as you need to.
    if ((pdhStatus = PdhCollectQueryData(hQuery)) != ERROR_SUCCESS)
    {
		return pdhStatus;
    }


    DWORD   i, j;
    PDH_FMT_COUNTERVALUE counterValue;
	LPSTR ptr;

	hCounterPtr = hCounter;
	for (i = 0, ptr = szCtrPath; i < dwPathCount;
		i++, ptr += strlen(ptr) + 1,
		hCounterPtr++
		)
	{

		if ((pdhStatus = PdhGetFormattedCounterValue(*hCounterPtr,
			PDH_FMT_DOUBLE,
			NULL, &counterValue)) != ERROR_SUCCESS)
		{
			//fprintf(stderr, "PGFCV failed %08x\n", s);
			continue;
		}
		printf("%s=[%3.3f]\n",
			ptr,
			counterValue.doubleValue);
	}

    // Only do this cleanup once.
	delete [] hCounter;
    PdhCloseQuery(hQuery);

	return pdhStatus;
}

int
ps_stats_win32(std::string& ps)
{
	typedef std::map<std::string, size_t> PROC_MAP; // process map

	int rc = -1;

	PDH_STATUS  status;
	LPSTR object = "Process";

	//LPSTR msz_instance = NULL;
	std::string str_instance;
	DWORD len_instance = 0;
	DWORD n_instance = 0;
	
	//LPSTR msz_counter = NULL;
	std::string str_counter;
	DWORD len_counter = 0;
	DWORD n_counter = 0;

	status = ::PdhEnumObjectItems(NULL, NULL, object,
		NULL, &len_counter,
		NULL, &len_instance,
		PERF_DETAIL_WIZARD, 0);
	if ( status != PDH_MORE_DATA && status != ERROR_SUCCESS )
		return -1;

	str_instance.resize(len_instance * sizeof(CHAR) + 1);
	str_counter.resize(len_counter * sizeof(CHAR) + 1);

	status = ::PdhEnumObjectItems(NULL, NULL, object,
		(LPSTR) str_counter.c_str(), &len_counter,
		(LPSTR) str_instance.c_str(), &len_instance,
		PERF_DETAIL_WIZARD, 0);
	if ( status != ERROR_SUCCESS )
		return -1;

	// set counter manually
	str_counter.resize(0); n_counter = 0;
	str_counter.append("ID Process"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Creating Process ID"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("% Processor Time"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Working Set"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Thread Count"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Priority Base"); str_counter.append(1, '\0'); ++n_counter;

	// set instance map
	PROC_MAP map_instance;
	for(LPSTR ptr = (LPSTR) str_instance.c_str(); *ptr; ptr += strlen(ptr)+1)
	{
		//printf("%s\n", ptr);
		++n_instance;
		map_instance[ptr]++;
	}

	/*
	for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
	{
		printf("%s=%d\n", iter->first.c_str(), iter->second);
	}
	//*/

	HQUERY hQuery = NULL;

    // open query
	if ( (status = ::PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS )
		return -1;

	// add counters
	std::vector< HCOUNTER > counter_handles(n_instance * n_counter);
	size_t n_handle = 0;
	for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
	{
		size_t dup_count = iter->second;
		for(size_t n = 0; n < dup_count; ++n)
		{
			const char* ptr = str_counter.c_str();
			for(DWORD c = 0; c < n_counter; ++c, ptr += strlen(ptr)+1)
			{
				std::string counter_path = "\\";
				counter_path += (char*) object;
				counter_path += "(";
				counter_path += iter->first;
				if ( n )
				{
					char buf[32];
					counter_path += "#";
					counter_path += ACE_OS::itoa(n, buf, 10);
				}
				counter_path += ")\\";
				counter_path += ptr;
				//printf("%s\n", counter_path.c_str()); //@

				status = ::PdhAddCounter(hQuery, counter_path.c_str(), 0, &counter_handles[n_handle++]);
				if ( status != ERROR_SUCCESS )
				{
					printf("add counter_handles[%d] error!\n", n_handle-1);
				}
			}
		}
	}

	// get counter values
	if ( (status = ::PdhCollectQueryData(hQuery)) == ERROR_SUCCESS )
	{
		::Sleep(10);

		ps = "pid\tcpu\tmem\tthr\tstate\tname\ttitle\n";
		rc = n_instance;

		PDH_FMT_COUNTERVALUE counter_value;

		size_t n_handle = 0;
		for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
		{
			size_t dup_count = iter->second;
			for(size_t n = 0; n < dup_count; ++n)
			{
				std::string counter_path = iter->first;
				if ( n )
				{
					char buf[32];
					counter_path += "#";
					counter_path += ACE_OS::itoa(n, buf, 10);
				}

				char buf[PATH_MAX+1];

				// pid
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%d\t", (int) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;

				// ppid
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);

				// cpu
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%f\t", counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;
				double cpu = counter_value.doubleValue;
				
				// mem
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%llu\t", (long long) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;
				
				// thr
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%d\t", (int) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;

				// priority
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);

				// state
				(cpu > 0.000001)?
					ACE_OS::snprintf(buf, PATH_MAX, "RUNNING\t"):
					ACE_OS::snprintf(buf, PATH_MAX, "SLEEPING\t");
				ps += buf;

				// name
				ps += iter->first; ps += "\t";

				// title
				ps += counter_path; ps += "\t";

				ps += "\n";

				/*
				//const char* ptr = str_counter.c_str();
				//for(DWORD c = 0; c < n_counter; ++c, ptr += strlen(ptr)+1)
				//{
				//	std::string counter_path = iter->first;
				//	if ( n )
				//	{
				//		char buf[32];
				//		counter_path += "#";
				//		counter_path += ACE_OS::itoa(n, buf, 10);
				//	}
				//	counter_path += "\\";
				//	counter_path += ptr;
				//	//printf("%s\n", counter_path.c_str()); //@

				//	if ( (status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++],
				//		PDH_FMT_DOUBLE,
				//		NULL, &counter_value)) != ERROR_SUCCESS)
				//	{
				//		//fprintf(stderr, "PGFCV failed %08x\n", s);
				//		continue;
				//	}
				//	printf("%s=[%3.3f]\n", counter_path.c_str(), counter_value.doubleValue);
				//}
				//*/
			}
		}
	}

	// remove counters
	for(size_t n = 0, c = counter_handles.size(); n < c; ++n)
		::PdhRemoveCounter(counter_handles[n]);

	// close query
	::PdhCloseQuery(hQuery);

	return rc;
}

int run_pdh_enum_test2(int argc, ACE_TCHAR* argv[])
{
    if (argc > 1)
    {
        // argv[1] - Server Name
        EnumerateExistingInstances2(argv[1], "Thread");
    }
    else
    {
        // Local System
        //EnumerateExistingInstances(NULL, "Thread");
		EnumerateExistingInstances2(NULL, "Process");

		//GetAllMetricsFor("\\Process(*)\\ID Process");
		//GetAllMetricsFor("\\Process(*)\\%% Processor Time");
		
		//std::string ps;
		//ps_stats_win32(ps);
		//printf("%s", ps.c_str());

    }

	return 0;
}

#endif

