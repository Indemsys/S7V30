/***************************************************************************//**
* \file cyabs_rtos.h
*
* \brief
* Defines the Cypress RTOS Interface. Provides prototypes for functions that
* allow Cypress libraries to use RTOS resources such as threads, mutexes & timing
* functions in an abstract way. The APIs are implemented
* in the Port Layer RTOS interface which is specific to the RTOS in use.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef INCLUDED_CY_RTOS_INTERFACE_H_
#define INCLUDED_CY_RTOS_INTERFACE_H_

#include "cyabs_rtos_impl.h"
#include <cy_result.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Note, cyabs_rtos_impl.h above is included and is the implementation of some basic
 * types for the abstraction layer.  The types expected to be defined are.
 *
 * cy_thread_t              : typedef from underlying RTOS thread type
 * cy_thread_arg_t          : typedef from the RTOS type that is passed to the
 *                            entry function of a thread.
 * cy_time_t                : count of time in milliseconds
 * cy_rtos_error_t          : typedef from the underlying RTOS error type *
 *
 */


/**
 * \addtogroup group_abstraction_rtos RTOS abstraction
 * \ingroup group_abstraction
 * \{
 * Basic abstraction layer for dealing with RTOSes.
 *
 * \defgroup group_abstraction_rtos_macros Macros
 * \defgroup group_abstraction_rtos_enums Enums
 * \defgroup group_abstraction_rtos_data_structures Data Structures
 * \defgroup group_abstraction_rtos_functions Functions
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************** CONSTANTS **********************************************/

/**
 * \addtogroup group_abstraction_rtos_macros
 * \{
 */

/** Used with RTOS calls that require a timeout.  This implies the call will never timeout. */
#define CY_RTOS_NEVER_TIMEOUT ( (uint32_t)0xffffffffUL )

//
// Note on error strategy.  If the error is a normal part of operation (timeouts, full queues, empty
// queues), the these errors are listed here and the abstraction layer implementation must map from the
// underlying errors to these.  If the errors are special cases, the the error CY_RTOS_GENERAL_ERROR can be
// returns and cy_rtos_last_error() used to retrieve the RTOS specific error message.
//
/** Requested operationd did not complete in the specified time */
#define CY_RTOS_TIMEOUT                     CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 0)
/** The RTOS could not allocate memory for the specified operation */
#define CY_RTOS_NO_MEMORY                   CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 1)
/** An error occured in the RTOS */
#define CY_RTOS_GENERAL_ERROR               CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 2)
/** The Queue is already full and can't accept any more items at this time */
#define CY_RTOS_QUEUE_FULL                  CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 3)
/** The Queue is empty and has nothing to remove */
#define CY_RTOS_QUEUE_EMPTY                 CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 4)
/** A bad argument was passed into the APIs */
#define CY_RTOS_BAD_PARAM                   CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 5)

/** \} group_abstraction_rtos_macros */


/*********************************************** TYPES **********************************************/

/**
 * \addtogroup group_abstraction_rtos_data_structures
 * \{
 */

/**
 * The type of a function that is the entry point for a thread
 *
 * @param[in] arg the argument passed from the thread create call to the entry function
 */
typedef void (*cy_thread_entry_fn_t)(cy_thread_arg_t arg);

/** \} group_abstraction_rtos_data_structures */


/**
 * \addtogroup group_abstraction_rtos_functions
 * \{
 */

/*********************************************** Threads **********************************************/


