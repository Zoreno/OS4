#include <input/mouse.h>
#include <lib/stdio.h>

/**
 * Mouse packet from mouse IRQ
 */
typedef struct _mouse_packet
{
	/**
	 * Status of the left mouse button.
	 */
	uint8_t		left_button		: 1;

	/**
	 * Status of the right mouse button.
	 */
	uint8_t		right_button 	: 1;

	/**
	 * Status of the middle mouse button.
	 */
	uint8_t		middle_button 	: 1;

	/**
	 * This bit should always be one.
	 */
	uint8_t		always_one		: 1;

	/**
	 * Sign of the x-movement
	 */
	uint8_t		x_sign			: 1;

	/**
	 * Sign of the y-movement
	 */
	uint8_t		y_sign			: 1;

	/**
	 * Indication of that the x movement has exceeded it's maximum value.
	 */
	uint8_t		x_overflow		: 1;

	/**
	 * Indication of that the y movement has exceeded it's maximum value.
	 */
	uint8_t		y_overflow		: 1;

	/**
	 * Movement in the x direction since last packet.
	 */
	int8_t		x_movement		;

	/**
	 * Movement in the y direction since last packet.
	 */
	int8_t		y_movement		;

	union
	{
		/**
	 	 * Movement in the z direction since last packet.
		 *
		 * Valid if mouse is of type MOUSE_WHEEL
		 */
		int8_t 	z_movement		;

		struct
 		{
			/**
			 * Indication of scroll movement.
			 *
			 * Valid if mouse is of type MOUSE_WHEEL_BUTTON
			 */
			uint8_t z_opt 			: 4;

			/**
	 		 * Status of the 4th mouse button.
			 */
			uint8_t button4			: 1;

			/**
	 		 * Status of the 5th mouse button.
	 		 */
			uint8_t button5			: 1;

			/**
			 * Reserved.
			 */
			uint8_t res				: 2;
		};
	};
} __attribute__((packed)) mouse_packet;

#define MOUSE_DATA_PORT					0x60
#define MOUSE_CONTROL_PORT				0x64

#define MOUSE_COMMAND_RESET				0xFF
#define MOUSE_COMMAND_RESEND			0xFE
#define MOUSE_COMMAND_SET_DEFAULTS 		0xF6
#define MOUSE_COMMAND_DIS_PACK_STREAM	0xF5
#define MOUSE_COMMAND_ENA_PACK_STREAM	0xF4
#define MOUSE_COMMAND_SET_SAMPLE_RATE	0xF3
#define MOUSE_COMMAND_GET_MOUSE_ID		0xF2
#define MOUSE_COMMAND_REQ_SINGLE_PACKET	0xEB
#define MOUSE_COMMAND_STATUS_REQUEST	0xE9
#define MOUSE_COMMAND_SET_RESOLUTION	0xE8
#define MOUSE_COMMAND_SEND				0xD4
#define MOUSE_COMMAND_ENABLE_AUXILLARY	0xA8
#define MOUSE_COMMAND_GET_COMPAQ_STATUS	0x20
#define MOUSE_COMMAND_SET_COMPAQ_STATUS	0x60

#define MOUSE_COMMAND_SET_REMOTE		0xF0
#define MOUSE_COMMAND_SET_WRAP		   	0xEE
#define MOUSE_COMMAND_RESET_WRAP		0xEC
#define MOUSE_SET_STREAM_MODE		   	0xEA
#define MOUSE_SET_SCALING_2_1			0xE7
#define MOUSE_SET_SCALING_1_1			0xE6

typedef struct _status_packet
{
	uint8_t		right_button 	: 1;
	uint8_t		middle_button 	: 1;
	uint8_t		left_button		: 1;
	uint8_t		reserved1		: 1;
	uint8_t		scaling			: 1;
	uint8_t		enable			: 1;
	uint8_t		mode			: 1;
	uint8_t		reserved2		: 1;
	
	uint8_t		resolution		;
	uint8_t		sample_rate		;
} __attribute__((packed)) status_packet;

#define MOUSE_RESOLUTION_1_P_MM	0x00
#define MOUSE_RESOLUTION_2_P_MM	0x01
#define MOUSE_RESOLUTION_4_P_MM	0x10
#define MOUSE_RESOLUTION_8_P_MM	0x11

