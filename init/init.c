/*
 * Copyright (C) 2005 Martin Decky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "version.h"
#include <ipc/ipc.h>
#include <ipc/services.h>
#include <ipc/ns.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread.h>
#include <task.h>
#include <psthread.h>
#include <futex.h>
#include <as.h>
#include <ddi.h>
#include <string.h>
#include <errno.h>
#include <kbd.h>
#include <ipc/fb.h>
#include <async.h>
#include <time.h>

int a;
atomic_t ftx;

int __thread stage;

extern void utest(void *arg);
void utest(void *arg)
{
	printf("Uspace thread started.\n");
	if (futex_down(&ftx) < 0)
		printf("Futex failed.\n");
	if (futex_up(&ftx) < 0)
		printf("Futex failed.\n");
		
	printf("%s in good condition.\n", __FUNCTION__);
	
	for (;;)
		;
}

/* test different parameters types and modifiers */
static void test_printf(void)
{
	printf("Simple text.\n");
	printf("Now insert '%s' string.\n","this");
	printf("Signed formats on uns. numbers: '%d', '%+d', '% d', '%u' (,+, ,u)\n", 321, 321, 321, 321);
	printf("Signed formats on sig. numbers: '%d', '%+d', '% d', '%u' (,+, ,u)\n", -321, -321, -321, -321);
	printf("Signed with different sized: '%hhd', '%hd', '%d', '%ld', %lld;\n", -3, -32, -321, -32101l, -3210123ll);
	printf("And now... '%hhd' byte! '%hd' word! '%d' int! \n", 11, 11111, 1111111111);
	printf("Different bases: %#hx, %#hu, %#ho and %#hb\n", 123, 123, 123, 123);
	printf("Different bases signed: %#hx, %#hu, %#ho and %#hb\n", -123, -123, -123, -123);
	printf("'%llX' llX! Another '%llx' llx! \n", 0x1234567887654321ll, 0x1234567887654321ll);
	printf("'%llX' with 64bit value and '%x' with 32 bit value. \n", 0x1234567887654321ll, 0x12345678 );
	printf("'%llx' 64bit, '%x' 32bit, '%hhx' 8bit, '%hx' 16bit, '%llX' 64bit and '%s' string.\n", 0x1234567887654321ll, 0x12345678, 0x12, 0x1234, 0x1234567887654321ull, "Lovely string" );
	
	printf("Thats all, folks!\n");
}

/* test width and precision modifiers */
static void test_printf2(void)
{
	printf(" text 10.8s %*.*s \n", 5, 3, "text");
	printf(" very long text 10.8s %10.8s \n", "very long text");
	printf(" text 8.10s %8.10s \n", "text");
	printf(" very long text 8.10s %8.10s \n", "very long text");

	printf(" char: c '%c', 3.2c '%3.2c', -3.2c '%-3.2c', 2.3c '%2.3c', -2.3c '%-2.3c' \n",'a', 'b', 'c', 'd', 'e' );
	printf(" int: d '%d', 3.2d '%3.2d', -3.2d '%-3.2d', 2.3d '%2.3d', -2.3d '%-2.3d' \n",1, 1, 1, 1, 1 );
	printf(" -int: d '%d', 3.2d '%3.2d', -3.2d '%-3.2d', 2.3d '%2.3d', -2.3d '%-2.3d' \n",-1, -1, -1, -1, -1 );
	printf(" 0xint: x '%x', 5.3x '%#5.3x', -5.3x '%#-5.3x', 3.5x '%#3.5x', -3.5x '%#-3.5x' \n",17, 17, 17, 17, 17 );

}

extern char _heap;
static void test_mremap(void)
{
	printf("Writing to good memory\n");
	as_area_resize(&_heap, 120000, 0);
	printf("%P\n", ((char *)&_heap));
	printf("%P\n", ((char *)&_heap) + 80000);
	*(((char *)&_heap) + 80000) = 10;
	printf("Making small\n");
	as_area_resize(&_heap, 16000, 0);
	printf("Failing..\n");
	*((&_heap) + 80000) = 10;

	printf("memory done\n");
}
/*
static void test_sbrk(void)
{
	printf("Writing to good memory\n");
	printf("Got: %P\n", sbrk(120000));
	printf("%P\n", ((char *)&_heap));
	printf("%P\n", ((char *)&_heap) + 80000);
	*(((char *)&_heap) + 80000) = 10;
	printf("Making small, got: %P\n",sbrk(-120000));
	printf("Testing access to heap\n");
	_heap = 10;
	printf("Failing..\n");
	*((&_heap) + 80000) = 10;

	printf("memory done\n");
}
*/
/*
static void test_malloc(void)
{
	char *data;

	data = malloc(10);
	printf("Heap: %P, data: %P\n", &_heap, data);
	data[0] = 'a';
	free(data);
}
*/


static void test_ping(void)
{
	ipcarg_t result;
	int retval;

	printf("Pinging\n");
	retval = ipc_call_sync(PHONE_NS, NS_PING, 0xbeef,&result);
	printf("Retval: %d - received: %P\n", retval, result);
}

