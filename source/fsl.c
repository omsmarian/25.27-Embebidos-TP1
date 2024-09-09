#include "fsl.h"


static volatile char key_pressed = 0;
static volatile int key_flag = 0;


static uint8_t state = ADD_USER;

static bool return_flag = false;

void change_brightness_call(void) ;






void init_fsl()
{


    if(encoder_Init())
    {
		MY_PRINTF("Encoder initialized\n");
	}
    else
	{
		MY_PRINTF("Error initializing encoder\n");
	}

    if (DisplayInit()) {
		MY_PRINTF("Display initialized\n");
	} else {
		MY_PRINTF("Error initializing display\n");
	}

	if(!MagCardInit())
	{
		MY_PRINTF("MagCard initialized\n");
	}

	if(LEDS_Init())
		MY_PRINTF("LEDS initialized\n");

	timerInit();

	state = ADD_USER;
    print_menu(state);
    DisplaySetBrightness(100);

    gpioMode(PIN_TP_PER, OUTPUT);
    gpioMode(PIN_TP_DED, OUTPUT);


}

void update_fsl()
{
	update_menu(); // Update the menu
	manage_access(); //Manage access
}


void update_menu()
{

    int key = read_key();

    if (key == 1) { //LEFT
        if (state == ADD_USER) {
            state = BRIGHTNESS;
        } else {
            state--;
        }
    } else if (key == 3) { //RIGHT
        if (state == BRIGHTNESS) {
            state = ADD_USER;
        } else {
            state++;
        }
    }

    else if (key == 4)
    { //ENTER
            switch (state) {
            case ADD_USER:
                add_user_call();
                break;
            case DELETE_USER:
                delete_user_call();
                break;
            case CHANGE_PASSWORD:
                change_password_call();
                break;
            case ACCESS_SYSTEM:
            	access_system_call();
                break;
            case BRIGHTNESS:
            	change_brightness_call();
            	break;
            }
    }
	if (key == 1 || key == 3) {
		print_menu(state);
	}
}

void print_menu(enum states_fsl state) {
	MY_PRINTF("\n");
    switch (state) {
    case ADD_USER:
    	DisplayWriteChar("ADD USER");
        MY_PRINTF(">> Add User\n   Delete User\n   Change Password\n   Access System\n   Brightness\n");
        break;
    case DELETE_USER:
        DisplayWriteChar("DEL ");
        MY_PRINTF("   Add User\n>> Delete User\n   Change Password\n   Access System\n   Brightness\n");
        break;
    case CHANGE_PASSWORD:
    	DisplayWriteChar("CHNG");
        MY_PRINTF("   Add User\n   Delete User\n>> Change Password\n   Access System\n   Brightness\n");
        break;
    case ACCESS_SYSTEM:
    	DisplayWriteChar("ACCS");
        MY_PRINTF("   Add User\n   Delete User\n   Change Password\n>> Access System\n   Brightness\n");
        break;
        case BRIGHTNESS:
		DisplayWriteChar("BRIG");
		MY_PRINTF("   Add User\n   Delete User\n   Change Password\n   Access System\n>> Brightness\n");
    }
}


int read_key(void) {
    action_t key = encoderRead();

    switch (key)
    {
    case NONE :    return 0;
    case LEFT :
        ResetEncoder();
        return 1;
    case LONG_CLICK :
        ResetEncoder();
        return 2;
    case RIGHT :
        ResetEncoder();
        return 3;
    case CLICK :
        ResetEncoder();
        return 4;
    case DOUBLE_CLICK :
		ResetEncoder();
		return 5;
    }
    //
}

#include <stdio.h>

char* intToString(int num) {

	static char* str;
    sprintf(str, "%d", num);

    return str;

}

void print_code_on_display(char *id) {
    int id_length = strlen(id); // Obtener la longitud del ID
    DisplayClear(); // Limpiar el display

    // Comenzar desde el cuarto último dígito, o desde el inicio si el ID tiene menos de 4 dígitos
    int start = id_length > 4 ? id_length - 4 : 0;

    for (int i = start; i < id_length; i++) {
        // Convertir el carácter del ID a un número y imprimirlo en el display
        uint8_t number = id[i] - '0';
        DisplaySetDigitNum(i - start, number);
    }
}