/** Create a thread with specific thread argument.
 *
 * This function is called to startup a new thread. If the thread can exit, it must call
 * cy_rtos_finish_thread() just before doing so. All created threds that can terminate, either
 * by themselves or forcefully by another thread MUST be joined in order to cleanup any resources
 * that might have been allocated for them.
 *
 * @param[out] thread         Pointer to a variable which will receive the new thread handle
 * @param[in]  entry_function Function pointer which points to the main function for the new thread
 * @param[in]  name           String thread name used for a debugger
 * @param[in]  stack          The buffer to use for the thread stack
 * @param[in]  stack_size     The size of the thread stack in bytes
 * @param[in]  priority       The priority of the thread. Values are operating system specific, but some
 *                            common priority levels are defined:
 *                                CY_THREAD_PRIORITY_LOW
 *                                CY_THREAD_PRIORITY_NORMAL
 *                                CY_THREAD_PRIORITY_HIGH
 * @param[in]  arg            The argument to pass to the new thread
 *
 * @return The status of thread create request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_create_thread(cy_thread_t *thread, cy_thread_entry_fn_t entry_function,
                                       const char *name, void *stack, uint32_t stack_size,
                                       uint32_t prio, cy_thread_arg_t arg);



/** Terminates another thread.
 *
 * This function is called to terminate another thread and reap the resoruces claimed
 * by it thread. This should be called both when forcibly terminating another thread
 * as well as any time a thread can exit on its own. For some RTOS implementations
 * this is not required as the thread resoruces are claimed as soon as it exits. In
 * other cases, this must be called to reclaim resources. Threads that are terminated
 * must still be joined (cy_rtos_join_thread) to ensure their resources are fully
 * cleaned up.
 *
 * @param[in] thread Handle of the thread to terminate
 *
 * @returns The status of the thread terminate. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_terminate_thread(cy_thread_t *thread);

/** Checks if the thread is running
 *
 * This function is called to determine if a thread is running or not.
 *
 * @param[in] thread     handle of the terminated thread to delete
 * @param[out] state     returns true if the thread is running, otherwise false
 *
 * @returns The status of the thread check. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_is_thread_running(cy_thread_t *thread, bool *state);

/** Waits for a thread to complete.
 *
 * This must be called on any thread that can complete to ensure that any resources that
 * were allocated for it are cleaned up.
 *
 * @param[in] thread Handle of the thread to wait for
 *
 * @returns The status of thread join request. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_join_thread(cy_thread_t *thread);


/** Get current thread handle
 *
 * Returns the unique thread handle of the current running thread.
 *
 * @param[out] thread Handle of the current running thread
 *
 * @returns The status of thread join request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_thread_get_handle(cy_thread_t* thread);

/** Suspend current thread until notification is received
 *
 * This function suspends the execution of current thread until it is notified
 * by \ref cy_rtos_thread_set_notification from another thread or ISR, or timed out with
 * specify timeout value
 *
 * @param[in] timeout_ms  Maximum number of milliseconds to wait
 *                        Use the \ref CY_RTOS_NEVER_TIMEOUT constant to wait forever.
 *
 * @returns The status of thread wait. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_TIMEOUT, \ref
 *                                     CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_thread_wait_notification(cy_time_t timeout_ms);


/** Set the thread notification for a thread
 *
 * This function sets the thread notification for the target thread.
 * The target thread waiting for the notification to be set will resume from suspended state.
 *
 * @param[in] thread     Handle of the target thread
 *
 * @returns The status of thread wait. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR,
 *                                      \ref CY_RTOS_BAD_PARAM]
 */
cy_rslt_t cy_rtos_thread_set_notification(cy_thread_t* thread);

/** Get the name of a thread
 *
 * This function returns the name of the target thread
 *
 * @param[in] thread        Handle of the target thread
 *
 * @param[out] thread_name   Will be updated to point to the thread name. Can return
 * NULL in the case of no thread name.
 *
 * @returns The status of getting the thread name. [\ref CY_RSLT_SUCCESS, \ref
 * CY_RTOS_GENERAL_ERROR]
 *
 */
cy_rslt_t cy_rtos_thread_get_name(cy_thread_t* thread, const char** thread_name);


/** Get current thread handle
 *
 * Returns the unique thread handle of the current running thread.
 *
 * @param[out] thread Handle of the current running thread
 *
 * @returns The status of thread join request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_get_thread_handle(thread)         cy_rtos_thread_get_handle(thread)

/*********************************************
 * Scheduler
 ********************************************/

/**
 * \ingroup group_abstraction_rtos_scheduler
 * \{
 */

/** Suspend the scheduler
 *
 * This function suspends the scheduler and makes sure that the code executed after the call does
 * not get interrupted by a task switch.
 * Multiple suspend calls can be done making nesting possible granted
 * that the same number of resume calls are made.
 *
 * @note API functions that have the potential to cause a context switch
 *  must not be called while the scheduler is suspended.
 *
 * @return The status of scheduler suspend request. [\ref CY_RSLT_SUCCESS]
 */
cy_rslt_t cy_rtos_scheduler_suspend(void);

