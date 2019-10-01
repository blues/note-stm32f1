// Copyright 2018 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "main.h"
#include "stm32f1xx_it.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern void MY_UART_IRQHandler(UART_HandleTypeDef *huart);

// Cortex-M4 non-maskable interrupt
void NMI_Handler(void) {
}

// Cortex-M4 hardware fault interrupt
void HardFault_Handler(void) {
    while (1) ;
}

// Memory management fault
void MemManage_Handler(void) {
    while (1) ;
}

// Prefetch or memory access fault
void BusFault_Handler(void) {
    while (1) ;
}

// Undefined instruction or illegal state
void UsageFault_Handler(void) {
    while (1) ;
}

// System service call via SWI instruction
void SVC_Handler(void) {
}

// Debug monitor
void DebugMon_Handler(void) {
}

// Pendable request for system service.
void PendSV_Handler(void) {
}

// System tick timer
void SysTick_Handler(void) {
    HAL_IncTick();
}

// IC21 event interrupt
void I2C1_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c1);
}

// I2C1 error interrupt
void I2C1_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&hi2c1);
}

// USART1 global interrupt
void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
    MY_UART_IRQHandler(&huart1);
}

// Interrupt handlers
void EXTI0_IRQHandler( void ) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void EXTI1_IRQHandler( void ) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
void EXTI2_IRQHandler( void ) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
void EXTI3_IRQHandler( void ) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
void EXTI4_IRQHandler( void ) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}
// GPIO handler, enhanced from the base ST handler in a way that enables us to distinguish from the multiple
// pins that sharing the same EXTI.
void MY_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin) {
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET) {
        uint16_t GPIO_Line = GPIO_Pin & EXTI->PR;
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
        HAL_GPIO_EXTI_Callback(GPIO_Line);
    }
}
void EXTI9_5_IRQHandler( void ) {
  MY_GPIO_EXTI_IRQHandler(GPIO_PIN_9|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_5);
}
void EXTI15_10_IRQHandler( void ) {
  MY_GPIO_EXTI_IRQHandler(GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10);
}
