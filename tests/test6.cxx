#include "hw2_test.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define ASSERT_TEST(expr)   \
    do {    \
        if (!(expr)) {  \
            printf("Assertion failed %s:%d. %s ",  \
                __FILE__, __LINE__, #expr);   \
            return false;   \
        }   \
    } while (0)

#define ASSERT_TEST_CHILD(expr)   \
    do {    \
        if (!(expr)) {  \
            printf("Child assertion failed %s:%d. %s ",  \
                __FILE__, __LINE__, #expr);   \
            exit(1);   \
        }   \
    } while (0)	

#define ASSERT_TEST_FATHER(expr)   \
    do {    \
        if (!(expr)) {  \
            exit(1);   \
        }   \
    } while (0)	

/*************************************************************************/
/*
  _______        _       
 |__   __|      | |      
    | | ___  ___| |_ ___ 
    | |/ _ \/ __| __/ __|
    | |  __/\__ \ |_\__ \
    |_|\___||___/\__|___/
                         
*/                         
/*************************************************************************/
#define EE_FACULTY (0)
#define CS_FACULTY (1)
#define SET_SUCCESS (0)
#define SET_FAILED (-1)
#define REGISTRATION_SUCCESS  (0)

#define TEST_STATUS_INVALID(s) \
	errno = 0;\
	ASSERT_TEST( SET_FAILED == set_status(s)); \
	ASSERT_TEST(EINVAL == errno);

#define TEST_NO_PROCESS_IS_REGISTERED \
	errno = 0;\
	ASSERT_TEST( SET_FAILED == get_all_cs()); \
	ASSERT_TEST(ENODATA == errno);

#define CHILD_TEST_NO_PROCESS_IS_REGISTERED \
	errno = 0;\
	ASSERT_TEST_CHILD( -1 == get_all_cs()); \
	ASSERT_TEST_CHILD(ENODATA == errno);


static bool RemoveFileUtil(const char* filename)
{
    size_t length_file_name_plus_one = strlen(filename);
    ASSERT_TEST(length_file_name_plus_one > 0); 
    ++length_file_name_plus_one;
    char* rm_command = (char*)malloc(length_file_name_plus_one + 3u);  
    ASSERT_TEST(NULL != strncpy(rm_command,"rm ", 4));
    ASSERT_TEST(NULL != strncpy(rm_command+3u, filename, length_file_name_plus_one));
    ASSERT_TEST(0 == system(rm_command));
    free(rm_command);
	return true;
}


static bool testInTheRightOS(void)
{
	const char* tempOutFile = "tempOutFileKernel.txt";
	long h = hello();
	ASSERT_TEST(h == 0);
  
 	ASSERT_TEST(0 == system("dmesg | grep Hello > tempOutFileKernel.txt"));
	FILE* f = fopen(tempOutFile, "r");

	bool doesContainHello = false;
	ssize_t nRead;
	char *line = NULL;
	size_t len = 0;
    while ((nRead = getline(&line, &len, f)) != -1) 
	{
		if (NULL!= strstr(line, "Hello, Kernel!"))
		{
			doesContainHello = true;
			break;
		}
    }
	ASSERT_TEST(doesContainHello);
	ASSERT_TEST(0 == fclose(f));
	ASSERT_TEST(RemoveFileUtil(tempOutFile));
    return true;
}

static bool testSegel(void)
{
	ASSERT_TEST(EE_FACULTY == get_status());
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());
	return true;
}

static bool testHello(void)
{
	const char* tempOutFile = "tempOutFile.txt";
	long h = hello();
	ASSERT_TEST(h == 0);
  
 	ASSERT_TEST(0 == system("dmesg | grep World > tempOutFile.txt"));
	FILE* f = fopen(tempOutFile, "r");

	bool doesContainHello = false;
	ssize_t nRead;
	char *line = NULL;
	size_t len = 0;
    while ((nRead = getline(&line, &len, f)) != -1) 
	{
		if (NULL!= strstr(line, "Hello, World!"))
		{
			doesContainHello = true;
			break;
		}
    }
	ASSERT_TEST(doesContainHello);
	ASSERT_TEST(0 == fclose(f));
	ASSERT_TEST(RemoveFileUtil(tempOutFile));
    return true;
}

static bool testErrSetStatus(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());
	TEST_STATUS_INVALID(INT_MIN);
	ASSERT_TEST(CS_FACULTY == get_status());
	TEST_STATUS_INVALID(-1);
	ASSERT_TEST(CS_FACULTY == get_status());
	TEST_STATUS_INVALID(2);
	ASSERT_TEST(CS_FACULTY == get_status());
	TEST_STATUS_INVALID(INT_MAX);
	ASSERT_TEST(CS_FACULTY == get_status());
	return true;
}