bool read_from_encoder(char *id) {
    static int digit_index = 0;
    static int current_digit = 0;


    //static char* ptrKey = "    ";

    int key = read_key();
    if (key == 0)
    { // No key pressed
        return false;
    }
    print_code_on_display(id);

    if (key == 1) { //LEFT
        current_digit = (current_digit == 0) ? 9 : current_digit - 1;
        id[digit_index] = '0' + current_digit;
        id[digit_index + 1] = '\0'; // Null-terminate the ID string

        MY_PRINTF("%s\n", id);
        DisplayClear();
        print_code_on_display(id);

    } else if (key == 3) { //RIGHT
        current_digit = (current_digit == 9) ? 0 : current_digit + 1;
        id[digit_index] = '0' + current_digit;
        id[digit_index + 1] = '\0'; // Null-terminate the ID string

        MY_PRINTF("%s\n", id);
        DisplayClear();
        print_code_on_display(id);

    } else if (key == 4) { //ENTER
        id[digit_index] = '0' + current_digit;


        if (digit_index + 1  == 8) {
            id[digit_index + 1] = '\0'; // Null-terminate the ID string
            digit_index = 0; // Reset for next ID read
            return true;
        }
        else
        {
            id[digit_index + 1] = '0'; // Null-terminate the ID string
            id[digit_index + 2] = '\0'; // Null-terminate the ID string

            digit_index++;
            current_digit = 0;

			MY_PRINTF("%s\n", id);
			DisplayClear();
			print_code_on_display(id);
        }
    }

    else if(key == 2) //GO BACK IN MENU
    {
    	return_flag = true;
    	digit_index = 0;
    	current_digit = 0;
    	return false;
    }

    else if (key == 5)
    { //DOUBLE_CLICK
            if (digit_index > 0) {
                digit_index--;
                current_digit = 0;
                id[digit_index] = '0'; // Null-terminate the ID string
                id[digit_index + 1] = '\0'; // Null-terminate the ID string
                MY_PRINTF("%s\n", id);
                DisplayClear();
                print_code_on_display(id);
            }

    }
    return false;
}

#include <stdio.h>
#include <string.h>

bool read_from_card(char *id) {


	if (MagCardGetStatus())
	{
		uint64_t data;

		data = MagCardGetCardNumber();
		MagCardClearData();

	    char data_str[17]; // Buffer para almacenar la representación de cadena de data

	    sprintf(data_str, "%016llu", data); // Convertir data a cadena, asegurándose de que tenga 16 dígitos

	    strncpy(id, &data_str[8], 8); // Copiar los últimos 8 dígitos a id

	    id[8] = '\0'; // Asegurarse de que id sea una cadena terminada en null
	    MY_PRINTF("%s\n", id);


	    return true;

	}
	else
	{
		return false;
	}
}

int8_t change_brightness(void)
{
	int success = 0;
	int8_t brightness = 0;


	while (success != 2) {
		int key = read_key();
		if (key == 1) { //LEFT
			brightness = (uint8_t)DisplaySetBrightness(-1);

			uint8_t cent = brightness / 100; // Para obtener las centenas
			uint8_t dec = (brightness % 100) / 10; // Para obtener las decenas
			uint8_t uni = brightness % 10; // Para obtener las unidades

			DisplaySetDigitNum(0, 0);
			DisplaySetDigitNum(1, cent);
			DisplaySetDigitNum(2, dec);
			DisplaySetDigitNum(3, uni);

			MY_PRINTF("Brightness: %d%d%d\n", cent, dec, uni);

			success = 1;
		} else if (key == 3) { //RIGHT
			brightness = (uint8_t)DisplaySetBrightness(1);

			uint8_t cent = brightness / 100; // Para obtener las centenas
			uint8_t dec = (brightness % 100) / 10; // Para obtener las decenas
			uint8_t uni = brightness % 10; // Para obtener las unidades

			DisplaySetDigitNum(0, 0);
			DisplaySetDigitNum(1, cent);
			DisplaySetDigitNum(2, dec);
			DisplaySetDigitNum(3, uni);

			MY_PRINTF("Brightness: %d%d%d\n", cent, dec, uni);


			success = 1;

		} else if (key == 4) { //ENTER
			success = 2;
			return brightness;

		}
		else if (key == 2) { //Go back
			brightness = (uint8_t)DisplaySetBrightness(100);
			return -1;
		}



	}
}

