/******************************************************************************
* 
* File Name: eink_task.c
*
* Description: This file contains task and functions related to the of E-Ink
* that demonstrates controlling a EInk display using the EmWin Graphics Library.
* The project displays a start up screen with Cypress logo and
* text "CYPRESS EMWIN GRAPHICS DEMO EINK DISPLAY".
*
* The project then displays the following screens in a loop
*
*   1. A screen showing various text alignments, styles and modes
*   2. A screen showing normal fonts
*   3. A screen showing bold fonts
*   4. A screen showing 2D graphics with horizontal lines, vertical lines
*       arcs and filled rounded rectangle
*   5. A screen showing 2D graphics with concentric circles and ellipses
*   6. A screen showing a text box with wrapped text
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "GUI.h"
#include "cy8ckit_028_epd_pins.h"
#include "mtb_e2271cs021.h"
#include "LCDConf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "images.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
/* HAL SPI object to interface with display driver */
cyhal_spi_t spi; 

/* Configuration structure defining the necessary pins to communicate with
 * the E-ink display */
const mtb_e2271cs021_pins_t pins =
{
    .spi_mosi = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MOSI,
    .spi_miso = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MISO,
    .spi_sclk = CY8CKIT_028_EPD_PIN_DISPLAY_SPI_SCLK,
    .spi_cs = CY8CKIT_028_EPD_PIN_DISPLAY_CS,
    .reset = CY8CKIT_028_EPD_PIN_DISPLAY_RST,
    .busy = CY8CKIT_028_EPD_PIN_DISPLAY_BUSY,
    .discharge = CY8CKIT_028_EPD_PIN_DISPLAY_DISCHARGE,
    .enable = CY8CKIT_028_EPD_PIN_DISPLAY_EN,
    .border = CY8CKIT_028_EPD_PIN_DISPLAY_BORDER,
    .io_enable = CY8CKIT_028_EPD_PIN_DISPLAY_IOEN,
};

/* Buffer to the previous frame written on the display */
uint8_t previous_frame[PV_EINK_IMAGE_SIZE] = {0};

/* Pointer to the new frame that need to be written */
uint8_t *current_frame;

/*******************************************************************************
* Macros
*******************************************************************************/
#define DELAY_AFTER_STARTUP_SCREEN_MS       (2000)
#define AMBIENT_TEMPERATURE_C               (20)
#define SPI_BAUD_RATE_HZ                    (20000000)

/*******************************************************************************
* Forward declaration
*******************************************************************************/
void show_font_sizes_normal(void);
void show_font_sizes_bold(void);
void show_text_modes(void);
void show_text_wrap_and_orientation(void);
void show_2d_graphics_1(void);
void show_2d_graphics_2(void);
void show_startup_screen(void);
void show_instructions_screen(void);
void wait_for_switch_press_and_release(void);
void clear_screen(void);

/*******************************************************************************
* Function Name: void show_startup_screen(void)
********************************************************************************
*
* Summary: This function displays the startup screen with Cypress Logo and
*           the demo description text
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_startup_screen(void)
{
    /* Set foreground and background color and font size */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();

    GUI_DrawBitmap(&bmCypressLogoFullColor_PNG_1bpp, 2, 2);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CYPRESS", 132, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EMWIN GRAPHICS", 132, 105);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EINK DISPLAY DEMO", 132, 125);
}


/*******************************************************************************
* Function Name: void show_instructions_screen(void)
********************************************************************************
*
* Summary: This function shows screen with instructions to press SW2 to
*           scroll through various display pages
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_instructions_screen(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display instructions text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("PRESS SW2 ON THE KIT", 132, 58);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TO SCROLL THROUGH ", 132, 78);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("DEMO PAGES!", 132, 98);
}


/*******************************************************************************
* Function Name: void show_font_sizes_normal(void)
********************************************************************************
*
* Summary: This function shows various font sizes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_font_sizes_normal(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("1 OF 6: NORMAL FONTS", 132, 5);

    /* Font8_1*/
    GUI_SetFont(GUI_FONT_8_1);
    GUI_DispStringAt("GUI_Font8_1", 10, 30);

    /* Font10_1*/
    GUI_SetFont(GUI_FONT_10_1);
    GUI_DispStringAt("GUI_Font10_1", 10, 40);

    /* Font13_1*/
    GUI_SetFont(GUI_FONT_13_1);
    GUI_DispStringAt("GUI_Font13_1", 10, 52);

    /* Font16_1*/
    GUI_SetFont(GUI_FONT_16_1);
    GUI_DispStringAt("GUI_Font16_1", 10, 67);

    /* Font20_1*/
    GUI_SetFont(GUI_FONT_20_1);
    GUI_DispStringAt("GUI_Font20_1", 10, 85);

    /* Font24_1*/
    GUI_SetFont(GUI_FONT_24_1);
    GUI_DispStringAt("GUI_Font24_1", 10, 107);

    /* Font32_1*/
    GUI_SetFont(GUI_FONT_32_1);
    GUI_DispStringAt("GUI_Font32_1", 10, 133);
}