/** Resume the scheduler
 *
 * This function resumes the scheduler after it was suspended.
 * If incorrectly called (called more times than \ref cy_rtos_scheduler_suspend) it fails returning
 * \ref CY_RTOS_BAD_PARAM
 *
 * @return The status of scheduler resume request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_BAD_PARAM]
 */
cy_rslt_t cy_rtos_scheduler_resume(void);

/** \} group_abstraction_rtos_scheduler */

/********************************************* Mutexes ********************************************/

/**
 * \ingroup group_abstraction_rtos_mutex
 * \{
 */

/** Create a mutex which can support recursion or not.
 *
 * Creates a binary mutex which can be used for mutual exclusion to prevent simulatenous
 * access of shared resources. Created mutexes can support priority inheritance if recursive.
 *
 * \note Not all RTOS implementations support non-recursive mutexes. In this case a recursive
 * mutex will be created.
 *
 * @param[out] mutex     Pointer to the mutex handle to be initialized
 * @param[in]  recursive Should the created mutex support recursion or not
 *
 * @return The status of mutex creation request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_mutex_init(cy_mutex_t* mutex, bool recursive);

/** Get a mutex.
 *
 * If the mutex is available, it is acquired and this function returned.
 * If the mutex is not available, the thread waits until the mutex is available
 * or until the timeout occurs.
 *
 * @note This function must not be called from an interrupt context as it may block.
 *
 * @param[in] mutex       Pointer to the mutex handle
 * @param[in] timeout_ms  Maximum number of milliseconds to wait while attempting to get
 *                        the mutex. Use the \ref CY_RTOS_NEVER_TIMEOUT constant to wait forever.
 *
 * @return The status of the get mutex. Returns timeout if mutex was not acquired
 *                    before timeout_ms period. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_TIMEOUT, \ref
 *                    CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_mutex_get(cy_mutex_t* mutex, cy_time_t timeout_ms);

/** Set a mutex.
 *
 * The mutex is released allowing any other threads waiting on the mutex to
 * obtain the semaphore.
 *
 * @param[in] mutex   Pointer to the mutex handle
 *
 * @return The status of the set mutex request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 *
 */
cy_rslt_t cy_rtos_mutex_set(cy_mutex_t* mutex);

/** Deletes a mutex.
 *
 * This function frees the resources associated with a sempahore.
 *
 * @param[in] mutex Pointer to the mutex handle
 *
 * @return The status to the delete request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_mutex_deinit(cy_mutex_t* mutex);

/** \} group_abstraction_rtos_mutex */


/** Create a mutex which can support recursion.
 *
 * Creates a binary mutex which can be used for mutual exclusion to prevent simulatenous
 * access of shared resources. Created mutexes can support priority inheritance if recursive.
 *
 * \note Not all RTOS implementations support non-recursive mutexes. In this case a recursive
 * mutex will be created.
 *
 * @param[out] mutex     Pointer to the mutex handle to be initialized
 *
 * @return The status of mutex creation request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_init_mutex(mutex) cy_rtos_mutex_init(mutex, true)

/** Get a mutex.
 *
 * If the mutex is available, it is acquired and this function returned.
 * If the mutex is not available, the thread waits until the mutex is available
 * or until the timeout occurs.
 *
 * @note This function must not be called from an interrupt context as it may block.
 *
 * @param[in] mutex       Pointer to the mutex handle
 * @param[in] timeout_ms  Maximum number of milliseconds to wait while attempting to get
 *                        the mutex. Use the \ref CY_RTOS_NEVER_TIMEOUT constant to wait forever.
 *
 * @return The status of the get mutex. Returns timeout if mutex was not acquired
 *                    before timeout_ms period. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_TIMEOUT, \ref
 *                    CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_get_mutex(mutex, timeout_ms) cy_rtos_mutex_get(mutex, timeout_ms)

/** Set a mutex.
 *
 * The mutex is released allowing any other threads waiting on the mutex to
 * obtain the semaphore.
 *
 * @param[in] mutex   Pointer to the mutex handle
 *
 * @return The status of the set mutex request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 *
 */
#define cy_rtos_set_mutex(mutex)       cy_rtos_mutex_set(mutex)

/** Deletes a mutex.
 *
 * This function frees the resources associated with a sempahore.
 *
 * @param[in] mutex Pointer to the mutex handle
 *
 * @return The status to the delete request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_deinit_mutex(mutex)    cy_rtos_mutex_deinit(mutex)


/*********************************************** Semaphores **********************************************/

