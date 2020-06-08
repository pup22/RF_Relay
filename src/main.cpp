/*

*/

#include <RCSwitch.h>
#include <OneButton.h>

#define LED 13
#define BUTTON 5
#define RELAY 6
#define RECIVER 0				// 2 пин arduino (прерывение для ресивера)
#define EEPROM_ADDR 600 // начальный адрес EEPROM
#define COUNT_MAX 5			//максимальное количество записей
#define TIME_RELAY 5000 // время работы реле

RCSwitch mySwitch = RCSwitch();
OneButton button(BUTTON, true, true);
uint8_t count_addr = 0;
uint32_t array_addr[COUNT_MAX] = {0};

void copy_array_from_eeprom()
{ //  перенос записей в RAM
	for (int i = 0; i < count_addr; i++)
	{
		array_addr[i] = eeprom_read_dword(EEPROM_ADDR + i * 4 + 1);
	}
}

bool check_uniq_code(uint32_t code)
{ // проверка на уникальность
	for (int i = 0; i < count_addr; i++)
	{
		if (array_addr[i] == code)
			return false;
	}
	return true;
}

void doubleclick()
{ //запись нового пульта

	//for(int i = 0; i < count_addr; i++) {Serial.print(EEPROM_ADDR + i * 4 + 1);Serial.print(" ");Serial.println(eeprom_read_dword(EEPROM_ADDR + i * 4 + 1));}Serial.println();

	if (count_addr >= COUNT_MAX)
	{ // превышено кол-во записей
		digitalWrite(LED, HIGH);
		delay(3000);
		digitalWrite(LED, LOW);
		return;
	}

	delay(500);
	for (int i = 0; i < count_addr; i++)
	{ // мигаем кол-вом записей
		digitalWrite(LED, HIGH);
		delay(250);
		digitalWrite(LED, LOW);
		delay(250);
	}
	delay(1000);

	int32_t t = millis();
	while ((millis() - t) < 5000) //  ждем новый код несколько сек.
	{
		if (mySwitch.available())
		{
			uint32_t code = mySwitch.getReceivedValue();
			if (check_uniq_code(code))
			{																									// если код уникальный записываем
				eeprom_write_byte(EEPROM_ADDR, count_addr + 1); // увеличить кол-во записей
				//Serial.print(F("new code: ")); Serial.print(count_addr+1);Serial.print(" ");
				uint16_t addr = 4 * (++count_addr - 1) + 1;		// вычисляем адрес следующей записи
				eeprom_write_dword(EEPROM_ADDR + addr, code); // запись нового кода
				copy_array_from_eeprom();
				//Serial.print(EEPROM_ADDR + addr);Serial.print(" ");Serial.println(code);Serial.println();
			}
			delay(500);
			mySwitch.resetAvailable();
			break;
		}
		digitalWrite(LED, !digitalRead(LED));
		delay(50);
	}
	digitalWrite(LED, LOW);
}

void setup()
{
	// Serial.begin(9600);
	// Serial.print("Begin - ");
	// Serial.println(eeprom_read_byte(EEPROM_ADDR));

	pinMode(BUTTON, INPUT_PULLUP);
	pinMode(RELAY, OUTPUT);
	pinMode(LED, OUTPUT);

	if (!digitalRead(BUTTON))
	{ // если при старте нажата кнопка больше секунды - стираем все коды
		delay(2000);
		if (!digitalRead(BUTTON))
		{
			eeprom_write_byte(EEPROM_ADDR, 255);
			//Serial.println("Erase");
			count_addr = 0;
			for (int i = 0; i < 50; i++)
			{ // помигаем "типа память очищенна"
				digitalWrite(LED, !digitalRead(LED));
				delay(80);
			}
		}
	}

	count_addr = eeprom_read_byte(EEPROM_ADDR);
	if (count_addr > COUNT_MAX)
	{
		eeprom_write_byte(EEPROM_ADDR, 0);
		count_addr = 0;
	}
	copy_array_from_eeprom();

	//for(int i = 0; i < count_addr; i++) {Serial.print(EEPROM_ADDR + i * 4 + 1);Serial.print(" ");Serial.println(eeprom_read_dword(EEPROM_ADDR + i * 4 + 1));}Serial.println();

	mySwitch.enableReceive(RECIVER); // Receiver on interrupt 0 => that is pin #2

	button.attachDoubleClick(doubleclick);
}

void loop()
{
	button.tick();

	if (mySwitch.available())
	{
		uint32_t code = mySwitch.getReceivedValue();

		if (!check_uniq_code(code))
		{
			// Serial.println("Уррррааааааааааа!!!!!");
			digitalWrite(LED, HIGH);
			delay(TIME_RELAY); // "делай" потому как все равно делать нехрен
			digitalWrite(LED, LOW);
		}
		mySwitch.resetAvailable();
	}
}