/*******************************************************************************
* Function Name: void show_font_sizes_bold(void)
********************************************************************************
*
* Summary: This function shows various font sizes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_font_sizes_bold(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("2 OF 6: BOLD FONTS", 132, 5);

    /* Font13B_1*/
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_DispStringAt("GUI_Font13B_1", 5, 25);

    /* Font13HB_1*/
    GUI_SetFont(GUI_FONT_13HB_1);
    GUI_DispStringAt("GUI_Font13HB_1", 5, 40);

    /* Font16B_1*/
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_DispStringAt("GUI_Font16B_1", 5, 55);

    /* FontComic18B_1*/
    GUI_SetFont(GUI_FONT_COMIC18B_1);
    GUI_DispStringAt("GUI_FontComic18B_1", 5, 73);

    /* Font20B_1*/
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_DispStringAt("GUI_Font20B_1", 5, 93);

    /* Font24B_1*/
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_DispStringAt("GUI_Font24B_1", 5, 115);

    /* Font32B_1*/
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_DispStringAt("GUI_Font32B_1", 5, 141);
}


/*******************************************************************************
* Function Name: void show_text_modes(void)
********************************************************************************
*
* Summary: This function displays the following
*           1. Left, Center and Right aligned text
*           2. Underline, overline and strikethrough style text
*           3. Normal, reverse, transparent and XOR text modes
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_text_modes(void)
{
    /* Set font size, foreground and background colors */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("3 OF 6: TEXT ALIGNMENTS, STYLES AND MODES", 132, 5);

    /* Display left aligned text */
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT ALIGNMENT LEFT", 0, 30);

    /* Display center aligned text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TEXT ALIGNMENT CENTER", 132, 45);

    /* Display right aligned text */
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_DispStringAt("TEXT ALIGNMENT RIGHT", 263, 60);

    /* Display underlined text */
    GUI_SetTextStyle(GUI_TS_UNDERLINE);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE UNDERLINE", 0, 75);

    /* Display overlined text */
    GUI_SetTextStyle(GUI_TS_OVERLINE);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE OVERLINE", 0, 90);

    /* Display strikethrough text */
    GUI_SetTextStyle(GUI_TS_STRIKETHRU);
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt("TEXT STYLE STRIKETHROUGH", 0, 105);

    /* Create a rectangle filled with Black color */
    GUI_SetColor(GUI_BLACK);
    GUI_FillRect(0, 120, 263, 175);

    /* Draw two diagonal lines */
    GUI_SetColor(GUI_WHITE);
    GUI_SetPenSize(3);
    GUI_DrawLine(0, 120, 263, 175);
    GUI_DrawLine(0, 175, 263, 120);

    /* Set text color to white with black background */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);

    /* Set text style to normal */
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Display text in normal mode. This will print black text in
        a white box */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_DispStringAt("TEXT MODE NORMAL", 132, 135);

    /* Display text in reverse mode. This will print black text n
        a white box */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_SetTextMode(GUI_TM_REV);
    GUI_DispStringAt("TEXT MODE REVERSE", 132, 150);
}