static bool testErrGetAllCs(void)
{
	 /* should return error because no recognized processes yet*/
	errno = 0;
	TEST_NO_PROCESS_IS_REGISTERED;
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	TEST_NO_PROCESS_IS_REGISTERED;
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	return true;
}

static bool testGetStatus(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	ASSERT_TEST(EE_FACULTY == get_status());
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child */
	{
		ASSERT_TEST_CHILD(CS_FACULTY == get_status());
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(EE_FACULTY));
		ASSERT_TEST_CHILD(EE_FACULTY == get_status());	
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
		ASSERT_TEST_CHILD(CS_FACULTY == get_status());
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(EE_FACULTY));
		ASSERT_TEST_CHILD(EE_FACULTY == get_status());
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		ASSERT_TEST(CS_FACULTY == get_status());	
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		ASSERT_TEST(CS_FACULTY == get_status());	 // change in child won't affect father
	}

	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	ASSERT_TEST(EE_FACULTY == get_status());	
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());
	return true;
}

static bool testGetAllCS_FatherCS_NotRegistered(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());	
	TEST_NO_PROCESS_IS_REGISTERED;

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child - inherited CS*/
	{
		CHILD_TEST_NO_PROCESS_IS_REGISTERED;
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		long expectedSum = getpid();
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		TEST_NO_PROCESS_IS_REGISTERED;
	}	
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	return true;
}

static bool testGetAllCS_FatherEE_NotRegistered(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	ASSERT_TEST(EE_FACULTY == get_status());
	TEST_NO_PROCESS_IS_REGISTERED;
	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child - inherited EE*/
	{
		CHILD_TEST_NO_PROCESS_IS_REGISTERED;
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST_CHILD(0 == get_all_cs());
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
		const long expectedSum = getpid();
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process()); // do it again
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());	
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(EE_FACULTY)); // should decrease sum
		ASSERT_TEST_CHILD(0 == get_all_cs()); 
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		TEST_NO_PROCESS_IS_REGISTERED;
	}	
	return true;
}

static bool DepthUtil(unsigned depth, long accumulatedSum)
{
	static const unsigned maxDepth = 9;
	if (maxDepth == depth)
	{
		return true;
	}
	

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	
	if (0 == p) 
	{
		int addition = 0;
		if (depth > 1)
		{
			ASSERT_TEST_CHILD(accumulatedSum == get_all_cs());
		}
		else
		{
			CHILD_TEST_NO_PROCESS_IS_REGISTERED;
		}
		switch (depth)
		{
		case 0:
			ASSERT_TEST_CHILD(EE_FACULTY == get_status());
			break;
		case 1:
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
			ASSERT_TEST_CHILD(accumulatedSum == get_all_cs());
			break;
		case 2:
			ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
			addition = getpid();
			break;
		case 3:
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
			addition = getpid();		
			break;
		case 4:
			ASSERT_TEST_CHILD(SET_SUCCESS == set_status(EE_FACULTY));
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());		
			break;
		case 5:
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
			break;
		case 6:
			ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
			break;
		case 7:
			ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
			addition = getpid();		
			break;
		case 8:
			break;
		default:
			ASSERT_TEST_CHILD(false);
		}
		DepthUtil(depth + 1, accumulatedSum + addition);
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		if (0 == depth)
		{
			ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
			ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		}
		else
		{
			ASSERT_TEST_FATHER(p == waitpid(p, &childExitInfo, 0));
			ASSERT_TEST_FATHER(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));			
		}
	}	
	return true;		
}

static bool testDeepProcesses(void)
{
	TEST_NO_PROCESS_IS_REGISTERED;
	ASSERT_TEST(DepthUtil(0, 0));
	TEST_NO_PROCESS_IS_REGISTERED;
	return true;
}

static bool BreadthUtil(int facultyArr[], int isRecognizedArr[], unsigned int index)
{
	ASSERT_TEST_CHILD(SET_SUCCESS == set_status(facultyArr[index]));
	ASSERT_TEST_CHILD(facultyArr[index] == get_status());
	if (isRecognizedArr[index])
	{
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS == register_process());
	}
	
	return true;
}

static bool DoWeNeedToAddUtil(int facultyArr[], int isRecognizedArr[], unsigned int index)
{
	return CS_FACULTY == facultyArr[index] && 1 == isRecognizedArr[index];
}