#define MOUSE_SAMPLE_RATE_10	10
#define MOUSE_SAMPLE_RATE_20	20
#define MOUSE_SAMPLE_RATE_40	40
#define MOUSE_SAMPLE_RATE_60	60
#define MOUSE_SAMPLE_RATE_80	80
#define MOUSE_SAMPLE_RATE_100	100
#define MOUSE_SAMPLE_RATE_200	200

#define MOUSE_CONTROL_PORT_DATA_WAIT	0b00000001
#define MOUSE_CONTROL_PORT_SIGNAL_WAIT	0b00000010

void mouse_wait_data();
void mouse_wait_signal();
uint8_t mouse_read();
void mouse_write(uint8_t data);

extern mouse_irq();

static volatile int32_t mouse_x = 0;
static volatile int32_t mouse_y = 0;
static volatile int32_t mouse_z_vertical = 0;
static volatile int32_t mouse_z_horizontal = 0;

static volatile int32_t mouse_buttons = 0;

static mouse_type_t mouse_type = MOUSE_NORMAL;

static volatile uint8_t mouse_ack_recieved = 0;

static mouse_moved_event_handler current_moved_handler = 0;
static mouse_button_event_handler current_button_handler = 0;
static mouse_scroll_event_handler current_scroll_handler = 0;

void mouse_process_packet(mouse_packet* packet);

void mouse_wait_data()
{
	while(1)
	{
		if(!(inportb(MOUSE_CONTROL_PORT) & MOUSE_CONTROL_PORT_DATA_WAIT))
			return;
	}
}

void mouse_wait_signal()
{
	while(1)
	{
		if(!(inportb(MOUSE_CONTROL_PORT) & MOUSE_CONTROL_PORT_SIGNAL_WAIT))
			return;
	}
}

uint8_t mouse_read()
{
	mouse_wait_data();

	return inportb(MOUSE_DATA_PORT);
}

void mouse_wait_for_ack()
{
	while(!mouse_ack_recieved);
}

void mouse_write(uint8_t data)
{
	// Clear ack flag
	mouse_ack_recieved = 0;
	
	// Wait until we are clear to send command
	mouse_wait_signal();

	// Indicate that we want to send a command

	outportb(MOUSE_CONTROL_PORT, MOUSE_COMMAND_SEND);

	// Wait until we are clear to send next part
	mouse_wait_signal();

	// Send the data.
	outportb(MOUSE_DATA_PORT, data);

	// Reset command do not send ACK.
	if(data == 0xFF)
		return;	

	mouse_wait_for_ack();	
}

