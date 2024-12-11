/*
 * Copyright (c) 2024 Roman Vasut
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

#include <pcut/pcut.h>
#include <uuid.h>
#include <stdbool.h>
#include <stdio.h>
#include <str.h>
#include <ctype.h>
#include <stdatomic.h>
#include <barrier.h>
#include "../generic/private/thread.h"

#define SLEEP_SEC 1

static atomic_int done_count;

static void reset_coutner()
{
	atomic_store(&done_count, 0);
	memory_barrier();
}

static int get_counter()
{
	int count = atomic_load(&done_count);
	memory_barrier();
	return count;
}

static void incr_counter()
{
	atomic_fetch_add(&done_count, 1);
	memory_barrier();
}

static void test_thread_sleeper(void *arg)
{
	thread_sleep(SLEEP_SEC);
	incr_counter();
}

PCUT_INIT;

PCUT_TEST_SUITE(thread);

PCUT_TEST(thread_join_once, PCUT_TEST_SET_TIMEOUT(SLEEP_SEC + 5))
{
	thread_id_t thread_id;

	reset_coutner();
	errno_t rc = thread_create(test_thread_sleeper, NULL, "thread-sleeper", &thread_id);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = thread_join(thread_id);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_INT_EQUALS(1, get_counter());
}

PCUT_TEST(thread_join_twice, PCUT_TEST_SET_TIMEOUT(SLEEP_SEC + 5))
{
	thread_id_t thread_id;

	reset_coutner();
	errno_t rc = thread_create(test_thread_sleeper, NULL, "thread-sleeper", &thread_id);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = thread_join(thread_id);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = thread_join(thread_id);
	PCUT_ASSERT_ERRNO_VAL(ENOENT, rc);

	PCUT_ASSERT_INT_EQUALS(1, get_counter());
}

static errno_t nested_join_result;

static void test_thread_joiner(void *arg)
{
	thread_id_t *target_tid = arg;
	nested_join_result = EINVAL;
	nested_join_result = thread_join(*target_tid);
}

PCUT_TEST(thread_join_nested, PCUT_TEST_SET_TIMEOUT(SLEEP_SEC + 5))
{
	thread_id_t sleeper_tid;
	thread_id_t joiner_tid;

	reset_coutner();
	errno_t rc = thread_create(test_thread_sleeper, NULL, "thread-sleeper", &sleeper_tid);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = thread_create(test_thread_joiner, &sleeper_tid, "thread-joiner", &joiner_tid);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = thread_join(joiner_tid);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	PCUT_ASSERT_ERRNO_VAL(EOK, nested_join_result);
	PCUT_ASSERT_INT_EQUALS(1, get_counter());
}

PCUT_TEST(thread_join_current, PCUT_TEST_SET_TIMEOUT(SLEEP_SEC + 5))
{
	thread_id_t current_tid = thread_get_id();

	errno_t rc = thread_join(current_tid);
	PCUT_ASSERT_ERRNO_VAL(EINVAL, rc);
}

PCUT_EXPORT(thread);
