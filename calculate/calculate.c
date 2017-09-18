
#include "calculate.h"

struct termios oldSet,newSet;

volatile int gabort = 0;

void exit_function(int sig)
{
	gabort = 1;
}

int reset_console(void)
{
	int err = EOK;
	tcgetattr(fileno(stdin), &oldSet);
	
	newSet = oldSet;  
	newSet.c_lflag &= ~ECHO;
	newSet.c_lflag &= ~ICANON;  
	
	newSet.c_cc[VMIN] = 1;    
	
	newSet.c_cc[VTIME] = 0;  
	
	if (tcsetattr(fileno(stdin), TCSAFLUSH, &newSet) != 0)
	{
		err = errno;
		printf("Could not set attrbutes of terminal\n");
	}

	return err;
}

int recover_console(void)
{
	int err = EOK;

	if (tcsetattr(fileno(stdin), TCSAFLUSH, &oldSet) != 0)
	{
		err = errno;
		printf("Could not set attrbutes of terminal\n");
	}

	return err;
}

char oldCh = '+';
char newCh = 0;
pDataList	currentListData = NULL;
pDataList list_head = NULL;

void push_list(pDataList *list, Data data)
{
	if (*list == NULL)
	{
		*list = (pDataList)malloc(sizeof(DataList));
		(*list)->next = NULL;
		(*list)->prev = *list;
		(*list)->data = data;
		currentListData = *list;
	}
	else
	{
		pDataList plistdata;
		plistdata = (pDataList)malloc(sizeof(DataList));
		plistdata->next = NULL;
		plistdata->prev = currentListData;
		plistdata->data = data;
		currentListData->next = plistdata;
		currentListData = plistdata;		
	}
}

void clean_list(pDataList *list)
{
	pDataList temp_head = *list;
	pDataList temp;
	if (temp_head != NULL)
	{
		while (temp_head->next != NULL)
		{
			temp = temp_head->next;
			free(temp_head);
			temp_head = temp;
		}
		free(temp_head);
	}
	*list = NULL;
}

void display_list(pDataList list, int bool)
{
	pDataList temp_head = list;
	if (temp_head != NULL && bool)
	{
		while (temp_head->next != NULL)
		{
			if (temp_head->data.type == NUM)
			{
				printf("%lf", temp_head->data.num);
			}
			else
			{
				printf("%c", temp_head->data.data);
			}
			temp_head = temp_head->next;
		}
		printf("%c\n", temp_head->data.data);
	}
	else if (temp_head != NULL)
	{
		pDataList temp_head = list;
		if (temp_head != NULL)
		{
			while (temp_head->next != NULL)
			{
				printf("%c", temp_head->data.data);
				temp_head = temp_head->next;
			}
			printf("%c\n", temp_head->data.data);
		}
	}
}

int switch_char(char ch, pDataList *list)
{
	int err = EOK;
	Data data;
	
	switch (ch)
	{
		case '+':
		case '-':
		case 'x':
		case '/':
			data.data = ch;
			data.type = SYM;
		break;

		case '(':	
			data.data = ch;
			data.type = BAR;
		break;
		case ')':			
			data.data = ch;
			data.type = BAR;
		break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			data.data = ch;
			data.type = NUM;
		break;
		
		case '.':
			data.data = ch;
			data.type = NUM;
		break;
		
		default:
			err = 1;
		break;
	}

	if ((ch == '+' || ch == '-' || ch == 'x' || ch == '/' || ch == '.') && 
		(oldCh == '+' || oldCh == '-' || oldCh == 'x' || oldCh == '/' || oldCh == '.'))
	{
		return 1;
	}

	if ((oldCh == '(') && (ch == '+' || ch == '-' || ch == 'x' || ch == '/' || ch == '.'))
	{
		return 1;
	}

	if ((oldCh == '+' || oldCh == '-' || oldCh == 'x' || oldCh == '/' || oldCh == '.') && (ch == ')'))
	{
		return 1;
	}

	if ((oldCh == '(') && (ch == ')') || (oldCh == ')') && (ch == '('))
	{
		return 1;
	}

	if ((currentListData != NULL) && (currentListData->data.type == NUM) && ch == '(')
	{
		return 1;
	}

	if ((oldCh == ')') && (data.type == NUM))
	{
		return 1;
	}

	if (!err)
	{
		push_list(list, data);
		oldCh = ch;
	}

	return err;
}