/**
 * Create a semaphore
 *
 * This is basically a counting semaphore.
 *
 * @param[in,out] semaphore  Pointer to the semaphore handle to be initialized
 * @param[in] maxcount       The maximum count for this semaphore
 * @param[in] initcount      The initial count for this sempahore
 *
 * @return The status of the sempahore creation. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_init_semaphore(cy_semaphore_t *semaphore, char* name, uint32_t maxcount, uint32_t initcount);

/**
 * Get/Acquire a semaphore
 *
 * If the semaphore count is zero, waits until the semaphore count is greater than zero.
 * Once the semaphore count is greater than zero, this function decrements
 * the count and return.  It may also return if the timeout is exceeded.
 *
 * @param[in] semaphore   Pointer to the semaphore handle
 * @param[in] timeout_ms  Maximum number of milliseconds to wait while attempting to get
 *                        the semaphore. Use the NEVER_TIMEOUT constant to wait forever. Must
 *                        be zero is in_isr is true
 * @param[in] in_isr      true if we are trying to get the semaphore from with an ISR
 * @return The status of get semaphore operation [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t *semaphore, cy_time_t timeout_ms, bool in_isr);

/**
 * Set/Release a semaphore
 *
 * Increments the semaphore count, up to the maximum count for this semaphore.
 *
 * @param[in] semaphore   Pointer to the semaphore handle
 * @param[in] in_isr      Value of true indicates calling from interrupt context
 *                        Value of false indicates calling from normal thread context
 * @return The status of set semaphore operation [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t *semaphore, bool in_isr);

/**
 * Get the count of a semaphore.
 *
 * Gets the number of available tokens on the semaphore.
 *
 * @param[in]  semaphore   Pointer to the semaphore handle
 * @param[out] count       Pointer to the return count
 * @return The status of get semaphore count operation [\ref CY_RSLT_SUCCESS, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_semaphore_get_count(cy_semaphore_t* semaphore, size_t* count);

/**
 * Deletes a sempahore
 *
 * This function frees the resources associated with a sempahore.
 *
 * @param[in] semaphore   Pointer to the sempahore handle
 *
 * @return The status of semaphore deletion [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t *semaphore);

/*********************************************** Events **********************************************/

/** Create an event.
 *
 * This is an event which can be used to signal a set of threads
 * with a 32 bit data element.
 *
 * @param[in,out] event Pointer to the event handle to be initialized
 *
 * @return The status of the event initialization request.
 *         [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_init_event(cy_event_t *evnt, char *name) ;

/** Set the event flag bits.
 *
 * This is an event which can be used to signal a set of threads
 * with a 32 bit data element. Any threads waiting on this event are released
 *
 * @param[in] event  Pointer to the event handle
 * @param[in] bits   The value of the 32 bit flags
 * @param[in] in_isr If true, this is called from an ISR, otherwise from a thread
 *
 * @return The status of the set request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_setbits_event(cy_event_t *evnt, uint32_t bits, bool in_isr)  ;

/**
 * Clear the event flag bits
 *
 * This function clears bits in the event.
 *
 * @param[in] event   Pointer to the event handle
 * @param[in] bits    Any bits set in this value, will be cleared in the event.
 * @param[in] in_isr  if true, this is called from an ISR, otherwise from a thread
 *
 * @return The status of the clear flags request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_clearbits_event(cy_event_t *evnt, uint32_t bits, bool in_isr)  ;

/** Get the event bits.
 *
 * Returns the current bits for the event.
 *
 * @param[in]  event Pointer to the event handle
 * @param[out] bits  pointer to receive the value of the event flags
 *
 * @return The status of the get request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_getbits_event(cy_event_t *evnt, uint32_t *bits) ;

/** Wait for the event and return bits.
 *
 * Waits for the event to be set and then returns the bits assocaited
 * with the event, or waits for the given timeout period.
 * @note This function returns if any bit in the set is set.
 *
 * @param[in] event     Pointer to the event handle
 * @param[in,out] bits  pointer to receive the value of the event flags
 * @param[in] clear     if true, clear any bits set that cause the wait to return
 *                      if false, do not clear bits
 * @param[in] all       if true, all bits in the initial bits value must be set to return
 *                      if false, any one bit in the initial bits value must be set to return
 * @param[in] timeout   The amount of time to wait in milliseconds
 *
 * @return The status of the wait for event request. [CY_RSLT_SUCCESS, CY_RTOS_NO_MEMORY, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_waitbits_event(cy_event_t *evnt, uint32_t *bits, bool clear, bool all, cy_time_t timeout_ms) ;

/** Deinitialize a event.
 *
 * This function frees the resources associated with an event.
 *
 * @param[in] event Pointer to the event handle
 *
 * @return The status of the deletion request. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_deinit_event(cy_event_t *evnt) ;

/********************************************* Queues *********************************************/

