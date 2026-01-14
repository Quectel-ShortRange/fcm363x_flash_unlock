/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi.h"
#include "app.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src);
status_t flexspi_nor_read_data(FLEXSPI_Type *base, uint32_t startAddress, uint32_t *buffer, uint32_t length);
extern status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId, uint16_t *deviceId);
extern status_t flexspi_nor_get_status1(FLEXSPI_Type *base, uint8_t *status1);
extern status_t flexspi_nor_get_status2(FLEXSPI_Type *base, uint8_t *status2);
extern status_t flexspi_nor_get_status3(FLEXSPI_Type *base, uint8_t *status3);
extern status_t flexspi_nor_reset_device(FLEXSPI_Type *base);
extern status_t flexspi_nor_write_status1(FLEXSPI_Type *base, uint8_t status1);
extern status_t flexspi_nor_write_status2(FLEXSPI_Type *base, uint8_t status2);
extern status_t flexspi_nor_write_status3(FLEXSPI_Type *base, uint8_t status3);
extern status_t flexspi_nor_erase_chip(FLEXSPI_Type *base);
extern status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base);
extern void flexspi_nor_flash_init(FLEXSPI_Type *base);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;
    uint8_t vendorID = 0;
    uint16_t deviceID = 0;
    uint8_t status1 = 0, status2 = 0, status3 = 0;

    BOARD_InitHardware();

    PRINTF("\r\nFLEXSPI Flash Unlock Tool Started!\r\n");

    /* FLEXSPI init */
    flexspi_nor_flash_init(EXAMPLE_FLEXSPI);

    /* Get vendor ID and device ID */
    status = flexspi_nor_get_vendor_id(EXAMPLE_FLEXSPI, &vendorID, &deviceID);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting flash IDs: 0x%x\r\n", status);
        return status;
    }
    PRINTF("\r\n--- Flash Information ---\r\n");
    PRINTF("Vendor ID: 0x%02X ", vendorID);
    
    /* Check vendor support */
    if (vendorID == 0x20)
    {
        PRINTF("(XMC - Supported)\r\n");
    }
    else if (vendorID == 0xEF)
    {
        PRINTF("(Winbond - Supported by NXP article, may need different parameters)\r\n");
    }
    else
    {
        PRINTF("(Unknown - Not Supported)\r\n");
        PRINTF("This tool only supports XMC (0x20) flash.\r\n");
        return kStatus_Fail;
    }
    PRINTF("Device ID: 0x%04X\r\n", deviceID);
    PRINTF("-------------------------\r\n\r\n");

    /* Read initial status registers */
    PRINTF("--- Initial Status Registers ---\r\n");
    status = flexspi_nor_get_status1(EXAMPLE_FLEXSPI, &status1);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status1: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 1: 0x%02X\r\n", status1);

    status = flexspi_nor_get_status2(EXAMPLE_FLEXSPI, &status2);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status2: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 2: 0x%02X\r\n", status2);

    status = flexspi_nor_get_status3(EXAMPLE_FLEXSPI, &status3);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status3: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 3: 0x%02X\r\n", status3);
    PRINTF("--------------------------------\r\n\r\n");

    /* Check if flash needs unlocking */
    bool needs_unlock = false;
    if ((status1 & 0xFC) != 0) /* Check BP[2:0], TB, SEC, SRP0 */
    {
        PRINTF("SR1 protection detected: 0x%02X\r\n", status1);
        needs_unlock = true;
    }
    if ((status2 & 0x79) != 0) /* Check CMP, LB[3:1], SRP1 (exclude QE bit) */
    {
        PRINTF("SR2 protection detected: 0x%02X\r\n", status2);
        needs_unlock = true;
    }

    /* Check for hardware write protection */
    if (status2 & 0x01)
    {
        PRINTF("\r\n*** WARNING: Hardware Write Protection Detected! ***\r\n");
        PRINTF("SRP1=1: Status registers are hardware protected.\r\n");
        PRINTF("This typically requires WP# pin to be HIGH.\r\n");
        PRINTF("Unlock may fail - proceed with ISP mode if needed.\r\n\r\n");
    }

    if (!needs_unlock)
    {
        PRINTF("Flash is not locked - no unlock needed.\r\n");
        PRINTF("Status registers are already in unlocked state.\r\n");
        return kStatus_Success;
    }

    /* Perform unlock */
    PRINTF("\r\n--- Starting Flash Unlock Procedure ---\r\n");
    status1 = 0x00;  /* Clear all protection bits */
    status2 = 0x02;  /* Keep QE=1, clear all protection */

    /* Write Status Register 1 */
    status = flexspi_nor_write_status1(EXAMPLE_FLEXSPI, status1);
    if (status != kStatus_Success)
    {
        PRINTF("Error writing status1: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 1 written: 0x%02X\r\n", status1);

    /* Write Status Register 2 */
    status = flexspi_nor_write_status2(EXAMPLE_FLEXSPI, status2);
    if (status != kStatus_Success)
    {
        PRINTF("Error writing status2: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 2 written: 0x%02X\r\n", status2);
    PRINTF("---------------------------------------\r\n\r\n");

    /* Read status registers again to verify */
    PRINTF("--- Status Registers After Unlock ---\r\n");
    status = flexspi_nor_get_status1(EXAMPLE_FLEXSPI, &status1);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status1: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 1: 0x%02X\r\n", status1);

    status = flexspi_nor_get_status2(EXAMPLE_FLEXSPI, &status2);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status2: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 2: 0x%02X\r\n", status2);

    status = flexspi_nor_get_status3(EXAMPLE_FLEXSPI, &status3);
    if (status != kStatus_Success)
    {
        PRINTF("Error getting status3: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Status Register 3: 0x%02X\r\n", status3);
    PRINTF("--------------------------------------\r\n\r\n");

    /* Verify unlock was successful */
    if ((status1 == 0x00) && ((status2 & 0x79) == 0))
    {
        PRINTF("\r\n");
        PRINTF("                                       \r\n");
        PRINTF("                                       \r\n");
        PRINTF("     OOOOOOOOO     KKKKKKKKK    KKKKKKK\r\n");
        PRINTF("   OO:::::::::OO   K:::::::K    K:::::K\r\n");
        PRINTF(" OO:::::::::::::OO K:::::::K    K:::::K\r\n");
        PRINTF("O:::::::OOO:::::::OK:::::::K   K::::::K\r\n");
        PRINTF("O::::::O   O::::::OKK::::::K  K:::::KKK\r\n");
        PRINTF("O:::::O     O:::::O  K:::::K K:::::K   \r\n");
        PRINTF("O:::::O     O:::::O  K::::::K:::::K    \r\n");
        PRINTF("O:::::O     O:::::O  K:::::::::::K     \r\n");
        PRINTF("O:::::O     O:::::O  K:::::::::::K     \r\n");
        PRINTF("O:::::O     O:::::O  K::::::K:::::K    \r\n");
        PRINTF("O:::::O     O:::::O  K:::::K K:::::K   \r\n");
        PRINTF("O::::::O   O::::::OKK::::::K  K:::::KKK\r\n");
        PRINTF("O:::::::OOO:::::::OK:::::::K   K::::::K\r\n");
        PRINTF(" OO:::::::::::::OO K:::::::K    K:::::K\r\n");
        PRINTF("   OO:::::::::OO   K:::::::K    K:::::K\r\n");
        PRINTF("     OOOOOOOOO     KKKKKKKKK    KKKKKKK\r\n");
        PRINTF("                                       \r\n");
        PRINTF("                                       \r\n");
        PRINTF("\r\n");
        PRINTF("========================================\r\n");
        PRINTF("  Flash Unlock SUCCESSFUL!\r\n");
        PRINTF("========================================\r\n");
        PRINTF("Protection bits have been cleared.\r\n");
        PRINTF("Flash is now unlocked and ready to use.\r\n");
        PRINTF("You can now flash your application normally.\r\n");
        PRINTF("========================================\r\n");
    }
    else
    {
        PRINTF("\r\n");
        PRINTF("                                                                                \r\n");
        PRINTF("                                                                                \r\n");
        PRINTF("FFFFFFFFFFFFFFFFFFFFFF      AAA               IIIIIIIIIILLLLLLLLLLL             \r\n");
        PRINTF("F::::::::::::::::::::F     A:::A              I::::::::IL:::::::::L             \r\n");
        PRINTF("F::::::::::::::::::::F    A:::::A             I::::::::IL:::::::::L             \r\n");
        PRINTF("FF::::::FFFFFFFFF::::F   A:::::::A            II::::::IILL:::::::LL             \r\n");
        PRINTF("  F:::::F       FFFFFF  A:::::::::A             I::::I    L:::::L               \r\n");
        PRINTF("  F:::::F              A:::::A:::::A            I::::I    L:::::L               \r\n");
        PRINTF("  F::::::FFFFFFFFFF   A:::::A A:::::A           I::::I    L:::::L               \r\n");
        PRINTF("  F:::::::::::::::F  A:::::A   A:::::A          I::::I    L:::::L               \r\n");
        PRINTF("  F:::::::::::::::F A:::::A     A:::::A         I::::I    L:::::L               \r\n");
        PRINTF("  F::::::FFFFFFFFFFA:::::AAAAAAAAA:::::A        I::::I    L:::::L               \r\n");
        PRINTF("  F:::::F         A:::::::::::::::::::::A       I::::I    L:::::L               \r\n");
        PRINTF("  F:::::F        A:::::AAAAAAAAAAAAA:::::A      I::::I    L:::::L         LLLLLL\r\n");
        PRINTF("FF:::::::FF     A:::::A             A:::::A   II::::::IILL:::::::LLLLLLLLL:::::L\r\n");
        PRINTF("F::::::::FF    A:::::A               A:::::A  I::::::::IL::::::::::::::::::::::L\r\n");
        PRINTF("F::::::::FF   A:::::A                 A:::::A I::::::::IL::::::::::::::::::::::L\r\n");
        PRINTF("FFFFFFFFFFF  AAAAAAA                   AAAAAAAIIIIIIIIIILLLLLLLLLLLLLLLLLLLLLLLL\r\n");
        PRINTF("                                                                                \r\n");
        PRINTF("                                                                                \r\n");
        PRINTF("\r\n");
        PRINTF("========================================\r\n");
        PRINTF("  Flash Unlock FAILED!\r\n");
        PRINTF("========================================\r\n");
        PRINTF("Status registers did not reach expected values.\r\n");
        PRINTF("Expected: SR1=0x00, SR2=0x02\r\n");
        PRINTF("Got:      SR1=0x%02X, SR2=0x%02X\r\n", status1, status2);
        PRINTF("\r\n");
        PRINTF("** RECOVERY PROCEDURE **\r\n");
        PRINTF("1. Disconnect power from the board\r\n");
        PRINTF("2. Set BOOT jumper to ISP mode\r\n");
        PRINTF("3. Reconnect power to the board\r\n");
        PRINTF("4. Run this unlock tool again\r\n");
        PRINTF("\r\n");
        PRINTF("If the issue persists, the flash may be\r\n");
        PRINTF("hardware write-protected (WP# pin).\r\n");
        PRINTF("========================================\r\n");
        
        while (1)
        {
        }
    }

    PRINTF("\r\nErasing the whole chip, it will take several minutes to complete...\r\n");
    status = flexspi_nor_erase_chip(EXAMPLE_FLEXSPI);
    if (status != kStatus_Success)
    {
        PRINTF("Error erasing: 0x%x\r\n", status);
        return status;
    }
    PRINTF("Chip erased successfully!\r\n");
    PRINTF("\r\nFlash unlock and erase completed.\r\n");
    PRINTF("You can now program your application.\r\n");

    while (1)
    {
    }
}