void mouse_process_packet(mouse_packet* packet)
{
	// printf("Starting to process mouse packet...\n");
	
	//=========================================================
	// Check validity of packet
	//=========================================================

	// Very weak but better than nothing.
	if(packet->always_one != 1)
		return;

	//=========================================================
	// Update Mouse Buttons
	//=========================================================

	// Left button
	
	if(packet->left_button && !(mouse_buttons & MOUSE_BUTTON_LEFT))
	{
		// Mouse Button was pressed
		
		mouse_buttons |= MOUSE_BUTTON_LEFT;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_LEFT;
		ev.action = MOUSE_ACTION_PRESS;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}
	else if(!packet->left_button && (mouse_buttons & MOUSE_BUTTON_LEFT))
	{
		// Mouse button was released
		
		mouse_buttons &= ~MOUSE_BUTTON_LEFT;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_LEFT;
		ev.action = MOUSE_ACTION_RELEASE;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}

	// Middle button
	
	if(packet->middle_button && !(mouse_buttons & MOUSE_BUTTON_MIDDLE))
	{
		// Mouse Button was pressed
		mouse_buttons |= MOUSE_BUTTON_MIDDLE;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_MIDDLE;
		ev.action = MOUSE_ACTION_PRESS;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}
	else if(!packet->middle_button && (mouse_buttons & MOUSE_BUTTON_MIDDLE))
	{
		// Mouse button was released
		mouse_buttons &= ~MOUSE_BUTTON_MIDDLE;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_MIDDLE;
		ev.action = MOUSE_ACTION_RELEASE;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}

    if(packet->right_button && !(mouse_buttons & MOUSE_BUTTON_RIGHT))
	{
		// Mouse Button was pressed
		mouse_buttons |= MOUSE_BUTTON_RIGHT;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_RIGHT;
		ev.action = MOUSE_ACTION_PRESS;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}
	else if(!packet->right_button && (mouse_buttons & MOUSE_BUTTON_RIGHT))
	{
		// Mouse button was released
		mouse_buttons &= ~MOUSE_BUTTON_RIGHT;

		mouse_button_event_t ev;
		ev.button = MOUSE_BUTTON_RIGHT;
		ev.action = MOUSE_ACTION_RELEASE;

		if(current_button_handler)
		{
			current_button_handler(&ev);
		}
	}

	if(mouse_type == MOUSE_WHEEL_BUTTON)
	{
		// Button 4
	    if(packet->button4 && !(mouse_buttons & MOUSE_BUTTON_4))
		{
			// Mouse Button was pressed
			mouse_buttons |= MOUSE_BUTTON_4;

			mouse_button_event_t ev;
			ev.button = MOUSE_BUTTON_4;
			ev.action = MOUSE_ACTION_PRESS;

			if(current_button_handler)
			{
				current_button_handler(&ev);
			}
		}
		else if(!packet->button4 && (mouse_buttons & MOUSE_BUTTON_4))
		{
			// Mouse button was released
			mouse_buttons &= ~MOUSE_BUTTON_4;

			mouse_button_event_t ev;
			ev.button = MOUSE_BUTTON_4;
			ev.action = MOUSE_ACTION_RELEASE;

			if(current_button_handler)
			{
				current_button_handler(&ev);
			}
		}
		// Button 5

		if(packet->button5 && !(mouse_buttons & MOUSE_BUTTON_5))
		{
			// Mouse Button was pressed
			mouse_buttons |= MOUSE_BUTTON_5;

			mouse_button_event_t ev;
			ev.button = MOUSE_BUTTON_5;
		   	ev.action = MOUSE_ACTION_PRESS;

			if(current_button_handler)
			{
				current_button_handler(&ev);
			}
		}
		else if(!packet->button5 && (mouse_buttons & MOUSE_BUTTON_5))
		{
			// Mouse button was released
			mouse_buttons &= ~MOUSE_BUTTON_5;

			mouse_button_event_t ev;
			ev.button = MOUSE_BUTTON_5;
			ev.action = MOUSE_ACTION_RELEASE;

			if(current_button_handler)
			{
				current_button_handler(&ev);
			}
		}
	}

	//=========================================================
	// Update Move Position
	//=========================================================
	
	mouse_x += packet->x_movement;
	mouse_y += packet->y_movement;

	if(packet->x_movement || packet->y_movement)
	{
		if(current_moved_handler)
		{
			mouse_moved_event_t ev;
			
			ev.x = mouse_x;
			ev.y = mouse_y;

			ev.dx = packet->x_movement;
			ev.dy = packet->y_movement;

			current_moved_handler(&ev);
		}
	}

	//=========================================================
	// Update Mouse Wheel Movement
	//=========================================================

	mouse_scroll_event_t ev;
	
	switch(mouse_type)
	{
		// Handle Type 3 mouse
	case MOUSE_WHEEL:
		mouse_z_vertical += packet->z_movement;

		ev.horizontal = 0;
		ev.vertical = packet->z_movement;

		if(current_scroll_handler)
		{
			current_scroll_handler(&ev);
		}
		
		break;

		// Handle Type 4 mouse
	case MOUSE_WHEEL_BUTTON:
		switch(packet->z_opt)
		{
		case 0x1:
			mouse_z_vertical += 1;

			ev.horizontal = 0;
			ev.vertical = 1;

			if(current_scroll_handler)
			{
				current_scroll_handler(&ev);
			}
			
			break;
		case 0x2:
			mouse_z_horizontal += 1;

			ev.horizontal = 1;
			ev.vertical = 0;

			if(current_scroll_handler)
			{
				current_scroll_handler(&ev);
			}
			
			break;
		case 0xE:
			mouse_z_horizontal -= 1;

			ev.horizontal = -1;
			ev.vertical = 0;

			if(current_scroll_handler)
			{
				current_scroll_handler(&ev);
			}
			
			break;
		case 0xF:
			mouse_z_vertical -= 1;

			ev.horizontal = 0;
			ev.vertical = -1;

			if(current_scroll_handler)
			{
				current_scroll_handler(&ev);
			}
		default:
			break;
		}
		break;
		// Last byte is invalid for other types.
	default:
		break;
	}

	// printf("Processed Mouse Packet!\n");
}

