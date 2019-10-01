// Copyright 2018 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "note.h"

// Choose whether to use I2C or SERIAL for the Notecard
#define NOTECARD_USE_I2C    false

// I/O device handles
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
bool i2c1Initialized = false;
bool uart1Initialized = false;

// Data used for Notecard I/O functions
static char serialInterruptBuffer[1];
static volatile size_t serialFillIndex = 0;
static volatile size_t serialDrainIndex = 0;
static uint32_t serialOverruns = 0;
static char serialBuffer[512];

// Forwards
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_USART1_UART_Init(void);
void noteSerialReset(void);
void noteSerialTransmit(uint8_t *text, size_t len, bool flush);
bool noteSerialAvailable(void);
char noteSerialReceive(void);
void noteI2CReset(void);
const char *noteI2CTransmit(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size);
const char *noteI2CReceive(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size, uint32_t *avail);

// Main entry point
int main(void) {

    // Initialize peripherals
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
#ifdef EVENT_TIMER
    MX_LPTIM1_Init();
#endif

    // Register callbacks with note-c subsystem that it needs for I/O, memory, timer
    NoteSetFn(malloc, free, delay, millis);

    // Register callbacks for Notecard I/O
#if NOTECARD_USE_I2C
    NoteSetFnI2C(NOTE_I2C_ADDR_DEFAULT, NOTE_I2C_MAX_DEFAULT, noteI2CReset, noteI2CTransmit, noteI2CReceive);
#else
    NoteSetFnSerial(noteSerialReset, noteSerialTransmit, noteSerialAvailable, noteSerialReceive);
#endif

    // Use this method of invoking main app code so that we can re-use familiar Arduino examples
    setup();
    while (true)
        loop();

}

// System clock configuration
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Initializes the CPU, AHB and APB busses clocks
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    // Initializes the CPU, AHB and APB busses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
        |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        Error_Handler();

}


// I2C1 Initialization
void MX_I2C1_Init(void) {

    // Exit if already done
    if (i2c1Initialized)
        return;
    i2c1Initialized = true;

// Primary initialization
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        Error_Handler();

}

// I2C1 De-initialization
void MX_I2C1_DeInit(void) {

    // Exit if already done
    if (!i2c1Initialized)
        return;
    i2c1Initialized = false;

    // Deinitialize
    HAL_I2C_DeInit(&hi2c1);

}

// USART1 Initialization
void MX_USART1_UART_Init(void) {

    // Exit if already done
    if (uart1Initialized)
        return;
    uart1Initialized = true;

    // Primary initialization
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();

    // Reset our buffer management
    serialFillIndex = serialDrainIndex = serialOverruns = 0;

    // Unused, but included for documentation
    ((void)(serialOverruns));

    // Start the inbound receive
    HAL_UART_Receive_IT(&huart1, (uint8_t *) &serialInterruptBuffer, sizeof(serialInterruptBuffer));

}

// USART1 IRQ handler
void MY_UART_IRQHandler(UART_HandleTypeDef *huart) {

    // See if the transfer is completed
    if (huart->RxXferCount == 0) {
        if (serialFillIndex < sizeof(serialBuffer)) {
            if (serialFillIndex+1 == serialDrainIndex)
                serialOverruns++;
            else
                serialBuffer[serialFillIndex++] = serialInterruptBuffer[0];
        } else {
            if (serialDrainIndex == 1)
                serialOverruns++;
            else {
                serialBuffer[0] = serialInterruptBuffer[0];
                serialFillIndex = 1;
            }
        }
    }

    // Start another receive
    HAL_UART_Receive_IT(&huart1, (uint8_t *) &serialInterruptBuffer, sizeof(serialInterruptBuffer));

}

// USART1 De-initialization
void MX_USART1_UART_DeInit(void) {

    // Exit if already done
    if (!uart1Initialized)
        return;
    uart1Initialized = false;

    // Deinitialize
    HAL_UART_DeInit(&huart1);

}

// GPIO initialization
void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : B1_Pin */
    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitStruct.Pin = LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

// Called when a GPIO interrupt occurs
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

    // Handle the button
#ifdef EVENT_BUTTON
    if ((GPIO_Pin & GPIO_BUTTON_PIN) != 0)
        event(EVENT_BUTTON);