/**
 * \ingroup group_abstraction_rtos_queue
 * \{
 */

/** Create a queue.
 *
 * This is a queue of data where entries are placed on the back of the queue
 * and removed from the front of the queue.
 *
 * @param[out] queue    Pointer to the queue handle
 * @param[in]  length   The maximum length of the queue in items
 * @param[in]  itemsize The size of each item in the queue.
 *
 * @return The status of the init request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_queue_init(cy_queue_t* queue, const char *name,  size_t length, size_t itemsize);

/** Put an item in a queue.
 *
 * This function puts an item in the queue. The item is copied
 * into the queue using a memory copy and the data pointed to by item_ptr
 * is no longer referenced once the call returns.
 *
 * @note If in_isr is true, timeout_ms must be zero.
 *
 * @param[in] queue      Pointer to the queue handle
 * @param[in] item_ptr   Pointer to the item to place in the queue
 * @param[in] timeout_ms The time to wait to place the item in the queue
 *
 * @return The status of the put request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR, \ref CY_RTOS_QUEUE_FULL]
 */
cy_rslt_t cy_rtos_queue_put(cy_queue_t* queue, const void* item_ptr, cy_time_t timeout_ms);

/** Gets an item in a queue.
 *
 * This function gets an item from the queue. The item is copied
 * out of the queue into the memory provide by item_ptr. This space must be
 * large enough to hold a queue entry as defined when the queue was initialized.
 *
 * @note If in_isr is true, timeout_ms must be zero.
 *
 * @param[in] queue      Pointer to the queue handle
 * @param[in] item_ptr   Pointer to the memory for the item from the queue
 * @param[in] timeout_ms The time to wait to get an item from the queue
 *
 * @return The status of the get request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR, \ref CY_RTOS_QUEUE_EMPTY]
 */
cy_rslt_t cy_rtos_queue_get(cy_queue_t* queue, void* item_ptr, cy_time_t timeout_ms);

/** Return the number of items in the queue.
 *
 * This function returns the number of items currently in the queue.
 *
 * @param[in]  queue       Pointer to the queue handle
 * @param[out] num_waiting Pointer to the return count
 *
 * @return The status of the count request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_queue_count(cy_queue_t* queue, size_t* num_waiting);

/** Return the amount of empty space in the queue.
 *
 * This function returns the amount of empty space in the
 * queue. For instance, if the queue was created with 10 entries max and there
 * are currently 2 entries in the queue, this will return 8.
 *
 * @param[in]  queue      Pointer to the queue handle
 * @param[out] num_spaces Pointer to the return count.
 *
 * @return The status of the space request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_queue_space(cy_queue_t* queue, size_t* num_spaces);

/** Reset the queue.
 *
 * This function sets the queue to empty.
 *
 * @param[in] queue pointer to the queue handle
 *
 * @return The status of the reset request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_queue_reset(cy_queue_t* queue);

/** Deinitialize the queue handle.
 *
 * This function de-initializes the queue and returns all
 * resources used by the queue.
 *
 * @param[in] queue Pointer to the queue handle
 *
 * @return The status of the deinit request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
cy_rslt_t cy_rtos_queue_deinit(cy_queue_t* queue);

/** \} group_abstraction_rtos_queue */

/**
 * \ingroup group_abstraction_rtos_queue
 * \{
 */

