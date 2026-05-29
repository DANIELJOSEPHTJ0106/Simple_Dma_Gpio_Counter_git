#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xaxidma.h"
#include "xil_cache.h"

// -----------------------------------------------------------------------
// PARAMETERS
// -----------------------------------------------------------------------
#define DMA_DEV_ID          XPAR_AXIDMA_0_DEVICE_ID
#define DDR_BASE_ADDR       0x01000000  // We will write data here in DDR

// NOTE: Packet Size
// We are using 1023 to match your current hardware.
#define PACKET_LEN          1024
#define MEM_SIZE_BYTES      (PACKET_LEN * 4)

// DMA Instance
XAxiDma AxiDma;

int main()
{
    int Status;
    volatile u32 *RxBuffer = (volatile u32 *)DDR_BASE_ADDR;

    xil_printf("\r\n--- FPGA Counter DMA Test ---\r\n");

    // Initialize DMA Engine
    XAxiDma_Config *CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
    if (!CfgPtr) return XST_FAILURE;

    Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    // -------------------------------------------------------------------
    // STEP 1: CLEAN THE BUFFER FIRST
    // -------------------------------------------------------------------
    for(int i=0; i < 1024; i++) {
        RxBuffer[i] = 0xFFFFFFFF;
    }

    // -------------------------------------------------------------------
    // STEP 2: FLUSH CACHE
    // -------------------------------------------------------------------
    Xil_DCacheFlushRange((UINTPTR)RxBuffer, MEM_SIZE_BYTES);

    // Start DMA Transfer
    xil_printf("Starting DMA Receive transaction...\r\n");
    Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)RxBuffer,
                                    MEM_SIZE_BYTES, XAXIDMA_DEVICE_TO_DMA);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    xil_printf("DMA is ready. >>> PLEASE TURN ON SWITCH (SW0) NOW <<<\r\n");

    // Wait for DMA
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) {}

    xil_printf("DMA Transfer Complete!\r\n");

    // -------------------------------------------------------------------
    // STEP 3: INVALIDATE CACHE
    // -------------------------------------------------------------------
    Xil_DCacheInvalidateRange((UINTPTR)RxBuffer, MEM_SIZE_BYTES);

    // Verify Data
    int error_count = 0;
    for (int i = 0; i < PACKET_LEN; i++) {
        if (RxBuffer[i] == 0xFFFFFFFF) {
            xil_printf("Error at index %d: DMA did not write this location!\r\n", i);
            error_count++;
            break;
        }
        else if (RxBuffer[i] != i) {
            xil_printf("Error at index %d: Expected %d, Got %d\r\n", i, i, RxBuffer[i]);
            error_count++;
            if (error_count > 5) break;
        }
    }

    if (error_count == 0) {
        xil_printf("SUCCESS! Data verification passed.\r\n");
        xil_printf("----------------------------------------\r\n");
        xil_printf("FULL DATA DUMP:\r\n");

        // -------------------------------------------------------
        // PRINT ALL VALUES LOOP
        // -------------------------------------------------------
        for (int i = 0; i < PACKET_LEN; i++) {
            xil_printf("%d ", RxBuffer[i]);

            // New line after every 8 values
            if ((i + 1) % 32 == 0) {
                xil_printf("\r\n");
            }
        }
        // -------------------------------------------------------

        xil_printf("----------------------------------------\r\n");
        xil_printf("End of Data.\r\n");

    } else {
        xil_printf("FAILED. Please check the errors above.\r\n");
    }

    return 0;
}