void change_brightness_call(void) {
	MY_PRINTF("Changing brightness\n");
	int8_t brightness = change_brightness();

	if (brightness == -1) {
		state = ADD_USER;
		return_flag = false;
		print_menu(state);
	}
	else
	{
		MY_PRINTF("Brightness changed to %d\n", brightness);

		state = ADD_USER;
		print_menu(state);
	}
}

void read_id(char *id) {

    int read_successful = 0;

    while (!read_successful) {
    	manage_access();
        read_successful = read_from_encoder(id);
        if (return_flag)
        	break;
        if (!read_successful) {
            read_successful = read_from_card(id);
        }
    }
    if (read_successful)
    	MY_PRINTF("User id: %s\n", id);
}


int type_of_pasword(void)
{
	char* str = " ";
    int digit = 4;

	while(1)
	{
		manage_access();
		int key = read_key();

		if(key == 1 || key == 3) //LEFT or RIGHT
		{
			digit = (digit == 4) ? 5 : 4;
			str = (digit == 4) ? "PL 4" : "PL 5";
			MY_PRINTF("%s\n", str);
			DisplayWriteChar(str);


		}
		else if (key == 4) //ENTER
		{
			MY_PRINTF("Enter password:")
			DisplayWriteChar("PASS");
			return digit;

		}
		else if (key == 2) //Go back
		{
			return_flag = true;
			return 0;
		}



	}
}
void read_password_without_length(char * id, char *password) {
    int digit_index = 0;
    int current_digit = 0;
    int digits = return_password_length(id);




	if (return_flag || digits == 0) {
		state = ADD_USER;
		print_menu(state);
		return;
	}



    while (digit_index < digits) {
    	manage_access();
        int key = read_key();
        if (key == 1) { //LEFT
            current_digit = (current_digit == 0) ? 9 : current_digit - 1;
            password[digit_index] = '0' + current_digit;
            password[digit_index + 1] = '\0'; // Null-terminate the password string



            MY_PRINTF("%s\n", password);
            //DisplayWriteChar(password);
            DisplayWritePassword(digit_index, current_digit);


        } else if (key == 3) { //RIGHT
            current_digit = (current_digit == 9) ? 0 : current_digit + 1;
            password[digit_index] = '0' + current_digit;
            password[digit_index + 1] = '\0'; // Null-terminate the password string

            MY_PRINTF("%s\n", password);
            //DisplayWriteChar(password);
            DisplayWritePassword(digit_index, current_digit);


        } else if (key == 4) { //ENTER
        	password[digit_index] = '0' + current_digit;


            if (digit_index + 1  == digits) {
            	password[digit_index+1] = '\0'; // Null-terminate the ID string
                digit_index = 0; // Reset for next ID read
                current_digit = 0;
                return true;
            }
            else
            {
            	password[digit_index + 1] = '0'; // Null-terminate the ID string
            	password[digit_index + 2] = '\0'; // Null-terminate the ID string

                digit_index++;
                current_digit = 0;

    			MY_PRINTF("%s\n", password);
    			DisplayWritePassword(digit_index, current_digit);
            }
        }
        else if (key == 2) //Go back
        {
				return_flag = true;
				digit_index = 0;
				current_digit = 0;
				break;
		}
        else if (key == 5)
        {
            if (digit_index > 0) {
                digit_index--;
                current_digit = 0;
                password[digit_index] = '0'; // Null-terminate the ID string
                password[digit_index + 1] = '\0'; // Null-terminate the ID string
                MY_PRINTF("%s\n", password);
                DisplayWritePassword(digit_index, current_digit);
            }
        }

    }
}