/*******************************************************************************
* Function Name: void show_text_wrap_and_orientation(void)
********************************************************************************
*
* Summary: This function shows the text orientation and text wrap functions
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_text_wrap_and_orientation(void)
{
    GUI_RECT left_rect = {4, 19, 24, 166};
    GUI_RECT right_rect = {238, 19, 258, 166};
    GUI_RECT middle_rect = {29, 19, 233, 166};
    GUI_RECT middle_rect_margins = {31, 20, 232, 165};

    const char left_text[] = "ROTATED TEXT CCW";
    const char right_text[] = "ROTATED TEXT CW";

    const char middle_text[] = "This project demonstrates displaying 2D graphics"
                               " in an EInk display using Segger EmWin Graphics "
                               "Library. \n\nThis page shows the text wrap and "
                               "text rotation features. In the left rectangle, "
                               "the text is rotated counter clockwise and in the"
                               " right rectangle, the text is rotated clockwise.";


    /* Set font size, foreground and background colors */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("4 OF 6: TEXT WRAP AND ROTATION", 132, 5);

    /* Draw rectangles to hold text */
    GUI_DrawRectEx(&left_rect);
    GUI_DrawRectEx(&right_rect);
    GUI_DrawRectEx(&middle_rect);

    /* Display string in left rectangle rotated counter clockwise */
    GUI_DispStringInRectEx(left_text, &left_rect, GUI_TA_HCENTER | GUI_TA_VCENTER,
                           strlen(left_text), GUI_ROTATE_CCW);

    /* Display string in right rectangle rotated clockwise */
    GUI_DispStringInRectEx(right_text, &right_rect, GUI_TA_HCENTER | GUI_TA_VCENTER,
                           strlen(right_text), GUI_ROTATE_CW);

    /* Display string in middle rectangle with word wrap */
    GUI_DispStringInRectWrap(middle_text, &middle_rect_margins,
                             GUI_TA_LEFT, GUI_WRAPMODE_WORD);
}


/*******************************************************************************
* Function Name: void show_2d_graphics_1(void)
********************************************************************************
*
* Summary: This function displays the following 2D graphics
*           1. Horizontal lines with various pen widths
*           2. Vertical lines with various pen widths
*           3. Arcs
*           4. Filled rounded rectangle
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_2d_graphics_1(void)
{
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("5 OF 6: 2D GRAPHICS - 1", 136, 5);

    /* Display labels */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("H-LINES", 68, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("V-LINES", 204, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ARCS", 68, 163);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ROUNDED RECT", 204, 163);

    /* Horizontal lines */
    GUI_SetPenSize(1);
    GUI_DrawLine(10, 25, 125, 25);
    GUI_SetPenSize(2);
    GUI_DrawLine(10, 35, 125, 35);
    GUI_SetPenSize(3);
    GUI_DrawLine(10, 45, 125, 45);
    GUI_SetPenSize(4);
    GUI_DrawLine(10, 55, 125, 55);
    GUI_SetPenSize(5);
    GUI_DrawLine(10, 65, 125, 65);
    GUI_SetPenSize(6);
    GUI_DrawLine(10, 75, 125, 75);

    /* Vertical lines */
    GUI_SetPenSize(1);
    GUI_DrawLine(168, 20, 168, 80);
    GUI_SetPenSize(2);
    GUI_DrawLine(180, 20, 180, 80);
    GUI_SetPenSize(3);
    GUI_DrawLine(192, 20, 192, 80);
    GUI_SetPenSize(4);
    GUI_DrawLine(204, 20, 204, 80);
    GUI_SetPenSize(5);
    GUI_DrawLine(216, 20, 216, 80);
    GUI_SetPenSize(6);
    GUI_DrawLine(228, 20, 228, 80);
    GUI_SetPenSize(7);
    GUI_DrawLine(240, 20, 240, 80);

    /* Arcs */
    GUI_SetPenSize(2);
    GUI_DrawArc(68, 160, 10, 10, 0, 180);
    GUI_DrawArc(68, 160, 20, 20, 0, 180);
    GUI_DrawArc(68, 160, 30, 30, 0, 180);
    GUI_DrawArc(68, 160, 40, 40, 0, 180);
    GUI_DrawArc(68, 160, 50, 50, 0, 180);

    /* Rounded rectangle */
    GUI_FillRoundedRect(146, 108, 262, 160, 5);
}


/*******************************************************************************
* Function Name: void show_2d_graphics_2(void)
********************************************************************************
*
* Summary: This function displays the following 2D graphics
*           1. Concentric circles
*           2. Concentric ellipses
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void show_2d_graphics_2(void)
{
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("6 OF 6: 2D GRAPHICS - 2", 132, 5);

    /* Display labels */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CIRCLE", 68, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("ELLIPSE", 204, 85);

    /* Concentric Circles */
    GUI_DrawCircle(68, 51, 30);
    GUI_DrawCircle(68, 51, 25);
    GUI_DrawCircle(68, 51, 20);
    GUI_DrawCircle(68, 51, 15);
    GUI_DrawCircle(68, 51, 10);

    /* Concentric Ellipses */
    GUI_DrawEllipse(204, 51, 50, 30);
    GUI_DrawEllipse(204, 51, 40, 25);
    GUI_DrawEllipse(204, 51, 30, 20);
    GUI_DrawEllipse(204, 51, 20, 15);;
}