static bool testBreadthZombiesOnDifferentWaits(void)
{
	ASSERT_TEST(EE_FACULTY == get_status());
	TEST_NO_PROCESS_IS_REGISTERED;
	const unsigned int numWaits = 4;
	const unsigned int numCombFacultyAndRecognition = 12;
	const unsigned int mul = numWaits * numCombFacultyAndRecognition;
	pid_t childrenPids[mul];
	int facultyArr[mul];
	int isRecognizedArr[mul];

	for (unsigned i = 0; i < mul; ++i)
	{
		if (i % 2 == 0)
			facultyArr[i] = EE_FACULTY;
		else
			facultyArr[i] = CS_FACULTY;
		if (i % 4 == 0 || i % 4 == 1)
			isRecognizedArr[i] = 0;
		else
			isRecognizedArr[i] = 1;	
	}
	
	pthread_mutex_t * pmutex = NULL;
	pthread_mutexattr_t attrmutex;
	pthread_mutexattr_init(&attrmutex);
	pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
	pmutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	ASSERT_TEST(NULL != pmutex);
	pthread_mutex_init(pmutex, &attrmutex);
	
	for (unsigned int i = 0; i < mul; ++i)
	{
		childrenPids[i] = fork();
		ASSERT_TEST(-1 != childrenPids[i]);
		if (0 == childrenPids[i]) /* child */
		{
			pthread_mutex_lock(pmutex);
			if (!BreadthUtil(facultyArr, isRecognizedArr, i))
			{
				pthread_mutex_unlock(pmutex);
				ASSERT_TEST_CHILD(false);
			}
			else
			{
				pthread_mutex_unlock(pmutex);
			}
			exit(0);
		}
	}

	long sum = 0;
	for (unsigned int i = 0; i < mul; ++i)
	{
		if(DoWeNeedToAddUtil(facultyArr, isRecognizedArr, i))
		{
			sum += childrenPids[i];
		}
	}
	
	sleep(3); // Let children do finish their job
	for (unsigned int i = 0; i < mul; ++i)
	{
		int childExitInfo;
		unsigned int j = (unsigned)-1;
		ASSERT_TEST(sum == get_all_cs());
		pid_t childFinished;
		if (i < mul / numWaits)
		{
			j = i;
			siginfo_t rv;
			ASSERT_TEST(0 == waitid(P_PID, childrenPids[i], &rv, WEXITED));
			childExitInfo = rv.si_status;
			ASSERT_TEST(rv.si_pid == childrenPids[i]);
			ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		}
		else if (i < (mul / numWaits) * 2)
		{
			childFinished = wait(&childExitInfo);
		}
		else if (i < (mul / numWaits) * 3)
		{
			childFinished = waitpid(-1, &childExitInfo, 0);
		}
		else
		{
			siginfo_t rv;
			ASSERT_TEST(0 == waitid(P_ALL, 0, &rv, WEXITED));
			childFinished = rv.si_pid;
		}

		if (!(i < mul / numWaits))
		{
			ASSERT_TEST(0 < childFinished);
			ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
			unsigned k = mul/numWaits;
			for (; k < mul; ++k)
			{
				if(childrenPids[k] == childFinished)
				{
					j = k;
					break;
				}
			}
			ASSERT_TEST(j != (unsigned)-1);
		}

		if(DoWeNeedToAddUtil(facultyArr, isRecognizedArr, j))
		{
			sum -= childrenPids[i];
		}		
	}

	TEST_NO_PROCESS_IS_REGISTERED;
	pthread_mutex_destroy(pmutex);
	pthread_mutexattr_destroy(&attrmutex);
	free(pmutex);
	return true;
}

static bool testGetAllCS_FatherCS(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	ASSERT_TEST(EE_FACULTY == get_status());
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	ASSERT_TEST(0 == get_all_cs());
	ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
	ASSERT_TEST(CS_FACULTY == get_status());	
	ASSERT_TEST(getpid() == get_all_cs());

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child - inherited CS*/
	{
		long expectedSum = getppid();
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		expectedSum += getpid();
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		ASSERT_TEST(getpid() == get_all_cs());
	}
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	return true;
}

static bool testGetAllCS_FatherEE(void)
{
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	ASSERT_TEST(EE_FACULTY == get_status());
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	ASSERT_TEST(0 == get_all_cs());

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child - inherited EE */
	{
		long expectedSum = 0;
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		ASSERT_TEST_CHILD(SET_SUCCESS == set_status(CS_FACULTY));
		ASSERT_TEST_CHILD(CS_FACULTY == get_status());
		expectedSum += getpid();
		ASSERT_TEST_CHILD(expectedSum == get_all_cs());
		exit(0);
	}
	else /* father */ 
	{
		int childExitInfo;
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		ASSERT_TEST(0 == get_all_cs());
		ASSERT_TEST(SET_SUCCESS == set_status(CS_FACULTY));
		ASSERT_TEST(CS_FACULTY == get_status());		
		ASSERT_TEST(getpid() == get_all_cs());
	}	
	ASSERT_TEST(SET_SUCCESS == set_status(EE_FACULTY));
	return true;
}


