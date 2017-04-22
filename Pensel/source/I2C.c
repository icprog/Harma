/*
 *
 */

#include <stdint.h>

// STM HAL libraries
#include "Drivers/stm32f3xx_hal.h"


#define I2Cx                            I2C1
#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()


/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1

/* Definition for I2Cx's NVIC */
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler

/* Size of Transmission buffers */
#define TX_BUFFER_SIZE                      (10)
#define RX_BUFFER_SIZE                      TX_BUFFER_SIZE

// Our I2C address
#define I2C_ADDRESS        (0x3F)

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 64 MHz */
/* This example use TIMING to 0x00400B27 to reach 1 MHz speed (Rise time = 26ns, Fall time = 2ns) */
// TODO: LOWER I2C speed to 400 kHz
#define I2C_TIMING      0x00400B27

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;


// Buffers used for transmission
uint8_t TX_buffer[TX_BUFFER_SIZE];
uint8_t RX_buffer[RX_BUFFER_SIZE];


ret_t I2C_init(void)
{
    /*##-1- Configure the I2C peripheral ######################################*/
    I2cHandle.Instance             = I2C1;
    I2cHandle.Init.Timing          = I2C_TIMING;
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0xFF;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        fatal_error_handler();
    }

    return RET_OK;
}


ret_t I2C_writeData(uint8_t dev_address, uint8_t mem_address,
                    uint8_t * data_ptr, uint8_t data_len, bool blocking)
{
    // check if we're available to send data
    if (I2C_isBusy()) {
        return RET_BUSY_ERR;
    }

    // make sure the length is possible
    if (data_len > TX_BUFFER_SIZE) {
        return RET_LEN_ERR;
    }
    // copy the data into the TX buffer and send it!
    memcpy(TX_buffer, data_ptr, data_len);
    if (HAL_I2C_Mem_Write_IT(&I2cHandle, (uint16_t)address, (uint16_t)mem_address,
                         I2C_MEMADD_SIZE_8BIT, (uint8_t *)TX_buffer, data_len) != HAL_OK) {
        return RET_COM_ERR;
    }

    // if we're supposed to block, wait here
    if (blocking == true) {
        while ( I2C_isBusy() );
    }

    return RET_OK;
}


ret_t I2C_readData(uint8_t address, uint8_t mem_address, uint8_t * data_ptr, uint8_t data_len)
{
    // check if we're available to send data
    if ( I2C_isBusy() ) {
        return RET_BUSY_ERR;
    }

    // make sure the length is possible
    if (data_len > RX_BUFFER_SIZE) {
        return RET_LEN_ERR;
    }

    // receive that data!
    if (HAL_I2C_Mem_Read_IT(&I2cHandle, (uint16_t)address, (uint16_t)mem_address,
                            I2C_MEMADD_SIZE_8BIT, (uint8_t *)data_ptr, data_len) != HAL_OK ) {
        return RET_COM_ERR;
    }

    // if we're supposed to block, wait here
    while ( I2C_isBusy() );

    return RET_OK;
}


bool I2C_isBusy(void)
{
    return (HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_READY);
}


/**
  * @brief  Tx Transfer completed callback.
  * @param  I2cHandle: I2C handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Toggle LED3: Transfer in transmission process is correct */
    BSP_LED_Toggle(LED3);
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* Txoggle LED3: Transfer in reception process is correct */
    BSP_LED_Toggle(LED3);
}

/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    fatal_error_handler();
}