int check_list_error(pDataList list)
{
	int err = EOK;
	int count = 0;
	int left = 0, right = 0;
	
	pDataList pTemp = list;

	while (pTemp != NULL)
	{
		if (pTemp->data.data == '(')
		{
			left++;
		}
		else if (pTemp->data.data== ')')
		{
			right++;
		}
		else if (pTemp->data.type == NUM)
		{
			while (pTemp != NULL && pTemp->data.type == NUM)
			{
				if (pTemp->data.data == '.')
				{
					count++;
				}
				pTemp = pTemp->next;
			}

			if (count > 1 || pTemp == NULL)
			{
				break;
			}
			else
			{
				pTemp = pTemp->prev;
				count = 0;
			}
		}
		pTemp = pTemp->next;
	}

	if (right != left)
	{
		err = EOK + 1;
	}
	
	if (count > 1)
	{
		err = EOK + 2;
	}

	return err;
}

void mul_calculate(pDataList *item)
{
	double res = 0;
	pDataList temp_item = *item;

	res = temp_item->prev->data.num * temp_item->next->data.num;
	printf("%lf = %lf %c %lf\n", res, temp_item->prev->data.num, temp_item->data.data, temp_item->next->data.num);
	temp_item->prev->data.num = res;

	*item = temp_item->prev;

	temp_item->next->next->prev = temp_item->prev;
	temp_item->prev->next= temp_item->next->next;
	
	free(temp_item->next);
	free(temp_item);
}

void div_calculate(pDataList *item)
{
	double res = 0;
	pDataList temp_item = *item;

	if (temp_item->next->data.num == 0)
	{
		printf("div 0\n");
		{
			clean_list(&list_head);
			recover_console();
		}
		exit(0);
	}
	
	res = temp_item->prev->data.num / temp_item->next->data.num;
	printf("%lf = %lf %c %lf\n", res, temp_item->prev->data.num, temp_item->data.data, temp_item->next->data.num);
	temp_item->prev->data.num = res;

	*item = temp_item->prev;

	temp_item->next->next->prev = temp_item->prev;
	temp_item->prev->next= temp_item->next->next;
	
	free(temp_item->next);
	free(temp_item);
}

void add_calculate(pDataList *item)
{
	double res = 0;
	pDataList temp_item = *item;

	res = temp_item->prev->data.num + temp_item->next->data.num;
	printf("%lf = %lf %c %lf\n", res, temp_item->prev->data.num, temp_item->data.data, temp_item->next->data.num);
	temp_item->prev->data.num = res;

	*item = temp_item->prev;

	temp_item->next->next->prev = temp_item->prev;
	temp_item->prev->next= temp_item->next->next;
	
	free(temp_item->next);
	free(temp_item);
}

void del_calculate(pDataList *item)
{
	double res = 0;
	pDataList temp_item = *item;

	res = temp_item->prev->data.num - temp_item->next->data.num;
	printf("%lf = %lf %c %lf\n", res, temp_item->prev->data.num, temp_item->data.data, temp_item->next->data.num);
	temp_item->prev->data.num = res;

	*item = temp_item->prev;

	temp_item->next->next->prev = temp_item->prev;
	temp_item->prev->next= temp_item->next->next;
	
	free(temp_item->next);
	free(temp_item);

}