static bool testRegisterProcess(void)
{
	/* Just call register_process many times */
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());

	pid_t p = fork();
	ASSERT_TEST(-1 != p);
	if (0 == p) /* child */
	{
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST_CHILD(REGISTRATION_SUCCESS  == register_process());
		exit(0);
	}
	else /* father */ 
	{
		ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
		int childExitInfo;
		ASSERT_TEST(p == waitpid(p, &childExitInfo, 0));
		ASSERT_TEST(WIFEXITED(childExitInfo) && (0 == WEXITSTATUS(childExitInfo)));
		ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
		ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	}
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	ASSERT_TEST(REGISTRATION_SUCCESS  == register_process());
	return true;
}

/*************************************************************************/
/*

   _____             __ _                       _   _             
  / ____|           / _(_)                     | | (_)            
 | |     ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __  
 | |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
 | |___| (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
  \_____\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                           __/ |                                  
                          |___/                                   
*/
/*************************************************************************/
static long NumTestsPassed = 0;

static void red () {
  printf("\033[1;31m");
  fflush(stdout);
}

static void green() {
  printf("\033[0;32m");
  fflush(stdout);
}

static void purple() {
  printf("\033[0;35m");
}

static void yellow () {
  printf("\033[0;33m");
  fflush(stdout);
}

static void reset () {
  printf("\033[0m");
  fflush(stdout);
}

static void printIfSuccess(long num_tests)
{
    if (0 == NumTestsPassed)
    {
        red();
    }
    if (num_tests == NumTestsPassed)
    {
        green();
    }
    else
    {
        yellow();
    }
    
    printf("####  Summary: Passed %ld out of %ld ####\n" ,NumTestsPassed, num_tests);
    reset();
}


#define RUN_COLORFULL_TEST(test, name, id)                  \
    do {                                 \
purple();      printf("Running test# %ld %s ... ", id, name);  reset(); \
      fflush(stdout); \
        if (test()) {                    \
            green();\
            printf("[OK]\n");            \
            reset();\
            ++NumTestsPassed;   \
        } else {    \
            red();\
            printf("[Failed]\n");        \
            reset();\
        }                                \
    } while (0)

/*The functions for the tests should be added here*/
bool (*tests[]) (void) = {
        // testInTheRightOS,
		testSegel,
        testHello,
		testErrSetStatus,
		testErrGetAllCs,
		testGetStatus,
		testGetAllCS_FatherCS_NotRegistered,
		testGetAllCS_FatherEE_NotRegistered,
		testDeepProcesses,
		testBreadthZombiesOnDifferentWaits,
		testGetAllCS_FatherCS,
		testGetAllCS_FatherEE,
		testRegisterProcess,
};

#define NUMBER_TESTS ((long)(sizeof(tests)/sizeof(*tests)))

/*The names of the test functions should be added here*/
const char* testNames[NUMBER_TESTS] = {
        // "Are we in the right OS version?",
		"Given by Segel",
        "Hello World",
		"Set Status - Err check",
		"Get All CS - Err check", 
		"Get Status",
		"Get All CS - Father before fork is CS - not registered",
		"Get All CS - Father before fork is EE - not registered",		
		"Deep processes",
		"Breadth Zombies On Different Waits",
		"Get All CS - Father before fork is CS",
		"Get All CS - Father before fork is EE",
		"Register Process",
};


/*************************************************************************/
int main(int argc, char *argv[]) 
{
    if (argc == 1)
    {
        fprintf(stdout, "Running %ld tests:\n", NUMBER_TESTS);
        for (long test_idx = 0; test_idx < NUMBER_TESTS; ++test_idx)
        {
            RUN_COLORFULL_TEST(tests[test_idx], testNames[test_idx], test_idx);
        }
        printIfSuccess(NUMBER_TESTS);
        return 0;
    }

    if (argc != 2) 
    {
        fprintf(stdout, "Usage: ./test.out <test index>\n");
        return 0;
    }

    long test_idx = strtol(argv[1], NULL, 10);
    if (test_idx < 0 || test_idx >= NUMBER_TESTS) 
    {
        fprintf(stderr, "Invalid test index %ld. \
Test index should be from 0 up to %ld\n", test_idx, NUMBER_TESTS - 1);
        return 0;
    }

    RUN_COLORFULL_TEST(tests[test_idx], testNames[test_idx], test_idx);
    return 0;
}

