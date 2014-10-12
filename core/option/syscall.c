/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs R0.08                   */
/* (C)ChaN, 2010                                                          */
/*------------------------------------------------------------------------*/

#include <stdlib.h>		/* ANSI memory controls */
#include <malloc.h>		/* ANSI memory controls */

#include "../ff.h"

#if _FS_REENTRANT

// volatile int* lock_owner = NULL;

/*------------------------------------------------------------------------*/
// Create a Synchronization Object
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to create a new
/  synchronization object, such as semaphore and mutex. When a false is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* true:Function succeeded, false:Could not create due to any error */
	BYTE vol,			/* Corresponding logical drive being processed */
	_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
	// where _SYNC_t is uint8_t*
	int ret = 0;
//	*sobj = malloc(sizeof(uint8_t));
//	**sobj = 0;
//	if(*sobj)
//		ret = 1;

	// // where _SYNC_t is lock_t*
	// int ret = 0;
	// lock_owner = (volatile int*)(vol+1000);
	// *sobj = malloc(sizeof(lock_t));
	// if(sched_init_lock(*sobj))
	// 	ret = sched_own_lock(*sobj, lock_owner);

//	*sobj = CreateMutex(NULL, false, NULL);					/* Win32 */
//	ret = (*sobj != INVALID_HANDLE_VALUE) ? true : false;

//	*sobj = SyncObjects[vol];	/* uITRON (give a static created sync object) */
//	ret = true;					/* The initial value of the semaphore must be 1. */

//	*sobj = OSMutexCreate(0, &err);				/* uC/OS-II */
//	ret = (err == OS_NO_ERR) ? true : false;

	(void)vol;
	*sobj = xSemaphoreCreateMutex();			/* FreeRTOS */
	ret = (*sobj != NULL) ? true : false;

	return ret;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a synchronization
/  object that created with ff_cre_syncobj function. When a false is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* true:Function succeeded, false:Could not delete due to any error */
	_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	// where _SYNC_t is uint8_t*
	int ret;
//	if(sobj)
//		free(sobj);
//	ret = 1;

	// // where _SYNC_t is lock_t*
	// int ret = 1;
	// lock_owner = NULL;
	// if(sobj)
	// 	free(sobj);

//	ret = CloseHandle(sobj);	// Win32

//	ret = true;					// uITRON (nothing to do)

//	OSMutexDel(sobj, OS_DEL_ALWAYS, &err);		/* uC/OS-II */
//	ret = (err == OS_NO_ERR) ? true : false;

	(void)sobj;
	ret = true;					/* FreeRTOS (nothing to do) */

	return ret;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a false is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* true:Got a grant to access the volume, false:Could not get a grant */
	_SYNC_t sobj	/* Sync object to wait */
)
{
	// where _SYNC_t is uint8_t*
	int ret;
//	if(!(*sobj))
//	{
//		*sobj = 1;
//		ret = 1;
//	}
//	else
//		ret = 0;

	// // where _SYNC_t is lock_t*
	// int ret = sched_acquire_lock(sobj, lock_owner);

//	ret = sched_acquire_lock(sobj, (void*)1);

//	ret = (WaitForSingleObject(sobj, _FS_TIMEOUT) == WAIT_OBJECT_0) ? true : false;	/* Win32 */

//	ret = (wai_sem(sobj) == E_OK) ? true : false;	/* uITRON */

//	OSMutexPend(sobj, _FS_TIMEOUT, &err));			/* uC/OS-II */
//	ret = (err == OS_NO_ERR) ? true : false;

	ret = (xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE) ? true : false;	/* FreeRTOS */

	return ret;
}

/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t sobj	/* Sync object to be signaled */
)
{
	// where _SYNC_t is uint8_t*
//	*sobj = 0;

	// // where _SYNC_t is lock_t*
	// sched_release_lock(sobj, lock_owner);

//	sched_release_lock(sobj, (void*)1);

//	ReleaseMutex(sobj);		/* Win32 */

//	sig_sem(sobj);			/* uITRON */

//	OSMutexPost(sobj);		/* uC/OS-II */

	xSemaphoreGive(sobj);	/* FreeRTOS */

}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT size		/* Number of bytes to allocate */
)
{
	return pvPortMalloc(size);
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
	void* mblock	/* Pointer to the memory block to free */
)
{
	vPortFree(mblock);
}

#endif
