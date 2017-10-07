.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start

Start:

    // Skriv din kode her...

		// start of led and button port
		LDR R0, =GPIO_BASE
		LDR R1, =PORT_SIZE

		// adress button Into R2
		LDR R2, =BUTTON_PORT
		MUL R2, R1, R2
		ADD R2, R2, R0 // address button port
		LDR R3, =GPIO_PORT_DIN
		ADD R4, R2, R3 // address button din


		LDR R2, =LED_PORT
		MUL R2, R1, R2
		ADD R2, R2, R0 // address led prot

		LDR R3, =GPIO_PORT_DOUTCLR
		ADD R6, R2, R3 // address doutclr

		LDR R3, =GPIO_PORT_DOUTSET
		ADD R5, R2, R3 // address doutset


		// set R0 input from button
		LDR R0, =#1 << BUTTON_PIN

		// set R1 output at led
		LDR R1, =#1 << LED_PIN

		B LOOP



LOOP:
		LDR R2, [R4]
		AND R2, R2, R0
		CMP R2, R0
		BNE LedOn
		B LedOff

LedOff:
		STR R1, [R6]
		B LOOP

LedOn:
		STR R1, [R5]
		B LOOP


















NOP // Behold denne på bunnen av fila
