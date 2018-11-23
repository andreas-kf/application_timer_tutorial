#include <stdbool.h>
#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"  //Included in tutorial
#include "boards.h"
#include "bsp.h"
#include "nrf_drv_clock.h" //Included in tutorial
#include "nrf_drv_gpiote.h"

// Pins for LED's and buttons.
// The diodes on the DK are connected with the cathodes to the GPIO pin, so
// clearing a pin will light the LED and setting the pin will turn of the LED.
#define LED_1_PIN                       BSP_LED_0     // LED 1 on the nRF51-DK or nRF52-DK
#define LED_2_PIN                       BSP_LED_1     // LED 3 on the nRF51-DK or nRF52-DK
#define LED_3_PIN                       BSP_LED_2     // LED 3 on the nRF51-DK or nRF52-DK
#define LED_4_PIN                       BSP_LED_3     // LED 3 on the nRF51-DK or nRF52-DK
#define BUTTON_1_PIN                    BSP_BUTTON_0  // Button 1 on the nRF51-DK or nRF52-DK
#define BUTTON_2_PIN                    BSP_BUTTON_1  // Button 2 on the nRF51-DK or nRF52-DK
#define BUTTON_3_PIN                    BSP_BUTTON_2  // Button 3 on the nRF51-DK or nRF52-DK
#define BUTTON_4_PIN                    BSP_BUTTON_3  // Button 4 on the nRF51-DK or nRF52-DK


//Variable that holds timer ID which will be populated by app_timer_create();
APP_TIMER_DEF(m_led_a_timer_id);  //Defines timer a
APP_TIMER_DEF(m_led_b_timer_id);  //Defines timer b

static uint32_t timeout = 0;  //Timeout variable for timer b


void button_handler(nrf_drv_gpiote_pin_t pin)
{
    uint32_t err_code;
    switch (pin)
    {
    case BUTTON_1_PIN:
        
        err_code = app_timer_start(m_led_a_timer_id, APP_TIMER_TICKS(200), NULL);
        APP_ERROR_CHECK(err_code);
        break;
    case BUTTON_2_PIN:
        err_code = app_timer_stop(m_led_a_timer_id);
        APP_ERROR_CHECK(err_code);
        break;
    case BUTTON_3_PIN:
        // Start single shot timer. Increase the timeout with 1 second every time.
        timeout += 1000;
        err_code = app_timer_start(m_led_b_timer_id, APP_TIMER_TICKS(timeout), NULL);
        break;
    case BUTTON_4_PIN:
        nrf_drv_gpiote_out_set(LED_2_PIN);
        break;
    default:
        break;
    }
}

void gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // The button_handler function could be implemented here directly, but is
    // extracted to a separate function as it makes it easier to demonstrate
    // the scheduler later in the tutorial.
    button_handler(pin);
}

// Function for configuring GPIO.
static void gpio_config()
{
    ret_code_t err_code;

    // Initialze driver.
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure 3 output pins for LED's.
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(LED_1_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(LED_2_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(LED_3_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(LED_4_PIN, &out_config);
    APP_ERROR_CHECK(err_code);

    // Set output pins (this will turn off the LED's).
    nrf_drv_gpiote_out_set(LED_1_PIN);
    nrf_drv_gpiote_out_set(LED_2_PIN);
    nrf_drv_gpiote_out_set(LED_3_PIN);
    nrf_drv_gpiote_out_set(LED_4_PIN);

    // Make a configuration for input pints. This is suitable for both pins in this example.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    // Configure input pins for buttons, with separate event handlers for each button.
    err_code = nrf_drv_gpiote_in_init(BUTTON_1_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_2_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_3_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_4_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable input pins for buttons.
    nrf_drv_gpiote_in_event_enable(BUTTON_1_PIN, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2_PIN, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_3_PIN, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_4_PIN, true);
}

// Initialization step
// Function starting the internal LFCLK oscillator.
// This is needed by RTC1 which is used by the application timer
// (When SoftDevice is enabled the LFCLK is always running and this is not needed).
static void lfclk_request(void)
{
    uint32_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);

}

//Timeout handler for the repeated timer a.
static void timer_a_handler(void * p_context)
{
    nrf_drv_gpiote_out_toggle(LED_1_PIN);
}

//Timeout handler for the repeated timer b.
static void timer_b_handler(void * p_context)
{
   nrf_drv_gpiote_out_clear(LED_2_PIN);
}

//Create timers
static void create_timers()
{
    uint32_t err_code;

    //Create timers
    //Timer a
    err_code = app_timer_create(&m_led_a_timer_id, APP_TIMER_MODE_REPEATED, timer_a_handler);
    APP_ERROR_CHECK(err_code);

    //Timer b
    err_code = app_timer_create(&m_led_b_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timer_b_handler);
    APP_ERROR_CHECK(err_code);
}

// Main function.
int main(void)
{
    uint32_t err_code;
    // Configure GPIO's.
    gpio_config();

    // Initialization step
    // Initialize the application timer module
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Initialization step
    //Request LF clock.
    lfclk_request();

    // Create application timers.
    create_timers();

    // Main loop.
    while (true)
    {
        // Wait for interrupt.
        __WFI();
    }
}
