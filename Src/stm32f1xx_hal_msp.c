// Copyright 2018 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "main.h"

// Initialize global peripheral init
void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    // NOJTAG: JTAG-DP Disabled and SW-DP Enabled
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

// Initialize all I2C ports
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {

    if (hi2c->Instance==I2C1) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        //*I2C1 GPIO Configuration
        // PB8     ------> I2C1_SCL
        // PB9     ------> I2C1_SDA
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_I2C1_ENABLE();

        // Peripheral clock enable
        __HAL_RCC_I2C1_CLK_ENABLE();

        // I2C1 interrupt Init
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

    }

}

// Deinitialize all I2C ports
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c) {

    if (hi2c->Instance==I2C1) {

        // Peripheral clock disable
        __HAL_RCC_I2C1_CLK_DISABLE();

        //*I2C1 GPIO Configuration
        // PB8     ------> I2C1_SCL
        // PB9     ------> I2C1_SDA
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

        // I2C1 interrupt DeInit
        HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);

    }

}

// Initialize all UART ports
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {

    if (huart->Instance==USART1) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        // Peripheral clock enable
        __HAL_RCC_USART1_CLK_ENABLE();

        // USART1 GPIO Configuration
        // PA9     ------> USART1_TX
        // PA10     ------> USART1_RX
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // USART1 interrupt Init
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

    }

}


// Deinitialize all UARTs
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {

    // Handle USART1
    if (huart->Instance==USART1) {

        // Peripheral clock disable
        __HAL_RCC_USART1_CLK_DISABLE();

        // USART1 GPIO Configuration
        // PA9     ------> USART1_TX
        // PA10     ------> USART1_RX
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

        // USART1 interrupt DeInit
        HAL_NVIC_DisableIRQ(USART1_IRQn);

    }

}