/*******************************************************************************
* Function Name: void clear_screen(void)
********************************************************************************
*
* Summary: This function clears the screen
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void clear_screen(void)
{
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
}


/*******************************************************************************
* Function Name: void wait_for_switch_press_and_release(void)
********************************************************************************
*
* Summary: This implements a simple "Wait for button press and release"
*           function.  It first waits for the button to be pressed and then
*           waits for the button to be released.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This is a blocking function and exits only on a button press and release
*
*******************************************************************************/
void wait_for_switch_press_and_release(void)
{
    /* Wait for SW2 to be pressed */
    while( CYBSP_BTN_PRESSED != cyhal_gpio_read(CYBSP_USER_BTN));

    /* Wait for SW2 to be released */
    while( CYBSP_BTN_PRESSED == cyhal_gpio_read(CYBSP_USER_BTN));
}

/*******************************************************************************
* Function Name: void eInk_task(void *arg)
********************************************************************************
*
* Summary: Following functions are performed
*           1. Initialize the EmWin library
*           2. Display the startup screen for 2 seconds
*           3. Display the instruction screen and wait for key press and release
*           4. Inside a while loop scroll through the 6 demo pages on every
*               key press and release
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void eInk_task(void *arg)
{
    cy_rslt_t result;
    uint8_t page_number = 0;

    /* Array of demo pages functions */
    void (*demoPageArray[])(void) = {
        show_font_sizes_normal,
        show_font_sizes_bold,
        show_text_modes,
        show_text_wrap_and_orientation,
        show_2d_graphics_1,
        show_2d_graphics_2
    };

    uint8_t num_of_demo_pages = (sizeof(demoPageArray)/sizeof(demoPageArray[0]));

    /* Configure Switch and LEDs*/
    cyhal_gpio_init( CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 
                     CYBSP_BTN_OFF);
    cyhal_gpio_init( CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG,
                     CYBSP_LED_STATE_OFF);
    
    /* Initialize SPI and EINK display */
    result = cyhal_spi_init(&spi, CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MOSI,
            CY8CKIT_028_EPD_PIN_DISPLAY_SPI_MISO,
            CY8CKIT_028_EPD_PIN_DISPLAY_SPI_SCLK, NC, NULL, 8,
            CYHAL_SPI_MODE_00_MSB, false);
    if (CY_RSLT_SUCCESS == result)
    {
        result = cyhal_spi_set_frequency(&spi, SPI_BAUD_RATE_HZ);
    }

    result = mtb_e2271cs021_init(&pins, &spi);

    /* Set ambient temperature, in degree C, in order to perform temperature
     * compensation of E-INK parameters */
    mtb_e2271cs021_set_temp_factor(AMBIENT_TEMPERATURE_C);

    current_frame = (uint8_t*)LCD_GetDisplayBuffer();

    /* Initialize EmWin driver*/
    GUI_Init();

    /* Show the startup screen */
    show_startup_screen();

    /* Update the display */
    mtb_e2271cs021_show_frame(previous_frame, current_frame,
                              MTB_E2271CS021_FULL_4STAGE, true);

    vTaskDelay(DELAY_AFTER_STARTUP_SCREEN_MS);

    /* Show the instructions screen */
    show_instructions_screen();

    /* Update the display */
    mtb_e2271cs021_show_frame(previous_frame, current_frame,
                              MTB_E2271CS021_FULL_4STAGE, true);

    wait_for_switch_press_and_release();

    for(;;)
    {
        cyhal_gpio_write( CYBSP_USER_LED, CYBSP_LED_STATE_ON);

        /* Using page_number as index, update the display with a demo screen
            Following are the functions that are called in sequence
                show_font_sizes_normal()
                show_font_sizes_bold()
                show_text_modes()
                show_text_wrap_and_orientation()
                show_2d_graphics_1()
                show_2d_graphics_2()
        */
        (*demoPageArray[page_number])();

        /* Update the display */
        mtb_e2271cs021_show_frame(previous_frame, current_frame,
                                  MTB_E2271CS021_FULL_4STAGE, true);

        cyhal_gpio_write( CYBSP_USER_LED, CYBSP_LED_STATE_OFF);

        /* Wait for a switch press event */
        wait_for_switch_press_and_release();

        /* Cycle through demo pages */
        page_number = (page_number+1) % num_of_demo_pages;
    }
}