static void got_answer(void *private, int retval, ipc_call_t *data)
{
	printf("Retval: %d...%s...%zX, %zX\n", retval, private,
	       IPC_GET_ARG1(*data), IPC_GET_ARG2(*data));
}
static void test_async_ipc(void)
{
	ipc_call_t data;
	int i;

	printf("Sending ping\n");
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Pong1", got_answer);
	ipc_call_async_2(PHONE_NS, NS_PING, 2, 0xbeefbee4, 
			 "Pong2", got_answer);
	ipc_call_async_2(PHONE_NS, NS_PING, 3, 0xbeefbee4, 
			 "Pong3", got_answer);
	ipc_call_async_2(PHONE_NS, NS_PING, 4, 0xbeefbee4, 
			 "Pong4", got_answer);
	ipc_call_async_2(PHONE_NS, NS_PING, 5, 0xbeefbee4, 
			 "Pong5", got_answer);
	ipc_call_async_2(PHONE_NS, NS_PING, 6, 0xbeefbee4, 
			 "Pong6", got_answer);
	printf("Waiting forever...\n");
	for (i=0; i<100;i++)
		printf(".");
	printf("\n");
	ipc_wait_for_call(&data);
	printf("Received call???\n");
}


static void got_answer_2(void *private, int retval, ipc_call_t *data)
{
	printf("Pong\n");
}
static void test_advanced_ipc(void)
{
	int res;
	ipcarg_t phonead;
	ipc_callid_t callid;
	ipc_call_t data;
	int i;

	printf("Asking 0 to connect to me...\n");
	res = ipc_connect_to_me(0, 1, 2, &phonead);
	printf("Result: %d - phonead: %llu\n", res, phonead);
	for (i=0; i < 100; i++) {
		printf("----------------\n");
		ipc_call_async(PHONE_NS, NS_PING_SVC, 0, "prov",
			       got_answer_2);
		callid = ipc_wait_for_call(&data);
		printf("Received ping\n");
		ipc_answer_fast(callid, 0, 0, 0);
	}
//	callid = ipc_wait_for_call(&data, NULL);
}

static void test_connection_ipc(void)
{
	int res;
	ipcarg_t result;
	int phoneid;

	printf("Starting connect...\n");
	res = ipc_connect_me_to(PHONE_NS, 10, 20);
	printf("Connected: %d\n", res);
	printf("pinging.\n");
	res = ipc_call_sync(res, NS_PING, 0xbeef,&result);
	printf("Retval: %d - received: %X\n", res, result);
	
}

static void test_hangup(void)
{
	int phoneid;
	ipc_call_t data;
	ipc_callid_t callid;
	int i;

	printf("Starting connect...\n");
	phoneid = ipc_connect_me_to(PHONE_NS, 10, 20);
	printf("Phoneid: %d, pinging\n", phoneid);
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Pong1", got_answer);
	printf("Hangin up\n");
	ipc_hangup(phoneid);
	printf("Connecting\n");
	phoneid = ipc_connect_me_to(PHONE_NS, 10, 20);
	printf("Newphid: %d\n", phoneid);
	for (i=0; i < 1000; i++) {
		if ((callid=ipc_trywait_for_call(&data)))
			printf("callid: %d\n");
	}
	printf("New new phoneid: %d\n", ipc_connect_me_to(PHONE_NS, 10, 20));
}

static void test_slam(void)
{
	int i;
	ipc_call_t data;
	ipc_callid_t callid;

	printf("ping");
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Pong1", got_answer);
	printf("slam");
	ipc_call_async_2(PHONE_NS, NS_HANGUP, 1, 0xbeefbee2,
			 "Hang", got_answer);
	printf("ping2\n");
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Ping2", got_answer);
	
	for (i=0; i < 1000; i++) {
		if ((callid=ipc_trywait_for_call(&data)))
			printf("callid: %d\n");
	}
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Pong1", got_answer);
	printf("Closing file\n");
	ipc_hangup(PHONE_NS);
	ipc_call_async_2(PHONE_NS, NS_PING, 1, 0xbeefbee2,
			 "Pong1", got_answer);
	ipc_wait_for_call(&data);
}

static int ptest(void *arg)
{
	stage = 1;
	printf("Pseudo thread stage%d.\n", stage);
	stage++;
	psthread_schedule_next();
	printf("Pseudo thread stage%d.\n", stage);
	stage++;
	psthread_schedule_next();
	printf("Pseudo thread stage%d.\n", stage);
	psthread_schedule_next();
	stage++;
	printf("Pseudo thread stage%d.\n", stage);
	psthread_schedule_next();
	printf("Pseudo thread exiting.\n");
	return 0;	
}