/** Create a queue.
 *
 * This is a queue of data where entries are placed on the back of the queue
 * and removed from the front of the queue.
 *
 * @param[out] queue    Pointer to the queue handle
 * @param[in]  length   The maximum length of the queue in items
 * @param[in]  itemsize The size of each item in the queue.
 *
 * @return The status of the init request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_init_queue(queue, name,  length, itemsize) cy_rtos_queue_init(queue, name, length, itemsize)

/** Put an item in a queue.
 *
 * This function puts an item in the queue. The item is copied
 * into the queue using a memory copy and the data pointed to by item_ptr
 * is no longer referenced once the call returns.
 *
 * @note If in_isr is true, timeout_ms must be zero.
 *
 * @param[in] queue      Pointer to the queue handle
 * @param[in] item_ptr   Pointer to the item to place in the queue
 * @param[in] timeout_ms The time to wait to place the item in the queue
 * @param[in] in_isr     If true this is being called from within and ISR
 *
 * @return The status of the put request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR, \ref CY_RTOS_QUEUE_FULL]
 */
#define cy_rtos_put_queue(queue, item_ptr, timeout_ms, in_isr) cy_rtos_queue_put(queue, item_ptr, timeout_ms)

/** Gets an item in a queue.
 *
 * This function gets an item from the queue. The item is copied
 * out of the queue into the memory provide by item_ptr. This space must be
 * large enough to hold a queue entry as defined when the queue was initialized.
 *
 * @note If in_isr is true, timeout_ms must be zero.
 * @note A value of CY_RTOS_NEVER_TIMEOUT assigned to timeout_ms will cause the task to block
 * indefinitely (without a timeout).
 *
 * @param[in] queue      Pointer to the queue handle
 * @param[in] item_ptr   Pointer to the memory for the item from the queue
 * @param[in] timeout_ms The time to wait to get an item from the queue
 * @param[in] in_isr     If true this is being called from within an ISR
 *
 * @return The status of the get request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR, \ref CY_RTOS_QUEUE_EMPTY]
 */
#define cy_rtos_get_queue(queue, item_ptr, timeout_ms, in_isr) cy_rtos_queue_get(queue, item_ptr, timeout_ms)

/** Return the number of items in the queue.
 *
 * This function returns the number of items currently in the queue.
 *
 * @param[in]  queue       Pointer to the queue handle
 * @param[out] num_waiting Pointer to the return count
 *
 * @return The status of the count request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_count_queue(queue, num_waiting) cy_rtos_queue_count(queue, num_waiting)

/** Return the amount of empty space in the queue.
 *
 * This function returns the amount of empty space in the
 * queue. For instance, if the queue was created with 10 entries max and there
 * are currently 2 entries in the queue, this will return 8.
 *
 * @param[in]  queue      Pointer to the queue handle
 * @param[out] num_spaces Pointer to the return count.
 *
 * @return The status of the space request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_space_queue(queue, num_spaces) cy_rtos_queue_space(queue, num_spaces)

/** Reset the queue.
 *
 * This function sets the queue to empty.
 *
 * @param[in] queue pointer to the queue handle
 *
 * @return The status of the reset request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_reset_queue(queue) cy_rtos_queue_reset(queue)

/** Deinitialize the queue handle.
 *
 * This function de-initializes the queue and returns all
 * resources used by the queue.
 *
 * @param[in] queue Pointer to the queue handle
 *
 * @return The status of the deinit request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_deinit_queue(queue) cy_rtos_queue_deinit(queue)

/** \} group_abstraction_rtos_queue */


/*********************************************** Time **********************************************/

/** Gets time in milliseconds since RTOS start.
 *
 * @note Since this is only 32 bits, it will roll over every 49 days, 17 hours, 2 mins, 47.296 seconds
 *
 * @param[out] tval Pointer to the struct to populate with the RTOS time
 *
 * @returns Time in milliseconds since the RTOS started.
 */
extern cy_rslt_t cy_rtos_get_time(cy_time_t *tval);

/** Delay for a number of milliseconds.
 *
 * Processing of this function depends on the minimum sleep
 * time resolution of the RTOS. The current thread should sleep for
 * the longest period possible which is less than the delay required,
 * then makes up the difference with a tight loop.
 *
 * @param[in] num_ms The number of miliseconds to delay for
 *
 * @return The status of the creation request. [CY_RSLT_SUCCESS, CY_RTOS_GENERAL_ERROR]
 */
extern cy_rslt_t cy_rtos_delay_milliseconds(cy_time_t num_ms);

/** \} group_abstraction_rtos_functions */

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ifndef INCLUDED_CY_RTOS_INTERFACE_H_ */