void read_password(char *password) {
    int digit_index = 0;
    int current_digit = 0;
    int digits = type_of_pasword();

	if (return_flag || digits == 0) {
		state = ADD_USER;
		print_menu(state);
		return;
	}



    while (digit_index < digits) {
    	manage_access();
        int key = read_key();
        if (key == 1) { //LEFT
            current_digit = (current_digit == 0) ? 9 : current_digit - 1;
            password[digit_index] = '0' + current_digit;
            MY_PRINTF("%s\n", password);
            print_code_on_display(password);
        } else if (key == 3) { //RIGHT
            current_digit = (current_digit == 9) ? 0 : current_digit + 1;
            password[digit_index] = '0' + current_digit;
            password[digit_index + 1] = '\0'; // Null-terminate the password string
            MY_PRINTF("%s\n", password);
            print_code_on_display(password);
        } else if (key == 4) { //ENTER
        	password[digit_index] = '0' + current_digit;


            if (digit_index + 1  == digits) {
            	password[digit_index+1] = '\0'; // Null-terminate the ID string
                digit_index = 0; // Reset for next ID read
                current_digit = 0;
                return true;
            }
            else
            {
            	password[digit_index + 1] = '0'; // Null-terminate the ID string
            	password[digit_index + 2] = '\0'; // Null-terminate the ID string

                digit_index++;
                current_digit = 0;

    			MY_PRINTF("%s\n", password);
    			print_code_on_display(password);
            }
        }
        else if (key == 2) //Go back
        {
				return_flag = true;
				digit_index = 0;
				current_digit = 0;
				break;
		}
        else if (key == 5)
        {
            if (digit_index > 0) {
                digit_index--;
                current_digit = 0;
                password[digit_index] = '0'; // Null-terminate the ID string
                password[digit_index + 1] = '\0'; // Null-terminate the ID string
                MY_PRINTF("%s\n", password);
                print_code_on_display(password);
            }
        }

    }
}

void add_user_call() {
	MY_PRINTF("Adding user\n");
	char id[20] = "";
	char password[20] = "";

	MY_PRINTF("Enter id: ");
	DisplayWriteChar("ID  ");
	read_id(id);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	MY_PRINTF("Enter length of password: ");
	DisplayWriteChar("LENG");
	read_password(password);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	add_user(id, password);

	print_menu(state);

}

void delete_user_call() {
	MY_PRINTF("Deleting user\n");
	char id[20] = "";
	char password[20] = "";

	MY_PRINTF("Enter id: ");
	DisplayWriteChar("ID  ");
	read_id(id);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	MY_PRINTF("Enter password: ");
	DisplayWriteChar("PASS");
	read_password_without_length(id,password);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}


	delete_user(id, password);

	print_menu(state);


}

void change_password_call() {
	MY_PRINTF("Changing password\n");
	char id[20] = "";
	char old_password[20] = "";


	MY_PRINTF("Enter id: ");
	DisplayWriteChar("ID  ");
	read_id(id);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}


	MY_PRINTF("Enter old password: ");
	DisplayWriteChar("OLD ");
	read_password_without_length(id, old_password);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	char new_password[20];
	MY_PRINTF("Enter new password: ");
	DisplayWriteChar("NEW ");
	read_password(new_password);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	change_password(id, old_password, new_password);

	print_menu(state);
}

void access_system_call(void)
{
	MY_PRINTF("Accessing system\n");
	char id[20] = "";
	char password[20] = "";


	MY_PRINTF("Enter id: ");
	DisplayWriteChar("ID  ");
	read_id(id);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	MY_PRINTF("Enter password: ");
	DisplayWriteChar("PASS");
	read_password_without_length(id, password);

	if (return_flag)
	{
			state = ADD_USER;
			return_flag = false;
			print_menu(state);
			return;
	}

	uint8_t access = access_system(id, password);

	if (access)
	{
		MY_PRINTF("Access granted\n");
		DisplayWriteChar("ACCE");
		if(access_flag == false)
		{
			access_flag = true;
			timer_access = timerStart(TIMER_MS2TICKS(5000));

			MY_PRINTF("LED ON\n");
//			LEDS_Set(ALL);
		}
		else
		{
			timer_access = timerStart(TIMER_MS2TICKS(5000));
//			LEDS_Set(ONLY_CENTER);
		}
	}
	else
	{
		MY_PRINTF("Access denied\n");
		DisplayWriteChar("ERR ");
		if (error_flag == false) {
			error_flag = true;
			timer_error = timerStart(TIMER_MS2TICKS(5000));
		} else {
			timer_error = timerStart(TIMER_MS2TICKS(5000));
		}
	}

//	print_menu(state);

}

void manage_access(void)
{
	if (timerExpired(timer_access))
	{
		access_flag = false;
		LEDS_Set(NOTHING);
	}
	else
	{
		LEDS_Set(0b001);
		LEDS_Set(0b010);
		LEDS_Set(0b100);
	}


	if (timerExpired(timer_error))
	{
		error_flag = false;
		LEDS_Set(NOTHING);

	}
	else
	{
		LEDS_Set(0b010);
	}

}