void mouse_irq_handler()
{
	uint8_t data = inportb(MOUSE_DATA_PORT);

	// Check if the data was an ack signal.
	if(data == 0xFA)
	{
		// printf("Mouse got ACK");
		mouse_ack_recieved = 1;
	}
	
	// Current byte being sent.
	static uint8_t byteCounter = 0;

	// Store for the incoming bytes.
	static union
	{
		uint8_t bytes[4];
	   	mouse_packet packet;
	} u;

	// Read the data from the mouse
	u.bytes[byteCounter] = data;

	if(u.packet.always_one == 1)
		++byteCounter;
	
	// If we have recieved all bytes, reset counter.
	switch(mouse_type)
	{
	case MOUSE_WHEEL:
	case MOUSE_WHEEL_BUTTON:
		if(byteCounter > 3)
		{
			mouse_process_packet(&u.packet);
			byteCounter = 0;
		}
		break;
	case MOUSE_NORMAL:
	default:
	    if(byteCounter > 2)
		{
			mouse_process_packet(&u.packet);
			byteCounter = 0;
		}
		break;
	}

	// This will print debug info.
	//printf("Handled mouse packet: x_[%i], y_[%i],z_[%i] buttons[%i]\n",
	//	   mouse_x, mouse_y,mouse_z_vertical, mouse_buttons);
}

void mouse_install()
{
	setvect(44, mouse_irq);

	// Enable the mouse
	mouse_wait_signal();
	outportb(MOUSE_CONTROL_PORT, MOUSE_COMMAND_ENABLE_AUXILLARY);

	// Send get compaq status byte
	mouse_wait_signal();
	outportb(MOUSE_CONTROL_PORT, MOUSE_COMMAND_GET_COMPAQ_STATUS);
	
	uint8_t status = mouse_read();
    uint8_t mod_status = status | 0x02; // Enable IRQ
	mod_status &= ~0x20; // Disable Mouse clock

	mouse_wait_signal();
	outportb(MOUSE_CONTROL_PORT, MOUSE_COMMAND_SET_COMPAQ_STATUS);

	mouse_wait_signal();
	outportb(MOUSE_DATA_PORT, mod_status);

	// Set default settings
	mouse_write(MOUSE_COMMAND_SET_DEFAULTS);

	// Test if we have a wheel mouse
	mouse_set_sample_freq(200);
	mouse_set_sample_freq(100);
	mouse_set_sample_freq(80);
	mouse_write(MOUSE_COMMAND_GET_MOUSE_ID);
   	if(mouse_read() == 0x03)
	{
		mouse_type = MOUSE_WHEEL;

		// Test if we have a wheel mouse with extra buttons
		mouse_set_sample_freq(200);
		mouse_set_sample_freq(200);
		mouse_set_sample_freq(80);
		mouse_write(MOUSE_COMMAND_GET_MOUSE_ID);
		if(mouse_read() == 0x04)
		{
			mouse_type = MOUSE_WHEEL_BUTTON;	
		}  
	}

	mouse_set_sample_freq(10);
	
	// Enable the package stream
	mouse_write(MOUSE_COMMAND_ENA_PACK_STREAM);
}

void mouse_set_sample_freq(uint8_t freq)
{
	// Initiate command
	mouse_write(MOUSE_COMMAND_SET_SAMPLE_RATE);

	// Make sure that the frequency is valid.
	switch(freq)
	{
	case 10:
	case 20:
	case 40:
	case 60:
	case 80:
	case 100:
	case 200:
		mouse_write(freq);
		break;
	default:
		// If the frequency does not match a valid value, set it to a default
		// value of 80 Hz
		mouse_write(80);
		break;
	}
}

void mouse_set_position(int32_t x, int32_t y)
{
	mouse_x = x;
	mouse_y = y;
}

void mouse_get_position(int32_t* x, int32_t* y)
{
	*x = mouse_x;
	*y = mouse_y;
}

int mouse_is_pressed(mouse_button_t button)
{
	return mouse_buttons & button;
}

void register_mouse_moved_handler(mouse_moved_event_handler newHandler)
{
	current_moved_handler = newHandler;
}

void register_mouse_button_handler(mouse_button_event_handler newHandler)
{
	current_button_handler = newHandler;
}

void register_mouse_scroll_handler(mouse_scroll_event_handler newHandler)
{
	current_scroll_handler = newHandler;
}
