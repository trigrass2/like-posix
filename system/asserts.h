
/**
 * @addtogroup system
 *
 * this code was copied from one of the official stm32 firmware demos.
 *
 * @file asserts.h
 * @{
 */
#ifndef ASSERTS_H_
#define ASSERTS_H_

#ifdef __cplusplus
 extern "C" {
#endif

// defined in the build system (makefile, eclipse, jenkins, wherever)
// #define USE_FULL_ASSERT    1

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  */
#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
  #define assert_true(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
#define assert_false(expr) (!(expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
  #define assert_true(expr) ((void)0)
#define assert_false(expr) ((void)0)
#endif /* USE_FULL_ASSERT */


#ifdef __cplusplus
 }
#endif

#endif /* ASSERTS_H_ */

 /**
  * @}
  */
