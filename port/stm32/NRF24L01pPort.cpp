/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NRF24L01pPort.cpp
 * Author: emon1
 * 
 * Created on January 29, 2017, 8:03 AM
 */

#include "../NRF24L01pPortConfig.h"
#include "../NRF24L01pPort.h"

#if (NRF24L01pPort_STM32 == 1)


#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */


#define NRF24L01P_SPI				SPI1
#define NRF24L01P_CE_PORT			GPIOC
#define NRF24L01P_CE_PIN			GPIO_PIN_7
#define NRF24L01P_CSN_PORT			GPIOB
#define NRF24L01P_CSN_PIN			GPIO_PIN_6
#define NRF24L01P_IRQ_PORT			GPIOA
#define NRF24L01P_IRQ_PIN			GPIO_PIN_8

/* Definition for SPIx clock resources */
#define NRF24L01P_SPIx                             SPI1
#define NRF24L01P_SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define NRF24L01P_SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define NRF24L01P_SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define NRF24L01P_SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define NRF24L01P_SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define NRF24L01P_SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define NRF24L01P_SPIx_SCK_PIN                     GPIO_PIN_5
#define NRF24L01P_SPIx_SCK_GPIO_PORT               GPIOA
#define NRF24L01P_SPIx_SCK_AF                      GPIO_AF5_SPI1
#define NRF24L01P_SPIx_MISO_PIN                    GPIO_PIN_6
#define NRF24L01P_SPIx_MISO_GPIO_PORT              GPIOA
#define NRF24L01P_SPIx_MISO_AF                     GPIO_AF5_SPI1
#define NRF24L01P_SPIx_MOSI_PIN                    GPIO_PIN_7
#define NRF24L01P_SPIx_MOSI_GPIO_PORT              GPIOA
#define NRF24L01P_SPIx_MOSI_AF                     GPIO_AF5_SPI1




static SPI_HandleTypeDef nrf24l01p_SpiHandle;
static GPIO_InitTypeDef nrf24l01p_CE_pin_Struct = {NRF24L01P_CE_PIN,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP,GPIO_SPEED_FREQ_HIGH,};
static GPIO_InitTypeDef nrf24l01p_CSN_pin_Struct = {NRF24L01P_CSN_PIN,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP,GPIO_SPEED_FREQ_HIGH,};
static GPIO_InitTypeDef nrf24l01p_IRQ_pin_Struct = {NRF24L01P_IRQ_PIN,GPIO_MODE_IT_FALLING,GPIO_PULLUP,GPIO_SPEED_FREQ_HIGH,};


NRF24L01pPort::NRF24L01pPort() {
}

NRF24L01pPort::NRF24L01pPort(const NRF24L01pPort& orig) {
}

NRF24L01pPort::~NRF24L01pPort() {
}

void NRF24L01pPort::port_Initialize(){

	port_SPI_initialize();

	/* Enable GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();


	HAL_SPI_Init(&nrf24l01p_SpiHandle);

	port_Pin_CE_Initialize();
	port_Pin_CSN_Initialize();
	port_Pin_IRQ_Initialize();
}
void NRF24L01pPort::port_DeInitialize(){
    
}


void NRF24L01pPort::port_Pin_CE_Initialize(){
	HAL_GPIO_Init(NRF24L01P_CE_PORT, &nrf24l01p_CE_pin_Struct);
}
void NRF24L01pPort::port_Pin_CSN_Initialize(){
	HAL_GPIO_Init(NRF24L01P_CSN_PORT, &nrf24l01p_CSN_pin_Struct);
}
void NRF24L01pPort::port_Pin_IRQ_Initialize(){
	HAL_GPIO_Init(NRF24L01P_IRQ_PORT, &nrf24l01p_IRQ_pin_Struct);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}


void NRF24L01pPort::port_Pin_CE(bool val){
	HAL_GPIO_WritePin(NRF24L01P_CE_PORT, NRF24L01P_CE_PIN, (GPIO_PinState)val);
}
void NRF24L01pPort::port_Pin_CSN(bool val){
	HAL_GPIO_WritePin(NRF24L01P_CSN_PORT, NRF24L01P_CSN_PIN, (GPIO_PinState)val);
}
bool NRF24L01pPort::port_Pin_CE(){
	return HAL_GPIO_ReadPin(NRF24L01P_CE_PORT, NRF24L01P_CE_PIN);
}
bool NRF24L01pPort::port_Pin_CSN(){
	return HAL_GPIO_ReadPin(NRF24L01P_CSN_PORT, NRF24L01P_CSN_PIN);
}
void NRF24L01pPort::port_SPI_initialize(){
	// TODO Auto-generated destructor stub
	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
	nrf24l01p_SpiHandle.Instance               = NRF24L01P_SPIx;
	nrf24l01p_SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	nrf24l01p_SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	nrf24l01p_SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	nrf24l01p_SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
	nrf24l01p_SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	nrf24l01p_SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	nrf24l01p_SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	nrf24l01p_SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	nrf24l01p_SpiHandle.Init.CRCPolynomial     = 7;
	//nrf24l01p_SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	nrf24l01p_SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	//nrf24l01p_SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;

	nrf24l01p_SpiHandle.Init.Mode = SPI_MODE_MASTER;

	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	NRF24L01P_SPIx_SCK_GPIO_CLK_ENABLE();
	NRF24L01P_SPIx_MISO_GPIO_CLK_ENABLE();
	NRF24L01P_SPIx_MOSI_GPIO_CLK_ENABLE();
	/* Enable SPI clock */
	NRF24L01P_SPIx_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* SPI SCK GPIO pin configuration  */
	GPIO_InitStruct.Pin       = NRF24L01P_SPIx_SCK_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = NRF24L01P_SPIx_SCK_AF;
	HAL_GPIO_Init(NRF24L01P_SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

	/* SPI MISO GPIO pin configuration  */
	GPIO_InitStruct.Pin = NRF24L01P_SPIx_MISO_PIN;
	GPIO_InitStruct.Alternate = NRF24L01P_SPIx_MISO_AF;
	HAL_GPIO_Init(NRF24L01P_SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

	/* SPI MOSI GPIO pin configuration  */
	GPIO_InitStruct.Pin = NRF24L01P_SPIx_MOSI_PIN;
	GPIO_InitStruct.Alternate = NRF24L01P_SPIx_MOSI_AF;
	HAL_GPIO_Init(NRF24L01P_SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
}

int NRF24L01pPort::port_SPI_Transcieve(uint8_t *dataInOut, unsigned int size){
	return HAL_SPI_TransmitReceive(&nrf24l01p_SpiHandle, dataInOut, dataInOut, size,1000);
}
void NRF24L01pPort::port_DelayMs(unsigned int ms){
	vTaskDelay(ms);
}
void NRF24L01pPort::port_DelayUs(unsigned int us){
	vTaskDelay(us/1000);
}
unsigned int NRF24L01pPort::port_ClockMs(){

}

void NRF24L01pPort::debug(const char *format, ...){
    va_list args;
    va_start(args,format);
    vfprintf(stdout,format, args);
    va_end(args);
}

void NRF24L01pPort::debug_if(bool condition, const char *format, ...){
    if(condition){
        va_list args;
        va_start(args,format);
        vfprintf(stdout,format, args);
        va_end(args);
    }
}

#endif