#endif

}

// Primary HAL error handler
void Error_Handler(void) {
}

// Primary Assertion handler
#ifdef  USE_FULL_ASSERT
void assert_failed(char *file, uint32_t line) {
}
#endif

// Computationally-delay the specified number of milliseconds
void delay(uint32_t ms) {
    HAL_Delay(ms);
}

// Get the number of app milliseconds since boot (this will wrap)
long unsigned int millis() {
    return (long unsigned int) HAL_GetTick();
}

// Serial port reset procedure, called before any I/O and called again upon I/O error
void noteSerialReset() {
    MX_USART1_UART_DeInit();
    MX_USART1_UART_Init();
}

// Serial write data function
void noteSerialTransmit(uint8_t *text, size_t len, bool flush) {
    HAL_UART_Transmit(&huart1, text, len, 5000);
}

// Serial "is anything available" function, which does a read-ahead for data into a serial buffer
bool noteSerialAvailable() {
    return (serialFillIndex != serialDrainIndex);
}

// Blocking serial read a byte function (generally only called if known to be available)
char noteSerialReceive() {
    char data;
    while (!noteSerialAvailable()) ;
    if (serialDrainIndex < sizeof(serialBuffer))
        data = serialBuffer[serialDrainIndex++];
    else {
        data = serialBuffer[0];
        serialDrainIndex = 1;
    }
    return data;
}

// I2C reset procedure, called before any I/O and called again upon I/O error
void noteI2CReset() {
    MX_I2C1_DeInit();
    MX_I2C1_Init();
}

// Transmits in master mode an amount of data, in blocking mode.     The address
// is the actual address; the caller should have shifted it right so that the
// low bit is NOT the read/write bit. An error message is returned, else NULL if success.
const char *noteI2CTransmit(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size) {
    char *errstr = NULL;
    int writelen = sizeof(uint8_t) + Size;
    uint8_t *writebuf = malloc(writelen);
    if (writebuf == NULL) {
        errstr = "i2c: insufficient memory (write)";
    } else {
        writebuf[0] = Size;
        memcpy(&writebuf[1], pBuffer, Size);
        HAL_StatusTypeDef err_code = HAL_I2C_Master_Transmit(&hi2c1, DevAddress<<1, writebuf, writelen, 250);
        free(writebuf);
        if (err_code != HAL_OK) {
            errstr = "i2c: write error";
        }
    }
    return errstr;
}

// Receives in master mode an amount of data in blocking mode. An error mesage returned, else NULL if success.
const char *noteI2CReceive(uint16_t DevAddress, uint8_t* pBuffer, uint16_t Size, uint32_t *available) {
    const char *errstr = NULL;
    HAL_StatusTypeDef err_code;

    // Retry transmit errors several times, because it's harmless to do so
    for (int i=0; i<3; i++) {
        uint8_t hdr[2];
        hdr[0] = (uint8_t) 0;
        hdr[1] = (uint8_t) Size;
        HAL_StatusTypeDef err_code = HAL_I2C_Master_Transmit(&hi2c1, DevAddress<<1, hdr, sizeof(hdr), 250);
        if (err_code == HAL_OK) {
            errstr = NULL;
            break;
        }
        errstr = "i2c: write error";
    }

    // Only receive if we successfully began transmission
    if (errstr == NULL) {

        int readlen = Size + (sizeof(uint8_t)*2);
        uint8_t *readbuf = malloc(readlen);
        if (readbuf == NULL) {
            errstr = "i2c: insufficient memory (read)";
        } else {
            err_code = HAL_I2C_Master_Receive(&hi2c1, DevAddress<<1, readbuf, readlen, 10);
            if (err_code != HAL_OK) {
                errstr = "i2c: read error";
            } else {
                uint8_t availbyte = readbuf[0];
                uint8_t goodbyte = readbuf[1];
                if (goodbyte != Size) {
                    errstr = "i2c: incorrect amount of data";
                } else {
                    *available = availbyte;
                    memcpy(pBuffer, &readbuf[2], Size);
                }
            }
            free(readbuf);
        }
    }

    // Done
    return errstr;

}