void item_calculate(pDataList *head, pDataList *trail)
{
	pDataList temp_head = *head, temp_trail = *trail;
	
	while (temp_head != temp_trail)
	{
		if (temp_head->data.data == 'x')
		{
			mul_calculate(&temp_head);
		}
		else if (temp_head->data.data == '/')
		{
			div_calculate(&temp_head);
		}
		temp_head = temp_head->next;
	}

	temp_head = *head;
	temp_trail = *trail;

	while (temp_head != temp_trail)
	{
		if (temp_head->data.data == '+')
		{
			add_calculate(&temp_head);
		}
		else if (temp_head->data.data == '-')
		{
			del_calculate(&temp_head);
		}
		temp_head = temp_head->next;
	}

	if (0/*(*head)->prev != *head*/)//6+(6+6)x2
	{
		temp_head = *head;
		temp_trail = *trail;

		(*head)->prev->next = temp_head->next;

		(*head)->next->prev = (*head)->prev;
		(*head)->next->next->next->prev = (*head)->next;
		(*head)->next->next = (*head)->next->next->next;
		*trail = temp_head->next;

		free(temp_head);
		free(temp_trail);
	}
	else //(6+6)x2
	{
		temp_head = (*head)->next;
		temp_trail = *trail;
		
		(*head)->next->next->next->prev = *head;
		(*head)->data.data = (*head)->next->data.data;
		(*head)->data.type = (*head)->next->data.type;
		(*head)->data.num = (*head)->next->data.num;
		(*head)->next = (*head)->next->next->next;
		*trail = *head;
		
		free(temp_head);
		free(temp_trail);
	}
}

void bar_calculate_callback(pDataList *list)
{
	pDataList temp_list = (*list)->next;
	pDataList head = *list, frail;
	while (temp_list != NULL)
	{
		if (temp_list->data.data == '(')
		{
			bar_calculate_callback(&temp_list);
		}
		else if (temp_list->data.data == ')')
		{
			frail = temp_list;
			break;
		}
		temp_list = temp_list->next;
	}
	
	item_calculate(&head, &frail);
	*list = frail;

}

void bar_calculate(pDataList list)
{
	pDataList temp_list = list;
	while (temp_list != NULL)
	{
		if (temp_list->data.data == '(')
		{
			bar_calculate_callback(&temp_list);
			break;
		}
		temp_list = temp_list->next;
	}
}

void change_to_double(pDataList head, pDataList frail)
{
	int p_f = 0;
	int iloop = 0;
	int count_p = 0, count_f = 0;
	double p = 0, f = 0;
	pDataList Temp_head = head, Temp_frail = frail;
	while (Temp_head != NULL && Temp_head != frail)
	{
		if (Temp_head->data.data == '.')
		{
			p_f = 1;
		}
		else
		{
			if (p_f == 0)
			{
				count_p++;
			}
			else
			{
				count_f++;
			}
		}
		Temp_head = Temp_head->next;
	}
	
	if (p_f == 0)
	{
		count_p++;
	}
	else
	{
		count_f++;
	}

	Temp_head = head;
	Temp_frail = frail;

	iloop = 0;
	while (iloop < count_p)
	{
		if (Temp_head != NULL && Temp_head->data.data != '.' && Temp_head->data.type == NUM)
		{
			p = p * 10 + (Temp_head->data.data - '0');
			iloop++;
			Temp_head = Temp_head->next;
		}
		else
		{
			break;
		}
	}

	iloop = 0;
	while (iloop < count_f)
	{
		if (Temp_frail != NULL && Temp_frail->data.data != '.' && Temp_frail->data.type == NUM)
		{
			f = f / 10 + (Temp_frail->data.data - '0');
			iloop++;
			Temp_frail = Temp_frail->prev;
		}
		else
		{
			break;
		}
	}
	Temp_head = head;
	Temp_frail = frail;


	if (Temp_head == Temp_frail)
	{
		head->data.num = p+f/10;
		frail->next->prev = head;
		head->next = frail->next;
	}
	else
	{
		pDataList temp;
		Temp_head = Temp_head->next;
		
		head->data.num = p+f/10;
		frail->next->prev = head;
		head->next = frail->next;
		
		
		while (Temp_head != Temp_frail)
		{
			temp = Temp_head->next;
			free(Temp_head);
			Temp_head = temp;
		}
		free(Temp_head);
	}	
}

