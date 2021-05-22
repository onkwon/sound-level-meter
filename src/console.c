#include "console.h"
#include "abov/hal/uart.h"
#include "abov/hal/gpio.h"
#include "abov/irq.h"

static uart_handle_t uart0_handle;

static void uart_rx_handler(uint32_t flags)
{
	(void)flags;
	uint8_t c;
	uart_read(PERIPH_USART1, &c, 1);
	uart_write(PERIPH_USART1, &c, 1);
}

static void uart_gpio_init(void)
{
	struct gpio_cfg cfg = {
		.mode = GPIO_MODE_PUSHPULL,
		.altfunc = true,
	};
	gpio_open(PERIPH_GPIOA, 9, &cfg); // TX

	cfg.mode = GPIO_MODE_INPUT_PULLUP;
	gpio_open(PERIPH_GPIOA, 10, &cfg); // RX
}

void _putchar(char character)
{
	uart_write(PERIPH_USART1, &character, 1);
}

void console_init(void)
{
	uart_gpio_init();
	uart_init(PERIPH_USART1, &(struct uart_cfg) {
			.wordsize = UART_WORDSIZE_8,
			.stopbit = UART_STOPBIT_1,
			.parity = UART_PARITY_NONE,
			.baudrate = 115200,
			.rx_interrupt = true,
			},
			&uart0_handle);
	uart_register_rx_handler(&uart0_handle, uart_rx_handler);
}

void ISR_USART1(void)
{
	uart_default_isr(PERIPH_USART1, &uart0_handle);
}