static void test_kbd()
{
	int res;
	ipcarg_t result;
	int phoneid;

	printf("Test: Starting connect...\n");
	while ((phoneid = ipc_connect_me_to(PHONE_NS, SERVICE_KEYBOARD, 0)) < 0) {
	};
	
	printf("Test: Connected: %d\n", res);
	printf("Test: pinging.\n");
	while (1) {
		res = ipc_call_sync(phoneid, KBD_GETCHAR, 0xbeef,&result);
//		printf("Test: Retval: %d - received: %c\n", res, result);
		printf("%c", result);
	}
	
	printf("Test: Hangin up\n");
	ipc_hangup(phoneid);
}

static void test_async_kbd()
{
	int res;
	ipcarg_t result;
	ipc_call_t kbddata;
	int phoneid;
	aid_t aid;

	printf("Test: Starting connect...\n");
	while ((phoneid = ipc_connect_me_to(PHONE_NS, SERVICE_KEYBOARD, 0)) < 0) {
	};
	
	printf("Test: Connected: %d\n", res);
	printf("Test: pinging.\n");

	aid = async_send_2(phoneid, KBD_GETCHAR, 0, 0, &kbddata);
	while (1) {
		if (async_wait_timeout(aid, NULL, 1000000)) {
			printf("^");
			continue;
		}
		printf("%c", IPC_GET_ARG1(kbddata));
		aid = async_send_2(phoneid, KBD_GETCHAR, 0, 0, &kbddata);
	}
	
	printf("Test: Hangin up\n");
	ipc_hangup(phoneid);
}

static void test_pci()
{
	int phone;
	while ((phone = ipc_connect_me_to(PHONE_NS, SERVICE_PCI, 0)) < 0)
		;
	printf("Connected to PCI service through phone %d.\n", phone);
}

static int test_as_area_send()
{
	char *as_area;
	int retval;
	ipcarg_t result;

	as_area = as_area_create((void *)(1024*1024), 16384, AS_AREA_READ | AS_AREA_WRITE);
	if (!as_area) {
		printf("Error creating as_area.\n");
		return 0;
	}

	memcpy(as_area, "Hello world.\n", 14);

	retval = ipc_call_sync_3(PHONE_NS, IPC_M_AS_AREA_SEND, (sysarg_t) as_area, 0, AS_AREA_READ,
		NULL, NULL, NULL);
	if (retval) {
		printf("AS_AREA_SEND failed.\n");
		return 0;
	}
	printf("Done\n");
}

static void test_fb()
{
	int res;
	ipcarg_t result;
	int phoneid;

//	printf("Test: Starting connect...\n");

	phoneid = ipc_connect_me_to(PHONE_NS, SERVICE_VIDEO, 0);

	while ((phoneid = ipc_connect_me_to(PHONE_NS, SERVICE_VIDEO, 0)) < 0) {
		volatile int a;
		for(a=0;a<1048576;a++);
	};
	
//	printf("Test: Connected: %d\n", res);
//	printf("Test: pinging.\n");
	while (1) {
		res = ipc_call_sync(phoneid, FB_GET_VFB, 0xbeef,&result);
//		printf("Test: Retval: %d - received: %c\n", res, result);
//		printf("%c", result);
	}
	
//	printf("Test: Hangin up\n");
	ipc_hangup(phoneid);
}

static void test_time(void)
{
	int rc;
	struct timeval tv;
	struct timezone tz;

	while (1) {
		rc = gettimeofday(&tv, &tz);
		printf("Rc: %d, Secs: %d, Usecs: %d\n", rc, tv.tv_sec,
		       tv.tv_usec);
	}
}

int main(int argc, char *argv[])
{
	pstid_t ptid;
	int tid;
	
//	version_print();

//	test_printf();
//	test_printf2();
//	test_ping();
//	test_async_ipc();
//	test_advanced_ipc();
//	test_connection_ipc();
//	test_hangup();
//	test_slam();
//	test_as_area_send();
//	test_pci();
//	test_kbd();
//	test_time();
	test_async_kbd();
//	test_fb();

	printf("Hello\nThis is Init\n\nBye.");
	

/*	
	printf("Userspace task, taskid=%llX\n", task_get_id());

	futex_initialize(&ftx, 1);
	if (futex_down(&ftx) < 0)
		printf("Futex failed.\n");
	if (futex_up(&ftx) < 0)
		printf("Futex failed.\n");

	if (futex_down(&ftx) < 0)
		printf("Futex failed.\n");

	if ((tid = thread_create(utest, NULL, "utest")) != -1) {
		printf("Created thread tid=%d\n", tid);
	}

	if ((tid = thread_create(utest, NULL, "utest")) != -1) {
		printf("Created thread tid=%d\n", tid);
	}

	int i;

	for (i = 0; i < 50000000; i++)
		;
		
	if (futex_up(&ftx) < 0)
		printf("Futex failed.\n");


	printf("Creating pseudo thread.\n");
	stage = 1;
	ptid = psthread_create(ptest, NULL);
	printf("Main thread stage%d.\n", stage);
	stage++;
	psthread_schedule_next();;
	printf("Main thread stage%d.\n", stage);
	stage++;
	psthread_schedule_next();;
	printf("Main thread stage%d.\n", stage);

	psthread_join(ptid);

	printf("Main thread exiting.\n");
*/

	return 0;
}