void char_to_double(pDataList list)
{
	pDataList temp = list;
	pDataList head, frail;
	while (temp != NULL)
	{
		if (temp->data.type == NUM)
		{
			head = temp;
			temp = temp->next;
			while (temp != NULL)
			{
				if (temp->data.type != NUM)
				{
					frail = temp->prev;
					change_to_double(head, frail);
					break;
				}
				temp = temp->next;
			}
		}
		else
		{
			temp = temp->next;
		}
	}
}

void mul_div_calculate(pDataList list)
{
	pDataList temp_list = list;
	while (temp_list != NULL)
	{
		if (temp_list->data.data == 'x')
		{
			mul_calculate(&temp_list);
		}
		else if (temp_list->data.data == '/')
		{
			div_calculate(&temp_list);
		}
		temp_list = temp_list->next;
	}
}

void add_del_calculate(pDataList list)
{
	pDataList temp_list = list;
	while (temp_list != NULL)
	{
		if (temp_list->data.data == '+')
		{
			add_calculate(&temp_list);
		}
		else if (temp_list->data.data == '-')
		{
			del_calculate(&temp_list);
		}
		temp_list = temp_list->next;
	}
}


double calculate(pDataList list)
{
	double result = 0;

	if (EOK != check_list_error(list))
	{
		printf("check_list_error error!\n");
		return 0;
	}
	else
	{
		printf("check_list_error success!\n");
	}

	char_to_double(list);
	{
		display_list(list, 1);
	}
	
	bar_calculate(list);
	{
		display_list(list, 1);
	}

	mul_div_calculate(list);

	{
		display_list(list, 1);
	}

	add_del_calculate(list);
	
	{
		display_list(list, 1);
	}

	result = list->data.num;
	
	return result;
}

int main(int argc, char *argv[])
{
	char ch = 0;
	
//	signal(SIGSEGV, exit_function);
	signal(SIGINT, exit_function);

	if (EOK != reset_console())
	{
		return 1;
	}

	while (!gabort)
	{
		ch = getchar();
		switch (ch)
		{
			case '\r':
			case '\n':
				if ((currentListData != NULL) && ((currentListData->data.type == NUM) || (currentListData->data.type == BAR)))
				{
					printf("%c", ch);
					gabort = 1;
				}
			break;

			case '=':
				if ((currentListData != NULL) && ((currentListData->data.type == NUM) || (currentListData->data.type == BAR)))
				{
					printf("%c\n", ch);
					gabort = 1;
				}
			break;

			case 127:
			{
				pDataList temp_head = list_head;
				if (temp_head == NULL)
				{
					break;
				}
				printf("\r");
				while (temp_head->next != NULL)
				{
					printf(" ");
					temp_head = temp_head->next;
				}
				printf(" ");
				printf("\r");
				fflush(0);
				temp_head = list_head;
				while (temp_head->next != NULL)
				{
					printf("%c", temp_head->data.data);
					temp_head = temp_head->next;
				}
				fflush(0);
				temp_head->prev->next = NULL;
				currentListData = temp_head->prev;
				if (temp_head->prev == temp_head)
				{
					free(temp_head);
					list_head = NULL;
					currentListData = NULL;
					oldCh = '+';
				}
				else
				{
					free(temp_head);
					oldCh = currentListData->data.data;
				}
			}
			break;

			default:
				if (!switch_char(ch, &list_head))
				{
					printf("%c", ch);
				}
			break;
			
		}
	}
	{
		Data data;
		data.data = '=';
		data.type = SYM;
		push_list(&list_head, data);
	}
	
	{
		display_list(list_head, 0);
	}
	
	printf("result: %lf\n", calculate(list_head));

	{
		clean_list(&list_head);
	}
	
	recover_console();
	return 0;
}

